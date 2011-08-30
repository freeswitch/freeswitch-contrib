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
 * - pg_prepare_copy	remove all carriage return (\r) and replace newlines (\n) by literal \\ and n
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
	uint8_t backslash = '\\';

	if (zstr(cmd)) {
		return SWITCH_STATUS_FALSE;
	}

	/* optimize still:
 	 * -	set stream->alloc_chunk or alloc_len so that it's at least larger than cmd see switch_console.h :46
 	 * -	don't copy byte-for-byte ?
 	 */

	while (*cmd) {
		if (*cmd == '\'' || *cmd == '\\') {
			stream->raw_write_function(stream, &backslash, 1);
		}
		stream->raw_write_function(stream, (uint8_t *) cmd, 1);
		cmd++;
	}

	return SWITCH_STATUS_SUCCESS;
}


SWITCH_STANDARD_API(pg_prepare_copy_function)
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


/*
 *	LOAD & SHUTDOWN
 */


SWITCH_MODULE_LOAD_FUNCTION(mod_mycommands_load)
{
	switch_api_interface_t *api_interface;

	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_API(api_interface, "pg_escape_string", "mod_mycommands pg_escape_string function", pg_escape_string_function, "<string>");
	SWITCH_ADD_API(api_interface, "pg_prepare_copy", "mod_mycommands pg_prepare_copy function", pg_prepare_copy_function, "<string>");
	SWITCH_ADD_API(api_interface, "generate_xml_cdr", "mod_mycommands generate_xml_cdr function", generate_xml_cdr_function, "");

	switch_console_set_complete("add pg_escape_string");
	switch_console_set_complete("add pg_prepare_copy");
	switch_console_set_complete("add generate_xml_cdr");

	return SWITCH_STATUS_SUCCESS;
}


SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_mycommands_shutdown)
{
	/* remove auto completion */
	switch_console_set_complete("del pg_escape_string");
	switch_console_set_complete("del pg_prepare_copy");
	switch_console_set_complete("del generate_xml_cdr");

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
