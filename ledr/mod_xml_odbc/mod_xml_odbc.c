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
	XML_ODBC_DIRECTORY
} xml_odbc_query_type_t;

SWITCH_MODULE_LOAD_FUNCTION(mod_xml_odbc_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_xml_odbc_shutdown);
SWITCH_MODULE_DEFINITION(mod_xml_odbc, mod_xml_odbc_load, mod_xml_odbc_shutdown, NULL);

typedef struct xml_binding {
	char *bindings;
} xml_binding_t;

static struct {
	char *dbname;
	char *odbc_dsn;
	char *query_domain_key_user;
	char *query_domain_params;
	char *query_domain_variables;
	char *query_user_attrs;
	char *query_user_params;
	char *query_user_variables;
	char *query_groups;
	switch_mutex_t *mutex;
	switch_memory_pool_t *pool;
#ifdef SWITCH_HAVE_ODBC
	switch_odbc_handle_t *master_odbc;
#else
	void *filler1;
#endif
} globals;

typedef struct did_uid_helper {
	char *domain_id;
	char *user_id;
} did_uid_helper_t;

static int did_uid_callback(void *pArg, int argc, char **argv, char **columnNames)
{
	did_uid_helper_t *h = (did_uid_helper_t *) pArg;

	h->domain_id = strdup(argv[0]);
	h->user_id = strdup(argv[1]);

	return 0;
}

static int set_xml_attr_callback(void *pArg, int argc, char **argv, char **columnNames)
{
	switch_xml_t *xml = (switch_xml_t *) pArg;
	switch_xml_set_attr_d(*xml, argv[0], argv[1]);
	return 0;
}

typedef struct xml_helper {
	switch_xml_t xml;
	char *str;
	int off;
} xml_helper_t;

static int add_xml_child_with_nvpair_callback(void *pArg, int argc, char **argv, char **columnNames)
{
	xml_helper_t *h = (xml_helper_t *) pArg;
	switch_xml_t xml = NULL;

	if ((xml = switch_xml_add_child_d(h->xml, h->str, h->off++))) {
		switch_xml_set_attr_d(xml, "name", argv[0]);
		switch_xml_set_attr_d(xml, "value", argv[1]);
	}

	return 0;
}

static int add_xml_group_and_user_callback(void *pArg, int argc, char **argv, char **columnName)
{
	xml_helper_t *h = (xml_helper_t *) pArg;
	switch_xml_t xml = NULL;

	if ((xml = switch_xml_add_child_d(h->xml, "group", h->off++))) {
		switch_xml_set_attr_d(xml, "name", argv[0]);
		if ((xml = switch_xml_add_child_d(xml, "users", h->off++))) {
			if ((xml = switch_xml_add_child_d(xml, "user", h->off++))) {
				switch_xml_set_attr_d(xml, "id", h->str);
				switch_xml_set_attr_d(xml, "type", "pointer");
			}
		}
	}

	return 0;
}

static int xml_odbc_result_not_found(switch_xml_t xml, int *off)
{
	switch_xml_t sub = NULL;

	if ((sub = switch_xml_add_child_d(xml, "section", *off++))) {
		switch_xml_set_attr_d(sub, "name", "result");
		if ((sub = switch_xml_add_child_d(sub, "result", *off++))) {
			switch_xml_set_attr_d(sub, "status", "not found");
		}
	}

	return 0;
}

static switch_xml_t xml_odbc_search(const char *section, const char *tag_name, const char *key_name, const char *key_value, switch_event_t *params, void *user_data)
{
	xml_binding_t *binding = (xml_binding_t *) user_data;
	switch_event_header_t *hi;

	switch_xml_t xml = NULL, sub = NULL, sub2 = NULL, sub3 = NULL;

	char *dir_user = NULL, *dir_domain = NULL, *dir_key = NULL;
	char *sql;

	int off = 0, ret = 1;

	xml_odbc_query_type_t query_type;

	did_uid_helper_t pdata;
	xml_helper_t pdata2;

	if (!binding) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No bindings... sorry bud returning now\n");
		return NULL;
	}

	if (!strcmp(section, "directory")) {
		query_type = XML_ODBC_DIRECTORY;
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Invalid section\n");
		return NULL;
	}

	if (params) {
		if ((hi = params->headers)) {
			for (; hi; hi = hi->next) {
				switch (query_type) {
				case XML_ODBC_DIRECTORY:

//switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG in xml_odbc_search with header name=[%s], value=[%s]\n", hi->name, hi->value);

					if (!strcmp(hi->name, "user")) {
						dir_user = strdup(hi->value);
					} else if (!strcmp(hi->name, "domain")) {
						dir_domain = strdup(hi->value);
					} else if (!strcmp(hi->name, "key")) {
						dir_key = strdup(hi->value);
					}
					break;
				}
			}
			switch (query_type) {
			case XML_ODBC_DIRECTORY: 
				//if (dir_user && dir_domain) {
				if (dir_domain) {
					if ((xml = switch_xml_new("document"))) {
						switch_xml_set_attr_d(xml, "type", "freeswitch/xml");

						if (!dir_user) {
//						  sql = switch_mprintf("SELECT count(*) FROM dir_domains WHERE name = '%q';", dir_domain);
//						  if (switch_odbc_handle_callback_exec(globals.master_odbc, sql, count_callback, &pdata10741) == SWITCH_ODBC_SUCCESS) {
// TODO: Factor out the domain search thing (with params and variables) from below, and call it from here as well..
//							xml_odbc_result_not_found(xml, &off);
//						  } else {
							xml_odbc_result_not_found(xml, &off);
//						  }

						} else {
							sql = switch_mprintf(globals.query_domain_key_user, dir_domain, dir_key, dir_user);
							if (switch_odbc_handle_callback_exec(globals.master_odbc, sql, did_uid_callback, &pdata) == SWITCH_ODBC_SUCCESS) {
								if (!pdata.domain_id || !pdata.user_id) {
									xml_odbc_result_not_found(xml, &off);

								} else {
									if ((sub = switch_xml_add_child_d(xml, "section", off++))) {
										switch_xml_set_attr_d(sub, "name", "directory");

										if ((sub = switch_xml_add_child_d(sub, "domain", off++))) {
											switch_xml_set_attr_d(sub, "name", dir_domain);

											if ((sub2 = switch_xml_add_child_d(sub, "params", off++))) {
												sql = switch_mprintf(globals.query_domain_params, pdata.domain_id);
												pdata2.xml = sub2;
												pdata2.off = off;
												pdata2.str = "param";
												if (switch_odbc_handle_callback_exec(globals.master_odbc, sql, add_xml_child_with_nvpair_callback, &pdata2) == SWITCH_ODBC_SUCCESS) {
												}
											}

											if ((sub2 = switch_xml_add_child_d(sub, "variables", off++))) {
												sql = switch_mprintf(globals.query_domain_variables, pdata.domain_id);
												pdata2.xml = sub2;
												pdata2.off = off;
												pdata2.str = "variable";
												if (switch_odbc_handle_callback_exec(globals.master_odbc, sql, add_xml_child_with_nvpair_callback, &pdata2) == SWITCH_ODBC_SUCCESS) {
												}
											}

											if ((sub = switch_xml_add_child_d(sub, "groups", off++))) {

												if ((sub2 = switch_xml_add_child_d(sub, "group", off++))) {
													switch_xml_set_attr_d(sub2, "name", "default");

													if ((sub2 = switch_xml_add_child_d(sub2, "users", off++))) {

														if ((sub2 = switch_xml_add_child_d(sub2, "user", off++))) {

															sql = switch_mprintf(globals.query_user_attrs, pdata.user_id);
															if (switch_odbc_handle_callback_exec(globals.master_odbc, sql, set_xml_attr_callback, &sub2)) {
															}

															if ((sub3 = switch_xml_add_child_d(sub2, "params", off++))) {
																sql = switch_mprintf(globals.query_user_params, pdata.user_id);
																pdata2.xml = sub3;
																pdata2.off = off;
																pdata2.str = "param";
																if (switch_odbc_handle_callback_exec(globals.master_odbc, sql, add_xml_child_with_nvpair_callback, &pdata2) == SWITCH_ODBC_SUCCESS) {
																}
															}

															if ((sub3 = switch_xml_add_child_d(sub2, "variables", off++))) {
																sql = switch_mprintf(globals.query_user_variables, pdata.user_id);
																pdata2.xml = sub3;
																pdata2.off = off;
																pdata2.str = "variable";
																if (switch_odbc_handle_callback_exec(globals.master_odbc, sql, add_xml_child_with_nvpair_callback, &pdata2) == SWITCH_ODBC_SUCCESS) {
																}
															}

														}

													}

												}

												sql = switch_mprintf(globals.query_groups, pdata.user_id);
												pdata2.xml = sub;
												pdata2.off = off;
												pdata2.str = dir_user;
												if (switch_odbc_handle_callback_exec(globals.master_odbc, sql, add_xml_group_and_user_callback, &pdata2) == SWITCH_ODBC_SUCCESS) {
												}

											}
										}
									}
								}
							}
						}
					}

//switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "DEBUG DUMP OF XML GENERATED:\n\n%s\n\n", switch_xml_toxml(xml, SWITCH_FALSE));

					free(dir_user);
					dir_user = NULL;

					free(dir_key);
					dir_key = NULL;

					free(dir_domain);
					dir_domain = NULL;

				} else {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR,
						"Something bad happened during the query construction phase likely exten(%s) or domain(%s) is null\n", dir_user, dir_domain);
					goto cleanup;

				}
				break;
			}

		} else {
			goto cleanup;

		}
	}

	ret = 0;

  cleanup:

	if (ret) {
		switch_xml_free(xml);
		return NULL;
	}
	
	return xml;
}

static switch_status_t do_config()
{
	char *cf = "xml_odbc.conf";
	switch_xml_t cfg, xml, bindings_tag, binding_tag, param;
	xml_binding_t *binding = NULL;

	//switch_core_db_t *db;
	switch_status_t status = SWITCH_STATUS_SUCCESS;

	char *odbc_user = NULL;
	char *odbc_pass = NULL;
	char *sql = NULL;

	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Open of %s failed\n", cf);
		return SWITCH_STATUS_TERM;
	}

	if (!(bindings_tag = switch_xml_child(cfg, "bindings"))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing <bindings> tag!\n");
		goto done;
	}

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
			} else if (!strcasecmp(var, "query-domain-key-user") && !switch_strlen_zero(val)) {
				globals.query_domain_key_user = strdup(val);
			} else if (!strcasecmp(var, "query-domain-params") && !switch_strlen_zero(val)) {
				globals.query_domain_params = strdup(val);
			} else if (!strcasecmp(var, "query-domain-variables") && !switch_strlen_zero(val)) {
				globals.query_domain_variables = strdup(val);
			} else if (!strcasecmp(var, "query-user-attrs") && !switch_strlen_zero(val)) {
				globals.query_user_attrs = strdup(val);
			} else if (!strcasecmp(var, "query-user-params") && !switch_strlen_zero(val)) {
				globals.query_user_params = strdup(val);
			} else if (!strcasecmp(var, "query-user-variables") && !switch_strlen_zero(val)) {
				globals.query_user_variables = strdup(val);
			} else if (!strcasecmp(var, "query-groups") && !switch_strlen_zero(val)) {
				globals.query_groups = strdup(val);
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

	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

#ifndef SWITCH_HAVE_ODBC
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "You must have ODBC support in FreeSWITCH to use this module\n");
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "\t./configure --enable-core-odbc-support\n");
	return SWITCH_STATUS_FALSE;
#endif

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "XML ODBC module loading...\n");

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
