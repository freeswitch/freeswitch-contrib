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
 * Mathieu Rene <mathieu.rene@gmail.com>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * Anyone writing code for the original mod_limit
 *
 * mod_limit_hash.c -- Resource Limit Module
 *
 */

#include <switch.h>

#define LIMIT_SYNTAX "<realm> <id> <max>[/interval] [transfer_destination_number [transfer_destination_dialplan [transfer_destination_context]]]"

SWITCH_MODULE_LOAD_FUNCTION(mod_limit_hash_load);
SWITCH_MODULE_DEFINITION(mod_limit_hash, mod_limit_hash_load, NULL , NULL);


static struct {
	switch_memory_pool_t *pool;
	switch_mutex_t *mutex;
	switch_hash_t *hash;
} globals;


struct limit_hash_item  {
	uint32_t total_usage;
	uint32_t rate_usage;
	time_t last_check;
};
typedef struct limit_hash_item limit_hash_item_t;

static char *limit_def_xfer_exten = "limit_exceeded";

/* Callback procedure so we know when to decrement counters */
static switch_status_t state_handler(switch_core_session_t *session) 
{
	switch_channel_t *channel = switch_core_session_get_channel(session);
	switch_channel_state_t state = switch_channel_get_state(channel);
	switch_hash_t *channel_hash = switch_channel_get_private(channel, "limit_hash");
	
	/* The call is either hung up, or is going back into the dialplan, decrement appropriate couters */
	if (state == CS_HANGUP || state == CS_ROUTING) {	
		switch_hash_index_t *hi;
		switch_mutex_lock(globals.mutex);

		/* Loop through the channel's hashtable which contains mapping to all the limit_hash_item_t referenced by that channel */
		for(hi = switch_hash_first(NULL, channel_hash); hi; hi = switch_hash_next(hi))
		{
			void *val = NULL;
			const void *key;
			switch_ssize_t keylen;
			limit_hash_item_t *item = NULL;
			
			switch_hash_this(hi, &key, &keylen, &val);
			
			item = (limit_hash_item_t*)val;
			
			/* We keep the structure even though the count is 0 so we do not allocate too often */
			item->total_usage--;	
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Usage for %s is now %d\n", (const char*)key, item->total_usage);	
		}
		switch_core_event_hook_remove_state_change(session, state_handler);
		switch_mutex_unlock(globals.mutex);
	}
	
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_STANDARD_APP(limit_hash_function)
{
	int argc = 0;
	char *argv[6] = { 0 };
	char *mydata = NULL;
	char *realm = NULL;
	char *id = NULL;
	char *hashkey = NULL;
	char *xfer_exten = NULL;
	int max = 0;
	int interval = 0;
	char *szinterval = NULL;
	limit_hash_item_t *item = NULL;
	switch_channel_t *channel = switch_core_session_get_channel(session);
	time_t now = switch_timestamp(NULL);
	switch_hash_t *channel_hash = NULL;
	uint8_t increment = 1;
	uint8_t new_channel = 0;
	
	/* Parse application data  */
	if (!switch_strlen_zero(data)) {
		mydata = switch_core_session_strdup(session, data);
		argc = switch_separate_string(mydata, ' ', argv, (sizeof(argv) / sizeof(argv[0])));
	}
	
	if (argc < 3) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "USAGE: limit_hash %s\n", LIMIT_SYNTAX);
		return;
	}
	
	realm = argv[0];
	id = argv[1];
	if ((szinterval = strchr(argv[2], '/')))
	{
		*szinterval++ = '\0';
		interval = atoi(szinterval);
	}
	
	max = atoi(argv[2]);

	if (argc >= 4) {
		xfer_exten = argv[3];
	} else {
		xfer_exten = limit_def_xfer_exten;
	}

	if (max < 0) {
		max = 0;
	}
	
	hashkey = switch_core_session_sprintf(session, "%s_%s", realm, id);
	
	switch_mutex_lock(globals.mutex);
	/* Check if that realm+id has ever been checked */
	if (!(item = (limit_hash_item_t*)switch_core_hash_find(globals.hash, hashkey))) {
		/* No, create an empty structure and add it, then continue like as if it existed */
		item = (limit_hash_item_t*)switch_core_alloc(globals.pool, sizeof(limit_hash_item_t));
		memset(item, 0, sizeof(limit_hash_item_t));
		switch_core_hash_insert(globals.hash, hashkey, item);
	}
	
	/* Did we already run on this channel before? */
	if ((channel_hash = switch_channel_get_private(channel, "limit_hash")))
	{
		// Yes, but check if we did that realm+id
		if (!switch_core_hash_find(channel_hash, hashkey)) {
			// No, add it to our table so the state handler can take care of it 
			switch_core_hash_insert(channel_hash, hashkey, item);
		} else {
			/* Yes, dont touch total counter */
			increment = 0;
		}
	} else {
		/* This is the first limit check on this channel, create a hashtable, set our prviate data and add a state handler */
		new_channel = 1;
	}

	if (interval > 0) {
		if (item->last_check <= (now - interval)) {
			item->rate_usage = 1;
			item->last_check = now;
		} else {
			// Always increment rate when its checked as it doesnt depend on the channel
			item->rate_usage++;
			
			if (item->rate_usage > max) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Usage for %s exceeds maximum rate of %d/%ds, now at %d\n", hashkey, max, interval, item->rate_usage);
				switch_ivr_session_transfer(session, xfer_exten, argv[4], argv[5]);
				goto end;
			}
		}
	} else if (/* interval == 0 && */ item->total_usage + increment > max) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Usage for %s is already at max value (%d)\n", hashkey, item->total_usage);
		switch_ivr_session_transfer(session, xfer_exten, argv[4], argv[5]);
		goto end;
	}

	if (increment) {
		item->total_usage++;
		
		if (interval == 0) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Usage for %s is now %d/%d\n", hashkey, item->total_usage, max);	
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Usage for %s is now %d/%d for the last %d seconds\n", hashkey, item->rate_usage, max, interval);
		}
	}

	
	if (new_channel) {
		switch_core_hash_init(&channel_hash, switch_core_session_get_pool(session));
		switch_core_hash_insert(channel_hash, hashkey, item);
		switch_channel_set_private(channel, "limit_hash", channel_hash);
		switch_core_event_hook_add_state_change(session, state_handler);
	}
	
end:	
	switch_mutex_unlock(globals.mutex);
}


SWITCH_MODULE_LOAD_FUNCTION(mod_limit_hash_load)
{
	switch_application_interface_t *app_interface;

	memset(&globals, 0, sizeof(&globals));
	
	globals.pool = pool;

	switch_mutex_init(&globals.mutex, SWITCH_MUTEX_NESTED, globals.pool);
	
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);
	
	switch_core_hash_init(&globals.hash, pool);
	
	SWITCH_ADD_APP(app_interface, "limit_hash", "Limit (hashtable backend)", "Limits various resources usage", limit_hash_function, LIMIT_SYNTAX, SAF_SUPPORT_NOMEDIA);
		
	return SWITCH_STATUS_NOUNLOAD; /* Potential Kaboom(tm) if we have active calls */
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
