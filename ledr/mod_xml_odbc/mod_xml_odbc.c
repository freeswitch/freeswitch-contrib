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
#ifdef SWITCH_HAVE_ODBC
#include <switch_odbc.h>
#endif

typedef enum {
	XML_ODBC_CONFIG = 0,
	XML_ODBC_DIRECTORY = 0,
	XML_ODBC_DIALPLAN = 0,
	XML_ODBC_PHRASE = 0
} xml_odbc_query_type_t;

SWITCH_MODULE_LOAD_FUNCTION(mod_xml_odbc_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_xml_odbc_shutdown);
SWITCH_MODULE_DEFINITION(mod_xml_odbc, mod_xml_odbc_load, mod_xml_odbc_shutdown, NULL);

static switch_bool_t debug = SWITCH_FALSE;

static switch_status_t xml_odbc_render_template(char *template, switch_hash_t *hash, switch_xml_t xml_out, int *off);
static switch_status_t xml_odbc_render_tag(switch_xml_t xml_in, switch_hash_t *hash, switch_xml_t xml_out, int *off);

#define XML_ODBC_SYNTAX "[debug_on|debug_off|render_template]"

static int logi = 0;
static void logger(char *str)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG[%i] [%s]\n", ++logi, str);
}

static struct {
	char *dbname;
	char *odbc_dsn;
	char *template;
	switch_xml_t templates_tag;
	switch_mutex_t *mutex;
	switch_memory_pool_t *pool;
#ifdef SWITCH_HAVE_ODBC
	switch_odbc_handle_t *master_odbc;
#else
	void *filler1;
#endif
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
		int off = 0;
		switch_xml_t xml_out = NULL;
		switch_hash_t *hash;
		if (switch_core_hash_init(&hash, globals.pool) != SWITCH_STATUS_SUCCESS) {
//		    need_vars_map = -1; // does it need to be freed ? :)
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Can't init params hash!\n");
		}
		xml_odbc_render_template("not_found", hash, xml_out, &off);
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
	switch_hash_t *hash;
} xml_odbc_query_helper_t;


static int xml_odbc_query_callback(void *pArg, int argc, char **argv, char **columnName)
{
	xml_odbc_query_helper_t *qh = (xml_odbc_query_helper_t *) pArg;
	switch_xml_t xml_in_tmp;
	int i;

	/* set all columnName/argv key/value pairs in qh->hash */
	for (i = 0; i < argc; i++) {
		switch_core_hash_insert(qh->hash, columnName[i], argv[i]); // does a hash insert overwrite old entries ?
	}

	/* render all xml children */
	for (xml_in_tmp = qh->xml_in->child; xml_in_tmp; xml_in_tmp = xml_in_tmp->ordered) {
		xml_odbc_render_tag(xml_in_tmp, qh->hash, qh->xml_out, qh->off);
	}

	return 0;
}


static switch_status_t xml_odbc_render_tag(switch_xml_t xml_in, switch_hash_t *hash, switch_xml_t xml_out, int *off)
{
	switch_xml_t xml_in_tmp = NULL;
	int i;

	xml_odbc_query_helper_t query_helper;

    if (!strcasecmp(xml_in->name, "xml-odbc-do")) {
		char *name = NULL;
		char *value = NULL;
		char *zero_rows_break_to = NULL;
		char *no_template_break_to = NULL;

		name = (char *) switch_xml_attr_soft(xml_in, "name");
		value = (char *) switch_xml_attr_soft(xml_in, "value");
		zero_rows_break_to = (char *) switch_xml_attr_soft(xml_in, "on-zero-rows-break-to");
		no_template_break_to = (char *) switch_xml_attr_soft(xml_in, "on-no-template-break-to");

		if (!strcasecmp(name, "break-to") && !switch_strlen_zero(value)) { // WHAT TO DO WITH FURTHER RENDERING LOWER ON THE STACK ?!?!?!
			xml_out = NULL;
			off = 0; // <- ?
			if (xml_odbc_render_template(value, hash, xml_out, off) == SWITCH_STATUS_FALSE) {
				if (!switch_strlen_zero(no_template_break_to)) {
					xml_odbc_render_template(no_template_break_to, hash, xml_out, off);
				}
			}
		} else if (!strcasecmp(name, "query") && !switch_strlen_zero(value)) {
			// do an auto expasion on value to replace all ${foo} parts based on switch_hash_t hash
			query_helper.xml_in = xml_in;
			query_helper.xml_out = xml_out;
			query_helper.off = off;
			query_helper.hash = hash;
			if (switch_odbc_handle_callback_exec(globals.master_odbc, value, xml_odbc_query_callback, &query_helper) == SWITCH_ODBC_SUCCESS) {
				// nothing
			} else if (!switch_strlen_zero(zero_rows_break_to)) { // if zero rows returned then switch_odbc_handle_callback_exec != SWITCH_ODBC_SUCCESS ??? 
				xml_out = NULL;
				off = 0; // <- ?
				xml_odbc_render_template(zero_rows_break_to, hash, xml_out, off);
			}
		}

	} else {

		/* set name if current root node */
		if (switch_strlen_zero(xml_out->name)) { // I should match here on off instead of on name == "" ?
			xml_out->name = strdup(xml_in->name);

		/* or create a child */
		} else if (!(xml_out = switch_xml_add_child_d(xml_out, xml_in->name, *off++))) {
			return SWITCH_STATUS_FALSE;
		}

//SWITCH_DECLARE(void *) switch_core_hash_find(_In_ switch_hash_t *hash, _In_z_ const char *key);
logger((char *)switch_core_hash_find(hash, "domain"));

		/* copy all attrs */
		for (i = 0; xml_in->attr[i]; i+=2) {
			// do an auto expasion on attr[i+1] to replace all ${foo} parts based on switch_hash_t hash
			switch_xml_set_attr(xml_out, xml_in->attr[i], xml_in->attr[i+1]);
		}

		/* copy all children and render them */
		for (xml_in_tmp = xml_in->child; xml_in_tmp; xml_in_tmp = xml_in_tmp->ordered) {
			xml_odbc_render_tag(xml_in_tmp, hash, xml_out, off);
		}

	}

	return SWITCH_STATUS_SUCCESS;
}


static switch_status_t xml_odbc_render_template(char *template_name, switch_hash_t *hash, switch_xml_t xml_out, int *off)
{
	switch_xml_t template_tag = NULL, sub_tag = NULL;

	for (template_tag = switch_xml_child(globals.templates_tag, "template"); template_tag; template_tag = template_tag->next) {
		char *template_tag_name = (char*) switch_xml_attr_soft(template_tag, "name");
		if (!strcmp(template_tag_name, template_name)) {
			for (sub_tag = template_tag->child; sub_tag; sub_tag = sub_tag->ordered) {
				xml_odbc_render_tag(sub_tag, hash, xml_out, off);
				return SWITCH_STATUS_SUCCESS;
			}
		}
	}
	return SWITCH_STATUS_FALSE;
}


static switch_xml_t xml_odbc_search(const char *section, const char *tag_name, const char *key_name, const char *key_value, switch_event_t *params, void *user_data)
{
	xml_binding_t *binding = (xml_binding_t *) user_data;
	switch_event_header_t *hi;

	switch_xml_t xml_out = NULL;
	if ((xml_out = switch_xml_new(""))) {
		//switch_xml_set_attr_d(xml_out, "type", "freeswitch/xml");
	}

	xml_odbc_query_type_t query_type;

	switch_hash_t *hash;

	int off = 0, ret = 1;

	if (!binding) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No bindings... sorry bud returning now\n");
		return NULL;
	}
	if (!strcmp(section, "configuration")) {
		query_type = XML_ODBC_CONFIG;
	} else if (!strcmp(section, "directory")) {
		query_type = XML_ODBC_DIRECTORY;
	} else if (!strcmp(section, "dialplan")) {
		query_type = XML_ODBC_DIALPLAN;
	} else if (!strcmp(section, "phrases")) {
		query_type = XML_ODBC_PHRASE;
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid section\n");
		return NULL;
	}

	if (params) {
		if ((hi = params->headers)) {

			/* initialize hash */
			if (switch_core_hash_init(&hash, globals.pool) != SWITCH_STATUS_SUCCESS) {
//			    need_vars_map = -1; // does it need to be freed ? :)
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Can't init params hash!\n");
			}

			for (; hi; hi = hi->next) {
				if (debug == SWITCH_TRUE) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG in xml_odbc_search, header [%s]=[%s]\n", hi->name, hi->value);
				}
				switch_core_hash_insert(hash, hi->name, hi->value); // prefix with 'h_' to avoid collision with keys returned by SELECT's ?
			}

			xml_odbc_render_template(globals.template, hash, xml_out, &off); // TODO globals.template should be replace with something specific for this section

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
	switch_xml_t cfg, xml, bindings_tag, binding_tag, templates_tag, param;
	xml_binding_t *binding = NULL;

	switch_status_t status = SWITCH_STATUS_SUCCESS;

	char *odbc_user = NULL;
	char *odbc_pass = NULL;
	char *sql = NULL;

logger("X");

	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Open of %s failed\n", cf);
		return SWITCH_STATUS_TERM;
	}

	if (!(bindings_tag = switch_xml_child(cfg, "bindings"))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing <bindings> tag!\n");
		goto done;
	}

	if (!(templates_tag = switch_xml_child(cfg, "templates"))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing <templates> tag!\n");
		goto done;
	}

	globals.templates_tag = templates_tag;

	for (binding_tag = switch_xml_child(bindings_tag, "binding"); binding_tag; binding_tag = binding_tag->next) {
		char *bname = (char*) switch_xml_attr_soft(binding_tag, "name");

		for (param = switch_xml_child(binding_tag, "param"); param; param = param->next) {
			char *var = NULL;
			char *val = NULL;

			var = (char *) switch_xml_attr_soft(param, "name");
			val = (char *) switch_xml_attr_soft(param, "value");

			if (!strcasecmp(var, "odbc-dsn") && !switch_strlen_zero(val)) {
				globals.odbc_dsn = switch_core_strdup(globals.pool, val);
				if ((odbc_user = strchr(globals.odbc_dsn, ':'))) {
					*odbc_user++ = '\0';
					if ((odbc_pass = strchr(odbc_user, ':'))) {
						*odbc_pass++ = '\0';
					}
				}
			} else if (!strcasecmp(var, "template") && !switch_strlen_zero(val)) {
				globals.template = strdup(val);
			}
		}

		if (switch_strlen_zero(globals.odbc_dsn) || switch_strlen_zero(odbc_user) || switch_strlen_zero(odbc_pass)) {
			globals.dbname = "xml_odbc";
		}

		if (!(binding = malloc(sizeof(*binding)))) {
			goto done;
		}
		memset(binding, 0, sizeof(*binding));

		binding->bindings = strdup(bname);

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Binding [%s] XML ODBC Fetch Function [%s]\n",
			switch_strlen_zero(bname) ? "N/A" : bname, binding->bindings ? binding->bindings : "all");
		switch_xml_bind_search_function(xml_odbc_search, switch_xml_parse_section_string(bname), binding);
		binding = NULL;
	}

#ifdef SWITCH_HAVE_ODBC
	if (globals.odbc_dsn) {

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
#endif

  done:

	switch_safe_free(sql);

	return status;
}


SWITCH_MODULE_LOAD_FUNCTION(mod_xml_odbc_load)
{

	switch_core_new_memory_pool(&globals.pool);
	switch_mutex_init(&globals.mutex, SWITCH_MUTEX_NESTED, globals.pool);

	switch_api_interface_t *xml_odbc_api_interface;

#ifndef SWITCH_HAVE_ODBC
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "You must have ODBC support in FreeSWITCH to use this module\n");
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "\t./configure --enable-core-odbc-support\n");
	return SWITCH_STATUS_FALSE;
#endif

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "XML ODBC module loading...\n");

	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_API(xml_odbc_api_interface, "xml_odbc", "XML ODBC", xml_odbc_function, XML_ODBC_SYNTAX);
	switch_console_set_complete("add xml_odbc debug_on");
	switch_console_set_complete("add xml_odbc debug_off");
	switch_console_set_complete("add xml_odbc render_template");

	if (do_config() != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Unable to load xml_odbc config file\n");
		return SWITCH_STATUS_FALSE;
	}

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}


SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_xml_odbc_shutdown)
{
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
