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
 *
 * mod_csta.c -- Handle CSTA stuff
 *
 */
#include <switch.h>

#define CSTA_SUBSCRIBE "csta::subscribe"
#define CSTA_SQLITE_DB_NAME "csta"

/* Prototypes */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_csta_shutdown);
SWITCH_MODULE_RUNTIME_FUNCTION(mod_csta_runtime);
SWITCH_MODULE_LOAD_FUNCTION(mod_csta_load);

/* SWITCH_MODULE_DEFINITION(name, load, shutdown, runtime)
 * Defines a switch_loadable_module_function_table_t and a static const char[] modname
 */
SWITCH_MODULE_DEFINITION(mod_csta, mod_csta_load, mod_csta_shutdown, mod_csta_runtime);

static const char *global_cf = "csta.conf";

static char create_dnd_subscriptions[] =
		"CREATE TABLE dnd_subscriptions (\n"
		"   sub_user     VARCHAR(255),\n"
		"   sub_host     VARCHAR(255),\n"
		"   contact         VARCHAR(1024),\n"
		"   call_id         VARCHAR(255),\n"
		"   state           VARCHAR(255),\n"
		"   device      	VARCHAR(255),\n"
		"   expires         INTEGER,\n"
		"   profile_name    VARCHAR(255),\n"
		"   hostname        VARCHAR(255)\n"
		");\n";

//static switch_xml_config_int_options_t config_int_0_86400 = { SWITCH_TRUE, 0, SWITCH_TRUE, 86400 };

static struct {
	char *odbc_dsn;
	char *dbname;
	int32_t running;
	int32_t threads;
	switch_mutex_t *mutex;
	switch_memory_pool_t *pool;
	switch_event_node_t *csta_event_handler;
} globals;

typedef enum {
	PFLAG_DESTROY = 1 << 0
} flags_t;

switch_time_t local_epoch_time_now(switch_time_t *t)
{
	switch_time_t now = switch_micro_time_now() / 1000000; /* APR_USEC_PER_SEC */
	if (t) {
		*t = now;
	}
	return now;
}

switch_time_t local_epoch_time_next(long t)
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

static switch_status_t csta_execute_sql(char *sql, switch_mutex_t *mutex)
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

char *csta_execute_sql2str(switch_mutex_t *mutex, char *sql, char *resbuf, size_t len)
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
/*
static switch_bool_t csta_execute_sql_callback(switch_mutex_t *mutex, char *sql, switch_core_db_callback_func_t callback)
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
		sql = switch_mprintf("DELETE FROM voicemail_callpage WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
		if(vm_callpage_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
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
		if (profile->page_enabled == SWITCH_FALSE) {
			sql = switch_mprintf("DELETE FROM voicemail_callpage WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
			if(vm_callpage_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
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
			if(h->attempt_number >= profile->attempt_limit) {
				sql = switch_mprintf("UPDATE voicemail_callpage SET c_state = 'LIMIT_REACHED' WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
				if(vm_callpage_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
				}
				switch_safe_free(sql);
			} else if (!zstr(profile->dial_string) && !zstr(profile->transfer)) {
				char *sql;
				sql = switch_mprintf("UPDATE voicemail_callpage SET c_state = 'CALL_IN_PROGRESS', c_status = '', attempt_count = attempt_count + 1 WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
				if(vm_callpage_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
				} else {
					//switch_channel_t *caller_channel;
					switch_core_session_t *caller_session = NULL;
					switch_call_cause_t cause = SWITCH_CAUSE_NORMAL_CLEARING;

					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Attempting voicemail call page: %s\n", profile->dial_string);
					// TODO I've tried to use a caller_profile so we can force the source domain/context, but that didn't work.  Also forcing this way the caller_id_name doesn't work well
					if (switch_ivr_originate(NULL, &caller_session, &cause, profile->dial_string, profile->attempt_timeout, NULL, profile->caller_name, h->user, NULL, ovars, SOF_NONE, NULL) != SWITCH_STATUS_SUCCESS
							|| !caller_session) {
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "-ERR Cannot create outgoing channel! [%s] cause: %s\n", profile->dial_string, switch_channel_cause2str(cause));
						if (cause == SWITCH_CAUSE_ORIGINATOR_CANCEL) {
							sql = switch_mprintf("DELETE FROM voicemail_callpage WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
							if(vm_callpage_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
								switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
							}
							switch_safe_free(sql);
						}
						if (cause == SWITCH_CAUSE_NO_ANSWER) {
							sql = switch_mprintf("UPDATE voicemail_callpage SET c_state = 'WAIT_FOR_NEXT_ATTEMPT', next_attempt = '%" SWITCH_TIME_T_FMT "' WHERE c_user = '%q' and c_domain = '%q';", local_epoch_time_next(profile->attempt_wait), h->user, h->domain);
							if(vm_callpage_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
								switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
							}
							switch_safe_free(sql);
						}
					} else {
						// TODO We could use switch_ivr_set_user(caller_session, account); to get value imported
						switch_ivr_set_user(caller_session, account);
						//caller_channel = switch_core_session_get_channel(caller_session);
						//switch_channel_set_variable(caller_channel, "mwi_page_account", NULL);
						switch_ivr_session_transfer(caller_session, dest, dp, context);
						switch_core_session_rwunlock(caller_session);
					}
					if (cause == SWITCH_CAUSE_SUCCESS && profile->page_end_on_answer) {
						sql = switch_mprintf("DELETE FROM voicemail_callpage WHERE c_user = '%q' and c_domain = '%q';", h->user, h->domain);
						if(vm_callpage_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
						}
						switch_safe_free(sql);
					}
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "ORIGINATE HANGUP CAUSE: %s\n", switch_channel_cause2str(cause));

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
*/

static void csta_subscribe_dnd(const char *user, const char *host, const char *contact, const char *call_id, const char *state, const char *device, const char *expires, const char *profile_name, const char *hostname)
{
	if (user && host && contact && call_id && state && device && expires && profile_name && hostname) {
		char *sql = NULL;
		char res[256];
		switch_event_t *event_notify;
		sql = switch_mprintf("SELECT COUNT(*) FROM dnd_subscriptions WHERE sub_user = '%q' AND sub_host = '%q';", user, host);
		csta_execute_sql2str(NULL, sql, res, sizeof(res));
		switch_safe_free(sql);
		if (atoi(res) == 0) {
			long exp = atoi(expires);
			sql = switch_mprintf("INSERT INTO dnd_subscriptions (sub_user, sub_host, contact, call_id, state, device, expires, profile_name, hostname) VALUES ('%q', '%q', '%q', '%q', '%q', '%q', '%" SWITCH_TIME_T_FMT "', '%q', '%q');", user, host, contact, call_id, state, device, local_epoch_time_next(exp), profile_name, hostname);
			csta_execute_sql(sql, NULL);
			switch_safe_free(sql);
		} else {
			long exp = atoi(expires);
			sql = switch_mprintf("UPDATE dnd_subscriptions SET contact = '%q', call_id = '%q', state = '%q', device = '%q', expires = '%" SWITCH_TIME_T_FMT "' WHERE sub_user = '%q' AND sub_host = '%q';", contact, call_id, state, device, local_epoch_time_next(exp), user, host);
			if(csta_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
			}
			switch_safe_free(sql);
		}
		switch_event_create(&event_notify, SWITCH_EVENT_NOTIFY);
		switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "contact-uri", contact);
		switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "no-sub-state", "true");
		switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "to-uri", "1000@14");
		switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "from-uri", "1000@14");
		switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "call-id", call_id);
		switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "event-string", "as-feature-event");
		switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "content-type", "application/x-as-feature-event+xml");
		switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "user", user);
		switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "host", host);
		switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "profile", profile_name);
		switch_event_add_body(event_notify, "%s", switch_mprintf("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<DoNotDisturbEvent xmlns=\"http://www.ecma-international.org/standards/ecma-323/csta/ed3\">\n<device>%s</device>\n<doNotDisturbOn>%s</doNotDisturbOn>\n</DoNotDisturbEvent>",device,state));
		switch_event_fire(&event_notify);
	}
}

static void csta_subscribe_reinit(const char *user, const char *host, const char *contact, const char *call_id, const char *expires, const char *profile_name, const char *hostname)
{
	if (user && host && contact && call_id && expires && profile_name && hostname) {
		char *sql = NULL;
		char res[256];
		sql = switch_mprintf("SELECT COUNT(*) FROM dnd_subscriptions WHERE sub_user = '%q' AND sub_host = '%q';", user, host);
		csta_execute_sql2str(NULL, sql, res, sizeof(res));
		switch_safe_free(sql);
		if (atoi(res) == 0) {
			long exp = atoi(expires);
			sql = switch_mprintf("INSERT INTO dnd_subscriptions (sub_user, sub_host, contact, call_id, expires, profile_name, hostname) VALUES ('%q', '%q', '%q', '%q', '%" SWITCH_TIME_T_FMT "', '%q', '%q');", user, host, contact, call_id, local_epoch_time_next(exp), profile_name, hostname);
			csta_execute_sql(sql, NULL);
			switch_safe_free(sql);
		} else {
			long exp = atoi(expires);
			sql = switch_mprintf("UPDATE dnd_subscriptions SET contact = '%q', call_id = '%q', expires = '%" SWITCH_TIME_T_FMT "' WHERE sub_user = '%q' AND sub_host = '%q';", contact, call_id, local_epoch_time_next(exp), user, host);
			if(csta_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
			}
			switch_safe_free(sql);
		}
	}
}

static void csta_subscribe_init(const char *user, const char *host, const char *contact, const char *call_id, const char *expires, const char *profile_name, const char *hostname)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "TEST\n");
	if (user && host && contact && call_id && expires && profile_name && hostname) {
		char *sql = NULL;
		char res[256];
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "TEST\n");
		sql = switch_mprintf("SELECT COUNT(*) FROM dnd_subscriptions WHERE sub_user = '%q' AND sub_host = '%q';", user, host);
		csta_execute_sql2str(NULL, sql, res, sizeof(res));
		switch_safe_free(sql);
		if (atoi(res) == 0) {
			switch_event_t *event_notify;
			char *pl;
			char *content;
			switch_stream_handle_t stream = { 0 };
			long exp = atoi(expires);
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "TEST\n");
			sql = switch_mprintf("INSERT INTO dnd_subscriptions (sub_user, sub_host, contact, call_id, state, device, expires, profile_name, hostname) VALUES ('%q', '%q', '%q', '%q', 'false', 'test', '%" SWITCH_TIME_T_FMT "', '%q', '%q');", user, host, contact, call_id, local_epoch_time_next(exp), profile_name, hostname);
			csta_execute_sql(sql, NULL);
			switch_safe_free(sql);

			SWITCH_STANDARD_STREAM(stream);

			content = "\n<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<DoNotDisturbEvent xmlns=\"http://www.ecma-international.org/standards/ecma-323/csta/ed3\">\n<device>7659366</device>\n<doNotDisturbOn>true</doNotDisturbOn>\n</DoNotDisturbEvent>";

			stream.write_function(&stream, "--boundary1\nContent-Type: application/x-as-feature-event+xml\nContent-Length: %d\nContent-ID: <est1104@19.local>\n%s\n", strlen(content) + 1, content);

			content = "\n<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n<ForwardingEvent xmlns=\"http://www.ecma-international.org/standards/ecma-323/csta/ed3\">\n<device>7659366</device>\n<forwardingType>forwardNoAns</forwardingType>\n<forwardStatus>true</forwardStatus>\n<forwardTo>1001</forwardTo>\n<ringCount>4</ringCount>\n</ForwardingEvent>";

			stream.write_function(&stream, "--boundary1\nContent-Type: application/x-as-feature-event+xml\nContent-Length: %d\nContent-ID: <est104@19.local>\n%s", strlen(content) + 1, content);

			stream.write_function(&stream, "\n--boundary1--\n");

			pl = stream.data;

			switch_event_create(&event_notify, SWITCH_EVENT_NOTIFY);
			switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "contact-uri", contact);
			switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "to-uri", "1000@14");
			switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "from-uri", "1000@14");
			switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "call-id", call_id);
			switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "event-string", "as-feature-event");
			switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "content-type", "multipart/mixed; boundary=\"boundary1\"");
			switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "user", user);
			switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "host", host);
			switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "profile", profile_name);
			switch_event_add_header_string(event_notify, SWITCH_STACK_BOTTOM, "no-sub-state", "true");
			switch_event_add_body(event_notify, "%s", pl);
			switch_event_fire(&event_notify);
		} /*else {
			long exp = atoi(expires);
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "TEST\n");
			sql = switch_mprintf("UPDATE dnd_subscriptions SET contact = '%q', call_id = '%q', expires = '%" SWITCH_TIME_T_FMT "' WHERE sub_user = '%q' AND sub_host = '%q';", contact, call_id, local_epoch_time_next(exp), user, host);
			if(csta_execute_sql(sql, NULL) == SWITCH_STATUS_FALSE) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error executing query %s\n", sql);
			}
			switch_safe_free(sql);
		}*/
	}
}

static void csta_event_handler(switch_event_t *event)
{
	if (event) {
		const char *user = switch_event_get_header(event, "user");
		const char *host = switch_event_get_header(event, "host");
		const char *contact = switch_event_get_header(event, "contact");
		const char *call_id = switch_event_get_header(event, "call-id");
		const char *expires = switch_event_get_header(event, "expires");
		const char *cseq = switch_event_get_header(event, "cseq");
		const char *profile_name = switch_event_get_header(event, "profile_name");
		const char *hostname = switch_event_get_header(event, "hostname");
		const char *body = switch_event_get_body(event);
		if (user && host && contact && call_id && expires && profile_name && hostname) {
			if (!strcasecmp(cseq, "1")) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "initial subscribe\n");
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "user %s\n", user);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "host %s\n", host);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "contact %s\n", contact);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "call_id %s\n", call_id);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "expires %s\n", expires);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "cseq %s\n", cseq);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "profile_name %s\n", profile_name);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "hostname %s\n", hostname);
				csta_subscribe_init(user, host, contact, call_id, expires, profile_name, hostname);
			} else if (body) {
				switch_xml_t csta_action;
				char *pl = strdup(body);
				if ((csta_action = switch_xml_parse_str(pl, strlen(pl)))) {
					if (!strcasecmp(csta_action->name, "SetDoNotDisturb")) {
						switch_xml_t device, dndstatus;
						if ((device = switch_xml_child(csta_action, "device")) && (dndstatus = switch_xml_child(csta_action, "doNotDisturbOn"))) {
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "user %s\n", user);
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "host %s\n", host);
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "contact %s\n", contact);
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "call_id %s\n", call_id);
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "dndstatus->txt %s\n", dndstatus->txt);
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "device->txt %s\n", device->txt);
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "expires %s\n", expires);
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "cseq %s\n", cseq);
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "profile_name %s\n", profile_name);
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "hostname %s\n", hostname);
							switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "%s\n", body);
							csta_subscribe_dnd(user, host, contact, call_id, dndstatus->txt, device->txt, expires, profile_name, hostname);
						}
					} else if (!strcasecmp(csta_action->name, "SetForwarding")) {
					/*
						switch_xml_t device, fwdtype, fwdstatus, fwdto, ringcount;
						switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "csta-event", "ForwardingEvent");
						if((device = switch_xml_child(csta_action, "device"))) {
							switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "device", device->txt);
						}
						if((fwdtype = switch_xml_child(csta_action, "forwardingType"))) {
							switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "forwardingType", fwdtype->txt);
						}
						if((fwdstatus = switch_xml_child(csta_action, "activateForward"))) {
							switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "forwardStatus", fwdstatus->txt);
						}
						if((fwdto = switch_xml_child(csta_action, "forwardDN"))) {
							switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "forwardTo", fwdto->txt);
						}
						if((ringcount = switch_xml_child(csta_action, "ringCount"))) {
							switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "ringCount", ringcount->txt);
						}
					*/
					} else {
						//switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "re-subscribe\n");
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "user %s\n", user);
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "host %s\n", host);
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "contact %s\n", contact);
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "call_id %s\n", call_id);
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "expires %s\n", expires);
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "cseq %s\n", cseq);
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "profile_name %s\n", profile_name);
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "hostname %s\n", hostname);
					}
				}
			} else {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "re-subscribe\n");
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "user %s\n", user);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "host %s\n", host);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "contact %s\n", contact);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "call_id %s\n", call_id);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "expires %s\n", expires);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "cseq %s\n", cseq);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "profile_name %s\n", profile_name);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "hostname %s\n", hostname);
				csta_subscribe_reinit(user, host, contact, call_id, expires, profile_name, hostname);
			}
		}
	}
}

static switch_status_t load_config(void)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	switch_xml_t cfg, xml, settings, param;
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
				continue;
			}
			if (!strcasecmp(var, "dbname")) {
				globals.dbname = strdup(val);
			} else if (!strcasecmp(var, "odbc-dsn")) {
				globals.odbc_dsn = strdup(val);
			}
		}
	}

	if (!globals.dbname) {
		globals.dbname = strdup(CSTA_SQLITE_DB_NAME);
	}

	if (!(dbh = get_db_handle())) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Cannot open DB!\n");
		status = SWITCH_STATUS_TERM;
		goto end;
	}
	switch_cache_db_test_reactive(dbh, "SELECT COUNT(*) FROM dnd_subscriptions", "DROP TABLE dnd_subscriptions", create_dnd_subscriptions);
	switch_cache_db_release_db_handle(&dbh);

end:
	switch_mutex_unlock(globals.mutex);

	if (xml) {
		switch_xml_free(xml);
	}

	return status;
}


/*
#define CSTA_API_SYNTAX "csta <action> <user> <domain>,\n"\
"\tcsta call <user> <domain>\n"\
"\tcsta add <user> <domain> [page number]\n"\
"\tcsta del <user> <domain>\n"\
"\tcsta set <user> <domain> <param name> <param value>\n"\
"\tcsta get <user> <domain> <param name> <param value>\n"
SWITCH_STANDARD_API(csta_function)
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
		stream->write_function(stream, "-USAGE: \n%s\n", CSTA_API_SYNTAX);
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

	if (action && !strcasecmp(action, "call")) {
		if (switch_event_create_subclass(&event, SWITCH_EVENT_CUSTOM, CSTA_EVENT) == SWITCH_STATUS_SUCCESS) {
			stream->write_function(stream, "%s", "+OK\n");
			switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Account", account);
			switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Action", "NEW");
			switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "Status", "1");
			switch_event_fire(&event);
		}
	} else if (action && !strcasecmp(action, "load")) {
		page_profile_t *profile = NULL;
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
		page_profile_t *profile = NULL;
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
*/

/* Macro expands to: switch_status_t mod_csta_load(switch_loadable_module_interface_t **module_interface, switch_memory_pool_t *pool) */
SWITCH_MODULE_LOAD_FUNCTION(mod_csta_load)
{
	//switch_api_interface_t *api_interface;
	switch_status_t status;

	memset(&globals, 0, sizeof(globals));
	globals.pool = pool;
	//switch_core_hash_init(&globals.profile_hash, globals.pool);
	switch_mutex_init(&globals.mutex, SWITCH_MUTEX_NESTED, globals.pool);

	if ((status = load_config()) != SWITCH_STATUS_SUCCESS) {
		return status;
	}


	switch_mutex_lock(globals.mutex);
	if ((switch_event_bind_removable(modname, SWITCH_EVENT_CUSTOM, CSTA_SUBSCRIBE, csta_event_handler, NULL, &globals.csta_event_handler) != SWITCH_STATUS_SUCCESS)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Couldn't bind our message waiting handler!\n");
	}

	switch_mutex_unlock(globals.mutex);

	switch_mutex_lock(globals.mutex);
	globals.running = 1;
	switch_mutex_unlock(globals.mutex);

	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	//SWITCH_ADD_API(api_interface, "csta", "CSTA API", csta_function, "syntax");

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

/*
  Called when the system shuts down
  Macro expands to: switch_status_t mod_csta_shutdown() */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_csta_shutdown)
{
	//switch_hash_index_t *hi;
	//void *val = NULL;
	//const void *key;
	//switch_ssize_t keylen;
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

	/*
	switch_mutex_lock(globals.mutex);
	while ((hi = switch_hash_first(NULL, globals.profile_hash))) {
		switch_hash_this(hi, &key, &keylen, &val);
		profile = (page_profile_t *) val;

		switch_core_hash_delete(globals.profile_hash, profile->name);

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Waiting for write lock (profile %s)\n", profile->name);
		switch_thread_rwlock_wrlock(profile->rwlock);

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Destroying profile %s\n", profile->name);

		switch_core_destroy_memory_pool(&profile->pool);
		profile = NULL;
	}
	switch_mutex_unlock(globals.mutex);
	*/

	switch_mutex_lock(globals.mutex);
	switch_event_unbind(&globals.csta_event_handler);
	switch_safe_free(globals.odbc_dsn);
	switch_safe_free(globals.dbname);
	switch_mutex_unlock(globals.mutex);
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_RUNTIME_FUNCTION(mod_csta_runtime)
{
	// TODO We need more control on the result
	while(globals.running) {
		char *sql = switch_mprintf("DELETE FROM dnd_subscriptions WHERE expires < '%" SWITCH_TIME_T_FMT "'", local_epoch_time_now(NULL));
		csta_execute_sql(sql, NULL);
		switch_safe_free(sql);
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
