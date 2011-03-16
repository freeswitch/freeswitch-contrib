/* 
 * High Availability Cluster Module for FreeSWITCH Modular Media Switching 
 * Software Library / Soft-Switch Application
 *
 * High Availability Cluster Module is 
 * Copyright (C) 2010, Eliot Gable <egable@gmail.com>
 * 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application is
 * Copyright (C) 2005-2010, Anthony Minessale II <anthm@freeswitch.org>
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
 * The Original Code is High Availability Module for FreeSWITCH Modular Media 
 * Switching Software Library / Soft-Switch Application
 *
 * The Initial Developer of the Original Code is
 * Eliot Gable <egable@gmail.com>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 *
 */
#include <mod_ha_cluster.h>


/* Don't forget to free "out" when done with it */
switch_status_t execute_command(const char *cmd, char **out)
{
	FILE* fh = NULL;
	char buf[HAC_BUFFSIZE];
	int ret_code = 0;
	ssize_t len;

	memset(buf, 0, HAC_BUFFSIZE);
#ifdef MACOSX

#elif defined(DARWIN)

#elif defined(WIN32)

#else
	setenv("PATH", "/bin:/sbin:/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin:/usr/local/freeswitch/bin:/opt/bin:/opt/sbin", 1);
	errno = 0;
	fh = popen(cmd, "r");
	if(!fh || errno) {
		char errbuf[256];
		memset(errbuf, 0, 256);
		strerror_r(errno, errbuf, sizeof(errbuf));
		if(strlen(errbuf)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to execute command '%s': %s\n", cmd, errbuf);
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to execute command '%s': process failed to open (errno %d)\n", cmd, errno);
		}
		return SWITCH_STATUS_FALSE;
	}
	len = fread(buf, 1, sizeof(buf), fh);
	if(errno) {
		char errbuf[256];
		memset(errbuf, 0, 256);
		strerror_r(errno, errbuf, sizeof(errbuf));
		if(strlen(errbuf)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to read file handle: %s\n", errbuf);
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to read file handle: an unknown error occurred\n");
		}
		return SWITCH_STATUS_FALSE;
	}
	errno = 0;
	ret_code = pclose(fh);
	*out = malloc(len+1);
	switch_assert(*out);
	memset(*out, 0, len+1);
	memcpy(*out, buf, len);
#endif
	if(errno == ECHILD) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to determine exit status of child command '%s'\n", cmd);
		return SWITCH_STATUS_FALSE;
	}
	if(ret_code != 0) {
		return SWITCH_STATUS_FALSE;
	}
	return SWITCH_STATUS_SUCCESS;
}

