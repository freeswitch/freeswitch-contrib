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
 * Also thanks to:
 * Scarlet Nederland for donating time and money
 *
 *
 * mod_xml_odbc.c -- Simple templating that can use ODBC to generate XML at search-time
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
	switch_xml_t templates_tag;
	switch_mutex_t *mutex;
	switch_memory_pool_t *pool;
	switch_odbc_handle_t *master_odbc;
	switch_bool_t debug;
	switch_bool_t keep_files_around;
	int max_render_template_count;
} globals;

typedef struct xml_odbc_session_helper {
	switch_memory_pool_t *pool;	/* memory pool that is destroyed at the end of the session to ease free'ing */
	char *next_template_name;	/* the name of the next template that has to be rendered */
	switch_event_t *event;		/* contains headers which is a hash that can easily be expanded ${var} to *chars */
	switch_xml_t xml_in_cur;	/* current tag in xml template that is rendered from */
	switch_xml_t xml_out;		/* root tag of xml that is rendered to */
	switch_xml_t xml_out_cur;	/* current tag in xml that is rendered to */
	int xml_out_cur_off;		/* depth counter of xml_out_cur */
	int render_template_count;	/* when this counter > globals.max_render_template_count then stop because there's probably a loop */
	int tmp_i;					/* temporary counter, used for counting rows in the callback */
} xml_odbc_session_helper_t;

static switch_status_t xml_odbc_render_tag(xml_odbc_session_helper_t *helper);
static switch_status_t xml_odbc_render_children(xml_odbc_session_helper_t *helper);
static switch_status_t xml_odbc_render_template(xml_odbc_session_helper_t *helper);

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

static char* switch_event_expand_headers_by_pool(switch_memory_pool_t *pool, switch_event_t *event, const char *in) /* perhaps it's handy if this is moved to switch_event.c ? */
{
    char *tmp, *out;

    tmp = switch_event_expand_headers(event, in);
    out = switch_core_strdup(pool, tmp);
    if (tmp != in) switch_safe_free(tmp);

    return out;
}

static int xml_odbc_query_callback(void *pArg, int argc, char **argv, char **columnName)
{
	xml_odbc_session_helper_t *helper = (xml_odbc_session_helper_t *) pArg;
	int i;

	if (!switch_strlen_zero(helper->next_template_name)) goto done;

	/* up the row counter */
	helper->tmp_i++; /* TODO: WILL THIS GO WRONG FOR NESTED QUERIES ?? THINK ABOUT IT !!! */

	/* loop through all columns and store them in helper->event->headers */
	for (i = 0; i < argc; i++) {
		switch_event_del_header(helper->event, columnName[i]);
		switch_event_add_header_string(helper->event, SWITCH_STACK_BOTTOM, columnName[i], argv[i]);
	}

	xml_odbc_render_children(helper);

  done:
	return 0;
}

static switch_status_t xml_odbc_do_break_to(xml_odbc_session_helper_t *helper)
{
	char *value = (char *) switch_xml_attr_soft(helper->xml_in_cur, "value");
	char *new_value = switch_event_expand_headers_by_pool(helper->pool, helper->event, value);

	helper->next_template_name = new_value;

	return SWITCH_STATUS_SUCCESS;
}

static switch_status_t xml_odbc_do_set_event_header(xml_odbc_session_helper_t *helper)
{
	char *name = (char *) switch_xml_attr(helper->xml_in_cur, "name"); /* the xml-odbc-do name attr */

	char *if_name = (char *) switch_xml_attr(helper->xml_in_cur, "if-name");
	if (if_name) { if_name = switch_event_expand_headers_by_pool(helper->pool, helper->event, if_name); }

	char *if_value = (char *) switch_xml_attr(helper->xml_in_cur, "if-value");
	if (if_value) { if_value = switch_event_expand_headers_by_pool(helper->pool, helper->event, if_value); }

	char *to_name = (char *) switch_xml_attr(helper->xml_in_cur, "to-name");
	if (to_name) { to_name = switch_event_expand_headers_by_pool(helper->pool, helper->event, to_name); }

	char *to_value = (char *) switch_xml_attr(helper->xml_in_cur, "to-value");
	if (to_value) { to_value = switch_event_expand_headers_by_pool(helper->pool, helper->event, to_value); }

	char *old_value = NULL;

	switch_status_t status = SWITCH_STATUS_FALSE;

	if (if_value && !if_name) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "xml-odbc-do [%s] if_name is required when if_value given\n", name);
		goto done;
	}

	if (!if_name && !to_name) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "xml-odbc-do [%s] either if_name or to_name are required\n", name);
		goto done;
	}

	if (if_name) {
		if (!(old_value = switch_event_get_header(helper->event, if_name))) {
			status = SWITCH_STATUS_SUCCESS;
			goto done;
		}
	}

	if (if_value && old_value && strcasecmp(if_value, old_value)) {
		status = SWITCH_STATUS_SUCCESS;
		goto done;
	}

	if (to_name || to_value) {
		switch_event_del_header(helper->event, if_name ? if_name : to_name);

		switch_event_add_header_string(helper->event, SWITCH_STACK_BOTTOM,
					to_name ? to_name : if_name,
					to_value ? to_value : old_value);
	}

	status = xml_odbc_render_children(helper);

  done:
	return status;
}

static switch_status_t xml_odbc_do_query(xml_odbc_session_helper_t *helper)
{
	char *value = (char *) switch_xml_attr_soft(helper->xml_in_cur, "value");
	char *empty_result_break_to = (char *) switch_xml_attr_soft(helper->xml_in_cur, "on-empty-result-break-to");
	char *new_value = switch_event_expand_headers_by_pool(helper->pool, helper->event, value);
	switch_status_t status = SWITCH_STATUS_FALSE;

	if (globals.debug == SWITCH_TRUE) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG Performing Query:\n%s\n", new_value);
	}

	if (switch_odbc_handle_callback_exec(globals.master_odbc, new_value, xml_odbc_query_callback, helper) != SWITCH_ODBC_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error running this query: [%s]\n", new_value);
		goto done;
	} 

	if (!switch_strlen_zero(empty_result_break_to) && helper->tmp_i == 0) {
		helper->next_template_name = empty_result_break_to;
	}
	
	status = SWITCH_STATUS_SUCCESS;

  done:
	return status;
}

/* render tag helper->xml_in_cur */
static switch_status_t xml_odbc_render_tag(xml_odbc_session_helper_t *helper)
{
	switch_status_t status = SWITCH_STATUS_FALSE;
	int i;

	/* special case xml-odbc-do */
	if (!strcasecmp(helper->xml_in_cur->name, "xml-odbc-do")) {
		char *name = (char *) switch_xml_attr_soft(helper->xml_in_cur, "name");

		if (switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Ignoring xml-odbc-do because no name is given\n");
			goto done;
		}

		if (!strcasecmp(name, "break-to")) {
			status = xml_odbc_do_break_to(helper);
		} else if (!strcasecmp(name, "check-event-header")) {  /* check-event-header is the same as set-event-header */
			status = xml_odbc_do_set_event_header(helper); /* except no to-name or to-value are given.. */
		} else if (!strcasecmp(name, "set-event-header")) {
			status = xml_odbc_do_set_event_header(helper);
		} else if (!strcasecmp(name, "query")) {
			status = xml_odbc_do_query(helper);
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Ignoring invalid xml-odbc-do name=[%s]\n", name);
		}

		goto done;
	}

	/* just copy xml_in_cur to xml_out */
	if (switch_strlen_zero(helper->xml_out_cur->name)) {
		helper->xml_out_cur->name = switch_core_strdup(helper->pool, helper->xml_in_cur->name);

	} else if (!(helper->xml_out_cur = switch_xml_add_child_d(helper->xml_out_cur, helper->xml_in_cur->name, helper->xml_out_cur_off++))) {
		goto done;
	}

	/* copy all expanded attrs */
	for (i=0; helper->xml_in_cur->attr[i]; i+=2) {
		char *tmp_value = switch_event_expand_headers_by_pool(helper->pool, helper->event, helper->xml_in_cur->attr[i+1]);
		switch_xml_set_attr(helper->xml_out_cur, helper->xml_in_cur->attr[i], tmp_value);
	}

	status = xml_odbc_render_children(helper);

  done:
	return status;
}

/* render all children of helper->xml_in_cur */
static switch_status_t xml_odbc_render_children(xml_odbc_session_helper_t *helper)
{
	switch_xml_t xml_in_cur_tmp, xml_out_cur_tmp, xml_in_cur_child;
	switch_status_t status = SWITCH_STATUS_FALSE;

	/* temporarily save helper->xml_in_cur and helper->xml_out_cur so they can be restored later */
	xml_in_cur_tmp = helper->xml_in_cur;
	xml_out_cur_tmp = helper->xml_out_cur;

	/* render all children of xml_in_cur */
	for (xml_in_cur_child = helper->xml_in_cur->child; xml_in_cur_child; xml_in_cur_child = xml_in_cur_child->ordered) {
		helper->xml_in_cur = xml_in_cur_child;

		xml_odbc_render_tag(helper);

		/* restore helper->xml_out_cur in case it was changed during render_tag */
		helper->xml_out_cur = xml_out_cur_tmp;

		if (!switch_strlen_zero(helper->next_template_name)) goto done;
	}

	status = SWITCH_STATUS_SUCCESS;

  done:
	/* restore helper->xml_in_cur in case it was changed during render_tag */
	helper->xml_in_cur = xml_in_cur_tmp;

    return status;
}

/* free xml_out and render template given by name helper->next_template_name */
static switch_status_t xml_odbc_render_template(xml_odbc_session_helper_t *helper)
{
	switch_status_t status = SWITCH_STATUS_FALSE;

	if (globals.debug == SWITCH_TRUE) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG GOING TO RENDER TEMPLATE [%s]\n", helper->next_template_name);
	}

	/* up the helper->render_template_count and check whether it's > globals.max_render_template_count */
	helper->render_template_count++;
	if (helper->render_template_count > globals.max_render_template_count) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
			"Stopped rendering template, called xml_odbc_render_template more than [%i] times, probably looping.\n", globals.max_render_template_count);
		goto done;
	}

	/* free helper -> xml_out */
	switch_xml_free(helper->xml_out);

	/* init helper-> xml_out xml_out_cur to the same address */
	if (!(helper->xml_out = helper->xml_out_cur = switch_xml_new(""))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Allocation Error!\n");
		helper->next_template_name = "";
		goto done;
	}

	/* reset helper->xml_out_cur_off */
	helper->xml_out_cur_off = 0;

	/* render children */
	if ((helper->xml_in_cur = switch_xml_find_child(globals.templates_tag, "template", "name", helper->next_template_name))) {
		helper->next_template_name = "";
		status = xml_odbc_render_children(helper);
		if (!switch_strlen_zero(helper->next_template_name)) goto reset;
	} else {
		helper->next_template_name = "not-found";
		goto reset;
	}

	goto done;

  reset:
	status = xml_odbc_render_template(helper);

  done:
	return status;
}

static switch_xml_t xml_odbc_search(const char *section, const char *tag_name, const char *key_name, const char *key_value, switch_event_t *event, void *user_data)
{
	xml_binding_t *binding = (xml_binding_t *) user_data;
	switch_event_header_t *hi;
	switch_uuid_t uuid;
	char uuid_str[SWITCH_UUID_FORMATTED_LENGTH + 1];
	char *xml_char;						/* a char representation of the generated xml that will be written to temp file */
	switch_memory_pool_t *pool;			/* for easy memory cleanup */
	xml_odbc_session_helper_t helper;	/* this helper is sent through all other functions to generate the xml */
	switch_xml_t xml_out = NULL;		/* the xml that will be returned by this function */
	char filename[512] = "";			/* the temporary, uuid-based filename */
	int fd;

	if (!binding) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No bindings... sorry bud returning now\n");
		return NULL;
	}

	/* create a new memory pool for this session and put it in the helper */
	switch_core_new_memory_pool(&pool);
	helper.pool = pool;

	/* set the render_template_count to 0 */
	helper.render_template_count = 0;

	/* set the default template to render */
	helper.next_template_name = "default";

	/* add some headers to event and put it in the helper */
	switch_event_add_header_string(event, SWITCH_STACK_TOP, "section", switch_str_nil(section));
	switch_event_add_header_string(event, SWITCH_STACK_TOP, "tag_name", switch_str_nil(tag_name));
	switch_event_add_header_string(event, SWITCH_STACK_TOP, "key_name", switch_str_nil(key_name));
	switch_event_add_header_string(event, SWITCH_STACK_TOP, "key_value", switch_str_nil(key_value));
	helper.event = event;

	/* debug print all switch_event_t event headers */
	if (globals.debug == SWITCH_TRUE) {
		if ((hi = event->headers)) {
			for (; hi; hi = hi->next) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG in xml_odbc_search, header [%s]=[%s]\n", hi->name, hi->value);
			}
		}
	}

	/* render xml to helper.xml_out */
	if (xml_odbc_render_template(&helper) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Something went horribly wrong while generating an XML template!\n");
		goto cleanup;
	}

	/* dump the generated file to *xml_char */
	xml_char = switch_xml_toxml(helper.xml_out, SWITCH_FALSE);

	/* debug dump the generated xml to console */
	if (globals.debug == SWITCH_TRUE) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Debug dump of generated XML:\n%s", xml_char);
	}

	/* generate a new uuid */
	switch_uuid_get(&uuid);
	switch_uuid_format(uuid_str, &uuid);

	/* generate a temporary filename to store the generated xml in */
	/* use switch_core_sprintf here ?!?! */
	switch_snprintf(filename, sizeof(filename), "%s%s.tmp.xml", SWITCH_GLOBAL_dirs.temp_dir, uuid_str);

	/* open (temporary) file */
	if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR| S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) < 0) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error opening temp file [%s]!\n", filename);
		switch_safe_free(xml_char);
		goto cleanup;
	}

	/* dump xml_char to (temporary) file */
	if (!write(fd, xml_char, (unsigned) strlen(xml_char))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error writing generated XML to temp file [%s]!\n", filename);
		close(fd);
		switch_safe_free(xml_char);
		goto cleanup;
	}

	/* close (temporary) file */
	close(fd);
	fd = -1;

	/* free xml_char */
	switch_safe_free(xml_char);

	/* copy the (temporary) file to switch_xml_t xml_out - this should never go wrong */
	if (!(xml_out = switch_xml_parse_file(filename))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error Parsing Result!\n");
	}

	/* debug by leaving the file behind for review */
	if (globals.keep_files_around == SWITCH_TRUE) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "Generated XML is in [%s]\n", filename);
	} else {
		if (unlink(filename) != 0) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Generated XML file [%s] delete failed\n", filename);
		}
	}

  cleanup:
	switch_xml_free(helper.xml_out);
	switch_core_destroy_memory_pool(&pool);

	return xml_out;
}


static switch_status_t do_config()
{
	char *cf = "xml_odbc.conf";
	switch_xml_t cfg, xml, settings_tag, templates_tag, param;
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

	if (!(templates_tag = switch_xml_child(cfg, "templates"))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing <templates> tag!\n");
		goto done;
	}

	/* copy the templates to globals.templates_tag */
	memcpy(&globals.templates_tag, &templates_tag, sizeof(templates_tag));

	/* set a default globals.max_render_template_count to avoid loops in render_template */
	globals.max_render_template_count = 32;

	/* get all the settings */
	for (param = switch_xml_child(settings_tag, "param"); param; param = param->next) {
		char *var = NULL;
		char *val = NULL;

		var = (char *) switch_xml_attr_soft(param, "name");
		val = (char *) switch_xml_attr_soft(param, "value");

		if (!strcasecmp(var, "binding") && !switch_strlen_zero(val)) {
			if (!(binding = malloc(sizeof(*binding)))) {
				goto done;
			}
			memset(binding, 0, sizeof(*binding));
			binding->bindings = strdup(val); /* use switch_core_strdup globals.pool for this ? */

			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Binding XML Search Function [%s]\n", val);

			switch_xml_bind_search_function(xml_odbc_search, switch_xml_parse_section_string(binding->bindings), binding);

			binding_count++;
			binding = NULL;

		/* change odbc-dsn to something like odbc-handle with name=default and dsn=a:b:c
		 * so a linked list or something is created with multiple handles that can be
		 * selected from xml-odbc-do name=query !!! that would be COOL !!! */
		} else if (!strcasecmp(var, "odbc-dsn") && !switch_strlen_zero(val)) {
			globals.odbc_dsn = switch_core_strdup(globals.pool, val);
			if ((odbc_user = strchr(globals.odbc_dsn, ':'))) {
				*odbc_user++ = '\0';
				if ((odbc_pass = strchr(odbc_user, ':'))) {
					*odbc_pass++ = '\0';
				}
			}
		} else if (!strcasecmp(var, "debug") && !switch_strlen_zero(val)) {
			if (!strcasecmp(val, "true") || !strcasecmp(val, "on")) {
				globals.debug = SWITCH_TRUE;
			} else {
				globals.debug = SWITCH_FALSE;
			}
		} else if (!strcasecmp(var, "keep-files-around") && !switch_strlen_zero(val)) {
			if (!strcasecmp(val, "true") || !strcasecmp(val, "on")) {
				globals.keep_files_around = SWITCH_TRUE;
			} else {
				globals.keep_files_around = SWITCH_FALSE;
			}
		}
	}

	/* check if any bindings were done */
	if (binding_count == 0) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "No bindings are set, is this really what you want?\n");
	}

	/* check if odbc_dsn is set */
	if (!globals.odbc_dsn) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No odbc-dsn setting is set!\n");
		goto done;
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
	switch_xml_free(xml);
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
