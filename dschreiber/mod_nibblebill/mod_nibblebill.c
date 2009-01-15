/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005/2006, Anthony Minessale II <anthmct@yahoo.com>
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
 * Anthony Minessale II <anthmct@yahoo.com>
 *
 * The Initial Developer of this module is
 * Darren Schreiber <d@d-man.org>
 *
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Darren Schreiber <d@d-man.org>
 *
 * mod_nibblebill.c - Nibble Billing
 * Set the session heartbeat on a call that has a billrate and deduct currency from
 * a database table each "tick" of the billing until funds are depleted.
 * Terminate the call upon depletion.
 *
 *
 * TODO: Create an override flag so you can force a start-time for when billing should count *from*
 * TODO: Fix what happens when the DB is not available
 * TODO: Fix what happens when the DB queries fail (right now, all are acting like success)
 * TODO: Make the actions function for when funds are depleted
 * TODO: Add app and CLI commands (pause, resume, reset, flush, deduct_funds, add_funds)
 * TODO: Add buffering abilities
 * TODO: Make error handling for database, such that when the database is down (or not installed) we just log to a text file
 * TODO: Make a sample .XML config file!
 * FUTURE: Possibly make the hooks not tied per-channel, and instead just do this as a supervision style application with one thread that watches all calls
 */

#include <switch.h>

#ifdef SWITCH_HAVE_ODBC
#include <switch_odbc.h>
#endif

/* Defaults */
/* static char SQL_LOOKUP[] = "SELECT cash FROM accounts WHERE id=\"%s\""; */
static char SQL_SAVE[] = "UPDATE accounts SET cash=cash-%f WHERE id=\"%s\"";

typedef struct
{
	switch_time_t lastts;	/* Last time we did any billing */
	float total;	/* Total amount billed so far */
} nibble_data_t;


typedef struct nibblebill_results
{
	float	funds;

	float	percall_max; /* Overrides global on a per-user level */
	float	lowbal_amt;  /*  ditto */
} nibblebill_results_t;


/* Keep track of our config, event hooks and database connection variables, for this module only */
static struct
{
	/* Event hooks */
	switch_event_node_t *node;

	/* Global mutex (don't touch a session when it's already being touched) */
	switch_mutex_t *mutex;

	/* Global billing config options */
	float	percall_max_amt;	/* Per-call billing limit (safety check, for fraud) */
	char	*percall_action;	/* Exceeded length of per-call action */
	float	lowbal_amt;		/* When we warn them they are near depletion */
	char	*lowbal_action;		/* Low balance action */
	float	nobal_amt;		/* Minimum amount that must remain in the account */
	char	*nobal_action;		/* Drop action */

	/* Database settings */
	char *db_username;
	char *db_password;
	char *db_dsn;
	char *lookup_query;
	char *save_query;
#ifdef SWITCH_HAVE_ODBC
	switch_odbc_handle_t *master_odbc;
#else   
	void *padding1;  /* Keep structures same size */
#endif
} globals;

/**************************
* Setup FreeSWITCH Macros *
**************************/
/* Define the module's load function */
SWITCH_MODULE_LOAD_FUNCTION(mod_nibblebill_load);

/* Define the module's shutdown function */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_nibblebill_shutdown);

/* Define the module's name, load function, shutdown function and runtime function */
SWITCH_MODULE_DEFINITION(mod_nibblebill, mod_nibblebill_load, mod_nibblebill_shutdown, NULL);

/* String setting functions */
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_db_username, globals.db_username);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_db_password, globals.db_password);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_db_dsn, globals.db_dsn);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_lookup_query, globals.lookup_query);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_save_query, globals.save_query);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_percall_action, globals.percall_action);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_lowbal_action, globals.lowbal_action);
SWITCH_DECLARE_GLOBAL_STRING_FUNC(set_global_nobal_action, globals.nobal_action);

static int nibblebill_callback(void *pArg, int argc, char **argv, char **columnNames)
{
	nibblebill_results_t *cbt = (nibblebill_results_t *) pArg;

	cbt->funds = atof(argv[0]);

	return 0;
}

static switch_status_t load_config(void)
{
	char *cf = "nibblebill.conf";
	switch_xml_t cfg, xml = NULL, param, settings;
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	
	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
		status = SWITCH_STATUS_SUCCESS;  /* We don't fail because we can still write to a text file or buffer */
		goto done;
	}
	
	if ((settings = switch_xml_child(cfg, "settings"))) {
		for (param = switch_xml_child(settings, "param"); param; param = param->next) {
			char *var = (char *) switch_xml_attr_soft(param, "name");
			char *val = (char *) switch_xml_attr_soft(param, "value");

			if (!strcasecmp(var, "db-username")) {
				set_global_db_username(val);
			} else if (!strcasecmp(var, "db-password")) {
				set_global_db_password(val);
			} else if (!strcasecmp(var, "db-dsn")) {
				set_global_db_dsn(val);
			} else if (!strcasecmp(var, "lookup_query")) {
				set_global_lookup_query(val);
			} else if (!strcasecmp(var, "save_query")) {
				set_global_save_query(val);
			} else if (!strcasecmp(var, "percall_action")) {
				set_global_percall_action(val);
			} else if (!strcasecmp(var, "lowbal_action")) {
				set_global_lowbal_action(val);
			} else if (!strcasecmp(var, "nobal_action")) {
				set_global_nobal_action(val);
			}
		}
	}
	
done:
	if (switch_strlen_zero(globals.db_username)) {
		set_global_db_username("phonebooth");
	}
	if (switch_strlen_zero(globals.db_password)) {
		set_global_db_password("dev");
	}
	if (switch_strlen_zero(globals.db_dsn)) {
	set_global_db_dsn("phonebooth");
	}
	if (switch_strlen_zero(globals.percall_action)) {
		set_global_percall_action("hangup");
	}
	if (switch_strlen_zero(globals.lowbal_action)) {
		set_global_lowbal_action("play ding");
	}
	if (switch_strlen_zero(globals.nobal_action)) {
		set_global_nobal_action("hangup");
	}
	/* Need to add defaults for the integers! */

#ifdef SWITCH_HAVE_ODBC
	if (globals.db_dsn) {
		if (!(globals.master_odbc = switch_odbc_handle_new(globals.db_dsn, globals.db_username, globals.db_password))) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Cannot create handle to ODBC Database!\n");
			status = SWITCH_STATUS_FALSE;
			goto reallydone;
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Opened ODBC Database handle!\n");
		}

		if (switch_odbc_handle_connect(globals.master_odbc) != SWITCH_ODBC_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Cannot connect to ODBC driver/database %s (user: %s / pass %s)!\n", globals.db_dsn, globals.db_username, globals.db_password);
			status = SWITCH_STATUS_FALSE;
			goto reallydone;
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Opened ODBC Database!\n");
		}
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Connected ODBC DSN: %s\n", globals.db_dsn);
	} else {
#endif
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "ODBC does not appear to be installed in the core. You need to run ./configure --enable-core-odbc-support\n");
#ifdef SWITCH_HAVE_ODBC
	}
#endif

#ifdef SWITCH_HAVE_ODBC
reallydone:
#endif

	if (xml) {
		switch_xml_free(xml);
	}
	return status;
}

void debug_event_handler(switch_event_t *event)
{
	switch_event_header_t *event_header = NULL;

	 if (!event) {
		return;
	}

	/* Print out all event headers, for fun */
	if (event->headers) {
		for (event_header = event->headers; event_header; event_header = event_header->next) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Header info: %s => %s\n", event_header->name, event_header->value);
		}
	}
}


static switch_status_t sched_billing(switch_core_session_t *session)
{

	switch_core_session_enable_heartbeat(session, 60);

	return SWITCH_STATUS_SUCCESS;

}

/* This is where we actually charge the guy 
  This can be called anytime a call is in progress or at the end of a call before the session is destroyed */
static switch_status_t do_billing(switch_core_session_t *session)
{
	/* FS vars we will use */
	switch_channel_t *channel;
	switch_caller_profile_t *profile;

	/* Local vars */
	nibble_data_t *nibble_data;
	switch_time_t ts = switch_timestamp_now();
	float billamount;
	char date[80] = "";
	char *uuid;
	switch_size_t retsize;
	switch_time_exp_t tm;

	if (!session) {
		/* wtf? Why are we here? */
		return SWITCH_STATUS_GENERR;
	}

	uuid = switch_core_session_get_uuid(session);

	/* Get channel var */
	channel = switch_core_session_get_channel(session);
	if (!channel) {
		return SWITCH_STATUS_GENERR;
	}

	/* Variables kept in FS but relevant only to this module */
	const char *billrate = switch_channel_get_variable(channel, "nibble_rate");
	const char *billaccount = switch_channel_get_variable(channel, "nibble_account");

	/* Return if there's no billing information on this session */
	if (!billrate || !billaccount) {
		return SWITCH_STATUS_GENERR;
	}

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Attempting to bill at $%s per minute to account %s\n", billrate, billaccount);

	/* Get caller profile info from channel */
	profile = switch_channel_get_caller_profile(channel);

	if (!profile) {
		/* No caller profile (why would this happen?) */
		return SWITCH_STATUS_GENERR;
	}

	if (profile->times->answered < 1) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Not billing %s - call is not in answered state\n", billaccount);
		return SWITCH_STATUS_SUCCESS;
	}

	/* Lock this session's data for this module while we tinker with it */
	if (globals.mutex) {
		switch_mutex_lock(globals.mutex);
	}

	/* Get our nibble data var. This will be NULL if it's our first call here for this session */
	nibble_data = (nibble_data_t *) switch_channel_get_private(channel, "_nibble_data_");

	/* Have we done any billing on this channel yet? If no, set up vars for doing so */
	if (!nibble_data) {
		nibble_data = switch_core_session_alloc(session, sizeof(*nibble_data));
		if (!nibble_data) {
			switch_assert(nibble_data);
		}
		memset(nibble_data, 0, sizeof(*nibble_data));

		/* Setup new billing data (based on call answer time, in case this module started late with active calls) */
		nibble_data->lastts = profile->times->answered;		/* Set the initial answer time to match when the call was really answered */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Beginning new billing on %s\n", uuid);
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Last successful billing time was %s\n", date);
	}

	switch_time_exp_lt(&tm, nibble_data->lastts);
	switch_strftime_nocheck(date, &retsize, sizeof(date), "%Y-%m-%d %T", &tm);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "%d seconds passed since last bill time of %s\n", (int) ((ts - nibble_data->lastts) / 1000000), date);


	if ((ts - nibble_data->lastts) > 0) {
		/* Convert billrate into microseconds and multiply by # of microseconds that have passed */
		billamount = (atof(billrate) / 1000000 / 60) * ((ts - nibble_data->lastts));

		/* if ODBC call fails, we should return BEFORE updating the timestamp of last success! */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Billing $%f to %s (Call: %s / %f so far)\n", billamount, billaccount, uuid, nibble_data->total);

		/* DO ODBC BILLING HERE! */
#ifdef SWITCH_HAVE_ODBC
		char sql[1024] = "";
		nibblebill_results_t pdata;

		memset(&pdata, 0, sizeof(pdata));
		if (!globals.save_query){
			snprintf(sql, 1024, SQL_SAVE, billamount, billaccount);
		} else {
			snprintf(sql, 1024, globals.save_query, billamount, billaccount);
		}
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG,  "Doing static update query\n[%s]\n", sql);

		if (!(switch_odbc_handle_callback_exec(globals.master_odbc, sql, nibblebill_callback, &pdata) == SWITCH_ODBC_SUCCESS)){
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "DB Error while updating cash!\n");

			/* TODO: If this is a hangup event, we should store this billing in a text file while the DB is unavailable */
		}
#endif

		/* Increment total cost */
		nibble_data->total += billamount;
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Just tried to bill %s negative minutes! wtf?\n", uuid);
	}

	/* Update the last time we billed */
	nibble_data->lastts = ts;

	/* Save everything back */
	switch_channel_set_private(channel, "_nibble_data_", nibble_data);

	/* Release the session lock */
	switch_core_session_rwunlock(session);

	/* Done changing - release lock */
	if (globals.mutex) {
		switch_mutex_unlock(globals.mutex);
	}

	return SWITCH_STATUS_SUCCESS;
}

/* You can turn on session heartbeat on a channel to have us check billing more often */
static void event_handler(switch_event_t *event)
{
	if (!event){
		/* WTF? We should never get here - it means an event came in without the event info */
		return;
	}

	/* FS vars we'll need */
	switch_core_session_t *session;
	char *uuid = switch_event_get_header(event, "Unique-ID");

	/* Make sure everything is sane */
	if (!uuid){
		/* Donde esta channel? */
		return;
	}

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Received request via %s!\n", switch_event_name (event->event_id));

	/* Display debugging info */
	if (switch_event_get_header(event, "nibble_debug")) {
		debug_event_handler(event);
	}

	/* Get session var */
	session = switch_core_session_locate(uuid);
	if (!session) {
		return;
	}

	/* Go bill */
	do_billing(session);

	switch_core_session_rwunlock(session);
}

SWITCH_STANDARD_APP(nibblebill_app_function)
{
	switch_channel_t *channel = switch_core_session_get_channel(session);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "We made it!\n");

	if (!channel) {
		return;
	}
	
/*	if (!(mydata = switch_core_session_strdup(session, data))) {
		return;
	}
	
	if ((argc = switch_separate_string(mydata, ' ', argv, (sizeof(argv) / sizeof(argv[0]))))) {
		destnum = argv[0];
		do_billing(session);
	}*/
}

/* We get here from the API only (theoretically) */
SWITCH_STANDARD_API(nibblebill_function)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "We made it!\n");
	
/* Check if session has variable "billrate" set. If it does, activate the heartbeat variable
 switch_core_session_enable_heartbeat(switch_core_session_t *session, uint32_t seconds)
 switch_core_session_sched_heartbeat(switch_core_session_t *session, uint32_t seconds)*/


	return SWITCH_STATUS_SUCCESS;

}

switch_state_handler_table_t nibble_state_handler =
{
	/* on_init */   	NULL,
	/* on_routing */   	NULL,	/* Need to add a check here for anything in their account before routing */
	/* on_execute */   	sched_billing,  /* Turn on heartbeat for this session */
	/* on_hangup */   	do_billing, /* On hangup - most important place to go bill */
	/* on_exch_media */   	NULL,
	/* on_soft_exec */   	NULL,
	/* on_consume_med */   	NULL,
	/* on_hibernate */   	NULL,
	/* on_reset */   	NULL
};

SWITCH_MODULE_LOAD_FUNCTION(mod_nibblebill_load)
{
	switch_api_interface_t *api_interface;
	switch_application_interface_t *app_interface;

	/* Set every byte in this structure to 0 */	
	memset(&globals, 0, sizeof(globals));
	load_config();
	
	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);
	SWITCH_ADD_API(api_interface, "check", "Check the balance of an account", nibblebill_function, "");
	SWITCH_ADD_APP(app_interface, "check", "Check the balance on an account", "Perform an account check for fund balance", nibblebill_app_function, "<number>", SAF_SUPPORT_NOMEDIA);

	/* register state handlers for billing */
	switch_core_add_state_handler(&nibble_state_handler);

	/* bind to heartbeat events */
	if (switch_event_bind_removable(modname, SWITCH_EVENT_SESSION_HEARTBEAT, SWITCH_EVENT_SUBCLASS_ANY, event_handler, NULL, &globals.node) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind event to monitor for session heartbeats!\n");
		return SWITCH_STATUS_GENERR;
	}

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_nibblebill_shutdown)
{	
	switch_event_unbind(&globals.node);
	switch_core_remove_state_handler(&nibble_state_handler);

#ifdef SWITCH_HAVE_ODBC
	switch_odbc_handle_disconnect(globals.master_odbc);
#endif

	return SWITCH_STATUS_UNLOAD;
}

/* For Emacs:
 * Local Variables:
 * mode:c
 * indent-tabs-mode:t
 * tab-width:4
 * c-basic-offset:4
 * End:
 * For VIM:
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 expandtab:
 */
