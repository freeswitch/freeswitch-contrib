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
 * Do ODBC queries through API.
 *
 */
#include <switch.h>

#define dbg(str) switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, str)


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

  const void *key;
  void *val;
  char *t_name, *t_value;
  char *query;

  /* no one may access globals.queries for a moment */
  switch_mutex_lock(globals.mutex);

  /* general settings */
  if (switch_xml_config_parse_module_settings(cf, reload, instructions) != SWITCH_STATUS_SUCCESS) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not open %s\n", cf);
    goto done;
  }

  /* also parse queries section here */
  if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
    goto done;
  }

  /* empty the globals.queries hash */
  for (hi = switch_core_hash_first(globals.queries); hi;) {
    switch_core_hash_this(hi, &key, NULL, &val);
    query = (char *) val;
    hi = switch_core_hash_next(&hi);
    switch_safe_free(query);
    switch_core_hash_delete(globals.queries, (char *) key);
  }

  /* get queries and insert them in globals.queries */
  if ((x_queries = switch_xml_child(cfg, "queries"))) {
    for (x_query = switch_xml_child(x_queries, "query"); x_query; x_query = x_query->next) {
      t_name = (char *) switch_xml_attr_soft(x_query, "name");
      t_value = (char *) switch_xml_attr_soft(x_query, "value");

      if (zstr(t_name)) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "query defined without a name\n");
        continue;
      }

      if (zstr(t_value)) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "query %s doesn't have a value defined\n", t_name);
        continue;
      }

      query = strdup(t_value);
      switch_core_hash_insert(globals.queries, t_name, query);
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
  switch_stream_handle_t *stream;
  int rowcount;
} callback_t;


/* Execute SQL callback -- char *query WILL be modified !! */
static switch_bool_t execute_sql_callback(char *query, switch_core_db_callback_func_t callback, callback_t *cbt, char **err)
{
  switch_cache_db_connection_options_t options = { {0} };
  switch_cache_db_handle_t *dbh = NULL;
  char *first_space, *first_colon, *second_colon;
  char *sql_err = NULL;

  /* does the first 'word' (before the first space) of char *query have syntax db:user:pass ? */
  if ( ((first_space  = strchr(query, ' ')))
    && ((first_colon  = strchr(query, ':')))
    && ((second_colon = strchr(first_colon + 1, ':')))
    && (first_colon  > query)
    && (second_colon < first_space - 1) ) /* yea, what can I say.. */
  {
    /* copy the dsn options from the first 'word' of the query, then move *query a few chars to the right so it points to the query itself */
    *first_colon++  = '\0';
    *second_colon++ = '\0';
    *first_space++  = '\0';
    options.odbc_options.dsn  = query;
    options.odbc_options.user = first_colon;
    options.odbc_options.pass = second_colon;
    query = first_space;
  } else {
    /* copy the global dsn options */
    switch_mutex_lock(globals.mutex);
    if (!globals.odbc_dsn) { /* in case the module was unloaded while in this function */
      switch_mutex_unlock(globals.mutex);
      return SWITCH_FALSE;
    }
    options.odbc_options.dsn  = switch_core_strdup(cbt->pool, globals.odbc_dsn);
    options.odbc_options.user = switch_core_strdup(cbt->pool, globals.odbc_user);
    options.odbc_options.pass = switch_core_strdup(cbt->pool, globals.odbc_pass);
    switch_mutex_unlock(globals.mutex);
  }

  /* get database handle */
  if (switch_cache_db_get_db_handle(&dbh, SCDB_TYPE_ODBC, &options) != SWITCH_STATUS_SUCCESS) {
    *err = switch_core_sprintf(cbt->pool, "Unable to get ODBC handle.  dsn: %s:%s:%s, dbh is %s\n", 
      options.odbc_options.dsn, options.odbc_options.user, options.odbc_options.pass, dbh ? "not null" : "null");
    return SWITCH_FALSE;
  }

  /* perform the query */
  if (switch_cache_db_execute_sql_callback(dbh, query, callback, (void *) cbt, &sql_err) != SWITCH_STATUS_SUCCESS) {
    switch_cache_db_release_db_handle(&dbh);
    if (sql_err) { /* strdup via pool, so err doesn't need to be freed in calling function */
      *err = switch_core_strdup(cbt->pool, sql_err);
      free(sql_err);
    }
    return SWITCH_FALSE;
  }

  switch_cache_db_release_db_handle(&dbh);
  return SWITCH_TRUE;
}


/* Called for each row returned by query - when storing result in channel variables */
static int odbc_query_callback_channel(void *pArg, int argc, char **argv, char **columnName)
{
  callback_t *cbt = (callback_t *) pArg;
  cbt->rowcount++;

  if ((argc == 2) && (!strcmp(columnName[0], "name")) && (!strcmp(columnName[1], "value"))) {
    if (!zstr(argv[1])) {
      switch_channel_set_variable(cbt->channel, switch_str_nil(argv[0]), switch_str_nil(argv[1]));
    }
  } else {
    for (int i = 0; i < argc; i++) {
      if (!zstr(argv[i])) {
        switch_channel_set_variable(cbt->channel, columnName[i], switch_str_nil(argv[i]));
      }
    }
  }

  return 0;
}

/* Generate a txt string */
static int odbc_query_callback_txt(void *pArg, int argc, char **argv, char **columnName)
{
  callback_t *cbt = (callback_t *) pArg;
  cbt->rowcount++;

  for (int i = 0; i < argc; i++) {
    cbt->stream->write_function(cbt->stream, "%25s : ", columnName[i]);
    cbt->stream->write_function(cbt->stream, "%s\n", switch_str_nil(argv[i]));
  }
  cbt->stream->write_function(cbt->stream, "\n");

  return 0;
}

/* Generate a tab string */
static int odbc_query_callback_tab(void *pArg, int argc, char **argv, char **columnName)
{
  callback_t *cbt = (callback_t *) pArg;
  cbt->rowcount++;

  /* column names and a nice horizontal line */
  if (cbt->rowcount == 1) {
    for (int i = 0; i < argc; i++) {
      cbt->stream->write_function(cbt->stream, "%-20s", columnName[i]);
    }
    cbt->stream->write_function(cbt->stream, "\n");
    for (int i = 0; i < argc; i++) {
      cbt->stream->write_function(cbt->stream, "===================="); /* 20x = */
    }
    cbt->stream->write_function(cbt->stream, "\n");
  }

  for (int i = 0; i < argc; i++) {
    cbt->stream->write_function(cbt->stream, "%-20s", switch_str_nil(argv[i]));
  }
  cbt->stream->write_function(cbt->stream, "\n");

  return 0;
}

/* Generate an xml string */
/* TODO There is no proper rewriting of < to &lt;, > to &gt;, etc FIXIT! */
static int odbc_query_callback_xml(void *pArg, int argc, char **argv, char **columnName)
{
  callback_t *cbt = (callback_t *) pArg;
  cbt->rowcount++;

  cbt->stream->write_function(cbt->stream,   "    <row>\n");
  for (int i = 0; i < argc; i++) {
    cbt->stream->write_function(cbt->stream, "      <column name=\"%s\" value=\"%s\"/>\n", columnName[i], switch_str_nil(argv[i]));
  }
  cbt->stream->write_function(cbt->stream,   "    </row>\n");

  return 0;
}

SWITCH_STANDARD_APP(odbc_query_app_function)
{
  switch_time_t start_time = switch_micro_time_now();
  int elapsed_ms;
  callback_t cbt = { 0 };
  char *query = NULL;
  char *t_query = NULL;
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

  /* if data contains no space then it must be the name of a query */
  if (!strchr(data, ' ')) {
    switch_mutex_lock(globals.mutex);
    if ((query = switch_core_hash_find(globals.queries, data))) {
      t_query = switch_core_session_strdup(session, query);
    } else {
      switch_mutex_unlock(globals.mutex);
      switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not find a query named: [%s]\n", data);
      goto done;
    }
    switch_mutex_unlock(globals.mutex);
  } else {
    t_query = switch_core_session_strdup(session, data);
  }

  query = switch_channel_expand_variables(switch_core_session_get_channel(session), t_query);

  /* Execute expanded query */
  if (!execute_sql_callback(query, odbc_query_callback_channel, &cbt, &err)) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Unable to perform query: %s\n", err ? err : "(null)");
  }

  if (query != t_query) free(query);

 done:

  /* How long did it take ? */
  elapsed_ms = (int) (switch_micro_time_now() - start_time) / 1000;
  switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG10, "Got %d rows in %d ms\n", cbt.rowcount, elapsed_ms);
}


#define ODBC_QUERY_API_FUNCTION_SYNTAX "[txt|tab|xml] [db:user:pass] <SELECT * FROM foo WHERE true;>"
SWITCH_STANDARD_API(odbc_query_api_function)
{
  switch_time_t start_time = switch_micro_time_now();
  int elapsed_ms;
  switch_core_db_callback_func_t callback;
  callback_t cbt = { 0 };
  char *format;
  char *err = NULL;
  char *query;

  if (zstr(cmd)) {
    stream->write_function(stream, "-USAGE: %s\n", ODBC_QUERY_API_FUNCTION_SYNTAX);
    return SWITCH_STATUS_SUCCESS;
  }

  /* initialize memory pool */
  if (switch_core_new_memory_pool(&cbt.pool) != SWITCH_STATUS_SUCCESS) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not initialize memory pool\n");
    return SWITCH_STATUS_FALSE;
  }

  cbt.rowcount = 0;

  /* set format, callback, optionally (xml) generate the header of the response (in stream) and point cmd to the start of the query */
  if (strstr(cmd, "txt ") == cmd) {
    format = "txt";
    callback = odbc_query_callback_txt;
    cmd += 4;
  } else if (strstr(cmd, "tab ") == cmd) {
    format = "tab";
    callback = odbc_query_callback_tab;
    cmd += 4;
  } else if (strstr(cmd, "xml ") == cmd) {
    format = "xml";
    callback = odbc_query_callback_xml;
    stream->write_function(stream, "<result>\n  <rows>\n");
    cmd += 4;
  } else {
    format = "txt";
    callback = odbc_query_callback_txt;
  }

  cbt.stream = stream;

  query = strdup(cmd);

  /* perform the query with the correct callback */
  if (!execute_sql_callback(query, callback, &cbt, &err)) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Unable to perform query: %s\n", err ? err : "(null)");
  }

  elapsed_ms = (int) (switch_micro_time_now() - start_time) / 1000;

/* TODO FIX IT SO THAT err HAS THE " < > REPLACED BY &quot; &lt; &gt; IN CASE OF XML !!!! */

  /* generate trailer with meta data */
  if (!strcmp(format, "txt") || !strcmp(format, "tab")) {
    stream->write_function(stream, "\nGot %d rows returned in %d ms.", cbt.rowcount, elapsed_ms);
  } else if (!strcmp(format, "xml")) {
    stream->write_function(stream, "  </rows>\n");
    stream->write_function(stream, "  <meta>\n");
    stream->write_function(stream, "    <error>%s</error>\n", err);
    stream->write_function(stream, "    <rowcount>%d</rowcount>\n", cbt.rowcount);
    stream->write_function(stream, "    <elapsed_ms>%d</elapsed_ms>\n", elapsed_ms);
    stream->write_function(stream, "  </meta>\n");
    stream->write_function(stream, "</result>\n");
  }

  /* cleanup time */
  switch_safe_free(query);
  switch_core_destroy_memory_pool(&cbt.pool);

  return SWITCH_STATUS_SUCCESS;
}


/* Macro expands to: switch_status_t mod_odbc_query_load(switch_loadable_module_interface_t **module_interface, switch_memory_pool_t *pool) */
SWITCH_MODULE_LOAD_FUNCTION(mod_odbc_query_load)
{
  switch_application_interface_t *app_interface;
  switch_api_interface_t *api_interface;

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
  if (switch_core_hash_init(&globals.queries) != SWITCH_STATUS_SUCCESS) {
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
  SWITCH_ADD_API(api_interface, "odbc_query", "Perform an ODBC query", odbc_query_api_function, ODBC_QUERY_API_FUNCTION_SYNTAX);

  switch_console_set_complete("add odbc_query");
  switch_console_set_complete("add odbc_query txt");
  switch_console_set_complete("add odbc_query tab");
  switch_console_set_complete("add odbc_query xml");

  /* indicate that the module should continue to be loaded */
  return SWITCH_STATUS_SUCCESS;
}


/*
  Called when the system shuts down
  Macro expands to: switch_status_t mod_odbc_query_shutdown() */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_odbc_query_shutdown)
{
  switch_hash_index_t *hi;
  const void *key;
  void *val;
  char *query;

  switch_console_set_complete("del odbc_query");

  switch_event_unbind_callback(reload_event_handler);

  switch_mutex_lock(globals.mutex);
  for (hi = switch_core_hash_first(globals.queries); hi;) {
    switch_core_hash_this(hi, &key, NULL, &val);
    query = (char *) val;
    hi = switch_core_hash_next(&hi);
    switch_safe_free(query);
    switch_core_hash_delete(globals.queries, (char *) key);
  }
  switch_safe_free(globals.odbc_dsn);
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
