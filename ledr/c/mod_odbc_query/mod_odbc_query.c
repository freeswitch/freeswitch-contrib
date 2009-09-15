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
 * Leon de Rooij <leon@toyos.nl>
 *
 *
 * mod_odbc_query.c -- ODBC Query
 *
 * Do ODBC queries from the dialplan and store all returned values as channel variables.
 *
 */
#include <switch.h>

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_odbc_query_shutdown);
SWITCH_MODULE_LOAD_FUNCTION(mod_odbc_query_load);
SWITCH_MODULE_DEFINITION(mod_odbc_query, mod_odbc_query_load, mod_odbc_query_shutdown, NULL);

static switch_event_node_t *NODE = NULL;

static struct {
	char *odbc_dsn;
	switch_memory_pool_t *pool;
	switch_odbc_handle_t *odbc_handle;
	switch_hash_t *queries_hash;
} globals;

static char* switch_channel_expand_variables_by_pool(switch_memory_pool_t *pool, switch_channel_t *channel, const char *in)
{
	char *tmp, *out;

	tmp = switch_channel_expand_variables(channel, in);
	out = switch_core_strdup(pool, tmp);
	if (tmp != in) switch_safe_free(tmp);

	return out;
}

static int odbc_query_callback(void *pArg, int argc, char **argv, char **columnName)
{
	switch_channel_t *channel = (switch_channel_t *) pArg;

	if ((argc == 2) && (!strcmp(columnName[0], "name")) && (!strcmp(columnName[1], "value"))) {
		switch_channel_set_variable(channel, argv[0], argv[1]);
	} else {
		for (int i = 0; i < argc; i++) {
			switch_channel_set_variable(channel, columnName[i], argv[i]);
		}
	}

	return 0;
}

static switch_status_t do_config(switch_bool_t reload)
{
	char *cf = "odbc_query.conf";
	switch_xml_t cfg, xml = NULL, settings, param, queries, query;
	char *odbc_user = NULL;
	char *odbc_pass = NULL;
	switch_hash_index_t *hi;
	void *val;
	char *query_name;

	switch_status_t status = SWITCH_STATUS_FALSE;

	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
		status = SWITCH_STATUS_FALSE;
		goto done;
	}
	
	/* get settings - only odbc-dsn for now */
	if ((settings = switch_xml_child(cfg, "settings"))) {
		for (param = switch_xml_child(settings, "param"); param; param = param->next) {
			char *var = (char *) switch_xml_attr_soft(param, "name");
			char *val = (char *) switch_xml_attr_soft(param, "value");
			if (!strcasecmp(var, "odbc-dsn") && (!switch_strlen_zero(val))) {
				globals.odbc_dsn = switch_core_strdup(globals.pool, val);
				if ((odbc_user = strchr(globals.odbc_dsn, ':'))) {
					*odbc_user++ = '\0';
					if ((odbc_pass = strchr(odbc_user, ':'))) {
						*odbc_pass++ = '\0';
					}
				}
			}
		}
	}

	/* get queries and put them in globals.queries_hash */
	if ((queries = switch_xml_child(cfg, "queries"))) {
		for (query = switch_xml_child(queries, "query"); query; query = query->next) {
			char *var = (char *) switch_xml_attr_soft(query, "name");
			char *val = (char *) switch_xml_attr_soft(query, "value");
			if (!switch_strlen_zero(var) && !switch_strlen_zero(val)) {
				switch_core_hash_insert(globals.queries_hash, var, val);
			}
		}
	}

	/* remove entries from globals.queries_hash that are not in current configuration */
	for (hi = switch_hash_first(NULL, globals.queries_hash); hi;) {
		switch_hash_this(hi, NULL, NULL, &val);
		query_name = (char *) val;
		hi = switch_hash_next(hi); /* if this works, I'm going to eat my shoe */
		if (!switch_xml_find_child(queries, "query", "name", query_name)) {
			switch_core_hash_delete(globals.queries_hash, query_name);
		}
	}

	/* check if odbc_dsn is set */
	if (!globals.odbc_dsn) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No odbc-dsn setting is set!\n");
		goto done;
	}

	/* make odbc connection */
	if (!reload) {
		if (!(globals.odbc_handle = switch_odbc_handle_new(globals.odbc_dsn, odbc_user, odbc_pass))) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Cannot Open ODBC Database!\n");
			goto done;
		}

		if (switch_odbc_handle_connect(globals.odbc_handle) != SWITCH_ODBC_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Cannot Open ODBC Database!\n");
			goto done;
		}

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Connected ODBC DSN: [%s]\n", globals.odbc_dsn);
	}

	status = SWITCH_STATUS_SUCCESS;

 done:
	switch_xml_free(xml);
	return status;
}

static void reload_event_handler(switch_event_t *event)
{
	do_config(SWITCH_TRUE);
}

SWITCH_STANDARD_APP(odbc_query_app_function)
{
	char *query = NULL;
	char *expanded_query = NULL;
	switch_channel_t *channel = switch_core_session_get_channel(session);
	switch_memory_pool_t *pool = switch_core_session_get_pool(session);

	if (!channel) {
		return;
	}

	if (!data || switch_strlen_zero(data)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No query provided!\n");
	}

	/* get query from data if it contains space, otherwise get it from globals.queries_hash */
	if (strchr(data, ' ')) {
		query = switch_core_session_strdup(session, data);
	} else if (!(query = switch_core_hash_find(globals.queries_hash, data))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not find a query named: [%s]\n", data);
		return;
	}

	expanded_query = switch_channel_expand_variables_by_pool(pool, channel, query);

	/* TODO is it possible that expanded_query now still points points to an entry in globals.queries_hash ?! */

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Performing query: [%s]\n", expanded_query);

	if (switch_odbc_handle_callback_exec(globals.odbc_handle, expanded_query, odbc_query_callback, channel) != SWITCH_ODBC_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error running this query: [%s]\n", expanded_query);
	}
}

/* Macro expands to: switch_status_t mod_odbc_query_load(switch_loadable_module_interface_t **module_interface, switch_memory_pool_t *pool) */
SWITCH_MODULE_LOAD_FUNCTION(mod_odbc_query_load)
{
	switch_application_interface_t *app_interface;

	memset(&globals, 0, sizeof(globals));

	globals.pool = pool;

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "ODBC Query module loading...\n");

	/* check if core odbc is available */
	if (!switch_odbc_available()) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No core ODBC available!\n");
		return SWITCH_STATUS_FALSE;
	}

	/* allocate the queries hash */
	if (switch_core_hash_init(&globals.queries_hash, globals.pool) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error initializing the queries hash\n");
		return SWITCH_STATUS_GENERR;
	}

	if (do_config(SWITCH_FALSE) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Unable to load xml_odbc config file\n");
		return SWITCH_STATUS_FALSE;
	}
	
	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	/* subscribe to reloadxml event, and hook it to reload_event_handler */
	if ((switch_event_bind_removable(modname, SWITCH_EVENT_RELOADXML, NULL, reload_event_handler, NULL, &NODE) != SWITCH_STATUS_SUCCESS)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind event!\n");
		return SWITCH_STATUS_TERM;
	}

	SWITCH_ADD_APP(app_interface, "odbc_query", "Perform an ODBC query", "Perform an ODBC query", odbc_query_app_function, "<query|query-name>", SAF_SUPPORT_NOMEDIA);

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/*
  Called when the system shuts down
  Macro expands to: switch_status_t mod_odbc_query_shutdown() */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_odbc_query_shutdown)
{
	switch_odbc_handle_disconnect(globals.odbc_handle);
	switch_odbc_handle_destroy(&globals.odbc_handle);
	switch_core_hash_destroy(&globals.queries_hash);

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
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4
 */
