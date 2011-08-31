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
 * Leon de Rooij <leon@scarlet-internet.nl>
 *
 *
 * mod_mycommands
 *
 * several commands apis that I need
 *
 * - pg_escape_string	escape all quotes (') and backslashes (\) with a backslash (\)
 * - pg_prepare_copy	remove all carriage return (\r) and replace newlines (\n) by literal\ and n
 * - generate_xml_cdr	if run on a session, returns an xml cdr
 *
 */
#include <switch.h>

#define er(x) switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "%s\n", x);


SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_mycommands_shutdown);
SWITCH_MODULE_LOAD_FUNCTION(mod_mycommands_load);
SWITCH_MODULE_DEFINITION(mod_mycommands, mod_mycommands_load, mod_mycommands_shutdown, NULL);


/*
 *	SWITCH_STANDARD_API
 */


SWITCH_STANDARD_API(pg_escape_string_function)
{
	const char *ptr, *pptr = cmd;
	const char chars[2] = { '\'', '\\' }; /* characters that need to be escaped */
	uint8_t backslash = '\\'; /* escape characters with a backslash */

	if (zstr(cmd)) {
		return SWITCH_STATUS_FALSE;
	}

	/* optimize still:
 	 * -	set stream->alloc_chunk or alloc_len so that it's at least larger than cmd see switch_console.h :46
 	 */

	for (ptr = strpbrk(pptr, chars); ptr; ptr = strpbrk(pptr + 1, chars)) {
		if (ptr > pptr) {
			stream->raw_write_function(stream, (uint8_t *) pptr, ptr - pptr);
		}
		stream->raw_write_function(stream, &backslash, 1);
		pptr = ptr;
	}
	stream->raw_write_function(stream, (uint8_t *) pptr, strlen(pptr) + 1);

	return SWITCH_STATUS_SUCCESS;
}


SWITCH_STANDARD_API(pg_prepare_copy_function)
{
	const char *ptr, *pptr = cmd;
	const char chars[6] = { '\n', '\t', '\r', '\b', '\f', '\v' }; /* characters that need to be replaced */

	if (zstr(cmd)) {
		return SWITCH_STATUS_FALSE;
	}

	/* optimize still:
 	 * -	set stream->alloc_chunk or alloc_len so that it's at least larger than cmd see switch_console.h :46
 	 */

	for (ptr = strpbrk(pptr, chars); ptr; ptr = strpbrk(pptr, chars)) {
		if (ptr > pptr) {
			stream->raw_write_function(stream, (uint8_t *) pptr, ptr - pptr);
		}

		switch (ptr[0]) {
		case '\n':
			stream->raw_write_function(stream, (uint8_t *) "\\n", 2);
			break;
		case '\t':
			stream->raw_write_function(stream, (uint8_t *) "\\t", 2);
			break;
		case '\r':
			/* just remove this character */
			break;
		case '\b':
			stream->raw_write_function(stream, (uint8_t *) "\\b", 2);
			break;
		case '\f':
			stream->raw_write_function(stream, (uint8_t *) "\\f", 2);
			break;
		case '\v':
			stream->raw_write_function(stream, (uint8_t *) "\\v", 2);
			break;
		}

		pptr = ptr + 1;
	}
	stream->raw_write_function(stream, (uint8_t *) pptr, strlen(pptr) + 1);

	return SWITCH_STATUS_SUCCESS;
}

SWITCH_STANDARD_API(old_pg_prepare_copy_function)
{
	char *tab = "\\t";
	char *newline = "\\n";

	if (zstr(cmd)) {
		return SWITCH_STATUS_FALSE;
	}

	/* optimize still:
 	 * -	set stream->alloc_chunk or alloc_len so that it's at least larger than cmd see switch_console.h :46
 	 * -	don't copy byte-for-byte ?
 	 */

	while (*cmd) {
		if (*cmd == '\r') {
			/* remove this character */
		} else if (*cmd == '\t') {
			stream->raw_write_function(stream, (uint8_t *) tab, 2);
		} else if (*cmd == '\n') {
			stream->raw_write_function(stream, (uint8_t *) newline, 2);
		} else {
			stream->raw_write_function(stream, (uint8_t *) cmd, 1);
		}
		cmd++;
	}

	return SWITCH_STATUS_SUCCESS;
}


SWITCH_STANDARD_API(generate_xml_cdr_function)
{
	switch_xml_t cdr = NULL;
	char *xml_text = NULL;

	if (session) {
		if (switch_ivr_generate_xml_cdr(session, &cdr) == SWITCH_STATUS_SUCCESS) {
			xml_text = switch_xml_toxml(cdr, SWITCH_TRUE);
			if (xml_text) {
				switch_assert(xml_text);
				stream->raw_write_function(stream, (unsigned char *) xml_text, strlen(xml_text));
				free(xml_text);
			} else {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Memory Error!\n");
			}
			switch_xml_free(cdr);
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error Generating XML CDR!\n");
		}
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "No Session Available!\n");
	}

	return SWITCH_STATUS_SUCCESS;
}


SWITCH_STANDARD_API(xml_function)
{
char *myxml = "<?xml version=\"1.0\"?>\n"
"<cdr>\n"
"  <channel_data>\n"
"    <state>CS_REPORTING</state>\n"
"    <direction>outbound</direction>\n"
"    <state_number>11</state_number>\n"
"    <flags>0=1;1=1;2=1;3=1;35=1;36=1;38=1;41=1;46=1;51=1</flags>\n"
"    <caps>1=1;2=1;3=1;4=1;5=1</caps>\n"
"  </channel_data>\n"
"  <variables>\n"
"    <direction>outbound</direction>\n"
"    <is_outbound>true</is_outbound>\n"
"    <uuid>8fe7eb6a-cefe-4a23-8cf5-f50ecf10ed25</uuid>\n"
"    <sip_gateway_name>kpn-gw-1</sip_gateway_name>\n"
"    <sip_profile_name>gateway</sip_profile_name>\n"
"    <channel_name>sofia/c4-kpn/%2B31104791972</channel_name>\n"
"    <sip_destination_url>sip%3A%2B31104791972%4062.93.147.149</sip_destination_url>\n"
"    <max_forwards>68</max_forwards>\n"
"    <originator_codec>G729%408000h%4010i%408000b,PCMA%408000h%4010i%4064000b,PCMU%408000h%4010i%4064000b</originator_codec>\n"
"    <originator>becad4e7-10d1-49cf-a3e4-ca26c8b10836</originator>\n"
"    <switch_m_sdp>v%3D0%0D%0Ao%3DMG4000%7C2.0%2013490%2013490%20IN%20IP4%2080.84.31.123%0D%0As%3D-%0D%0Ac%3DIN%20IP4%2080.84.31.123%0D%0At%3D0%200%0D%0Am%3Daudio%2031832%20RTP/AVP%2018%2098%2096%2097%208%200%204%20106%20105%20108%20107%20101%0D%0Aa%3Dfmtp%3A18%20annexb%3Dno%0D%0Aa%3Drtpmap%3A98%20G.729a/8000%0D%0Aa%3Drtpmap%3A96%20G.729ab/8000%0D%0Aa%3Drtpmap%3A97%20G.729b/8000%0D%0Aa%3Dfmtp%3A4%20annexa%3Dno%0D%0Aa%3Drtpmap%3A106%20G.723.1a-L/8000%0D%0Aa%3Drtpmap%3A105%20G.723.1a-H/8000%0D%0Aa%3Drtpmap%3A108%20G.723.1-L/8000%0D%0Aa%3Drtpmap%3A107%20G.723.1-H/8000%0D%0Aa%3Drtpmap%3A101%20telephone-event/8000%0D%0Aa%3Dfmtp%3A101%200-15%0D%0Aa%3Dptime%3A10%0D%0Aa%3DX-vrzcap%3Avbd%20Ver%3D1%20Mode%3DFaxPr%20ModemRtpRed%3D0%0D%0Aa%3DX-vrzcap%3Aidentification%20bin%3DDSR1e814%20Prot%3Dmgcp%20App%3DMG%0D%0A</switch_m_sdp>\n"
"    <lcr_carrier>KPN</lcr_carrier>\n"
"    <lcr_rate>0.00000</lcr_rate>\n"
"    <origination_caller_id_number>%2B31620251461</origination_caller_id_number>\n"
"    <sip_cid_type>pid</sip_cid_type>\n"
"    <carrier_name>KPN</carrier_name>\n"
"    <carrier_id>1</carrier_id>\n"
"    <route_id>216</route_id>\n"
"    <absolute_codec_string>G729%408000h%4010i%408000b,PCMA%408000h%4010i%4064000b,PCMU%408000h%4010i%4064000b</absolute_codec_string>\n"
"    <originate_early_media>true</originate_early_media>\n"
"    <sip_req_uri>%2B31104791972%4062.93.147.149</sip_req_uri>\n"
"    <sofia_profile_name>c4-kpn</sofia_profile_name>\n"
"    <switch_r_sdp>v%3D0%0D%0Ao%3Dprxams01%2088436606%200%20IN%20IP4%2062.93.147.149%0D%0As%3Dsip%20call%0D%0Ac%3DIN%20IP4%2062.93.143.141%0D%0At%3D0%200%0D%0Am%3Daudio%2023286%20RTP/AVP%2018%20101%0D%0Aa%3Drtpmap%3A18%20G729/8000%0D%0Aa%3Dfmtp%3A18%20annexb%3Dno%0D%0Aa%3Drtpmap%3A101%20telephone-event/8000%0D%0Aa%3Dfmtp%3A101%200-15%0D%0Aa%3Dptime%3A10%0D%0Aa%3Dsqn%3A0%0D%0Aa%3Dcdsc%3A%201%20audio%20RTP/AVP%2018%20101%0D%0Aa%3Dcdsc%3A%203%20image%20udptl%20t38%0D%0Aa%3Dcpar%3A%20a%3DT38FaxVersion%3A0%0D%0Aa%3Dcpar%3A%20a%3DT38FaxRateManagement%3AtransferredTCF%0D%0Aa%3Dcpar%3A%20a%3DT38FaxMaxDatagram%3A160%0D%0Aa%3Dcpar%3A%20a%3DT38FaxUdpEC%3At38UDPRedundancy%0D%0Aa%3DX-sqn%3A0%0D%0Aa%3DX-cap%3A%201%20image%20udptl%20t38%0D%0A</switch_r_sdp>\n"
"    <sip_audio_recv_pt>18</sip_audio_recv_pt>\n"
"    <sip_use_codec_name>G729</sip_use_codec_name>\n"
"    <sip_use_codec_fmtp>annexb%3Dno</sip_use_codec_fmtp>\n"
"    <sip_use_codec_rate>8000</sip_use_codec_rate>\n"
"    <sip_use_codec_ptime>10</sip_use_codec_ptime>\n"
"    <read_codec>G729</read_codec>\n"
"    <read_rate>8000</read_rate>\n"
"    <write_codec>G729</write_codec>\n"
"    <write_rate>8000</write_rate>\n"
"    <local_media_ip>213.204.235.18</local_media_ip>\n"
"    <local_media_port>25612</local_media_port>\n"
"    <sip_use_pt>18</sip_use_pt>\n"
"    <rtp_use_ssrc>3122435163</rtp_use_ssrc>\n"
"    <remote_media_ip>62.93.143.141</remote_media_ip>\n"
"    <remote_media_port>23286</remote_media_port>\n"
"    <last_bridge_to>becad4e7-10d1-49cf-a3e4-ca26c8b10836</last_bridge_to>\n"
"    <bridge_channel>sofia/c4-bics/31620251461%4080.84.31.123</bridge_channel>\n"
"    <bridge_uuid>becad4e7-10d1-49cf-a3e4-ca26c8b10836</bridge_uuid>\n"
"    <signal_bond>becad4e7-10d1-49cf-a3e4-ca26c8b10836</signal_bond>\n"
"    <sip_local_network_addr>213.204.235.18</sip_local_network_addr>\n"
"  </variables>\n"
"</cdr>\n";

	stream->raw_write_function(stream, (uint8_t *) myxml, strlen(myxml));
	return SWITCH_STATUS_SUCCESS;
}


/*
 *	LOAD & SHUTDOWN
 */


SWITCH_MODULE_LOAD_FUNCTION(mod_mycommands_load)
{
	switch_api_interface_t *api_interface;

	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_API(api_interface, "pg_escape_string", "mod_mycommands pg_escape_string function", pg_escape_string_function, "<string>");
	SWITCH_ADD_API(api_interface, "pg_prepare_copy", "mod_mycommands pg_prepare_copy function", pg_prepare_copy_function, "<string>");
	SWITCH_ADD_API(api_interface, "old_pg_prepare_copy", "mod_mycommands old_pg_prepare_copy function", old_pg_prepare_copy_function, "<string>");
	SWITCH_ADD_API(api_interface, "generate_xml_cdr", "mod_mycommands generate_xml_cdr function", generate_xml_cdr_function, "");

	SWITCH_ADD_API(api_interface, "xml", "mod_mycommands xml function", xml_function, "");

	switch_console_set_complete("add pg_escape_string");
	switch_console_set_complete("add pg_prepare_copy");
	switch_console_set_complete("add old_pg_prepare_copy");
	switch_console_set_complete("add generate_xml_cdr");
	switch_console_set_complete("add xml");

	return SWITCH_STATUS_SUCCESS;
}


SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_mycommands_shutdown)
{
	/* remove auto completion */
	switch_console_set_complete("del pg_escape_string");
	switch_console_set_complete("del pg_prepare_copy");
	switch_console_set_complete("del old_pg_prepare_copy");
	switch_console_set_complete("del generate_xml_cdr");
	switch_console_set_complete("del xml");

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
