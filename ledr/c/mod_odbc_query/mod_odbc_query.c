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

static struct {
	switch_odbc_handle_t *master_odbc;
} globals;

static char* switch_channel_expand_variables_by_pool(switch_memory_pool_t *pool, switch_channel_t *channel, const char *in)
{
	char *tmp, *out;

	tmp = switch_channel_expand_variables(channel, in);
	out = switch_core_strdup(pool, tmp);
	if (tmp != in) switch_safe_free(tmp);

	return out;
}

static switch_xml_config_item_t instructions[] = {
	/* parameter name, type, reloadable, pointer, default value, options structure */
  SWITCH_CONFIG_ITEM_END()
};

static switch_status_t do_config(switch_bool_t reload)
{
	memset(&globals, 0, sizeof(globals));

	if (switch_xml_config_parse_module_settings("odbc_query.conf", reload, instructions) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not open odbc_query.conf\n");
		return SWITCH_STATUS_FALSE;
	}
	
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_STANDARD_API(odbc_query_function)
{
	do_config(SWITCH_TRUE);
	
	return SWITCH_STATUS_SUCCESS;
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

SWITCH_STANDARD_APP(odbc_query_app_function)
{
	char *query = NULL;
	char *expanded_query = NULL;
	switch_channel_t *channel = switch_core_session_get_channel(session);
	switch_memory_pool_t *pool = switch_core_session_get_pool(session);

	if (!channel) {
		return;
	}

	/* get query from data if it contains space, otherwise get it from <queries> section in configuration */
	if (strchr(data, ' ')) {
		query = switch_core_session_strdup(session, data);
	} else {
	}

	expanded_query = switch_channel_expand_variables_by_pool(pool, channel, query);

	if (switch_odbc_handle_callback_exec(globals.master_odbc, expanded_query, odbc_query_callback, channel) != SWITCH_ODBC_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error running this query: [%s]\n", expanded_query);
	}
}

/* Macro expands to: switch_status_t mod_odbc_query_load(switch_loadable_module_interface_t **module_interface, switch_memory_pool_t *pool) */
SWITCH_MODULE_LOAD_FUNCTION(mod_odbc_query_load)
{
	switch_api_interface_t *api_interface;
	switch_application_interface_t *app_interface;

	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Hello World!\n");
	
	do_config(SWITCH_FALSE);
	
	SWITCH_ADD_API(api_interface, "odbc_query", "ODBC Query API", odbc_query_function, "syntax");
	SWITCH_ADD_APP(app_interface, "odbc_query", "Perform an ODBC query", "Perform an ODBC query", odbc_query_app_function, "<query>", SAF_SUPPORT_NOMEDIA);

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/*
  Called when the system shuts down
  Macro expands to: switch_status_t mod_odbc_query_shutdown() */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_odbc_query_shutdown)
{
	/* Cleanup dynamically allocated config settings */
	switch_xml_config_cleanup(instructions);
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
