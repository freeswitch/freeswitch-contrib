/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2009, Anthony Minessale II <anthm@freeswitch.org>
 *
 * Version: MPL 1.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Anthony Minessale II <anthm@freeswitch.org>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * All the FreeSWITCH contributors that I borrowed code from
 * Leon de Rooij <leon@scarlet-internet.nl>
 *
 *
 * mod_xml_odbc.c -- Use FreeSWITCH ODBC to realtime create XML
 *
 *
 */
#include <switch.h>

typedef enum {
	XML_ODBC_CONFIGURATION = 0,
	XML_ODBC_DIRECTORY = 0,
	XML_ODBC_DIALPLAN = 0,
	XML_ODBC_PHRASES = 0
} xml_odbc_query_type_t;

SWITCH_MODULE_LOAD_FUNCTION(mod_xml_odbc_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_xml_odbc_shutdown);
SWITCH_MODULE_DEFINITION(mod_xml_odbc, mod_xml_odbc_load, mod_xml_odbc_shutdown, NULL);

static struct {
	char *odbc_dsn;
	char *configuration_template_name;
	char *directory_template_name;
	char *dialplan_template_name;
	char *phrases_template_name;
	switch_xml_t templates_tag;
	switch_mutex_t *mutex;
	switch_memory_pool_t *pool;
	switch_odbc_handle_t *master_odbc;
	switch_bool_t debug;
} globals;

static struct xml_odbc_session_helper {
	switch_memory_pool_t *pool;
	char *template_name;
	switch_event_t *event;
	switch_xml_t xml_in;
	switch_xml_t xml_out;
	int *off;
	int tmp_i;
} xml_odbc_session_helper_t;

static switch_status_t xml_odbc_render_template(xml_odbc_session_helper_t helper);
static switch_status_t xml_odbc_render_tag(xml_odbc_session_helper_t helper);

#define XML_ODBC_SYNTAX "[debug_on|debug_off]"

/* cli commands */
SWITCH_STANDARD_API(xml_odbc_function)
{
	if (session) {
		return SWITCH_STATUS_FALSE;
	}

	if (switch_strlen_zero(cmd)) {
		goto usage;
	}

	if (!strcasecmp(cmd, "debug_on")) {
		globals.debug = SWITCH_TRUE;
	} else if (!strcasecmp(cmd, "debug_off")) {
		globals.debug = SWITCH_FALSE;
	} else {
		goto usage;
	}

	stream->write_function(stream, "OK\n");
	return SWITCH_STATUS_SUCCESS;

  usage:
	stream->write_function(stream, "USAGE: %s\n", XML_ODBC_SYNTAX);
	return SWITCH_STATUS_SUCCESS;
}

typedef struct xml_binding {
	char *bindings;
} xml_binding_t;

static int xml_odbc_query_callback(void *pArg, int argc, char **argv, char **columnName)
{
	session_helper_t *qh = (session_helper_t *) pArg;
	switch_xml_t xml_in_tmp;
	int i;

	/* up the row counter */
	qh->tmp_i++; // TODO THIS WILL GO WRONG FOR NESTED QUERIES.. THINK ABOUT IT !!!

	/* loop through all columns and store them in qh->event->params */
	for (i = 0; i < argc; i++) {
		switch_event_del_header(qh->event, columnName[i]);
		switch_event_add_header_string(qh->event, SWITCH_STACK_BOTTOM, columnName[i], argv[i]);
	}

	/* render all children of xml_in */
	for (xml_in_tmp = qh->xml_in->child; xml_in_tmp; xml_in_tmp = xml_in_tmp->ordered) {
		xml_odbc_render_tag(xml_in_tmp, qh->event, qh->xml_out, qh->off);
	}

	return 0;
}

static switch_status_t xml_odbc_do_break_to(xml_odbc_session_helper_t helper)
{
	char *name = (char *) switch_xml_attr_soft(helper->xml_in, "name");
	char *value = (char *) switch_xml_attr_soft(helper->xml_in, "value");
	char *new_value = switch_event_expand_headers(helper->event, value);

	helper->template_name = strdup(new_value);
}

static switch_status_t xml_odbc_do_replace_header_value(xml_odbc_session_helper_t helper)
{
	char *old_header_value = NULL;
	char *name = (char *) switch_xml_attr_soft(helper->xml_in, "name");
	char *when_name = (char *) switch_xml_attr_soft(helper->xml_in, "when_name");
	char *when_value = (char *) switch_xml_attr_soft(helper->xml_in, "when_value");
	char *value = (char *) switch_xml_attr_soft(helper->xml_in, "value");
	char *new_value = switch_event_expand_headers(helper->event, value);
	switch_status_t status = SWITCH_STATUS_FALSE;

	if (switch_strlen_zero(when_name)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Ignoring xml-odbc-do name=[%s] because no when_name is given\n", name);
		goto done;
	}

	if (switch_strlen_zero(value)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Ignoring xml-odbc-do name=[%s] because no value is given\n", name);
		goto done;
	}

	if ((old_header_value = switch_event_get_header(helper->event, when_name))) {
		if (switch_strlen_zero(when_value) || !strcasecmp(when_value, old_header_value)) {
			switch_event_del_header(helper->event, when_name);
			switch_event_add_header_string(helper->event, SWITCH_STACK_BOTTOM, when_name, new_value);
		}
	}

	status = SWITCH_STATUS_SUCCESS;

  done:
	return status;
}

static switch_status_t xml_odbc_do_query(xml_odbc_session_helper_t helper)
{
	char *name = (char *) switch_xml_attr_soft(helper->xml_in, "name");
	char *value = (char *) switch_xml_attr_soft(helper->xml_in, "value");
	char *empty_result_break_to = (char *) switch_xml_attr_soft(xml_in, "on-empty-result-break-to");
	char *new_value = switch_event_expand_headers(helper->event, value);
	switch_status_t status = SWITCH_STATUS_FALSE;

	if (globals.debug == SWITCH_TRUE) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG Performing Query:\n%s\n", new_value);
	}

	if (switch_odbc_handle_callback_exec(globals.master_odbc, new_value, xml_odbc_query_callback, helper) != SWITCH_ODBC_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error running this query: [%s]\n", new_value);
		goto done;
	} 

	if (!switch_strlen_zero(empty_result_break_to) && helper.tmp_i == 0) {
		helper->template_name = empty_result_break_to;
	}
	
	status = SWITCH_STATUS_SUCCESS;

  done:
	return status;
}

static switch_status_t xml_odbc_render_tag(xml_odbc_session_helper_t helper)
{
	switch_status_t status = SWITCH_STATUS_FALSE;
	switch_xml_t xml_in_tmp, xml_out_tmp;
	int i;

	if (!strcasecmp(helper->xml_in->name, "xml-odbc-do")) {
		char *name = (char *) switch_xml_attr_soft(helper->xml_in, "name");

		if (switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Ignoring xml-odbc-do because no name is given\n");
			goto done;
		}

		/* special cases for xml-odbc-do */
		if (!strcasecmp(name, "break-to")) {
			status = xml_odbc_do_break_to(helper);
			goto done;
		} else if (!strcasecmp(name, "replace-header-value")) {
			status = xml_odbc_do_replace_header_value(helper);
			goto done;
		} else if (!strcasecmp(name, "query")) {
			status = xml_odbc_do_query(helper);
			goto done;
		}

		/* just copy xml_in to xml_out */
		if (switch_strlen_zero(helper->xml_out->name)) {
			/* remove all children from helper->xml_out TODO DO THAT HERE !!!!!!!!!!!!!!!!!!!!!! */

			helper->xml_out->name = strdup(helper->xml_in->name);

		} else if (!(xml_out_tmp = switch_xml_add_child_d(helper->xml_out, helper->xml_in->name, (helper->off)++))) {
			goto done;
		}

		/* copy all attrs */
		for (i=0; helper->xml_in->attr[i]; i+=2) {
			char *tmp_attr;
			tmp_attr = switch_event_exapand_headers(helper->event, helper->xml_in->attr[i+1]);
			switch_xml_set_attr(helper->xml_out, helper->xml_in->attr[i], tmp_attr);

			// if (tmp_attr != helper->xml_in->attr[i+1]) {
			// 	switch_safe_free(tmp_attr);
			// }
		}

		/* render all children */
		for (xml_in_tmp = helper->xml_in->child; xml_in_tmp; xml_in_tmp = xml_in_tmp->ordered) {
			if (xml_odbc_render_tag(xml_in_tmp, helper->event, event->xml_out, helper->off) != SWITCH_STATUS_SUCCESS) {
				goto done;
			}
		}

	}

	status = SWITCH_STATUS_SUCCESS;

  done:
	return status;
}

static switch_status_t xml_odbc_render_template(xml_odbc_session_helper_t helper)
{
	switch_xml_t template_tag = NULL, sub_tag = NULL;
	switch_status_t status = SWITCH_STATUS_FALSE;

	if ((template_tag = switch_xml_find_child(globals.templates_tag, "template", "name", helper->template_name))) {
		for (sub_tag = template_tag->child; sub_tag; sub_tag = sub_tag->ordered) {
			if (xml_odbc_render_tag(sub_tag, helper->event, helper->xml_out, helper->off) != SWITCH_STATUS_SUCCESS) {
			}

			if ((!switch_strlen_zero(helper->template_name))) {
				goto rewind;
			}
		}
		goto done;
	}

	helper->template_name = "not_found";

  rewind:
	/* remove all children of helper->xml_out here */
	xml_odbc_render_template(helper);

  done:
	return status;
}

static switch_xml_t xml_odbc_search(const char *section, const char *tag_name, const char *key_name, const char *key_value, switch_event_t *params, void *user_data)
{
	xml_binding_t *binding = (xml_binding_t *) user_data;
	switch_event_header_t *hi;
	char *template_name;
	switch_uuid_t uuid;
	char uuid_str[SWITCH_UUID_FORMATTED_LENGTH + 1];
	int off = 0, ret = 1;
	switch_xml_t xml_out = NULL;
	session_helper_t helper;

	if (!binding) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No bindings... sorry bud returning now\n");
		return NULL;
	}

	/* create a new xml_out used to render the template into */
	if (!(xml_out = switch_xml_new(""))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Allocation Error\n");
		return NULL;		
	}

	/* find out what template to render, based on the section */
	if (!strcmp(section, "configuration")) {
		template_name = strdup(globals.configuration_template_name);
	} else if (!strcmp(section, "directory")) {
		template_name = strdup(globals.directory_template_name);
	} else if (!strcmp(section, "dialplan")) {
		template_name = strdup(globals.dialplan_template_name);
	} else if (!strcmp(section, "phrases")) {
		template_name = strdup(globals.phrases_template_name);
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid section\n");
		return NULL;
	}

	/* add some headers to params */
	switch_event_add_header_string(params, SWITCH_STACK_TOP, "hostname", hostname);
	switch_event_add_header_string(params, SWITCH_STACK_TOP, "section", switch_str_nil(section));
	switch_event_add_header_string(params, SWITCH_STACK_TOP, "tag_name", switch_str_nil(tag_name));
	switch_event_add_header_string(params, SWITCH_STACK_TOP, "key_name", switch_str_nil(key_name));
	switch_event_add_header_string(params, SWITCH_STACK_TOP, "key_value", switch_str_nil(key_value));

	/* generate a new uuid */
	switch_uuid_get(&uuid);
	switch_uuid_format(uuid_str, &uuid);

	/* generate a temporary filename to store the generated xml in */
	switch_snprintf(filename, sizeof(filename), "%s%s.tmp.xml", SWITCH_GLOBAL_dirs.temp_dir, uuid_str);

	/* debug print all switch_event_t params headers */
	if (globals.debug == SWITCH_TRUE) {
		if ((hi = params->headers)) {
			for (; hi; hi = hi->next) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG in xml_odbc_search, header [%s]=[%s]\n", hi->name, hi->value);
			}
		}
	}

	/* put all necessary data in session_helper_t helper */
	// switch_memory_pool_t
	// template_name
	// params
	// xml_out
	// &off

	/* render xml from template */
	if (xml_odbc_render_template(helper) == SWITCH_STATUS_SUCCESS) {

	/* dump the generated file to *xml_char */
	xml_char = switch_xml_toxml(xml_out, SWITCH_TRUE);

	/* debug dump the generated xml to console */
	if (globals.debug == SWITCH_TRUE) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Debug dump of generated XML:\n%s", xml_char);
	}

	/* dump *xml_char to disk */

	/* close the file */

	/* free *xml_char */

	/* free switch_memory_pool_t */

	/* free helper ?? */

	/* copy the generated xml to xml_out - this should never happen */
	if (!(xml_out = switch_xml_parse_file(filename))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error Parsing Result!\n");
	}

	/* debug by leaving the file behind for review */
	if (keep_files_around) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "Generated XML is in %s\n", filename);
	} else {
		if (unlink(filename) != 0) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Generated XML file [%s] delete failed\n", filename);
		}
	}

	/* other things to free ?! */

	return xml_out;
}


static switch_status_t do_config()
{
	char *cf = "xml_odbc.conf";
	switch_xml_t cfg, xml, settings_tag, bindings_tag, binding_tag, templates_tag, param;
	xml_binding_t *binding = NULL;

	switch_status_t status = SWITCH_STATUS_FALSE;

	char *odbc_user = NULL;
	char *odbc_pass = NULL;
	int binding_count = 0;

	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Open of %s failed\n", cf);
		return SWITCH_STATUS_TERM;
	}

	if (!(settings_tag = switch_xml_child(cfg, "settings"))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing <settings> tag!\n");
		goto done;
	}

	if (!(bindings_tag = switch_xml_child(cfg, "bindings"))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing <bindings> tag!\n");
		goto done;
	}

	if (!(templates_tag = switch_xml_child(cfg, "templates"))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing <templates> tag!\n");
		goto done;
	}

// is this allowed ? or should I copy the entire tree ?
	globals.templates_tag = templates_tag;

	/* get all the settings */
	for (param = switch_xml_child(settings_tag, "param"); param; param = param->next) {
		char *var = NULL;
		char *val = NULL;

		var = (char *) switch_xml_attr_soft(param, "name");
		val = (char *) switch_xml_attr_soft(param, "value");

		/* set globals.odbc_dsn */
		if (!strcasecmp(var, "odbc-dsn") && !switch_strlen_zero(val)) {
			globals.odbc_dsn = switch_core_strdup(globals.pool, val);
			if ((odbc_user = strchr(globals.odbc_dsn, ':'))) {
				*odbc_user++ = '\0';
				if ((odbc_pass = strchr(odbc_user, ':'))) {
					*odbc_pass++ = '\0';
				}
			}
		} else if (!strcasecmp(var, "debug") && !switch_strlen_zero(val)) {
			if (!strcasecmp(val, "true") || !strcasecmp(val, "on")) {
				debug = SWITCH_TRUE;
			}
		}
	}

	/* check if odbc_dsn is set */
	if (!globals.odbc_dsn) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No odbc-dsn setting is set!\n");
		goto done;
	}

	/* get all bindings */
	for (binding_tag = switch_xml_child(bindings_tag, "binding"); binding_tag; binding_tag = binding_tag->next) {
		char *bname = (char*) switch_xml_attr_soft(binding_tag, "name");

		for (param = switch_xml_child(binding_tag, "param"); param; param = param->next) {
			char *var = NULL;
			char *val = NULL;

			var = (char *) switch_xml_attr_soft(param, "name");
			val = (char *) switch_xml_attr_soft(param, "value");

			if (!strcasecmp(var, "template") && !switch_strlen_zero(val)) {
				if (!strcmp(bname, "configuration")) {
					globals.configuration_template_name = strdup(val);
				} else if (!strcmp(bname, "directory")) {
					globals.directory_template_name = strdup(val);
				} else if (!strcmp(bname, "dialplan")) {
					globals.dialplan_template_name = strdup(val);
				} else if (!strcmp(bname, "phrases")) {
					globals.phrases_template_name = strdup(val);
				} else {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid binding name [%s]\n", bname);
					goto done;
				}
			}
		}

		binding_count++;

		if (!(binding = malloc(sizeof(*binding)))) {
			goto done;
		}
		memset(binding, 0, sizeof(*binding));

		binding->bindings = strdup(bname);

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Binding [%s] XML ODBC Fetch Function [%s]\n",
			switch_strlen_zero(bname) ? "N/A" : bname, binding->bindings);

		switch_xml_bind_search_function(xml_odbc_search, switch_xml_parse_section_string(bname), binding);

		binding = NULL;
	}

	/* check if a binding is set */
	if (binding_count == 0) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "No binding is set, is this really what you want?\n");
	}

	/* make odbc connection */
	if (switch_odbc_available() && globals.odbc_dsn) {

		if (!(globals.master_odbc = switch_odbc_handle_new(globals.odbc_dsn, odbc_user, odbc_pass))) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Cannot Open ODBC Database!\n");
			status = SWITCH_STATUS_FALSE;
			goto done;
		}

		if (switch_odbc_handle_connect(globals.master_odbc) != SWITCH_ODBC_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Cannot Open ODBC Database!\n");
			status = SWITCH_STATUS_FALSE;
			goto done;
		}

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Connected ODBC DSN: %s\n", globals.odbc_dsn);
	}

	/* all went fine */
	status = SWITCH_STATUS_SUCCESS;

  done:
	return status;
}


SWITCH_MODULE_LOAD_FUNCTION(mod_xml_odbc_load)
{
	switch_api_interface_t *xml_odbc_api_interface;

	switch_core_new_memory_pool(&globals.pool);
	switch_mutex_init(&globals.mutex, SWITCH_MUTEX_NESTED, globals.pool);

	if (!switch_odbc_available()) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "You must have ODBC support in FreeSWITCH to use this module\n");
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "\t./configure --enable-core-odbc-support\n");
		return SWITCH_STATUS_FALSE;
	}

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "XML ODBC module loading...\n");

	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	if (do_config() != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Unable to load xml_odbc config file\n");
		return SWITCH_STATUS_FALSE;
	}

	SWITCH_ADD_API(xml_odbc_api_interface, "xml_odbc", "XML ODBC", xml_odbc_function, XML_ODBC_SYNTAX);
	switch_console_set_complete("add xml_odbc debug_on");
	switch_console_set_complete("add xml_odbc debug_off");
	switch_console_set_complete("add xml_odbc render_template");

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}


SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_xml_odbc_shutdown)
{
	switch_odbc_handle_disconnect(globals.master_odbc);
	switch_odbc_handle_destroy(&globals.master_odbc);

	switch_xml_unbind_search_function_ptr(xml_odbc_search);

	return SWITCH_STATUS_SUCCESS;
}


/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
* vim:set softtabstop=4 shiftwidth=4 tabstop=4:
 */
