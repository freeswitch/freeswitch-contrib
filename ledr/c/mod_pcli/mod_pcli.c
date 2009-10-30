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
 * mod_pcli.c -- PacketCable Lawful Intercept
 *
 */
#include <switch.h>

SWITCH_MODULE_LOAD_FUNCTION(mod_pcli_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_pcli_shutdown);
SWITCH_MODULE_DEFINITION(mod_pcli, mod_pcli_load, mod_pcli_shutdown, NULL);

SWITCH_STANDARD_APP(pcli_start_function);

typedef enum {
	PCLI_MEDIA_DIRECTION_UNKNOWN,     /* 0 */
	PCLI_MEDIA_DIRECTION_FROM_TARGET, /* 1 */
	PCLI_MEDIA_DIRECTION_TO_TARGET,   /* 2 */
	PCLI_MEDIA_DIRECTION_MONO_MODE    /* 3 */
} pcli_media_direction_t;

static struct {
	switch_memory_pool_t *pool;
	int switch_id;
	char *local_addr;
	int local_port;
	char *remote_addr;
	int remote_port;
	switch_socket_t *socket;
} pcli_globals;

// temporary declaration here.. remove after testing is done !!!!
static switch_status_t gen_pcli_header(uint32_t *pcli_header, pcli_media_direction_t media_direction, uint16_t call_id, uint8_t switch_id, uint16_t ini_id);

typedef struct {
	switch_core_session_t *session;
	char *ini_id;
} pcli_session_helper_t;

/* config item validations */
static switch_xml_config_int_options_t config_opt_valid_switch_id = { SWITCH_TRUE, 0, SWITCH_TRUE, 15 };
static switch_xml_config_string_options_t config_opt_valid_addr = { NULL, 0, ".+" };
static switch_xml_config_int_options_t config_opt_valid_port = { SWITCH_TRUE, 0, SWITCH_TRUE, 65535 };

/* config items */
static switch_xml_config_item_t instructions[] = {
	SWITCH_CONFIG_ITEM("switch_id", SWITCH_CONFIG_INT, CONFIG_RELOADABLE, &pcli_globals.switch_id,
		(void*)0, &config_opt_valid_switch_id, NULL, NULL),
	SWITCH_CONFIG_ITEM("local_addr", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &pcli_globals.local_addr,
		"localhost", &config_opt_valid_addr, NULL, NULL),
	SWITCH_CONFIG_ITEM("local_port", SWITCH_CONFIG_INT, CONFIG_RELOADABLE, &pcli_globals.local_port,
		(void*)10741, &config_opt_valid_port, NULL, NULL),
	SWITCH_CONFIG_ITEM("remote_addr", SWITCH_CONFIG_STRING, CONFIG_RELOADABLE, &pcli_globals.remote_addr,
		"localhost", &config_opt_valid_addr, NULL, NULL),
	SWITCH_CONFIG_ITEM("remote_port", SWITCH_CONFIG_INT, CONFIG_RELOADABLE, &pcli_globals.remote_port,
		(void*)10741, &config_opt_valid_port, NULL, NULL),
	SWITCH_CONFIG_ITEM_END()
};

/* called at startup and reload of the module */
static switch_status_t do_config(switch_bool_t reload)
{
	switch_socket_t *socket = NULL;
	switch_sockaddr_t *local_sockaddr;
	switch_sockaddr_t *remote_sockaddr;

	if (switch_xml_config_parse_module_settings("pcli.conf", reload, instructions) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Could not open pcli.conf\n");
		return SWITCH_STATUS_FALSE;
	}

	/* for now, only init port on startup, fix reload later !! */
	if (!reload) {

		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Trying to create a udp socket !!\n");

		if (switch_sockaddr_info_get(&local_sockaddr, pcli_globals.local_addr, SWITCH_UNSPEC, pcli_globals.local_port, 0, pcli_globals.pool) != SWITCH_STATUS_SUCCESS || !local_sockaddr) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Local Address Error!\n");
			return SWITCH_STATUS_FALSE;
		}

		if (switch_socket_create(&socket, switch_sockaddr_get_family(local_sockaddr), SOCK_DGRAM, 0, pcli_globals.pool) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Socket Error!\n");
			return SWITCH_STATUS_FALSE;
		}

		if (switch_socket_opt_set(socket, SWITCH_SO_REUSEADDR, 1) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Socket Error!\n");
			return SWITCH_STATUS_FALSE;
		}

		if (switch_socket_bind(socket, local_sockaddr) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Bind Error!\n");
			return SWITCH_STATUS_FALSE;
		}

		pcli_globals.socket = socket;

		/* TEST SENDING A PACKET */
		if (switch_sockaddr_info_get(&remote_sockaddr, pcli_globals.remote_addr, SWITCH_UNSPEC, pcli_globals.remote_port, 0, pcli_globals.pool) != SWITCH_STATUS_SUCCESS || !remote_sockaddr) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Remote Address Error!\n");
			return SWITCH_STATUS_FALSE;
		}

		/* create a pcli header */
		uint32_t pcli_header;
		pcli_media_direction_t media_direction = PCLI_MEDIA_DIRECTION_FROM_TARGET;
		uint16_t call_id = 123;
		uint8_t switch_id = 12;
		uint16_t ini_id = 10741;
		gen_pcli_header(&pcli_header, media_direction, call_id, switch_id, ini_id);

		uint32_t network_byte_order_pcli_header;
		network_byte_order_pcli_header = htonl(pcli_header);

		/* create a pcli body */
		char *pcli_body = "ABCD";

		/* create a full packet */
		char *packet;
		packet = malloc(sizeof(network_byte_order_pcli_header) + sizeof(pcli_body));
		memcpy(packet, &network_byte_order_pcli_header, sizeof(network_byte_order_pcli_header));
		memcpy(packet + sizeof(network_byte_order_pcli_header), pcli_body, sizeof(pcli_body));

		size_t packetsize;
		packetsize = sizeof(packet);

		switch_socket_sendto(pcli_globals.socket, remote_sockaddr, 0, packet, &packetsize);

	}

	return SWITCH_STATUS_SUCCESS;
}

/* called when SWITCH_EVENT_RELOADXML is sent to this module */
static void reload_event_handler(switch_event_t *event)
{
	do_config(SWITCH_TRUE);
}

/* generate a PacketCable Lawful Intercept header, that can be prepended to an RTP packet */
static switch_status_t gen_pcli_header(uint32_t *pcli_header, pcli_media_direction_t media_direction, uint16_t call_id, uint8_t switch_id, uint16_t ini_id)
{
	*pcli_header = 0;

	/* some sanity checks */

	if (media_direction > 3) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Invalid media direction, may only be 0-3\n");
		return SWITCH_STATUS_FALSE;
	}

	if (call_id > 1023) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Invalid call_id, may only be 0-1023\n");
		return SWITCH_STATUS_FALSE;
	}

	if (switch_id > 15) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Invalid switch_id, may only be 0-15\n");
		return SWITCH_STATUS_FALSE;
	}

	/* ini_id needs no check as it may use the full uint16_t */

	*pcli_header |= (media_direction);
	*pcli_header |= (call_id << 2);
	*pcli_header |= (switch_id << 12);
	*pcli_header |= (ini_id << 16);

	return SWITCH_STATUS_SUCCESS;
}

static switch_bool_t pcli_callback(switch_media_bug_t *bug, void *user_data, switch_abc_type_t type)
{
//	pcli_session_helper_t *pcli_session_helper = (pcli_session_helper_t *) user_data;

	switch (type) {
	case SWITCH_ABC_TYPE_INIT:
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "--> INIT\n");
		break;

	case SWITCH_ABC_TYPE_READ_PING:
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "--> PING\n");
		break;

	case SWITCH_ABC_TYPE_CLOSE:
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "--> CLOSE\n");
		break;

	case SWITCH_ABC_TYPE_READ:
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "--> READ\n");
		break;

	case SWITCH_ABC_TYPE_WRITE:
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "--> WRITE\n");
		break;

	case SWITCH_ABC_TYPE_READ_REPLACE:
		{
			switch_frame_t *rframe = switch_core_media_bug_get_read_replace_frame(bug);
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "--> READ_REPLACE seq[%u] ssrc[%lu] packetlen[%lu] rate [%lu] samples [%lu]\n",
				(unsigned)rframe->seq, (unsigned long)rframe->ssrc, (unsigned long)rframe->packetlen, (unsigned long)rframe->rate, (unsigned long)rframe->samples);
switch_size_t packetlength = sizeof(rframe->packet);
switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, ">>> [%lu]\n", (unsigned long)packetlength);

		}
		break;

	case SWITCH_ABC_TYPE_WRITE_REPLACE:
		{
			switch_frame_t *rframe = switch_core_media_bug_get_write_replace_frame(bug);
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "--> WRITE_REPLACE seq[%u] ssrc[%lu] packetlen[%lu] rate [%lu] samples[%lu]\n",
				(unsigned)rframe->seq, (unsigned long)rframe->ssrc, (unsigned long)rframe->packetlen, (unsigned long)rframe->rate, (unsigned long)rframe->samples);
switch_size_t packetlength = sizeof(rframe->packet);
switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, ">>> [%lu]\n", (unsigned long)packetlength);

		}
		break;
	}

	return SWITCH_TRUE;
}

/* necessary for hooking to SWITCH_EVENT_RELOADXML */
static switch_event_node_t *NODE = NULL;

SWITCH_STANDARD_APP(pcli_start_function)
{
	switch_media_bug_t *bug;
	switch_status_t status;
	switch_channel_t *channel;
	pcli_session_helper_t *pcli_session_helper;
	const char *p;

	if (session == NULL)
		return;

	channel = switch_core_session_get_channel(session);

	/* Is this channel already set? */
	bug = (switch_media_bug_t *) switch_channel_get_private(channel, "_pcli_");
	/* If yes */
	if (bug != NULL) {
		/* If we have a stop remove audio bug */
		if (strcasecmp(data, "stop") == 0) {
			switch_channel_set_private(channel, "_pcli_", NULL);
			switch_core_media_bug_remove(session, &bug);
			return;
		}

		/* We have already started */
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_WARNING, "Cannot run 2 at once on the same channel!\n");

		return;
	}

	/* create a new pcli_session_helper */
	pcli_session_helper = (pcli_session_helper_t *) switch_core_session_alloc(session, sizeof(pcli_session_helper_t));

	/* make session available from pcli_session_helper */
	pcli_session_helper->session = session;

	/* get LI_ID and store in pcli_session_helper */
	if ((p = switch_channel_get_variable(channel, "LI_ID")) && switch_true(p)) {
		pcli_session_helper->ini_id = switch_core_session_strdup(session, p);
	} else {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "No LI_ID was set!\n");
		return;
	}

	/* add the bug */
	//status = switch_core_media_bug_add(session, pcli_callback, pcli_session_helper, 0, SMBF_BOTH, &bug);
	//status = switch_core_media_bug_add(session, pcli_callback, pcli_session_helper, 0, SMBF_READ_STREAM | SMBF_WRITE_STREAM, &bug);
	status = switch_core_media_bug_add(session, pcli_callback, pcli_session_helper, 0, SMBF_READ_REPLACE | SMBF_WRITE_REPLACE, &bug);
	//status = switch_core_media_bug_add(session, pcli_callback, pcli_session_helper, 0, SMBF_READ_REPLACE, &bug);

	if (status != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "Failure hooking to stream\n");
		return;
	}

	switch_channel_set_private(channel, "_pcli_", bug);
}

SWITCH_MODULE_LOAD_FUNCTION(mod_pcli_load)
{
	switch_application_interface_t *app_interface;

	/* initialize pcli_globals struct */
	memset(&pcli_globals, 0, sizeof(pcli_globals));

	/* make the pool available from pcli_globals */
	pcli_globals.pool = pool;

	/* do config */
	do_config(SWITCH_FALSE);

	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	/* make the pcli application available to the system */
	SWITCH_ADD_APP(app_interface, "pcli", "PacketCable Lawful Intercept", "PacketCable Lawful Intercept", pcli_start_function, "<start>", SAF_NONE);

	/* subscribe to reloadxml event, and hook it to reload_event_handler */
	if ((switch_event_bind_removable(modname, SWITCH_EVENT_RELOADXML, NULL, reload_event_handler, NULL, &NODE) != SWITCH_STATUS_SUCCESS)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind event!\n");
		return SWITCH_STATUS_TERM;
	}

	/* say it */
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Lawful Intercept enabled\n");

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}


SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_pcli_shutdown)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "PacketCable Lawful Intercept disabled\n");

/* TODO DESTROY THE SOCKET HERE ! */

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
