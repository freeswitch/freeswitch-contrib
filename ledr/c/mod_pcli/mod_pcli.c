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

#define PCLI_HEADER_LEN 4
#define IP_HEADER_LEN 20
#define UDP_HEADER_LEN 8
#define RTP_HEADER_LEN 12

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
	char *remote_addr;
	int local_port;
	int remote_port;
	switch_socket_t *socket;
	switch_sockaddr_t *local_sockaddr;
	switch_sockaddr_t *remote_sockaddr;
} pcli_globals;

typedef struct {
	switch_core_session_t *session;
	uint16_t ini_id;
	uint16_t instance_id;
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

	if (!reload) {

		if (switch_sockaddr_info_get(&local_sockaddr, pcli_globals.local_addr, SWITCH_UNSPEC,
				pcli_globals.local_port, 0, pcli_globals.pool) != SWITCH_STATUS_SUCCESS || !local_sockaddr) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Local Address Error!\n");
			return SWITCH_STATUS_FALSE;
		}

		if (switch_sockaddr_info_get(&remote_sockaddr, pcli_globals.remote_addr, SWITCH_UNSPEC,
				pcli_globals.remote_port, 0, pcli_globals.pool) != SWITCH_STATUS_SUCCESS || !remote_sockaddr) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Remote Address Error!\n");
			return SWITCH_STATUS_FALSE;
		}

		if (switch_socket_create(&socket, switch_sockaddr_get_family(local_sockaddr), SOCK_DGRAM,
				0, pcli_globals.pool) != SWITCH_STATUS_SUCCESS) {
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
		pcli_globals.local_sockaddr = local_sockaddr;
		pcli_globals.remote_sockaddr = remote_sockaddr;

	}

	return SWITCH_STATUS_SUCCESS;
}

/* called when SWITCH_EVENT_RELOADXML is sent to this module */
static void reload_event_handler(switch_event_t *event)
{
	do_config(SWITCH_TRUE);
}

/* generate a PacketCable Lawful Intercept header, that can be prepended to an RTP packet */
static switch_status_t gen_pcli_header(uint8_t *pcli_header, pcli_media_direction_t media_direction, uint16_t instance_id, uint8_t switch_id, uint16_t ini_id)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "In gen_pcli_header - media_direction[%u] instance_id[%u] switch_id[%u] ini_id[%u]\n",
		media_direction, instance_id, switch_id, ini_id);

	/* some sanity checks */

	if (media_direction > 3) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Invalid media direction, may only be 0-3\n");
		return SWITCH_STATUS_FALSE;
	}

	if (instance_id > 1023) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Invalid instance_id, may only be 0-1023\n");
		return SWITCH_STATUS_FALSE;
	}

	if (switch_id > 15) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Invalid switch_id, may only be 0-15\n");
		return SWITCH_STATUS_FALSE;
	}

	/* ini_id needs no check as it may use the full uint16_t */

	uint32_t pcli_header_i = 0;
	pcli_header_i |= (media_direction);
	pcli_header_i |= (instance_id << 2);
	pcli_header_i |= (switch_id << 12);
	pcli_header_i |= (ini_id << 16);

	uint32_t pcli_header_i_n = htonl(pcli_header_i);

	memcpy(pcli_header, &pcli_header_i_n, sizeof(pcli_header_i_n));

	return SWITCH_STATUS_SUCCESS;
}

/* generate an ip header */
static switch_status_t gen_ip_header(uint8_t *ip_header, uint16_t payload_size_i)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "In gen_ip_header - payload_size_i[%u]\n", payload_size_i);

	memset(ip_header, 0, (size_t)IP_HEADER_LEN); /* zero */

	ip_header[0] |= (4 << 4); /* set version 4 */
	ip_header[0] |= 5; /* set header length 5 */

	uint16_t payload_size_i_n = htons(payload_size_i);
	memcpy(&ip_header[2], &payload_size_i_n, sizeof(payload_size_i_n));

	ip_header[9] |= 17; /* set protocol to UDP */

	return SWITCH_STATUS_SUCCESS;
}

/* generate a udp header */
static switch_status_t gen_udp_header(uint8_t *udp_header, uint16_t payload_size_i)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "In gen_udp_header - payload_size_i[%u]\n", payload_size_i);

	memset(udp_header, 0, (size_t)UDP_HEADER_LEN); /* zero */

	uint16_t payload_size_i_n = htons(payload_size_i); /* set payload size */
	memcpy(&udp_header[4], &payload_size_i_n, sizeof(payload_size_i_n));

	return SWITCH_STATUS_SUCCESS;
}

/* generate an rtp header */
static switch_status_t gen_rtp_header(uint8_t *rtp_header, uint16_t seq_i, uint32_t timestamp_i, uint32_t ssrc_i)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "In gen_rtp_header - seq_i[%u] timestamp_i[%lu] ssrc_i[%lu]\n",
		seq_i, (unsigned long)timestamp_i, (unsigned long)ssrc_i);

	memset(rtp_header, 0, (size_t)RTP_HEADER_LEN); /* zero */

	rtp_header[0] |= 2; /* set version 2 */
	rtp_header[1] |= (10 << 1); /* set payload type L16 (10) */
	
	uint16_t seq_i_n = htons(seq_i); /* set seq */
	memcpy(&rtp_header[2], &seq_i_n, sizeof(seq_i_n));

	uint32_t timestamp_i_n = htonl(timestamp_i); /* set timestamp */
	memcpy(&rtp_header[4], &timestamp_i_n, sizeof(timestamp_i_n));

	uint32_t ssrc_i_n = htonl(ssrc_i); /* set ssrc */
	memcpy(&rtp_header[8], &ssrc_i_n, sizeof(ssrc_i_n));

	return SWITCH_STATUS_SUCCESS;
}

static void print_frame_stats(switch_frame_t *frame)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "FRAME STATISTICS: seq[%u] ssrc[%lu] datalen[%lu] rate[%lu] samples[%lu]\n",
		(unsigned)frame->seq, (unsigned long)frame->ssrc, (unsigned long)frame->datalen, (unsigned long)frame->rate, (unsigned long)frame->samples);
}

static switch_bool_t pcli_callback(switch_media_bug_t *bug, void *user_data, switch_abc_type_t type)
{
	pcli_session_helper_t *helper = (pcli_session_helper_t *) user_data;

	switch (type) {
		case SWITCH_ABC_TYPE_INIT:
		case SWITCH_ABC_TYPE_READ_PING:
		case SWITCH_ABC_TYPE_CLOSE:
		case SWITCH_ABC_TYPE_READ:
		case SWITCH_ABC_TYPE_WRITE:
			break;

		case SWITCH_ABC_TYPE_READ_REPLACE:
			{
				switch_frame_t *frame = switch_core_media_bug_get_read_replace_frame(bug);
				print_frame_stats(frame);

				uint8_t pcli_header[PCLI_HEADER_LEN];
				uint8_t ip_header[IP_HEADER_LEN];
				uint8_t udp_header[UDP_HEADER_LEN];
				uint8_t rtp_header[RTP_HEADER_LEN];

				uint8_t packet[PCLI_HEADER_LEN + IP_HEADER_LEN + UDP_HEADER_LEN + RTP_HEADER_LEN + frame->datalen];

				uint8_t *in_packet_pointer = packet;

				memset(packet, 0, sizeof(packet)); /* zero - TODO test if this is still necessary - don't think so! */

				gen_rtp_header(rtp_header, frame->seq, frame->timestamp, frame->ssrc);
				gen_udp_header(udp_header, UDP_HEADER_LEN + RTP_HEADER_LEN + frame->datalen);
				gen_ip_header(ip_header, IP_HEADER_LEN + UDP_HEADER_LEN + RTP_HEADER_LEN + frame->datalen);
				gen_pcli_header(pcli_header, PCLI_MEDIA_DIRECTION_FROM_TARGET, helper->instance_id, pcli_globals.switch_id, helper->ini_id);

				memcpy(in_packet_pointer, pcli_header, sizeof(pcli_header));				
				in_packet_pointer += PCLI_HEADER_LEN;

				memcpy(in_packet_pointer, ip_header, sizeof(ip_header));		
				in_packet_pointer += IP_HEADER_LEN;

				memcpy(in_packet_pointer, udp_header, sizeof(udp_header));
				in_packet_pointer += UDP_HEADER_LEN;

				memcpy(in_packet_pointer, rtp_header, sizeof(rtp_header));
				in_packet_pointer += RTP_HEADER_LEN;

				memcpy(in_packet_pointer, frame->data, frame->datalen);

				switch_size_t packetsize = sizeof(packet);

				switch_socket_sendto(pcli_globals.socket, pcli_globals.remote_sockaddr, 0, (void *) packet, &packetsize);
			}
			break;

		case SWITCH_ABC_TYPE_WRITE_REPLACE:
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

	/* get pcli_ini_id and store in pcli_session_helper */
	if ((p = switch_channel_get_variable(channel, "pcli_ini_id")) && switch_true(p)) {
		pcli_session_helper->ini_id = (uint16_t)atoi(p);
	} else {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "No pcli_ini_id channel variable was set!\n");
		return;
	}

	/* get pcli_instance_id and store in pcli_session_helper */
	if ((p = switch_channel_get_variable(channel, "pcli_instance_id")) && switch_true(p)) {
		pcli_session_helper->instance_id = (uint16_t)atoi(p);
	} else {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "No pcli_instance_id channel variable was set!\n");
		return;
	}

	/* add the bug */
	status = switch_core_media_bug_add(session, pcli_callback, pcli_session_helper, 0, SMBF_READ_REPLACE | SMBF_WRITE_REPLACE, &bug);

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
