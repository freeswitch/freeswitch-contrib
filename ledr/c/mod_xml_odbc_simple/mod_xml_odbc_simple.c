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
 * Leon de Rooij <leon@scarlet-internet.nl> (user ledr on irc)
 *
 * Also thanks to:
 * Scarlet Telecom Nederland for funding this work.
 * Rupa Schumaker for borrowing his switch_cache_db* code from mod_cidlookup
 *
 *
 * mod_xml_odbc_simple.c -- Module for generating user directory XML through an ODBC SQL query.
 *
 *
 */
#include <switch.h>


SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_xml_odbc_simple_shutdown);
SWITCH_MODULE_LOAD_FUNCTION(mod_xml_odbc_simple_load);
SWITCH_MODULE_DEFINITION(mod_xml_odbc_simple, mod_xml_odbc_simple_load, mod_xml_odbc_simple_shutdown, NULL);


/* Globals struct */
static struct {
  char *odbc_dsn;
  char *odbc_user;
  char *odbc_pass;
  char *query;

  char *attrs;
  int attrs_c;
  char *attrs_v[10];

  char *params;
  int params_c;
  char *params_v[30];

  char *variables;
  int variables_c;
  char *variables_v[30];

  switch_hash_t *queries;

  switch_memory_pool_t *pool;
} globals;


/* Get database handle */
static switch_cache_db_handle_t *get_db_handle(void)
{
  switch_cache_db_connection_options_t options = { {0} };
  switch_cache_db_handle_t *dbh = NULL;

  if (!zstr(globals.odbc_dsn)) {
    options.odbc_options.dsn = globals.odbc_dsn;
    options.odbc_options.user = globals.odbc_user;
    options.odbc_options.pass = globals.odbc_pass;

    if (switch_cache_db_get_db_handle(&dbh, SCDB_TYPE_ODBC, &options) != SWITCH_STATUS_SUCCESS)
      dbh = NULL;
  }
  return dbh;
}


/* Config callback - split dsn into db:user:password and test connect to database */
static switch_status_t config_callback_dsn(switch_xml_config_item_t *data, const char *newvalue,
  switch_config_callback_type_t callback_type, switch_bool_t changed)
{
  switch_status_t status = SWITCH_STATUS_SUCCESS;

  switch_cache_db_handle_t *dbh = NULL;

  if (!switch_odbc_available()) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "ODBC is not compiled in.  Do not configure odbc-dsn parameter!\n");
    return SWITCH_STATUS_FALSE;
  }

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

      switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Connecting to dsn: %s\n", globals.odbc_dsn);

      if (!(dbh = get_db_handle())) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Cannot Open ODBC Database!\n");
        switch_goto_status(SWITCH_STATUS_FALSE, done);
      }
    }
  }

  switch_goto_status(SWITCH_STATUS_SUCCESS, done);

 done:
  switch_cache_db_release_db_handle(&dbh);
  return status;
}


/* Config callback - separate attrs, params or variables string and save elements in their globals. _c and _v */
static switch_status_t config_callback_separate_string(switch_xml_config_item_t *data, const char *newvalue,
  switch_config_callback_type_t callback_type, switch_bool_t changed)
{
  if ((callback_type == CONFIG_LOAD || callback_type == CONFIG_RELOAD) && changed) {
    if (!strcmp(data->key, "attrs")) {
      globals.attrs_c = switch_separate_string(globals.attrs, ',', globals.attrs_v,
                               (sizeof(globals.attrs_v) / sizeof(globals.attrs_v[0])));
    } else if (!strcmp(data->key, "params")) {
      globals.params_c = switch_separate_string(globals.params, ',', globals.params_v,
                               (sizeof(globals.params_v) / sizeof(globals.params_v[0])));
    } else if (!strcmp(data->key, "variables")) {
      globals.variables_c = switch_separate_string(globals.variables, ',', globals.variables_v,
                               (sizeof(globals.variables_v) / sizeof(globals.variables_v[0])));
    } else {
      return SWITCH_STATUS_FALSE;
    }
  }
  return SWITCH_STATUS_SUCCESS;
}


/* Config item validations */
static switch_xml_config_string_options_t config_opt_valid_anything = { NULL, 0, NULL };
static switch_xml_config_string_options_t config_opt_valid_odbc_dsn = { NULL, 0, "^.+:.+:.+$" };


/* Config items */
static switch_xml_config_item_t instructions[] = {
  SWITCH_CONFIG_ITEM_CALLBACK("odbc-dsn", SWITCH_CONFIG_STRING, CONFIG_REQUIRED | CONFIG_RELOADABLE,
    &globals.odbc_dsn, "db:user:password", config_callback_dsn, &config_opt_valid_odbc_dsn, "db:user:password", "ODBC DSN to use"),
  SWITCH_CONFIG_ITEM_CALLBACK("attrs", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE,
    &globals.attrs, "id,mailbox,cidr,number-alias", config_callback_separate_string, &config_opt_valid_anything, NULL, NULL),
  SWITCH_CONFIG_ITEM_CALLBACK("params", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE,
    &globals.params, "password", config_callback_separate_string, &config_opt_valid_anything, NULL, NULL),
  SWITCH_CONFIG_ITEM_CALLBACK("variables", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE,
    &globals.variables, "user_context", config_callback_separate_string, &config_opt_valid_anything, NULL, NULL),
  SWITCH_CONFIG_ITEM_END()
};


/* Do Config - Called at startup and reload of this module */
static switch_status_t do_config(switch_bool_t reload)
{
  char *cf = "xml_odbc_simple.conf";
  switch_xml_t cfg, xml = NULL, queries, query;
  switch_status_t status = SWITCH_STATUS_FALSE;
  switch_hash_index_t *hi;
  char *name, *value;
  void *val;

  if (switch_xml_config_parse_module_settings(cf, reload, instructions) != SWITCH_STATUS_SUCCESS) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not open xml_odbc_simple.conf\n");
    return SWITCH_STATUS_FALSE;
  }

  /* also parse queries section here (based on purpose event-header)
   * <query name="network-list" value="select..."/>
   * <query name="default" value="select..."/>
   */

  if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
    goto done;
  }

  /* get queries and put them in globals.queries */
  if ((queries = switch_xml_child(cfg, "queries"))) {
    for (query = switch_xml_child(queries, "query"); query; query = query->next) {
      name = (char *) switch_xml_attr_soft(query, "name");
      value = (char *) switch_xml_attr_soft(query, "value");
      if (!zstr(name) && !zstr(value)) {
        switch_core_hash_insert(globals.queries, name, value);
      }
    }
  }

  /* remove entries from globals.queries that are not in current configuration */
  for (hi = switch_hash_first(NULL, globals.queries); hi;) {
    switch_hash_this(hi, NULL, NULL, &val);
    name = (char *) val;
    hi = switch_hash_next(hi);
    if (!switch_xml_find_child(queries, "query", "name", name)) {
      switch_core_hash_delete(globals.queries, name);
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


/* Callback_t struct passed to each callback */
typedef struct callback_obj {
  switch_memory_pool_t *pool;
  switch_xml_t xml;
  int off;
  int rowcount;
} callback_t;


/* Execute SQL callback */
static switch_bool_t execute_sql_callback(char *sql, switch_core_db_callback_func_t callback, callback_t *cbt, char **err)
{
  switch_bool_t retval = SWITCH_FALSE;
  switch_cache_db_handle_t *dbh = NULL;

  if (globals.odbc_dsn && (dbh = get_db_handle())) {
    if (switch_cache_db_execute_sql_callback(dbh, sql, callback, (void *) cbt, err) == SWITCH_ODBC_FAIL) {
      retval = SWITCH_FALSE;
    } else {
      retval = SWITCH_TRUE;
    }
  } else {
    // NO POOL !! FIX IT ? TODO
    //*err = switch_core_sprintf(cbt->pool, "Unable to get ODBC handle.  dsn: %s, dbh is %s\n", globals.odbc_dsn, dbh ? "not null" : "null");
  }

  switch_cache_db_release_db_handle(&dbh);
  return retval;
}


/* Called for each row returned by query */
static int lookup_callback(void *pArg, int col_c, char **col_v, char **col_n)
{
  callback_t *cbt = (callback_t *) pArg;
  switch_xml_t user = NULL, params = NULL, variables = NULL, sub = NULL;
  int i, j;

  cbt->rowcount++;

  /* Add user, params and variables children */
  if ((user = switch_xml_add_child_d(cbt->xml, "user", cbt->off++))) {

    /* Add user attrs */
    for (i = 0; i < globals.attrs_c; i++) {
      for (j = 0; j < col_c; j++) {
        if (!strcmp(globals.attrs_v[i], col_n[j])) {
          if (strcmp(col_v[j], "")) {
            switch_xml_set_attr_d(user, col_n[j], col_v[j]);
          }
          break;
        }
      }
    }

    /* Add param children */
    for (i = 0; i < globals.params_c; i++) {
      for (j = 0; j < col_c; j++) {
        if (!strcmp(globals.params_v[i], col_n[j])) {
          if (strcmp(col_v[j], "")) {
            if (!params) params = switch_xml_add_child_d(user, "params", cbt->off++);
            if ((sub = switch_xml_add_child_d(params, "param", cbt->off++))) {
              switch_xml_set_attr_d(sub, "name", col_n[j]);
              switch_xml_set_attr_d(sub, "value", col_v[j]);
            }
          }
          break;
        }
      }
    }

    /* Add variable children */
    for (i = 0; i < globals.variables_c; i++) {
      for (j = 0; j < col_c; j++) {
        if (!strcmp(globals.variables_v[i], col_n[j])) {
          if (strcmp(col_v[j], "")) {
            if (!variables) variables = switch_xml_add_child_d(user, "variables", cbt->off++);
            if ((sub = switch_xml_add_child_d(variables, "variable", cbt->off++))) {
              switch_xml_set_attr_d(sub, "name", col_n[j]);
              switch_xml_set_attr_d(sub, "value", col_v[j]);
            }
          }
          break;
        }
      }
    }

  }

  return SWITCH_STATUS_SUCCESS;
}


/* Used to bind search function */
typedef struct xml_binding {
  char *bindings;
} xml_binding_t;


/* Actual search function */
static switch_xml_t xml_odbc_simple_search(const char *section, const char *tag_name, const char *key_name,
  const char *key_value, switch_event_t *event, void *user_data)
{
  switch_time_t start = switch_micro_time_now(), done;
  xml_binding_t *binding = (xml_binding_t *) user_data;
  switch_event_header_t *hi;
  switch_xml_t xml = NULL, sub = NULL;
  callback_t cbt = { 0 };
  char *domain = NULL, *purpose = NULL;
  char *query = NULL, *expanded_query = NULL;
  char *err = NULL;
  int off = 0;
  char *xml_char;

  if (!binding) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "How can there be no binding ?!\n");
    return NULL;
  }

  if (!event) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "How can there be no event ?!\n");
    return NULL;
  }

  /* Get value of domain from event headers */
  if ((hi = event->headers)) {
    for (; hi; hi = hi->next) {
      switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG10, "Event Header %s [%s]\n", hi->name, hi->value);
      if (!strcmp(hi->name, "domain")) {
        domain = strdup(hi->value);
      }
      else if (!strcmp(hi->name, "purpose")) {
        purpose = strdup(hi->value);
      }
    }
  }

  if (!domain) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No domain was specified !\n");
    switch_safe_free(purpose);
    return NULL;
  }

  if (!purpose) purpose = strdup("default");

  /* Create simple directory xml */
  if ((xml = switch_xml_new("directory"))) {
    switch_xml_set_attr_d(xml, "type", "freeswitch/xml");
    if ((sub = switch_xml_add_child_d(xml, "section", off++))) {
      switch_xml_set_attr_d(sub, "name", "directory");
      if (( sub = switch_xml_add_child_d(sub, "domain", off++))) {
        switch_xml_set_attr_d(sub, "name", domain);
      }
    }
  }

  /* Populate cbt */
  cbt.xml = sub;
  cbt.off = off;
  cbt.rowcount = 0;

  /* Generate query */
  if (!(query = switch_core_hash_find(globals.queries, purpose))) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Could not find a query named: [%s]\n", purpose);
    switch_safe_free(purpose);
    return NULL;
  }

  expanded_query = switch_event_expand_headers(event, query);
  switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG10, "SQL Query %s [%s]\n", purpose, expanded_query);

  /* Execute expanded_query */
  if (!execute_sql_callback(expanded_query, lookup_callback, &cbt, &err)) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Unable to lookup cid: %s\n", err ? err : "(null)");
  }

  /* Cleanup */

  switch_safe_free(purpose);

  if (expanded_query != query) {
    switch_safe_free(expanded_query);
  }

  switch_safe_free(domain);

  /* See if we got any entries returned */
  if (cbt.rowcount == 0) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG10, "Empty Result - No XML was generated\n");
  } else {
    xml_char = switch_xml_toxml(xml, SWITCH_FALSE);
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG10, "Result %i Rows - XML generated:\n%s", cbt.rowcount, xml_char);
    switch_safe_free(xml_char);
  }

  /* How long did it take ? */
  done = switch_micro_time_now();
  switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG10, "Elapsed Time %lu ms\n", (done - start)/1000 );

  if (cbt.rowcount == 0) {
    switch_xml_free(xml); /* don't forget */
    return NULL;
  }

  return xml;
}


SWITCH_MODULE_LOAD_FUNCTION(mod_xml_odbc_simple_load)
{
  xml_binding_t *binding = NULL;

  switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "XML ODBC Simple module loading...\n");

  memset(&globals, 0, sizeof(globals));

  globals.pool = pool;

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

  /* xml bind directory lookups */
  if (!(binding = malloc(sizeof(*binding)))) {
    return SWITCH_STATUS_FALSE;
  }
  memset(binding, 0, sizeof(*binding));
  binding->bindings = "directory";
  switch_xml_bind_search_function(xml_odbc_simple_search, switch_xml_parse_section_string(binding->bindings), binding);
  binding = NULL;

  /* connect my internal structure to the blank pointer passed to me */
  *module_interface = switch_loadable_module_create_module_interface(pool, modname);

  /* subscribe to reloadxml event, and hook it to reload_event_handler */
  if ((switch_event_bind_removable(modname, SWITCH_EVENT_RELOADXML, NULL, reload_event_handler, NULL, &reload_xml_event) != SWITCH_STATUS_SUCCESS)) {
    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind event!\n");
    return SWITCH_STATUS_TERM;
  }

  /* indicate that the module should continue to be loaded */
  return SWITCH_STATUS_SUCCESS;
}


SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_xml_odbc_simple_shutdown)
{
  switch_xml_unbind_search_function_ptr(xml_odbc_simple_search);
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
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4:
 */
