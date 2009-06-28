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
 * Anthony Minessale II <anthm@freeswitch.org>
 * Other FreeSWITCH module contributors that I borrowed code from
 * Leon de Rooij <leon@scarlet-internet.nl>
 *
 *
 * mod_xml_odbc.c -- use fs odbc to realtime create xml directory 
 * perhaps later also config, dialplan, etc ?
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

static switch_bool_t debug = SWITCH_FALSE;

static switch_status_t xml_odbc_render_template(char *template, switch_event_t *params, switch_xml_t xml_out, int *off);
static switch_status_t xml_odbc_render_tag(switch_xml_t xml_in, switch_event_t *params, switch_xml_t xml_out, int *off);

#define XML_ODBC_SYNTAX "[debug_on|debug_off|render_template]"

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
} globals;


SWITCH_STANDARD_API(xml_odbc_function)
{
	if (session) {
		return SWITCH_STATUS_FALSE;
	}

	if (switch_strlen_zero(cmd)) {
		goto usage;
	}

	if (!strcasecmp(cmd, "debug_on")) {
		debug = SWITCH_TRUE;
	} else if (!strcasecmp(cmd, "debug_off")) {
		debug = SWITCH_FALSE;
	} else if (!strcasecmp(cmd, "render_template")) {
// TODO make it configurable what themplate is rendered instead of static "not_found"
	//	int off = 0;
	//	switch_xml_t xml_out = NULL;
//		switch_hash_t *hash;
	//	switch_event_t *params;
//		if (switch_core_hash_init(&hash, globals.pool) != SWITCH_STATUS_SUCCESS) {
////		    need_vars_map = -1; // does it need to be freed ? :)
//			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Can't init params hash!\n");
//		}
	//	xml_odbc_render_template("not_found", params, xml_out, &off);
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


typedef struct xml_odbc_query_helper {
	switch_xml_t xml_in;
	switch_xml_t xml_out;
	int *off;
	switch_event_t *params;
	int rowcount;
} xml_odbc_query_helper_t;


static int xml_odbc_query_callback(void *pArg, int argc, char **argv, char **columnName)
{
	xml_odbc_query_helper_t *qh = (xml_odbc_query_helper_t *) pArg;
	switch_xml_t xml_in_tmp;
	int i;

	qh->rowcount++;

	for (i = 0; i < argc; i++) {
		switch_event_del_header(qh->params, columnName[i]);
		switch_event_add_header_string(qh->params, SWITCH_STACK_BOTTOM, columnName[i], argv[i]);
	}

	/* render all xml children */
	for (xml_in_tmp = qh->xml_in->child; xml_in_tmp; xml_in_tmp = xml_in_tmp->ordered) {
		xml_odbc_render_tag(xml_in_tmp, qh->params, qh->xml_out, qh->off);
	}

	return 0;
}


static switch_status_t xml_odbc_render_tag(switch_xml_t xml_in, switch_event_t *params, switch_xml_t xml_out, int *off)
{
	switch_xml_t xml_in_tmp = NULL;
	int i;

	xml_odbc_query_helper_t query_helper;

	/* special case xml-odbc-do - this tag is not copied, but action is done */
	if (!strcasecmp(xml_in->name, "xml-odbc-do")) {
		char *name = NULL;
		char *value = NULL, *new_value = NULL;
		char *empty_result_break_to = NULL;
		char *no_template_break_to = NULL;

		name = (char *) switch_xml_attr_soft(xml_in, "name");
		value = (char *) switch_xml_attr_soft(xml_in, "value");
		empty_result_break_to = (char *) switch_xml_attr_soft(xml_in, "on-empty-result-break-to");
		no_template_break_to = (char *) switch_xml_attr_soft(xml_in, "on-no-template-break-to");

		if (switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Ignoring xml-odbc-do because no name attribute is given\n");
			goto done;
		}

		if (switch_strlen_zero(value)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Ignoring xml-odbc-do name=[%s] because no value attr is given\n", name);
			goto done;
		}

		new_value = switch_event_expand_headers(params, value);

		if (!strcasecmp(name, "break-to")) {
/* have a look at this again, not too happy about this next_template_name thing.. */
			switch_event_del_header(params, "next_template_name");
			switch_event_add_header_string(params, SWITCH_STACK_BOTTOM, "next_template_name", value);
			return SWITCH_STATUS_FALSE;
		} else if (!strcasecmp(name, "query")) {
			query_helper.xml_in = xml_in;
			query_helper.xml_out = xml_out;
			query_helper.off = off;
			query_helper.params = params;
			query_helper.rowcount = 0;

			if (switch_odbc_handle_callback_exec(globals.master_odbc, new_value, xml_odbc_query_callback, &query_helper) != SWITCH_ODBC_SUCCESS) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error running this query: [%s]\n", new_value);
			} else {
				if (!switch_strlen_zero(empty_result_break_to) && query_helper.rowcount == 0) {
/* have a look at this again, not too happy about this next_template_name thing.. */
					switch_event_del_header(params, "next_template_name");
					switch_event_add_header_string(params, SWITCH_STACK_BOTTOM, "next_template_name", empty_result_break_to);
					return SWITCH_STATUS_FALSE;
				}
			}

		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Ignoring unknown xml-odbc-do name=[%s]\n", name);
		}

		switch_safe_free(new_value);

	/* just copy current tag xml_in to xml_out and recurse for all children */
	} else {

		/* set name if current root node */
		if (switch_strlen_zero(xml_out->name)) { // I should match here on off instead of on name == "" ?
			xml_out->name = strdup(xml_in->name);

		/* or create a child */
		} else if (!(xml_out = switch_xml_add_child_d(xml_out, xml_in->name, *off++))) {
			return SWITCH_STATUS_FALSE;
		}

		/* copy all attrs */
		for (i = 0; xml_in->attr[i]; i+=2) {
			char *tmp_attr;
			tmp_attr = switch_event_expand_headers(params, xml_in->attr[i+1]);
			switch_xml_set_attr(xml_out, xml_in->attr[i], tmp_attr);
		}

		/* copy all children and render them */
		for (xml_in_tmp = xml_in->child; xml_in_tmp; xml_in_tmp = xml_in_tmp->ordered) {
			if (xml_odbc_render_tag(xml_in_tmp, params, xml_out, off) != SWITCH_STATUS_SUCCESS) {
				return SWITCH_STATUS_FALSE;
			}
		}

	}

  done:
	return SWITCH_STATUS_SUCCESS;
}


static switch_status_t xml_odbc_render_template(char *template_name, switch_event_t *params, switch_xml_t xml_out, int *off)
{
	switch_xml_t template_tag = NULL, sub_tag = NULL;
	char *next_template_name = NULL;

	if ((template_tag = switch_xml_find_child(globals.templates_tag, "template", "name", template_name))) {
		for (sub_tag = template_tag->child; sub_tag; sub_tag = sub_tag->ordered) {
			if (xml_odbc_render_tag(sub_tag, params, xml_out, off) != SWITCH_STATUS_SUCCESS) {
			}

			if ((next_template_name = switch_event_get_header(params, "next_template_name"))) {
				goto rewind;
			}

		}
		goto done;
	}

    next_template_name = "not_found";

  rewind:
/* have a look at this again, not too happy about this next_template_name thing.. */
	switch_event_del_header(params, "next_template_name");
	xml_out->name = "";
	xml_odbc_render_template(next_template_name, params, xml_out, off);

  done:
	return SWITCH_STATUS_SUCCESS;
}


static switch_xml_t xml_odbc_search(const char *section, const char *tag_name, const char *key_name, const char *key_value, switch_event_t *params, void *user_data)
{
	xml_binding_t *binding = (xml_binding_t *) user_data;
	switch_event_header_t *hi;

	switch_xml_t xml_out = NULL;
	if ((xml_out = switch_xml_new(""))) {
		//switch_xml_set_attr_d(xml_out, "type", "freeswitch/xml");
	}

	char *template_name;
	int off = 0, ret = 1;

	if (!binding) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No bindings... sorry bud returning now\n");
		return NULL;
	}

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

	if (params) {
		if ((hi = params->headers)) {
			for (; hi; hi = hi->next) {
				if (debug == SWITCH_TRUE) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG in xml_odbc_search, header [%s]=[%s]\n", hi->name, hi->value);
				}
			}
			xml_odbc_render_template(template_name, params, xml_out, &off);
		}
	} else {
		goto cleanup;
	}

	if (debug == SWITCH_TRUE) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Debug dump of XML generated:\n%s", switch_xml_toxml(xml_out, SWITCH_FALSE));
	}

	ret = 0;

  cleanup:

	if (ret) {
		switch_xml_free(xml_out);
		return NULL;
	}
	
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

	switch_core_new_memory_pool(&globals.pool);
	switch_mutex_init(&globals.mutex, SWITCH_MUTEX_NESTED, globals.pool);

	switch_api_interface_t *xml_odbc_api_interface;

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
