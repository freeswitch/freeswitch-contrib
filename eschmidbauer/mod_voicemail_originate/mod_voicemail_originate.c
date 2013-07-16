/*
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2012, Anthony Minessale II <anthm@freeswitch.org>
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
 * Emmanuel Schmidbauer <e.schmidbauer@gmail.com>
 *
 * mod_voicemail_originate.c -- Monitor MWI and originate call based on module profile parameters and voicemail user parameters
 *
 */
#include <switch.h>

#define ORIGINATE_EVENT "cp::event"
#define VM_ORIGINATE_SQLITE_DB_NAME "voicemail_originate"

/* Prototypes */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_voicemail_originate_shutdown);
SWITCH_MODULE_RUNTIME_FUNCTION(mod_voicemail_originate_runtime);
SWITCH_MODULE_LOAD_FUNCTION(mod_voicemail_originate_load);

/* SWITCH_MODULE_DEFINITION(name, load, shutdown, runtime)
 * Defines a switch_loadable_module_function_table_t and a static const char[] modname
 */
SWITCH_MODULE_DEFINITION(mod_voicemail_originate, mod_voicemail_originate_load, mod_voicemail_originate_shutdown, mod_voicemail_originate_runtime);

static const char *global_cf = "voicemail_originate.conf";

static char create_sql[] =
"CREATE TABLE voicemail_originate (\n"
"   c_user           VARCHAR(255),\n"
"   c_domain         VARCHAR(255),\n"
"   c_state          VARCHAR(255) DEFAULT 'WAIT_FOR_NEXT_ATTEMPT',\n"
"   c_status         VARCHAR(255) DEFAULT '',\n"
"   mwi_status       VARCHAR(255),\n"
"   created          INTEGER NOT NULL,\n"
"   next_attempt     INTEGER NOT NULL DEFAULT 0,\n"
"   attempt_count    INTEGER NOT NULL DEFAULT 0\n"
"   );\n";

static switch_xml_config_int_options_t config_int_0_86400 = { SWITCH_TRUE, 0, SWITCH_TRUE, 86400 };

static struct {
	char *odbc_dsn;
	char *dbname;
	switch_hash_t *profile_hash;
	int32_t running;
	int32_t threads;
	switch_mutex_t *mutex;
	switch_memory_pool_t *pool;
	switch_event_node_t *message_waiting_event_handler;
	switch_event_node_t *originate_event_handler;
} globals;

struct user_profile {
	switch_bool_t profile_enabled;
	int first_wait;
	int wait;
	int timeout;
	int attempts;
	const char *transfer;
	const char *caller_name;
	const char *caller_number;
	switch_bool_t enable_confirm;
	const char *confirm_digits;
	const char *confirm_play_file;
	const char *user_variables;
	const char *dial_string;
	switch_memory_pool_t *pool;
};
typedef struct user_profile user_profile_t;

typedef enum {
	PFLAG_DESTROY = 1 << 0
} flags_t;

#define PROFILE_CONFIGITEM_COUNT 14

struct config_profile {
	char *name;
	switch_bool_t profile_enabled;
	uint32_t first_wait;
	uint32_t wait;
	uint32_t timeout;
	uint32_t attempts;
	const char *transfer;
	const char *caller_name;
	const char *caller_number;
	switch_bool_t enable_confirm;
	const char *confirm_digits;
	const char *confirm_play_file;
	const char *user_variables;
	const char *dial_string;
	uint32_t flags;
	switch_mutex_t *mutex;
	switch_thread_rwlock_t *rwlock;
	switch_xml_config_item_t config[PROFILE_CONFIGITEM_COUNT];
	switch_memory_pool_t *pool;
	switch_xml_config_string_options_t config_str_pool;
};
typedef struct config_profile config_profile_t;

struct call_helper {
	const char *user;
	const char *domain;
	int attempt_number;
	switch_memory_pool_t *pool;
};

typedef enum {
	VM_ORIGINATE_SUCCESS,
	VM_ORIGINATE_FAIL,
	VM_ORIGINATE_CANNOT_MODIFY,
	VM_ORIGINATE_USER_NOT_EXIST,
	VM_ORIGINATE_USER_EXISTS,
	VM_ORIGINATE_INVALID_KEY,
	VM_ORIGINATE_INVALID_VALUE_BOOL
} vm_originate_status_t;

switch_time_t local_epoch_time_now(switch_time_t *t)
{
	switch_time_t now = switch_micro_time_now() / 1000000; /* APR_USEC_PER_SEC */
	if (t) {
		*t = now;
	}
	return now;
}

switch_time_t local_epoch_time_next(int t)
{
	switch_time_t now = switch_micro_time_now() / 1000000; /* APR_USEC_PER_SEC */
	if (t) {
		now += t;
	}
	return now;
}

switch_cache_db_handle_t *get_db_handle(void)
{
	switch_cache_db_handle_t *dbh = NULL;
	char *dsn;
	if (!zstr(globals.odbc_dsn)) {
		dsn = globals.odbc_dsn;
	} else {
		dsn = globals.dbname;
	}
	if (switch_cache_db_get_db_handle_dsn(&dbh, dsn) != SWITCH_STATUS_SUCCESS) {
		dbh = NULL;
	}
	return dbh;
}

static switch_status_t vm_originate_execute_sql(char *sql, switch_mutex_t *mutex)
{
	switch_cache_db_handle_t *dbh = NULL;
	switch_status_t status = SWITCH_STATUS_FALSE;

	if (mutex) {
		switch_mutex_lock(mutex);
	} else {
		switch_mutex_lock(globals.mutex);
	}

	if (!(dbh = get_db_handle())) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error Opening DB\n");
		goto end;
	}

	status = switch_cache_db_execute_sql(dbh, sql, NULL);

end:

	switch_cache_db_release_db_handle(&dbh);

	if (mutex) {
		switch_mutex_unlock(mutex);
	} else {
		switch_mutex_unlock(globals.mutex);
	}

	return status;
}

char *vm_originate_execute_sql2str(switch_mutex_t *mutex, char *sql, char *resbuf, size_t len)
{
	char *ret = NULL;

	switch_cache_db_handle_t *dbh = NULL;

	if (!(dbh = get_db_handle())) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error Opening DB\n");
		return NULL;
	}

	if (mutex) {
		switch_mutex_lock(mutex);
	} else {
		switch_mutex_lock(globals.mutex);
	}

	ret = switch_cache_db_execute_sql2str(dbh, sql, resbuf, len, NULL);

	if (mutex) {
		switch_mutex_unlock(mutex);
	} else {
		switch_mutex_unlock(globals.mutex);
	}

	switch_cache_db_release_db_handle(&dbh);

	return ret;
}

static switch_bool_t vm_originate_execute_sql_callback(switch_mutex_t *mutex, char *sql, switch_core_db_callback_func_t callback)
{
	switch_bool_t ret = SWITCH_FALSE;
	char *errmsg = NULL;
	switch_cache_db_handle_t *dbh = NULL;

	if (mutex) {
		switch_mutex_lock(mutex);
	} else {
		switch_mutex_lock(globals.mutex);
	}

	if (!(dbh = get_db_handle())) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error Opening DB\n");
		goto end;
	}

	switch_cache_db_execute_sql_callback(dbh, sql, callback, NULL, &errmsg);

	if (errmsg) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "SQL ERR: [%s] %s\n", sql, errmsg);
		free(errmsg);
	}

end:

	switch_cache_db_release_db_handle(&dbh);

	if (mutex) {
		switch_mutex_unlock(mutex);
	} else {
		switch_mutex_unlock(globals.mutex);
	}

	return ret;
}

static void free_user_profile(user_profile_t *profile) {
	if (profile) {
		switch_core_destroy_memory_pool(&profile->pool);
	}
}

config_profile_t *profile_set_config(config_profile_t *profile)
{
	int i = 0;

	profile->config_str_pool.pool = profile->pool;

	/*
	   SWITCH _CONFIG_SET_ITEM(item, "key", type, flags,
	   pointer, default, options, help_syntax, help_description)
	 */
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "enabled", SWITCH_CONFIG_BOOL, 0, &profile->profile_enabled, SWITCH_TRUE, NULL, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "first-wait", SWITCH_CONFIG_INT, 0, &profile->first_wait, 10, &config_int_0_86400, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "wait", SWITCH_CONFIG_INT, 0, &profile->wait, 10, &config_int_0_86400, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "timeout", SWITCH_CONFIG_INT, 0, &profile->timeout, 20, &config_int_0_86400, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "attempts", SWITCH_CONFIG_INT, 0, &profile->attempts, 1, &config_int_0_86400, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "transfer", SWITCH_CONFIG_STRING, 0, &profile->transfer, NULL, &profile->config_str_pool, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "caller-name", SWITCH_CONFIG_STRING, 0, &profile->caller_name, NULL, &profile->config_str_pool, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "caller-number", SWITCH_CONFIG_STRING, 0, &profile->caller_number, NULL, &profile->config_str_pool, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "enable-confirm", SWITCH_CONFIG_BOOL, 0, &profile->enable_confirm, SWITCH_FALSE, NULL, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "confirm-digits", SWITCH_CONFIG_STRING, 0, &profile->confirm_digits, NULL, &profile->config_str_pool, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "confirm-play-file", SWITCH_CONFIG_STRING, 0, &profile->confirm_play_file, NULL, &profile->config_str_pool, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "user-variables", SWITCH_CONFIG_STRING, 0, &profile->user_variables, NULL, &profile->config_str_pool, NULL, NULL);
	SWITCH_CONFIG_SET_ITEM(profile->config[i++], "dial-string", SWITCH_CONFIG_STRING, 0, &profile->dial_string, NULL, &profile->config_str_pool, NULL, NULL);

	switch_assert(i < PROFILE_CONFIGITEM_COUNT);

	return profile;
}

static config_profile_t * load_profile(const char *profile_name)
{
	config_profile_t *profile = NULL;
	switch_xml_t x_profiles, cfg, xml, x_profile;
	switch_event_t *event = NULL;

	if (!(xml = switch_xml_open_cfg(global_cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Open of %s failed\n", global_cf);
		return profile;
	}
	if (!(x_profiles = switch_xml_child(cfg, "profiles"))) {
		goto end;
	}

	if ((x_profile = switch_xml_find_child(x_profiles, "profile", "name", profile_name))) {
		switch_memory_pool_t *pool;
		int count;

		if (switch_core_new_memory_pool(&pool) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Pool Failure\n");
			goto end;
		}

		if (!(profile = switch_core_alloc(pool, sizeof(config_profile_t)))) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Alloc Failure\n");
			switch_core_destroy_memory_pool(&pool);
			goto end;
		}

		profile->pool = pool;
		profile_set_config(profile);

		count = switch_event_import_xml(switch_xml_child(x_profile, "param"), "name", "value", &event);

		if (switch_xml_config_parse_event(event, count, SWITCH_FALSE, profile->config) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to process configuration\n");
			switch_core_destroy_memory_pool(&pool);
			goto end;
		}

		switch_thread_rwlock_create(&profile->rwlock, pool);
		profile->name = switch_core_strdup(pool, profile_name);

		switch_mutex_init(&profile->mutex, SWITCH_MUTEX_NESTED, profile->pool);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Added profile %s\n", profile->name);
		switch_core_hash_insert(globals.profile_hash, profile->name, profile);
	}

end:

	if (xml) {
		switch_xml_free(xml);
	}
	if (event) {
		switch_event_destroy(&event);
	}
	return profile;
}

static config_profile_t *get_profile(const char *profile_name)
{
	config_profile_t *profile = NULL;

	switch_mutex_lock(globals.mutex);
	if (!(profile = switch_core_hash_find(globals.profile_hash, profile_name))) {
		profile = load_profile(profile_name);
	}
	if (profile) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG10, "[%s] rwlock\n", profile->name);

		switch_thread_rwlock_rdlock(profile->rwlock);
	}
	switch_mutex_unlock(globals.mutex);

	return profile;
}

static void free_profile(config_profile_t *profile) {
	if (profile) {
		switch_core_destroy_memory_pool(&profile->pool);
	}
}

static void profile_rwunlock(config_profile_t *profile)
{
	switch_thread_rwlock_unlock(profile->rwlock);
	if (switch_test_flag(profile, PFLAG_DESTROY)) {
		if (switch_thread_rwlock_trywrlock(profile->rwlock) == SWITCH_STATUS_SUCCESS) {
			switch_thread_rwlock_unlock(profile->rwlock);
			free_profile(profile);
		}
	}
}

static void destroy_profile(const char *profile_name, switch_bool_t block)
{
	config_profile_t *profile = NULL;
	switch_mutex_lock(globals.mutex);
	if ((profile = switch_core_hash_find(globals.profile_hash, profile_name))) {
		switch_core_hash_delete(globals.profile_hash, profile_name);
	}
	switch_mutex_unlock(globals.mutex);

	if (!profile) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "[%s] Invalid profile\n", profile_name);
		return;
	}

	if (block) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "[%s] Waiting for write lock\n", profile->name);
		switch_thread_rwlock_wrlock(profile->rwlock);
	} else {
		if (switch_thread_rwlock_trywrlock(profile->rwlock) != SWITCH_STATUS_SUCCESS) {
			switch_set_flag(profile, PFLAG_DESTROY);
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "[%s] profile is in use, memory will be freed whenever its no longer in use\n",
					profile->name);
			return;
		}
	}

	free_profile(profile);
}

static user_profile_t * set_user_profile(const char *user, const char *domain, switch_event_t *event) {
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	switch_xml_t x_user;
	switch_xml_t x_param, x_params;
	user_profile_t *profile = NULL;

	if (switch_xml_locate_user_merged("id", user, domain, NULL, &x_user, NULL) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Can't find user [%s@%s]\n", user, domain);
		status = SWITCH_STATUS_FALSE;
	} else {
		switch_memory_pool_t *pool;
		char *user_variables = NULL;
		config_profile_t *config_profile = NULL;

		if (switch_core_new_memory_pool(&pool) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Pool Failure\n");
			status = SWITCH_STATUS_FALSE;
			goto end;
		}

		if (!(profile = switch_core_alloc(pool, sizeof(user_profile_t)))) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Alloc Failure\n");
			switch_core_destroy_memory_pool(&pool);
			status = SWITCH_STATUS_FALSE;
			goto end;
		}

		profile->pool = pool;

		/* Default Values */
		profile->caller_name = "0000000000";
		profile->caller_number = "0000000000";
		profile->timeout = 20;
		profile->attempts = 1;
		profile->wait = 30;
		profile->first_wait = 1;
		profile->confirm_digits = "0";
		profile->confirm_play_file = "";

		x_params = switch_xml_child(x_user, "params");
		for (x_param = switch_xml_child(x_params, "param"); x_param; x_param = x_param->next) {
			const char *var = switch_xml_attr_soft(x_param, "name");
			const char *val = switch_xml_attr_soft(x_param, "value");
			if (zstr(var) || zstr(val)) {
				continue; /* Ignore empty entires */
			} else if (!strcasecmp(var, "vm-originate-profile")) {
				if(!(config_profile = get_profile(val))) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Profile %s not found\n", val);
				} else {
					profile->profile_enabled = config_profile->profile_enabled;
					profile->enable_confirm = config_profile->enable_confirm;
					if (config_profile->confirm_digits) {
						profile->confirm_digits = config_profile->confirm_digits;
					}
					if (config_profile->confirm_play_file) {
						profile->confirm_play_file = config_profile->confirm_play_file;
					}
					if (config_profile->dial_string) {
						profile->dial_string = config_profile->dial_string;
					}
					if (config_profile->caller_name) {
						profile->caller_name = config_profile->caller_name;
					}
					if (config_profile->caller_number) {
						profile->caller_name = config_profile->caller_number;
					}
					if (config_profile->timeout) {
						profile->timeout = config_profile->timeout;
					}
					if (config_profile->wait) {
						profile->wait = config_profile->wait;
					}
					if (config_profile->first_wait) {
						profile->first_wait = config_profile->first_wait;
					}
					if (config_profile->attempts) {
						profile->attempts = config_profile->attempts;
					}
					if (config_profile->transfer) {
						profile->transfer = config_profile->transfer;
					}
					if (config_profile->user_variables) {
						user_variables = strdup(config_profile->user_variables);
					}
					profile_rwunlock(config_profile);
				}
			}
		}

		for (x_param = switch_xml_child(x_params, "param"); x_param; x_param = x_param->next) {
			const char *var = switch_xml_attr_soft(x_param, "name");
			const char *val = switch_xml_attr_soft(x_param, "value");
			if (zstr(var) || zstr(val)) {
				continue; /* Ignore empty entires */
			} else if (!strcasecmp(var, "vm-originate-enabled")) {
				if (!strcasecmp(val, "false")) {
					profile->profile_enabled = SWITCH_FALSE;
				} else {
					profile->profile_enabled = SWITCH_TRUE;
				}
			} else if (!strcasecmp(var, "vm-originate-enable-confirm")) {
				if (!strcasecmp(val, "true")) {
					profile->enable_confirm = SWITCH_TRUE;
				} else {
					profile->enable_confirm = SWITCH_FALSE;
				}
			} else if (!strcasecmp(var, "vm-originate-timeout")) {
				profile->timeout = atoi(val);
			} else if (!strcasecmp(var, "vm-originate-wait")) {
				profile->wait = atoi(val);
			} else if (!strcasecmp(var, "vm-originate-first-wait")) {
				profile->first_wait = atoi(val);
			} else if (!strcasecmp(var, "vm-originate-caller-name")) {
				profile->caller_name = switch_core_strdup(pool, val);
			} else if (!strcasecmp(var, "vm-originate-caller-number")) {
				profile->caller_number = switch_core_strdup(pool, val);
			} else if (!strcasecmp(var, "vm-originate-dial-string")) {
				profile->dial_string = switch_core_strdup(pool, val);
			} else if (!strcasecmp(var, "vm-originate-transfer")) {
				profile->transfer = switch_core_strdup(pool, val);
			} else if (!strcasecmp(var, "vm-originate-user-variables")) {
				user_variables = strdup(val);
			} else if (!strcasecmp(var, "vm-originate-attempts")) {
				profile->attempts = atoi(val);
			}
		}

		if(event && user_variables) {
			int var_num = 0;
			char *var_list[256] = { 0 };
			switch_xml_t x_var, x_vars;
			var_num = switch_separate_string(user_variables, ',', var_list, (sizeof(var_list) / sizeof(var_list[0])));
			x_vars = switch_xml_child(x_user, "variables");
			for (x_var = switch_xml_child(x_vars, "variable"); x_var; x_var = x_var->next) {
				const char *var = switch_xml_attr_soft(x_var, "name");
				const char *val = switch_xml_attr_soft(x_var, "value");
				if (zstr(var) || zstr(val)) {
					continue;
				} else {
					for (int var_i=0; var_i<var_num; var_i++) {
						const char *var_current = var_list[var_i];
						if(!strcasecmp(var, var_current)) {
							switch_event_add_header(event, SWITCH_STACK_BOTTOM, var, val);
						}
					}
				}
			}
		}
		switch_safe_free(user_variables);
	}
end:
	if (status == SWITCH_STATUS_SUCCESS) {
		return profile;
	} else {
		return NULL;
	}
}

static void *SWITCH_THREAD_FUNC originate_call_thread_run(switch_thread_t *thread, void *obj)
{
	struct call_helper *h = (struct call_helper *) obj;
	char *sql = NULL;
	switch_event_t *ovars;
	user_profile_t *profile = NULL;

	switch_mutex_lock(globals.mutex);
	globals.threads++;
	switch_mutex_unlock(globals.mutex);

	switch_event_create(&ovars, SWITCH_EVENT_REQUEST_PARAMS);

	if (!(profile = set_user_profile(h->user, h->domain, ovars))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Can't find user [%s@%s]\n", h->user, h->domain);
		sql = switch_mprintf("DELETE FROM voicemail_originate WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
		if(vm_originate_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
		}
		switch_safe_free(sql);
	} else {
		int transfer_num = 0;
		char *transfer_list[3] = { 0 };
		const char *dest = NULL, *dp = NULL, *context = NULL;
		//char *cid_num = "0000000000";
		char *account = switch_mprintf("%s@%s", h->user, h->domain);
		char *transfer_dup = NULL;

		switch_event_add_header(ovars, SWITCH_STACK_BOTTOM, "ignore_early_media", "true");
		switch_event_add_header(ovars, SWITCH_STACK_BOTTOM, "mwi_page_account", account);
		// TODO Not sure who have priority, the ovars of the {}[] from the dial string.. But one of the problem I saw is when the call is transfered to the dialplan, FS does a CID flipping... So if you dialled with loopback/5145551212/domain/XML it would set XML as the caller id.  Using origination_caller_id_number fix this issue...  But if you want to override it, not sure if it work or not...
		//switch_event_add_header(ovars, SWITCH_STACK_BOTTOM, "origination_caller_id_number", h->user); // TODO I'm also not a super fan of hard coding this... But if it can be overriden from the dial string, I guess it better than getting the XML from the loopback which is probably what most people will use
		if (profile->profile_enabled == SWITCH_FALSE) {
			sql = switch_mprintf("DELETE FROM voicemail_originate WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
			if(vm_originate_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
			}
			switch_safe_free(sql);
		} else {
			if(!zstr(profile->transfer)) {
				transfer_dup = strdup(profile->transfer);
				// This should give a compiler error
				transfer_num = switch_separate_string(transfer_dup, ' ', transfer_list, (sizeof(transfer_list) / sizeof(transfer_list[0])));
				dest = transfer_list[0];
				dp = transfer_list[1];
				context = transfer_list[2];
			}
			if(h->attempt_number >= profile->attempts) {
				sql = switch_mprintf("DELETE FROM voicemail_originate WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
				if(vm_originate_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
				}
				switch_safe_free(sql);
			} else if (!zstr(profile->dial_string) && !zstr(profile->transfer)) {
				char *sql;
				sql = switch_mprintf("UPDATE voicemail_originate SET c_state = 'CALL_IN_PROGRESS', c_status = '', attempt_count = attempt_count + 1 WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
				if(vm_originate_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
				} else {
					switch_core_session_t *caller_session = NULL;
					switch_status_t confirm_status = SWITCH_STATUS_SUCCESS;
					switch_call_cause_t cause = SWITCH_CAUSE_NORMAL_CLEARING;

					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Attempting voicemail call page: %s\n", profile->dial_string);
					if (switch_ivr_originate(NULL, &caller_session, &cause, profile->dial_string, profile->timeout, NULL, profile->caller_name, h->user, NULL, ovars, SOF_NONE, NULL) != SWITCH_STATUS_SUCCESS
							|| !caller_session) {
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "-ERR Cannot create outgoing channel! [%s] cause: %s\n", profile->dial_string, switch_channel_cause2str(cause));
						if (cause == SWITCH_CAUSE_ORIGINATOR_CANCEL) {
							sql = switch_mprintf("DELETE FROM voicemail_originate WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
							if(vm_originate_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
								switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
							}
							switch_safe_free(sql);
						}
						if (cause == SWITCH_CAUSE_NO_ANSWER) {
							sql = switch_mprintf("UPDATE voicemail_originate SET c_state = 'WAIT_FOR_NEXT_ATTEMPT', next_attempt = '%" SWITCH_TIME_T_FMT "' WHERE c_user = '%q' and c_domain = '%q';", local_epoch_time_next(profile->wait), h->user, h->domain);
							if(vm_originate_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
								switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
							}
							switch_safe_free(sql);
						}
					} else {
						if(profile->enable_confirm) {
							char pin_buf[80] = "";
							char *buf = pin_buf + strlen(pin_buf);
							char term = '\0';
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Confirming originate with DTMF %s\n", profile->confirm_digits);
							if (!zstr(profile->confirm_play_file)) {
								switch_ivr_play_file(caller_session, NULL, profile->confirm_play_file, NULL);
							}
							confirm_status = switch_ivr_collect_digits_count(caller_session, buf, sizeof(pin_buf) - strlen(pin_buf), strlen(profile->confirm_digits), "#", &term, 10000, 0, 0);
							if (confirm_status == SWITCH_STATUS_TIMEOUT) {
								confirm_status = SWITCH_STATUS_FALSE;
							}
							if (!strcmp(buf, profile->confirm_digits)) {
								switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Originate confirmed, %s entered.\n", profile->confirm_digits);
							} else {
								switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Originate unconfirmed, %s entered.\n", buf);
								confirm_status = SWITCH_STATUS_FALSE;
							}
						}
						switch_ivr_set_user(caller_session, account);
						switch_ivr_session_transfer(caller_session, dest, dp, context);
						switch_core_session_rwunlock(caller_session);
					}
					if (confirm_status == SWITCH_STATUS_SUCCESS && cause == SWITCH_CAUSE_SUCCESS) {
						sql = switch_mprintf("DELETE FROM voicemail_originate WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
						if(vm_originate_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
						}
						switch_safe_free(sql);
					} else {
						sql = switch_mprintf("UPDATE voicemail_originate SET c_state = 'WAIT_FOR_NEXT_ATTEMPT', next_attempt = '%" SWITCH_TIME_T_FMT "' WHERE c_user = '%q' and c_domain = '%q';", local_epoch_time_next(profile->wait), h->user, h->domain);
						if(vm_originate_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
						}
						switch_safe_free(sql);
					}
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Originate hangup cause: %s\n", switch_channel_cause2str(cause));
				}
				switch_safe_free(sql);
			}
		}
		switch_safe_free(transfer_dup);
		switch_safe_free(account);
	}

//done:
	free_user_profile(profile);
	switch_event_destroy(&ovars);
	switch_core_destroy_memory_pool(&h->pool);

	switch_mutex_lock(globals.mutex);
	globals.threads--;
	switch_mutex_unlock(globals.mutex);

	return NULL;
}

static int originate_callback(void *pArg, int argc, char **argv, char **columnNames)
{
	switch_thread_t *thread;
	switch_threadattr_t *thd_attr = NULL;
	switch_memory_pool_t *pool;
	struct call_helper *h;

	switch_core_new_memory_pool(&pool);
	h = switch_core_alloc(pool, sizeof(*h));
	h->pool = pool;
	h->user = switch_core_strdup(h->pool, argv[0]);
	h->domain = switch_core_strdup(h->pool, argv[1]);
	h->attempt_number = atoi(argv[2]);
	switch_threadattr_create(&thd_attr, h->pool);
	switch_threadattr_detach_set(thd_attr, 1);
	switch_threadattr_stacksize_set(thd_attr, SWITCH_THREAD_STACKSIZE);
	switch_thread_create(&thread, thd_attr, originate_call_thread_run, h, h->pool);
	return 0;
}

static void queue_originate_event(const char *account, const char *action, const char *status)
{
	if(account && action && status) {
		char *sql = NULL;
		char *account_split = strdup(account);
		int account_num = 0;
		char *account_list[2] = { 0 };
		const char *user = NULL, *domain = NULL;
		if ((account_num = switch_separate_string(account_split, '@', account_list, (sizeof(account_list) / sizeof(account_list[0])))) && account_num == 2) {
			user_profile_t *profile = NULL;

			user = account_list[0];
			domain = account_list[1];

			if (!(profile = set_user_profile(user, domain, NULL))) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Can't find user [%s@%s]\n", user, domain);
			} else {
				if (switch_false(status)) {
					sql = switch_mprintf("DELETE FROM voicemail_originate WHERE c_user = '%q' and c_domain = '%q';", user, domain);
					if(vm_originate_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
					}
					switch_safe_free(sql);
					switch_core_session_hupall_matching_var("mwi_page_account", account, SWITCH_CAUSE_ORIGINATOR_CANCEL);
				} else if (!strcasecmp(action, "NEW")) {
					char res[256];
					sql = switch_mprintf("SELECT COUNT(*) FROM voicemail_originate WHERE c_user = '%q' and c_domain = '%q';", user, domain);
					vm_originate_execute_sql2str(NULL, sql, res, sizeof(res));
					if (atoi(res) > 0) {
						/* if a new message is left while originate is happening, restart process */
						sql = switch_mprintf("DELETE FROM voicemail_originate WHERE c_user = '%q' and c_domain = '%q';", user, domain);
						if(vm_originate_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
						}
					}
					sql = switch_mprintf("INSERT INTO voicemail_originate (c_user, c_domain, created, next_attempt) VALUES ('%q', '%q', '%" SWITCH_TIME_T_FMT "', '%" SWITCH_TIME_T_FMT "');", user, domain, local_epoch_time_now(NULL), local_epoch_time_next(profile->first_wait));
					vm_originate_execute_sql(sql, NULL);
					switch_safe_free(sql);
				}

			}
		}
		switch_safe_free(account_split);
		switch_safe_free(sql);
	}

}

static void originate_event_handler(switch_event_t *event)
{
	if (event) {
		const char *account = switch_event_get_header(event, "Account");
		const char *action = switch_event_get_header(event, "Action");
		const char *status = switch_event_get_header(event, "Status");
		queue_originate_event(account, action, status);
	}
}
static void message_waiting_event_handler(switch_event_t *event)
{
	if (event) {
		const char *account = switch_event_get_header(event, "MWI-Message-Account");
		const char *status = switch_event_get_header(event, "MWI-Messages-Waiting");
		const char *action = switch_event_get_header(event, "MWI-Update-Reason");
		queue_originate_event(account, action, status);
	}
}

static switch_status_t load_config(void)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	switch_xml_t cfg, xml, settings, profiles, profile, param;
	switch_cache_db_handle_t *dbh = NULL;

	switch_mutex_lock(globals.mutex);

	if (!(xml = switch_xml_open_cfg(global_cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Open of %s failed\n", global_cf);
		status = SWITCH_STATUS_TERM;
		goto end;
	}

	if ((settings = switch_xml_child(cfg, "settings")) != NULL) {
		for (param = switch_xml_child(settings, "param"); param; param = param->next) {
			char *var = (char *) switch_xml_attr_soft(param, "name");
			char *val = (char *) switch_xml_attr_soft(param, "value");
			if (zstr(var) || zstr(val)) {
				continue; /* Ignore empty entires */
			}
			if (!strcasecmp(var, "dbname")) {
				globals.dbname = strdup(val);
			} else if (!strcasecmp(var, "odbc-dsn")) {
				globals.odbc_dsn = strdup(val);
			}
		}
	}

	/* Loading queue into memory struct */
	if ((profiles = switch_xml_child(cfg, "profiles"))) {
		for (profile = switch_xml_child(profiles, "profile"); profile; profile = profile->next) {
			load_profile(switch_xml_attr_soft(profile, "name"));
		}
	}
	if (!globals.dbname) {
		globals.dbname = strdup(VM_ORIGINATE_SQLITE_DB_NAME);
	}

	/* Initialize database */
	if (!(dbh = get_db_handle())) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Cannot open DB!\n");
		status = SWITCH_STATUS_TERM;
		goto end;
	}
	switch_cache_db_test_reactive(dbh, "SELECT count(*) from voicemail_originate", "drop table voicemail_originate", create_sql);
	switch_cache_db_release_db_handle(&dbh);

end:
	switch_mutex_unlock(globals.mutex);

	if (xml) {
		switch_xml_free(xml);
	}

	return status;
}

#define VM_ORIGINATE_API_SYNTAX "\
vm_originate load|unload|reload [profile_name]\n"\
"vm_originate init [user@domain]\n"
SWITCH_STANDARD_API(vm_originate_function)
{
	switch_event_t *event;
	char *mydata = NULL, *argv[2] = { 0 };
	const char *action = NULL;
	const char *account = NULL;
	int argc;

	if (!globals.running) {
		return SWITCH_STATUS_FALSE;
	}

	if (zstr(cmd)) {
		stream->write_function(stream, "-USAGE: \n%s\n", VM_ORIGINATE_API_SYNTAX);
		return SWITCH_STATUS_SUCCESS;
	}

	mydata = strdup(cmd);
	switch_assert(mydata);

	argc = switch_separate_string(mydata, ' ', argv, (sizeof(argv) / sizeof(argv[0])));

	if (argc < 2) {
		stream->write_function(stream, "%s", "-ERR Invalid!\n");
		goto done;
	}

	action = argv[0];
	account = argv[1];

	if (action && !strcasecmp(action, "init")) {
		if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, ORIGINATE_EVENT) == SWITCH_STATUS_SUCCESS) {
			stream->write_function(stream, "%s", "+OK\n");
			switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Account", account);
			switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Action", "NEW");
			switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Status", "1");
			switch_event_fire(&event);
		}
	} else if (action && !strcasecmp(action, "load")) {
		config_profile_t *profile = NULL;
		if ((profile = get_profile(account))) {
			profile_rwunlock(profile);
			stream->write_function(stream, "%s", "+OK\n");
		} else {
			stream->write_function(stream, "%s", "-ERR Invalid profile not found!\n");
		}
	} else if (action && !strcasecmp(action, "unload")) {
		destroy_profile(account, SWITCH_FALSE);
		stream->write_function(stream, "%s", "+OK\n");
	} else if (action && !strcasecmp(action, "reload")) {
		config_profile_t *profile = NULL;
		destroy_profile(account, SWITCH_FALSE);
		if ((profile = get_profile(account))) {
			profile_rwunlock(profile);
			stream->write_function(stream, "%s", "+OK\n");
		} else {
			stream->write_function(stream, "%s", "-ERR Invalid Profile not found!\n");
		}
	}

	goto done;
done:

	free(mydata);

	return SWITCH_STATUS_SUCCESS;
}

/* Macro expands to: switch_status_t mod_voicemail_originate_load(switch_loadable_module_interface_t **module_interface, switch_memory_pool_t *pool) */
SWITCH_MODULE_LOAD_FUNCTION(mod_voicemail_originate_load)
{
	switch_api_interface_t *api_interface;
	switch_status_t status;

	memset(&globals, 0, sizeof(globals));
	globals.pool = pool;
	switch_core_hash_init(&globals.profile_hash, globals.pool);
	switch_mutex_init(&globals.mutex, SWITCH_MUTEX_NESTED, globals.pool);

	if ((status = load_config()) != SWITCH_STATUS_SUCCESS) {
		return status;
	}

	// TODO We need to reset the DB so if we were in calling mode, we are definetly not anymore, etc...

	switch_mutex_lock(globals.mutex);
	if ((switch_event_bind_removable(modname, SWITCH_EVENT_MESSAGE_WAITING, NULL, message_waiting_event_handler, NULL, &globals.message_waiting_event_handler) != SWITCH_STATUS_SUCCESS)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Couldn't bind our message waiting handler!\n");
	}
	if ((switch_event_bind_removable(modname, SWITCH_EVENT_CUSTOM, ORIGINATE_EVENT, originate_event_handler, NULL, &globals.originate_event_handler) != SWITCH_STATUS_SUCCESS)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Couldn't bind our message waiting handler!\n");
	}
	switch_mutex_unlock(globals.mutex);

	switch_mutex_lock(globals.mutex);
	globals.running = 1;
	switch_mutex_unlock(globals.mutex);

	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_API(api_interface, "vm_originate", "VM-CallPage API", vm_originate_function, "syntax");

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/*
  Called when the system shuts down
  Macro expands to: switch_status_t mod_voicemail_originate_shutdown() */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_voicemail_originate_shutdown)
{
	switch_hash_index_t *hi;
	config_profile_t *profile;
	void *val = NULL;
	const void *key;
	switch_ssize_t keylen;
	int sanity = 0;

	switch_mutex_lock(globals.mutex);
	if (globals.running == 1) {
		globals.running = 0;
	}
	switch_mutex_unlock(globals.mutex);

	while (globals.threads) {
		switch_cond_next();
		if (++sanity >= 60000) {
			break;
		}
	}

	switch_mutex_lock(globals.mutex);
	while ((hi = switch_hash_first(NULL, globals.profile_hash))) {
		switch_hash_this(hi, &key, &keylen, &val);
		profile = (config_profile_t *) val;

		switch_core_hash_delete(globals.profile_hash, profile->name);

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Waiting for write lock (profile %s)\n", profile->name);
		switch_thread_rwlock_wrlock(profile->rwlock);

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Destroying profile %s\n", profile->name);

		switch_core_destroy_memory_pool(&profile->pool);
		profile = NULL;
	}
	switch_mutex_unlock(globals.mutex);

	switch_mutex_lock(globals.mutex);
	switch_event_unbind(&globals.message_waiting_event_handler);
	switch_event_unbind(&globals.originate_event_handler);
	switch_safe_free(globals.odbc_dsn);
	switch_safe_free(globals.dbname);
	switch_mutex_unlock(globals.mutex);
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_RUNTIME_FUNCTION(mod_voicemail_originate_runtime)
{
	// TODO We need more control on the result
	while(globals.running) {
		char *sql = switch_mprintf("SELECT c_user, c_domain, attempt_count FROM voicemail_originate where next_attempt <= '%" SWITCH_TIME_T_FMT "' AND (c_state='WAIT_FOR_NEXT_ATTEMPT' OR c_status='NEW')", local_epoch_time_now(NULL));
		//switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "SQL %s\n", sql);
		vm_originate_execute_sql_callback(NULL, sql, originate_callback);
		switch_safe_free(sql);
		switch_yield(1000000);
		switch_cond_next();
	}
	return SWITCH_STATUS_TERM;
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
