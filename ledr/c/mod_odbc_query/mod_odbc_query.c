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


/* Globals struct */
static struct {
	char *odbc_dsn;
  char *odbc_user;
  char *odbc_pass;
	switch_hash_t *queries;
	switch_memory_pool_t *pool;
} globals;


/* Per query DSN */
typedef struct query_obj {
  char *name;
  char *odbc_dsn;
  char *odbc_user;
  char *odbc_pass;
  char *value;
} query_t;


static char* switch_channel_expand_variables_by_pool(switch_memory_pool_t *pool, switch_channel_t *channel, const char *in)
{
	char *tmp, *out;

	tmp = switch_channel_expand_variables(channel, in);
	out = switch_core_strdup(pool, tmp);
	if (tmp != in) switch_safe_free(tmp);

	return out;
}


/* Get database handle */
static switch_cache_db_handle_t *get_db_handle(query_t *query)
{
  switch_cache_db_connection_options_t options = { {0} };
  switch_cache_db_handle_t *dbh = NULL;

  options.odbc_options.dsn = query->odbc_dsn;
  options.odbc_options.user = query->odbc_user;
  options.odbc_options.pass = query->odbc_pass;

  if (switch_cache_db_get_db_handle(&dbh, SCDB_TYPE_ODBC, &options) != SWITCH_STATUS_SUCCESS)
    dbh = NULL;

  return dbh;
}


/* Split ODBC DSN into DB, User and Pass */
static switch_status_t split_dsn_into_db_user_pass(char *db, char *user, char *pass)
{
  if ((user = strchr(db, ':'))) {
    *user++ = '\0';
    if ((pass = strchr(user, ':'))) {
      *pass++ = '\0';
    }
  }
  return SWITCH_STATUS_SUCCESS;
}


/* Config callback - save odbc_dsn, _user and _pass in globals */
static switch_status_t config_callback_dsn(switch_xml_config_item_t *data, const char *newvalue,
  switch_config_callback_type_t callback_type, switch_bool_t changed)
{
  if ((callback_type == CONFIG_LOAD || callback_type == CONFIG_RELOAD) && changed) {
    if (zstr(newvalue)) {
      switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "No local database defined.\n");
    } else {
      switch_safe_free(globals.odbc_dsn);
      globals.odbc_dsn = strdup(newvalue);
      split_dsn_into_db_user_pass(globals.odbc_dsn, globals.odbc_user, globals.odbc_pass);
    }
  }
  return SWITCH_STATUS_SUCCESS;
}


/* Config item validations */
/* static switch_xml_config_string_options_t config_opt_valid_anything = { NULL, 0, NULL }; */
static switch_xml_config_string_options_t config_opt_valid_odbc_dsn = { NULL, 0, "^.+:.+:.+$" };


/* Config items */
static switch_xml_config_item_t instructions[] = {
  SWITCH_CONFIG_ITEM_CALLBACK("odbc-dsn", SWITCH_CONFIG_STRING, CONFIG_REQUIRED | CONFIG_RELOADABLE,
    &globals.odbc_dsn, "db:user:password", config_callback_dsn, &config_opt_valid_odbc_dsn, "db:user:password", "ODBC DSN to use"),
  SWITCH_CONFIG_ITEM_END()
};


/* Do Config - Called at startup and reload of this module */
static switch_status_t do_config(switch_bool_t reload)
{
  char *cf = "odbc_query.conf";
  switch_xml_t cfg, xml = NULL, x_queries, x_query;
  switch_status_t status = SWITCH_STATUS_FALSE;
  switch_hash_index_t *hi;
  void *val;
  query_t *query, *query_tmp;

  if (switch_xml_config_parse_module_settings(cf, reload, instructions) != SWITCH_STATUS_SUCCESS) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not open %s\n", cf);
    return SWITCH_STATUS_FALSE;
  }

  /* also parse queries section here */
  if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
    goto done;
  }

  /* get queries and put them in globals.queries */
  if ((x_queries = switch_xml_child(cfg, "queries"))) {
    for (x_query = switch_xml_child(x_queries, "query"); x_query; x_query = x_query->next) {

      switch_zmalloc(query, sizeof(*query));

      query->name = (char *) switch_xml_attr_soft(x_query, "name");
      query->odbc_dsn = (char *) switch_xml_attr(x_query, "odbc-dsn");
      query->value = (char *) switch_xml_attr_soft(x_query, "value");

      if (zstr(query->name) || zstr(query->value)) {
        free(query);
      } else {
        if (query->odbc_dsn) {
          split_dsn_into_db_user_pass(query->odbc_dsn, query->odbc_user, query->odbc_pass);
        } else {
          query->odbc_dsn = globals.odbc_dsn;
          query->odbc_user = globals.odbc_pass;
          query->odbc_pass = globals.odbc_pass;
        }
        if ((query_tmp = switch_core_hash_find(globals.queries, query->name)))
          free(query_tmp);
        switch_core_hash_insert(globals.queries, query->name, query);
      }

    }
  }

  /* remove entries from globals.queries that are not in current configuration */
  for (hi = switch_hash_first(NULL, globals.queries); hi;) {
    switch_hash_this(hi, NULL, NULL, &val);
    query_tmp = (query_t *) val;
    hi = switch_hash_next(hi);
    if (!switch_xml_find_child(x_queries, "query", "name", query_tmp->name)) {
      switch_core_hash_delete(globals.queries, query_tmp->name);
      free(query_tmp);
    }
  }

  status = SWITCH_STATUS_SUCCESS;

 done:

  /* Cleanup */
  switch_xml_free(xml);

  return status;
}


/* Called on reload */
static void reload_event_handler(switch_event_t *event)
{
  do_config(SWITCH_TRUE);
}

static switch_event_node_t *reload_xml_event = NULL;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/* Callback_t struct passed to each callback */
typedef struct callback_obj {
  switch_channel_t *channel;
  switch_memory_pool_t *pool;
  int rowcount;
} callback_t;


/* Execute SQL callback */
static switch_bool_t execute_sql_callback(query_t *query, char *sql, switch_core_db_callback_func_t callback, callback_t *cbt, char **err)
{
  switch_bool_t retval = SWITCH_FALSE;
  switch_cache_db_handle_t *dbh = NULL;

  if (globals.odbc_dsn && (dbh = get_db_handle(query))) {
    if (switch_cache_db_execute_sql_callback(dbh, sql, callback, (void *) cbt, err) == SWITCH_ODBC_FAIL) {
      retval = SWITCH_FALSE;
    } else {
      retval = SWITCH_TRUE;
    }
  } else {
    *err = switch_core_sprintf(cbt->pool, "Unable to get ODBC handle.  dsn: %s, dbh is %s\n", globals.odbc_dsn, dbh ? "not null" : "null");
  }

  switch_cache_db_release_db_handle(&dbh);
  return retval;
}


/* Called for each row returned by query */
static int odbc_query_callback(void *pArg, int argc, char **argv, char **columnName)
{
  callback_t *cbt = (callback_t *) pArg;
  cbt->rowcount++;

	if ((argc == 2) && (!strcmp(columnName[0], "name")) && (!strcmp(columnName[1], "value"))) {
    if (!zstr(argv[1])) {
		  switch_channel_set_variable(cbt->channel, argv[0], argv[1]);
    }
	} else {
		for (int i = 0; i < argc; i++) {
      if (!zstr(argv[i])) {
			  switch_channel_set_variable(cbt->channel, columnName[i], argv[i]);
      }
		}
	}

	return 0;
}


SWITCH_STANDARD_APP(odbc_query_app_function)
{
  switch_time_t start = switch_micro_time_now();
  switch_time_t stop;
  callback_t cbt = { 0 };
  query_t *query;
  char *expanded_query_value = NULL;
  char *err = NULL;

  cbt.pool = switch_core_session_get_pool(session);
  cbt.channel = switch_core_session_get_channel(session);
  cbt.rowcount = 0;

  if (!cbt.channel) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "How can there be no channel ?!\n");
    goto done;
  }

	if (!data || zstr(data)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No odbc-query data was provided !\n");
    goto done;
	}

	/* get query from data if it contains space, otherwise get it from globals.queries */
	if (strchr(data, ' ')) {
    switch_zmalloc(query, sizeof(*query));
    query->name = "anonymous";
    query->odbc_dsn = globals.odbc_dsn;
    query->odbc_user = globals.odbc_user;
    query->odbc_pass = globals.odbc_pass;
		query->value = switch_core_session_strdup(session, data);
	} else if (!(query = switch_core_hash_find(globals.queries, data))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not find a query named: [%s]\n", data);
    goto done;
	}

  expanded_query_value = switch_channel_expand_variables_by_pool(cbt.pool, cbt.channel, query->value);
  switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG10, "Expanded query %s, value %s\n", query->name , expanded_query_value);

  /* Execute expanded_query */
  if (!execute_sql_callback(query, expanded_query_value, odbc_query_callback, &cbt, &err)) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Unable to lookup cid: %s\n", err ? err : "(null)");
  }

  /* cleanup */
  if (query && !strcmp(query->name, "anonymous")) {
    free(query);
  }

 done:

  /* How long did it take ? */
  stop = switch_micro_time_now();
  switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG10, "Elapsed Time %lu ms\n", (stop - start)/1000 );
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
	if (switch_core_hash_init(&globals.queries, globals.pool) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error initializing the queries hash\n");
		return SWITCH_STATUS_GENERR;
	}

	if (do_config(SWITCH_FALSE) != SWITCH_STATUS_SUCCESS) {
		return SWITCH_STATUS_FALSE;
	}
	
	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	/* subscribe to reloadxml event, and hook it to reload_event_handler */
	if ((switch_event_bind_removable(modname, SWITCH_EVENT_RELOADXML, NULL, reload_event_handler, NULL, &reload_xml_event) != SWITCH_STATUS_SUCCESS)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind event!\n");
		return SWITCH_STATUS_TERM;
	}

	SWITCH_ADD_APP(app_interface, "odbc_query", "Perform an ODBC query", "Perform an ODBC query", odbc_query_app_function, "<query|query-name>", SAF_SUPPORT_NOMEDIA | SAF_ROUTING_EXEC);

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}


/*
  Called when the system shuts down
  Macro expands to: switch_status_t mod_odbc_query_shutdown() */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_odbc_query_shutdown)
{
  query_t *query;
  switch_hash_index_t *hi;
  void *val;

  /* Free all queries in globals.queries */
  for (hi = switch_hash_first(NULL, globals.queries); hi; hi = switch_hash_next(hi)) {
    switch_hash_this(hi, NULL, NULL, &val);
    query = (query_t *) val;
    free(query);
  }

  switch_core_hash_destroy(&globals.queries);

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
