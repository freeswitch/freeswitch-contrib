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


switch_status_t initialize_interfaces()
{
	char *cmd_result = NULL, cmd[256];
	hac_interface_t* cur_int = NULL;
	int count = 0;
	switch_status_t status;
	char expected_speed[16];
	char expected_duplex[16];

	memset(expected_speed, 0, 16);
	memset(expected_duplex, 0, 16);

	/* First, make sure all interfaces exist. */
	cur_int = config.network.interfaces;
	while(cur_int != NULL) {
		/* We cannot list a vlan if it is not up */
		if(!strcasestr(cur_int->name, "vlan") && !strchr(cur_int->name, '.')) {
			memset(cmd, 0, 256);
			snprintf(cmd, 256, "ip link list %s", cur_int->name);
			if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result) {
				return SWITCH_STATUS_FALSE;
			}
			if(strstr(cmd_result, "does not exist")) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: interface '%s' does not exist\n", cur_int->name);
				fprintf(stderr, "HAC: interface '%s' does not exist\n", cur_int->name);
				switch_safe_free(cmd_result);
				return SWITCH_STATUS_FALSE;
			}
			switch_safe_free(cmd_result);
		}
		cur_int = cur_int->next;
	}
	/* After we know they exist, then take control over them. */
	cur_int = config.network.interfaces;
	while(cur_int != NULL) {
		switch_bool_t bridge_device = SWITCH_FALSE;
		switch_bool_t vlan_device = SWITCH_FALSE;
		/* Check to see if it is a VLAN device */
		FILE * fp = NULL;
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "/proc/net/vlan/%s", cur_int->name);
		if((fp = fopen(cmd, "r")) != NULL) {
			fclose(fp);
			vlan_device = SWITCH_TRUE;
		} else if(strcasestr(cur_int->name, "vlan")) {
			vlan_device = SWITCH_TRUE;
		} else if(strchr(cur_int->name, '.')) {
			vlan_device = SWITCH_TRUE;
		}
		/* Cannot get driver of vlan interface which is down, and
		   regardless, it will always match underlying device. */
		if(vlan_device == SWITCH_FALSE) {
			/* Get the driver of the device to see if it is a bridge */
			memset(cmd, 0, 256);
			snprintf(cmd, 256, "ethtool -i %s 2>&1", cur_int->name);
			if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result || (cmd_result && strstr(cmd_result, "does not exist"))) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to determine driver of interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
				fprintf(stderr, "HAC: unable to determine driver of interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
				switch_safe_free(cmd_result);
				return SWITCH_STATUS_FALSE;
			}
			if(strcasestr(cmd_result, "bridge")) {
				bridge_device = SWITCH_TRUE;
			}
			switch_safe_free(cmd_result);
		}
		/* This clears routes and puts the interface in a DOWN state
		   which then requires us to bring it back up. This helps to
		   check for issues to make sure we can actually bring it back
		   up. Sometimes we can take the interface down when something
		   is wrong, but not bring it back up. This helps us check
		   that condition. */
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ifdown %s 2>&1", cur_int->name);
		if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result || (cmd_result && strstr(cmd_result, "does not exist"))) {
			if(vlan_device == SWITCH_FALSE) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to control interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
				fprintf(stderr, "HAC: unable to control interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
				switch_safe_free(cmd_result);
				return SWITCH_STATUS_FALSE;
			}
		}
		switch_safe_free(cmd_result);
		if(vlan_device == SWITCH_FALSE) {
			/* We do not want to have an IP address conflict while starting
			   this node, so we flush the addresses. If this is a vlan, we
			   will not be able to reference the dev if it is down. We
			   expect the act of taking it down to remove the IPs. */
			memset(cmd, 0, 256);
			snprintf(cmd, 256, "ip addr flush %s 2>&1", cur_int->name);
			if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result || (cmd_result && strstr(cmd_result, "does not exist"))) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to control interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
				fprintf(stderr, "HAC: unable to control interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
				switch_safe_free(cmd_result);
				return SWITCH_STATUS_FALSE;
			}
			switch_safe_free(cmd_result);
		}
		if(vlan_device == SWITCH_FALSE && bridge_device == SWITCH_FALSE) {
			/* If speed and duplex are specified, set them and disable autoneg */
			if(!switch_strlen_zero(cur_int->speed) && !switch_strlen_zero(cur_int->duplex)) {
				memset(cmd, 0, 256);
				snprintf(cmd, 256, "ethtool -s %s speed %s duplex %s autoneg off 2>&1", cur_int->name, cur_int->speed, cur_int->duplex);
				if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result || (cmd_result && strstr(cmd_result, "Cannot set new settings"))) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to set speed and duplex on interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
					fprintf(stderr, "HAC: unable to set speed and duplex on interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
					switch_safe_free(cmd_result);
					return SWITCH_STATUS_FALSE;
				}
				switch_safe_free(cmd_result);
			} else {
				if(!switch_strlen_zero(cur_int->duplex)) {
					/* Set the duplex on the interface if specified */
					memset(cmd, 0, 256);
					snprintf(cmd, 256, "ethtool -s %s duplex %s 2>&1", cur_int->name, cur_int->duplex);
					if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result || (cmd_result && strstr(cmd_result, "Cannot set new settings"))) {
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to set duplex on interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
						fprintf(stderr, "HAC: unable to set duplex on interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
						switch_safe_free(cmd_result);
						return SWITCH_STATUS_FALSE;
					}
					switch_safe_free(cmd_result);
				}
				if(!switch_strlen_zero(cur_int->speed)) {
					/* Set the speed on the interface if specified */
					memset(cmd, 0, 256);
					snprintf(cmd, 256, "ethtool -s %s speed %s 2>&1", cur_int->name, cur_int->speed);
					if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result || (cmd_result && strstr(cmd_result, "Cannot set new settings"))) {
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to set speed on interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
						fprintf(stderr, "HAC: unable to set speed on interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
						switch_safe_free(cmd_result);
						return SWITCH_STATUS_FALSE;
					}
					switch_safe_free(cmd_result);
				}
			}
		}
		/* Now we bring the interface back up to ensure we *can* bring
		   it up. We turn off ARP so that when we assign IPs to it, we
		   do not immediately start talking over it. */
		memset(cmd, 0, 256);
		if(vlan_device == SWITCH_TRUE) {
			snprintf(cmd, 256, "ifup %s && sleep 1 && ip link set %s arp off 2>&1", cur_int->name, cur_int->name);
		} else {
			snprintf(cmd, 256, "ip link set %s up arp off 2>&1", cur_int->name);
		}
		if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result || (cmd_result && strstr(cmd_result, "does not exist"))) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to control interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
			fprintf(stderr, "HAC: unable to control interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
			switch_safe_free(cmd_result);
			return SWITCH_STATUS_FALSE;
		}
		switch_safe_free(cmd_result);
		if(bridge_device == SWITCH_FALSE && vlan_device == SWITCH_FALSE) {
			/* Run an internal test on the interface to help check to see
			   if it is good. */
			memset(cmd, 0, 256);
			snprintf(cmd, 256, "sleep 1 && ethtool -t %s online 2>&1", cur_int->name);
			count = 0;
			while(((status = execute_command(cmd, &cmd_result)) != SWITCH_STATUS_SUCCESS || !cmd_result || (cmd_result && !strstr(cmd_result, "PASS"))) && count < 30) {
				switch_yield(1000);
				count++;
			}
			if(status != SWITCH_STATUS_SUCCESS || count > 30) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: interface '%s' failed test: %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
				fprintf(stderr, "HAC: interface '%s' failed test: %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
				switch_safe_free(cmd_result);
				return SWITCH_STATUS_FALSE;
			}
			switch_safe_free(cmd_result);
		}
		/* Check for carrier signal on interface */
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ip link list dev %s 2>&1", cur_int->name);
		if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result || (cmd_result && (strstr(cmd_result, "NO-CARRIER") || strstr(cmd_result, "DOWN")))) {
			if(cmd_result && strstr(cmd_result, "NO-CARRIER")) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: interface '%s' loss of carrier detected\n", cur_int->name);
				fprintf(stderr, "HAC: interface '%s' loss of carrier detected\n", cur_int->name);
			} else if(cmd_result && strstr(cmd_result, "DOWN")) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: interface '%s' is down\n", cur_int->name);
				fprintf(stderr, "HAC: interface '%s' is down\n", cur_int->name);
			} else {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to check carrier status on interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
				fprintf(stderr, "HAC: unable to check carrier status on interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
			}
			switch_safe_free(cmd_result);
			return SWITCH_STATUS_FALSE;
		}
		switch_safe_free(cmd_result);
		if(vlan_device == SWITCH_FALSE && bridge_device == SWITCH_FALSE) {
			/* Check the duplex and speed on the interface if specified */
			memset(cmd, 0, 256);
			snprintf(cmd, 256, "ethtool %s 2>&1", cur_int->name);
			if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to query interface '%s' for duplex and speed: %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
				fprintf(stderr, "HAC: unable to query interface '%s' for duplex and speed: %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
				switch_safe_free(cmd_result);
				return SWITCH_STATUS_FALSE;
			}
			if(!switch_strlen_zero(cur_int->duplex)) {
				snprintf(expected_duplex, 16, "Duplex: %s", cur_int->duplex);
				if(!strcasestr(cmd_result, expected_duplex)) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: interface '%s' duplex not set correctly: %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
					fprintf(stderr, "HAC: interface '%s' duplex not set correctly: %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
					switch_safe_free(cmd_result);
					return SWITCH_STATUS_FALSE;
				}
			}
			if(!switch_strlen_zero(cur_int->speed)) {
				snprintf(expected_speed, 16, "Speed: %sMb/s", cur_int->speed);
				if(!strcasestr(cmd_result, expected_speed)) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: interface '%s' speed not set correctly: %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
					fprintf(stderr, "HAC: interface '%s' speed not set correctly: %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
					switch_safe_free(cmd_result);
					return SWITCH_STATUS_FALSE;
				}
			}
			switch_safe_free(cmd_result);
		}
		cur_int = cur_int->next;
	}
	/* If we get here, we are fairly confident all the interfaces are
	   working. */
	return SWITCH_STATUS_SUCCESS;
}

switch_status_t initialize_firewall()
{
	char *cmd_result = NULL, *cmd = NULL, *chain_start = NULL, *chain_end = NULL;
	int cmd_result_len = 0, bufsize = 0;
	hac_chain_list_t *chains = NULL;
	hac_chain_list_t *cur = NULL;
	switch_status_t status;
	switch_bool_t skip_create = SWITCH_FALSE;
	switch_bool_t added_delete_rule = SWITCH_FALSE;

	chains = malloc(sizeof(hac_chain_list_t));
	switch_assert(chains);
	memset(chains, 0, sizeof(hac_chain_list_t));
	cur = chains;

	/* List the chains that exist */
	if(execute_command("iptables -nL | grep Chain", &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result) {
		switch_goto_status(SWITCH_STATUS_FALSE, done);
	}
	cmd_result_len = strlen(cmd_result);
	chain_start = cmd_result;
	chain_end = cmd_result;
	/* Loop through the chains and look for ha- chains. */
	while(chain_start && chain_end && chain_start < (cmd_result + cmd_result_len) && chain_end < (cmd_result + cmd_result_len)) {
		chain_start = strstr(chain_end, "ha-");
		if(chain_start == NULL && chain_end == cmd_result) {
			switch_goto_status(SWITCH_STATUS_SUCCESS, clean);
		}
		chain_end = strchr(chain_start, ' ');
		if(chain_start && chain_end && chain_end < (cmd_result + cmd_result_len) && chain_start < (cmd_result + cmd_result_len)) {
			*chain_end = '\0';
			if(cur->chain_len > 0) {
				cur->next = malloc(sizeof(hac_chain_list_t));
				switch_assert(cur->next);
				memset(cur->next, 0, sizeof(hac_chain_list_t));
				cur = cur->next;
			}
			cur->chain_len = strlen(chain_start);
			cur->chain_name = malloc(cur->chain_len+1);
			switch_assert(cur->chain_name);
			memset(cur->chain_name, 0, cur->chain_len+1);
			memcpy(cur->chain_name, chain_start, cur->chain_len);
			snprintf(cur->chain_flush, 64, "iptables -F %s", cur->chain_name);
			snprintf(cur->chain_delete, 64, "iptables -X %s", cur->chain_name);
			bufsize += 35 + (2 * cur->chain_len);
			chain_start = chain_end + 15;
		}
	}

	cmd = malloc(bufsize+1);
	switch_assert(bufsize);
	memset(cmd, ' ', bufsize);
	cmd[bufsize] = '\0';
	cur = chains; chain_start = cmd;
	/* Use "flush" rules first since we cannot remove referenced chains */
	while(cur != NULL) {
		memcpy(chain_start, cur->chain_flush, 12 + cur->chain_len);
		chain_start += 12 + cur->chain_len;
		if(cur->next != NULL) {
			memcpy(chain_start, " && ", 4);
			chain_start += 4;
		}
		cur = cur->next;
	}
	memcpy(chain_start, " && ", 4);
	chain_start += 4;
	cur = chains;
	/* Then try to delete all the ha- chains */
	while(cur != NULL) {
		if(!strcasestr(cur->chain_name, "ha-cluster-rules")) {
			memcpy(chain_start, cur->chain_delete, 12 + cur->chain_len);
			chain_start += 12 + cur->chain_len;
			if(cur->next != NULL) {
				memcpy(chain_start, " && ", 4);
				chain_start += 4;
			}
			added_delete_rule = SWITCH_TRUE;
		} else {
			skip_create = SWITCH_TRUE;
		}
		cur = cur->next;
	}
	if(added_delete_rule != SWITCH_TRUE) {
		chain_start -= 4;
		*chain_start = '\0';
	}
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: executing '%s'\n", cmd);
	if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result) {
		switch_goto_status(SWITCH_STATUS_FALSE, done);
	}

clean:
	/* If the ha-cluster-rules chain does not exist, make it */
	if(skip_create != SWITCH_TRUE) {
		if(execute_command("iptables -N ha-cluster-rules", &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result) {
			switch_goto_status(SWITCH_STATUS_FALSE, done);
		}
	}

done:
	cur = chains;
	while(cur != NULL) {
		hac_chain_list_t *tmp = cur;
		switch_safe_free(cur->chain_name);
		cur = cur->next;
		switch_safe_free(tmp);
	}
	switch_safe_free(cmd);
	switch_safe_free(cmd_result);
	return SWITCH_STATUS_SUCCESS;
}

switch_status_t verify_interfaces()
{
	char *cmd_result = NULL, cmd[256];
	hac_interface_t* cur_int = NULL;
	struct timeval cur_time;

	gettimeofday(&cur_time, NULL);


	cur_int = config.network.interfaces;
	while(cur_int != NULL) {
		/* Check for carrier signal on interface */
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ip link list dev %s 2>&1", cur_int->name);
		if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result || (cmd_result && (strstr(cmd_result, "NO-CARRIER") || strstr(cmd_result, "DOWN")))) {
			if(cmd_result && strstr(cmd_result, "NO-CARRIER")) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: interface '%s' loss of carrier detected\n", cur_int->name);
				fprintf(stderr, "HAC: interface '%s' loss of carrier detected\n", cur_int->name);
			} else if(cmd_result && strstr(cmd_result, "DOWN")) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: interface '%s' is down\n", cur_int->name);
				fprintf(stderr, "HAC: interface '%s' is down\n", cur_int->name);
			} else {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: unable to check carrier status on interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
				fprintf(stderr, "HAC: unable to check carrier status on interface '%s': %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
			}
			switch_safe_free(cmd_result);
			return SWITCH_STATUS_FALSE;
		}
		switch_safe_free(cmd_result);
		/* Run an internal test on the interface to help check to see
		   if it is good. */
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ethtool -t %s 2>&1", cur_int->name);
		if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS || !cmd_result || (cmd_result && !strstr(cmd_result, "PASS"))) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: interface '%s' failed test: %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
			fprintf(stderr, "HAC: interface '%s' failed test: %s\n", cur_int->name, (switch_strlen_zero(cmd_result) ? "unkown error" : cmd_result));
			switch_safe_free(cmd_result);
			return SWITCH_STATUS_FALSE;
		}
		switch_safe_free(cmd_result);
		cur_int = cur_int->next;
	}
	return SWITCH_STATUS_SUCCESS;
}

switch_status_t routes_up(const hac_route_t *routes)
{
	char *cmd_result = NULL, cmd[256];
	const hac_route_t* cur = NULL;

	cur = routes;
	while(cur != NULL) {
		/* Check for carrier signal on interface */
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ip route add %s via %s dev %s metric %s 2>&1", cur->destination, cur->gateway, cur->dev, (switch_strlen_zero(cur->metric) ? "0" : cur->metric));
		if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: error bringing up route '%s': %s\n", cmd, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
			fprintf(stderr, "HAC: error bringing up route '%s': %s\n", cmd, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
			switch_safe_free(cmd_result);
			return SWITCH_STATUS_FALSE;
		}
		switch_safe_free(cmd_result);
		cur = cur->next;
	}
	return SWITCH_STATUS_SUCCESS;
}

switch_status_t routes_down(const hac_route_t *routes)
{
	char *cmd_result = NULL, cmd[256];
	const hac_route_t* cur = NULL;

	cur = routes;
	while(cur != NULL) {
		/* Check for carrier signal on interface */
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ip route del %s via %s dev %s metric %s 2>&1", cur->destination, cur->gateway, cur->dev, (switch_strlen_zero(cur->metric) ? "0" : cur->metric));
		if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: error bringing down route '%s': %s\n", cmd, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
			fprintf(stderr, "HAC: error bringing down route '%s': %s\n", cmd, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
			switch_safe_free(cmd_result);
			return SWITCH_STATUS_FALSE;
		}
		switch_safe_free(cmd_result);
		cur = cur->next;
	}
	return SWITCH_STATUS_SUCCESS;
}

switch_status_t ips_up(const hac_ip_t *ips)
{
	char *cmd_result = NULL, cmd[256];
	const hac_ip_t* cur = NULL;

	cur = ips;
	while(cur != NULL) {
		/* Check for carrier signal on interface */
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ip addr add %s dev %s 2>&1", cur->cidr, cur->dev);
		if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: error adding IP address '%s': %s\n", cmd, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
			fprintf(stderr, "HAC: error adding IP address '%s': %s\n", cmd, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
			switch_safe_free(cmd_result);
			return SWITCH_STATUS_FALSE;
		}
		switch_safe_free(cmd_result);
		cur = cur->next;
	}
	return SWITCH_STATUS_SUCCESS;
}

switch_status_t ips_down(const hac_ip_t *ips)
{
	char *cmd_result = NULL, cmd[256];
	const hac_ip_t* cur = NULL;

	cur = ips;
	while(cur != NULL) {
		/* Check for carrier signal on interface */
		memset(cmd, 0, 256);
		snprintf(cmd, 256, "ip addr del %s dev %s 2>&1", cur->cidr, cur->dev);
		if(execute_command(cmd, &cmd_result) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: error removing IP address '%s': %s\n", cmd, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
			fprintf(stderr, "HAC: error removing IP address '%s': %s\n", cmd, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
			switch_safe_free(cmd_result);
			return SWITCH_STATUS_FALSE;
		}
		switch_safe_free(cmd_result);
		cur = cur->next;
	}
	return SWITCH_STATUS_SUCCESS;
}

char *generate_rule_cmd(const hac_rule_t *rule, const hac_ip_t *ip, const int rule_num)
{
	char *cmd = NULL, *slash = NULL;
	char begin[128], action[128], match[4044], string[128], hashlimit[384],
		 proto[64], sport[32], dport[32], dest[256], src[256], destrange[512],
		 srcrange[512], length[64], mports[1024], state[256], rejectwith[128],
		 mark[64], ip_addr[64];

	/* Validate that we will actually build a fully formed command */
	if(switch_strlen_zero(rule->type)) {
		goto fail;
	}
	if(switch_strlen_zero(rule->string) && switch_strlen_zero(rule->rate) &&
			switch_strlen_zero(rule->burst) && switch_strlen_zero(rule->mode) &&
			switch_strlen_zero(rule->proto) && switch_strlen_zero(rule->sport) &&
			switch_strlen_zero(rule->dport) && switch_strlen_zero(rule->src) &&
			switch_strlen_zero(rule->srcrange) && switch_strlen_zero(rule->length) &&
			switch_strlen_zero(rule->mports) && switch_strlen_zero(rule->srcmports) &&
			switch_strlen_zero(rule->dstmports) && switch_strlen_zero(rule->state) &&
			switch_strlen_zero(rule->rejectwith) &&
			switch_strlen_zero(rule->mark)) {
		goto fail;
	}
	if(!strcasestr(rule->type, "limit") || !strcasestr(rule->type, "drop") ||
	   !strcasestr(rule->type, "accept") || !strcasestr(rule->type, "reject")) {
		goto fail;
	}

	memset(begin, 0, sizeof(begin));
	memset(action, 0, sizeof(action));
	memset(match, 0, sizeof(match));
	memset(string, 0, sizeof(string));
	memset(hashlimit, 0, sizeof(hashlimit));
	memset(proto, 0, sizeof(proto));
	memset(sport, 0, sizeof(sport));
	memset(dport, 0, sizeof(dport));
	memset(dest, 0, sizeof(dest));
	memset(src, 0, sizeof(src));
	memset(destrange, 0, sizeof(destrange));
	memset(srcrange, 0, sizeof(srcrange));
	memset(length, 0, sizeof(length));
	memset(mports, 0, sizeof(mports));
	memset(state, 0, sizeof(state));
	memset(rejectwith, 0, sizeof(rejectwith));
	memset(mark, 0, sizeof(mark));
	memset(ip_addr, 0, sizeof(ip_addr));

	cmd = malloc(4300);
	switch_assert(cmd);
	memset(cmd, 0, 4300);

	snprintf(begin, sizeof(begin),"iptables -A ha-%s", ip->name);
	if(strcasestr(rule->type, "reject")) {
		if(switch_strlen_zero(rule->rejectwith)) {
			goto fail;
		}
		snprintf(action, sizeof(action), "-j REJECT --reject-with %s", rule->rejectwith);
	} else if(strcasestr(rule->type, "limit")) {
		if(switch_strlen_zero(rule->rate)) {
			goto fail;
		}
		snprintf(hashlimit, sizeof(hashlimit), "iptables -N ha-%s-%s && iptables -I ha-%s-%s -m hashlimit --hashlimit-name %s-%s-%d --hashlimit %s --hashlimit-burst %s --hashlimit-mode %s --hashlimit-htable-size 24593 --hashlimit-htable-expire 90000 -j RETURN && iptables -A ha-%s-%s -j DROP", ip->name, rule->name, ip->name, rule->name, ip->name, rule->name, rule_num, rule->rate, (switch_strlen_zero(rule->burst) ? "5" : rule->burst), (switch_strlen_zero(rule->mode) ? "srcip" : rule->mode), ip->name, rule->name);
		snprintf(action, sizeof(action), "-j ha-%s-%s", ip->name, rule->name);
	} else if(strcasestr(rule->type, "drop")) {
		snprintf(action, sizeof(action), "-j DROP");
	} else if(strcasestr(rule->type, "accept")) {
		snprintf(action, sizeof(action), "-j ACCEPT");
	}
	if(!switch_strlen_zero(rule->string)) {
		snprintf(string, sizeof(string), "-m string --algo kmp --string %s", rule->string);
	}
	if(!switch_strlen_zero(rule->proto)) {
		snprintf(proto, sizeof(proto), "-p %s", rule->proto);
	}
	if(!switch_strlen_zero(rule->sport)) {
		snprintf(sport, sizeof(sport), "--sport %s", rule->sport);
	}
	if(!switch_strlen_zero(rule->dport)) {
		snprintf(dport, sizeof(dport), "--dport %s", rule->dport);
	}
	if(!switch_strlen_zero(rule->src)) {
		snprintf(src, sizeof(src), "-s %s", rule->src);
	}
	if(!switch_strlen_zero(rule->srcrange)) {
		snprintf(srcrange, sizeof(srcrange), "-m iprange --dst-range %s", rule->srcrange);
	}
	if(!switch_strlen_zero(rule->length)) {
		snprintf(length, sizeof(length), "-m length --length %s", rule->length);
	}
	if(!switch_strlen_zero(rule->mports)) {
		snprintf(mports, sizeof(mports), "-m mport --ports %s", rule->mports);
	}
	if(!switch_strlen_zero(rule->srcmports) && switch_strlen_zero(rule->mports) && switch_strlen_zero(rule->dstmports)) {
		snprintf(mports, sizeof(mports), "-m mport --source-ports %s", rule->srcmports);
	} else if(!switch_strlen_zero(rule->dstmports) && switch_strlen_zero(rule->mports) && switch_strlen_zero(rule->srcmports)) {
		snprintf(mports, sizeof(mports), "-m mport --destination-ports %s", rule->dstmports);
	} else if(!switch_strlen_zero(rule->srcmports) && !switch_strlen_zero(rule->dstmports) && switch_strlen_zero(rule->mports)) {
		snprintf(mports, sizeof(mports), "-m mport --source-ports %s --destination-ports %s", rule->srcmports, rule->dstmports);
	}
	if(!switch_strlen_zero(rule->state)) {
		snprintf(state, sizeof(state), "-m state --state %s", rule->state);
	}
	if(!switch_strlen_zero(rule->mark)) {
		snprintf(mark, sizeof(mark), "-m mark --mark %s", rule->mark);
	}

	memcpy(ip_addr, ip->cidr, strlen(ip->cidr));
	if((slash = strchr(ip_addr, '/')) == NULL) {
		goto fail;
	}
	*slash = '\0';
	snprintf(dest, sizeof(dest), "-d %s", ip_addr);
	snprintf(cmd, sizeof(cmd), "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
			begin, proto, sport, dport, src, dest,
			mports, srcrange, destrange, state, mark, length, string, action,
			(switch_strlen_zero(hashlimit) ? "" : " && "), hashlimit);

	return cmd;
fail:
	return NULL;
}

switch_status_t rules_up(const hac_rule_t *rules, const hac_ip_t *ip)
{
	char *cmd_result = NULL, **cmds = NULL, *cmd = NULL, *cmd_loc = NULL;
	const hac_rule_t* cur = NULL;
	int num_rules = 0, cur_rule = 0, cmd_size = 0;
	switch_status_t status;

	/* Compute number of commands needed */
	cur = rules;
	while(cur != NULL) {
		num_rules++;
		cur = cur->next;
	}
	cmds = malloc(num_rules * sizeof(char*));
	switch_assert(cmds);
	memset(cmds, 0, num_rules * sizeof(char*));

	/* Generate a command for each rule in the list */
	cur = rules;
	while(cur != NULL) {
		cmds[cur_rule] = generate_rule_cmd(cur, ip, cur_rule);
		cur_rule++;
		cur = cur->next;
	}

	/* Make enough space for the commands and ' && ' between them */
	cmd_size = 4300 * num_rules + 5 * num_rules;
	cmd = malloc(cmd_size);
	switch_assert(cmd);
	memset(cmd, 0, cmd_size);

	/* Loop through commands and dump them into a single command */
	cmd_loc = cmd;
	for(int r = 0; r < num_rules; r++) {
		if(cmd_loc > cmd && !switch_strlen_zero(cmds[r])) {
			sprintf(cmd_loc, " && %s", cmds[r]);
			cmd_loc += 4 + strlen(cmds[r]);
		} else if(!switch_strlen_zero(cmds[r])) {
			sprintf(cmd_loc, "%s", cmds[r]);
			cmd_loc += strlen(cmds[r]);
		}
	}

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: firewall rules command: %s\n", cmd);
	if((status = execute_command(cmd, &cmd_result)) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: error adding firewall rules for IP address '%s': %s\n", cmd, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
		fprintf(stderr, "HAC: error adding firewall rules for IP address '%s': %s\n", cmd, (switch_strlen_zero(cmd_result) ? "unknown error" : cmd_result));
		switch_goto_status(SWITCH_STATUS_FALSE, done);
	}

done:
	if(cmds && num_rules) {
		for(int r = 0; r < num_rules; r++) {
			switch_safe_free(cmds[r]);
		}
	}
	switch_safe_free(cmds);
	switch_safe_free(cmd);
	switch_safe_free(cmd_result);
	return status;
}

