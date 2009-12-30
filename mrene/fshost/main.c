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
 * Mathieu Rene <mathieu.rene@gmail.com>
 *
 *
 * fshost - Example on how to host freeswitch within your own programs
 *
 */

#include <stdio.h>
#include <switch.h>

int main(int argc, char *argv[])
{
	switch_core_flag_t flags = SCF_USE_SQL;
	const char *err = NULL;
	switch_bool_t console = SWITCH_TRUE;
	switch_status_t destroy_status;
	int ret = 0;
	
	/* Initialize libs & globals */
	printf("Initializing globals...\n");
	switch_core_setrlimits();
	switch_core_set_globals();
	
	/* If you need to override configuration directories, you need to change them in the SWITCH_GLOBAL_dirs global structure */
	printf("Initializing core...\n");
	/* Initialize the core and load modules, that will startup FS completely */
	if (switch_core_init_and_modload(flags, console, &err) != SWITCH_STATUS_SUCCESS) {
		fprintf(stderr, "Failed to initialize FreeSWITCH's core: %s\n", err);
		return 1;
	}
	
	printf("Everything OK, Entering runtime loop.\n");
	
	fflush(stdout);
	
	/* Go into the runtime loop. If the argument is true, this basically sets runtime.running = 1 and loops while that is set
	 * If its false, it initializes the libedit for the console, then does the same thing
	 */
	switch_core_runtime_loop(!console);
	
	/* When the runtime loop exits, its time to shutdown */
	destroy_status = switch_core_destroy();

	/* If we get SWITCH_STATUS_RESTART, it means the core received an fsctl shutdown restart command */
	if (destroy_status == SWITCH_STATUS_RESTART) {
		char buf[1024] = "";
		int i = 0;
		
		switch_assert(argv[0]);
		switch_sleep(1000000);
		ret = (int)execv(argv[0], argv);
		fprintf(stderr, "Restart Failed [%s] resorting to plan b\n", strerror(errno));

		for(i = 0; i < argc; i++) {
			switch_snprintf(buf + strlen(buf), sizeof(buf) - strlen(buf), "%s ", argv[i]);
		}

		ret = system(buf);
	}
	
	return ret;
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
