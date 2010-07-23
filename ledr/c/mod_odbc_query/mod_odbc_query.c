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
  switch_mutex_t *mutex;
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

  options.odbc_options.dsn  = query->odbc_dsn;
  options.odbc_options.user = query->odbc_user;
  options.odbc_options.pass = query->odbc_pass;

  if (switch_cache_db_get_db_handle(&dbh, SCDB_TYPE_ODBC, &options) != SWITCH_STATUS_SUCCESS)
    dbh = NULL;

  return dbh;
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
      if ((globals.odbc_user = strchr(globals.odbc_dsn, ':'))) {
        *globals.odbc_user++ = '\0';
        if ((globals.odbc_pass = strchr(globals.odbc_user, ':'))) {
          *globals.odbc_pass++ = '\0';
        }
      }
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

  query_t *query;
  char *t_name, *t_odbc_dsn, *t_odbc_dsn_2, *t_odbc_user, *t_odbc_pass, *t_value;

  switch_mutex_lock(globals.mutex);

  if (switch_xml_config_parse_module_settings(cf, reload, instructions) != SWITCH_STATUS_SUCCESS) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not open %s\n", cf);
    goto done;
  }

  /* also parse queries section here */
  if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
    goto done;
  }

  /* get queries and insert (or over-write) them in globals.queries */
  if ((x_queries = switch_xml_child(cfg, "queries"))) {
    for (x_query = switch_xml_child(x_queries, "query"); x_query; x_query = x_query->next) {
      t_name = (char *) switch_xml_attr_soft(x_query, "name");
      t_value = (char *) switch_xml_attr_soft(x_query, "value");
      t_odbc_dsn = (char *) switch_xml_attr_soft(x_query, "odbc-dsn");

      if (zstr(t_name)) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "query defined without a name\n");
        continue;
      }

      if (zstr(t_value)) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "query %s doesn't have a value defined\n", t_name);
        continue;
      }

      if ((query = switch_core_hash_find(globals.queries, t_name))) {
        switch_safe_free(query->odbc_dsn);
        switch_safe_free(query->value);
      } else {
        switch_zmalloc(query, sizeof(*query));
        switch_core_hash_insert(globals.queries, t_name, query);
        query->name = strdup(t_name);
      }

      query->value = strdup(t_value);

      if (!zstr(t_odbc_dsn)) {
        t_odbc_dsn_2 = strdup(t_odbc_dsn);
        if ((t_odbc_user = strchr(t_odbc_dsn_2, ':'))) {
          *t_odbc_user++ = '\0';
          if ((t_odbc_pass = strchr(t_odbc_user, ':'))) {
            *t_odbc_pass++ = '\0';
            query->odbc_dsn = strdup(t_odbc_dsn_2);
            query->odbc_user = strdup(t_odbc_user);
            query->odbc_pass = strdup(t_odbc_pass);
          } else {
            switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "invalid odbc-dsn, using global one\n");
          }
        } else {
          switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "invalid odbc-dsn, using global one\n");
        }
        switch_safe_free(t_odbc_dsn_2);
      }

      if (!query->odbc_dsn) {
        query->odbc_dsn = strdup(globals.odbc_dsn);
        query->odbc_user = strdup(globals.odbc_user);
        query->odbc_pass = strdup(globals.odbc_pass);
      }

    }
  }

  /* remove entries from globals.queries that are not in current configuration */
  for (hi = switch_hash_first(NULL, globals.queries); hi;) {
    switch_hash_this(hi, NULL, NULL, &val);
    query = (query_t *) val;
    hi = switch_hash_next(hi);
    if (!switch_xml_find_child(x_queries, "query", "name", query->name)) {
      switch_core_hash_delete(globals.queries, query->name);
      switch_safe_free(query->name);
      switch_safe_free(query->odbc_dsn);
      switch_safe_free(query->value);
      switch_safe_free(query);
    }
  }

  status = SWITCH_STATUS_SUCCESS;

 done:

  switch_mutex_unlock(globals.mutex);

  if (xml) {
    switch_xml_free(xml);
  }

  return status;
}


/* Called on reload */
static void reload_event_handler(switch_event_t *event)
{
  do_config(SWITCH_TRUE);
}

static switch_event_node_t *reload_xml_event = NULL;


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
  query_t *query, *t_query;
  char *expanded_query_value = NULL;
  char *err = NULL;

  cbt.pool = switch_core_session_get_pool(session);
  cbt.channel = switch_core_session_get_channel(session);
  cbt.rowcount = 0;

  switch_zmalloc(query, sizeof(*query));

  if (!cbt.channel) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "How can there be no channel ?!\n");
    goto done;
  }

	if (!data || zstr(data)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No odbc-query data was provided !\n");
    goto done;
	}

  if (strchr(data, ' ')) {
    switch_mutex_lock(globals.mutex);
    query->odbc_dsn  = switch_core_session_strdup(session, globals.odbc_dsn);
    query->odbc_user = switch_core_session_strdup(session, globals.odbc_user);
    query->odbc_pass = switch_core_session_strdup(session, globals.odbc_pass);
		query->value     = switch_core_session_strdup(session, data);
    switch_mutex_unlock(globals.mutex);
  }

	else if ((t_query = switch_core_hash_find(globals.queries, data))) {
    switch_mutex_lock(globals.mutex);
    query->name      = switch_core_session_strdup(session, t_query->name);
    query->odbc_dsn  = switch_core_session_strdup(session, t_query->odbc_dsn);
    query->odbc_user = switch_core_session_strdup(session, t_query->odbc_user);
    query->odbc_pass = switch_core_session_strdup(session, t_query->odbc_pass);
    query->value     = switch_core_session_strdup(session, t_query->value);
    switch_mutex_unlock(globals.mutex);
  }

	else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not find a query named: [%s]\n", data);
    goto done;
	}

  expanded_query_value = switch_channel_expand_variables_by_pool(cbt.pool, cbt.channel, query->value);

  switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG10, "Expanded query %s, value %s\n", query->name, expanded_query_value);

  /* Execute expanded_query */
  if (!execute_sql_callback(query, expanded_query_value, odbc_query_callback, &cbt, &err)) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Unable to lookup cid: %s\n", err ? err : "(null)");
  }

  /* cleanup */
  switch_safe_free(query);

 done:

  /* How long did it take ? */
  stop = switch_micro_time_now();
  switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG10, "Elapsed Time %u ms\n", (uint32_t) (stop - start)/1000 );
}


/* Macro expands to: switch_status_t mod_odbc_query_load(switch_loadable_module_interface_t **module_interface, switch_memory_pool_t *pool) */
SWITCH_MODULE_LOAD_FUNCTION(mod_odbc_query_load)
{
	switch_application_interface_t *app_interface;

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "ODBC Query module loading...\n");

	/* check if core odbc is available */
	if (!switch_odbc_available()) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No core ODBC available!\n");
		return SWITCH_STATUS_FALSE;
	}

	memset(&globals, 0, sizeof(globals));

	globals.pool = pool;
  switch_mutex_init(&globals.mutex, SWITCH_MUTEX_NESTED, globals.pool);

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

  // TODO: CHECK WHETHER THIS WORKS: !!!
  switch_event_unbind_callback(reload_event_handler);

  switch_mutex_lock(globals.mutex);

  for (hi = switch_hash_first(NULL, globals.queries); hi;) {
    switch_hash_this(hi, NULL, NULL, &val);
    query = (query_t *) val;
    hi = switch_hash_next(hi);
    switch_core_hash_delete(globals.queries, query->name);
    switch_safe_free(query->name);
    switch_safe_free(query->odbc_dsn);
    switch_safe_free(query->value);
    switch_safe_free(query);
  }

  switch_mutex_unlock(globals.mutex);

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
