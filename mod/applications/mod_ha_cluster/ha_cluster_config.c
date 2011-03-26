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

switch_status_t config_routes(hac_route_t **routes, switch_xml_t root, const char *dev)
{
	switch_xml_t x_list = NULL;
	hac_route_t *cur;

	cur = *routes;
	for(x_list = switch_xml_child(root, "route"); x_list; x_list = x_list->next) {
		const char* name = switch_xml_attr(x_list, "name");
		const char* dest = switch_xml_attr(x_list, "destination");
		const char* gwy = switch_xml_attr(x_list, "gateway");
		const char* metric = switch_xml_attr(x_list, "metric");
		if(switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: route is missing name attribute.\n");
			goto fail;
		} else if(switch_strlen_zero(dest)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: route is missing dest attribute.\n");
			goto fail;
		} else if(switch_strlen_zero(gwy)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: route is missing gateway attribute.\n");
			goto fail;
		}
		/* TODO: Check for route name conflicts */
		/* TODO: Check route destination format */
		/* TODO: Check route gateway format */
		/* TODO: Check route metric for sane value */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: route discovered: %s (%s -> %s metric %s)\n", name, dest, gwy, metric);
		if(!switch_strlen_zero(cur->name)) {
			cur->next = switch_core_alloc(config.pool, sizeof(hac_route_t));
			cur = cur->next;
		}
		cur->name = name;
		cur->destination = dest;
		cur->gateway = gwy;
		cur->metric = metric;
		cur->dev = dev;
		cur->next = NULL;
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_arp(hac_arp_t **arps, switch_xml_t root, const char *dev)
{
	switch_xml_t x_list = NULL;
	hac_arp_t *cur;

	cur = *arps;
	for(x_list = switch_xml_child(root, "arp"); x_list; x_list = x_list->next) {
		const char* ip = switch_xml_attr(x_list, "ip");
		const char* mac = switch_xml_attr(x_list, "mac");
		if(switch_strlen_zero(ip)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: arp entry missint IP attribute\n");
			goto fail;
		} else if(switch_strlen_zero(mac)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: arp entry missint MAC attribute\n");
			goto fail;
		}
		/* TODO: Check for ARP IP address conflict */
		/* TODO: Check IP address format */
		/* TODO: Check MAC address format */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: arp entry discovered: %s -> %s\n", ip, mac);
		if(!switch_strlen_zero(cur->ip)) {
			cur->next = switch_core_alloc(config.pool, sizeof(hac_arp_t));
			cur = cur->next;
		}
		cur->ip = ip;
		cur->mac = mac;
		cur->dev = dev;
		cur->next = NULL;
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_ips(hac_ip_t** ips, switch_xml_t root, const char *dev)
{
	switch_xml_t x_list = NULL;
	hac_ip_t *cur;

	cur = *ips;
	for(x_list = switch_xml_child(root, "ip"); x_list; x_list = x_list->next) {
		const char* name = switch_xml_attr(x_list, "name");
		const char* cidr = switch_xml_attr(x_list, "cidr");
		const char* var = switch_xml_attr(x_list, "var");
		const char* rgroup = switch_xml_attr(x_list, "rule-group");
		if(switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: IP addresses must be named!\n");
			goto fail;
		} else if(switch_strlen_zero(cidr)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: IP address cidr attribute missing.\n");
			goto fail;
		} else if(strlen(name) > 14) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: IP address names cannot exceed 14 characters; sorry!\n");
			goto fail;
		} else if(!strchr(cidr, '/')) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: IP address cidr attribute not in CIDR format.\n");
			goto fail;
		}
		/* TODO: Check for ip name conflicts */
		/* TODO: Add more stringent check on IP address CIDR format */
		/* TODO: Check to make sure the referenced rule group exists */
		/* TODO: Check for variable name conflict */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: ip discovered: %s (%s) ${%s}\n", name, cidr, var);
		if(!switch_strlen_zero(cur->name)) {
			cur->next = switch_core_alloc(config.pool, sizeof(hac_ip_t));
			cur = cur->next;
		}
		cur->name = name;
		cur->cidr = cidr;
		cur->rgroup = rgroup;
		cur->var = var;
		if(!switch_strlen_zero(var)) {
			char buf[64], *p = NULL;
			memset(buf, 0, 64);
			switch_copy_string(buf, cidr, 64);
			if((p = strchr(buf, '/')) != NULL) {
				*p = '\0';
			}
			switch_core_set_variable(var, buf);
		}
		cur->dev = dev;
		cur->next = NULL;
		if(config_routes(&cur->routes, x_list, dev) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_cluster_ips(hac_cluster_ips_t **cips, switch_xml_t root, const char *dev)
{
	switch_xml_t x_list = NULL;
	hac_cluster_ips_t *cur;

	cur = *cips;
	for(x_list = switch_xml_child(root, "cluster-ips"); x_list; x_list = x_list->next) {
		const char* name = switch_xml_attr(x_list, "name");
		const char* vmac = switch_xml_attr(x_list, "vmac");
		if(switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: clustered ip group has no name\n");
			goto fail;
		} else if(switch_strlen_zero(vmac)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: clustered ip group has no virtual MAC\n");
			goto fail;
		}
		/* TODO: verify virtual MAC address format and that it is
		 * multicast */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: clustered ip discovered: %s (%s)\n", name, vmac);
		if(!switch_strlen_zero(cur->name)) {
			cur->next = switch_core_alloc(config.pool, sizeof(hac_cluster_ips_t));
			cur = cur->next;
		}
		cur->name = name;
		cur->vmac = vmac;
		cur->dev = dev;
		cur->next = NULL;
		cur->ips = switch_core_alloc(config.pool, sizeof(hac_ip_t));
		cur->routes = switch_core_alloc(config.pool, sizeof(hac_route_t));
		if(config_ips(&cur->ips, x_list, dev) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
		if(config_routes(&cur->routes, x_list, dev) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_failover_ips(hac_failover_ips_t **fips, switch_xml_t root, const char *dev)
{
	switch_xml_t x_list = NULL;
	hac_failover_ips_t *cur;

	cur = *fips;
	for(x_list = switch_xml_child(root, "failover-ips"); x_list; x_list = x_list->next) {
		const char* name = switch_xml_attr(x_list, "name");
		const char* nodes = switch_xml_attr(x_list, "nodes");
		if(switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: failover ip missing name attribute\n");
			goto fail;
		}
		/* TODO: evaluate removing nodes attribute from failover-ips */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: failover ip discovered: %s (%s)\n", name, nodes);
		if(!switch_strlen_zero(cur->name)) {
			cur->next = switch_core_alloc(config.pool, sizeof(hac_failover_ips_t));
			cur = cur->next;
		}
		cur->name = name;
		cur->nodes = nodes;
		cur->dev = dev;
		cur->next = NULL;
		cur->ips = switch_core_alloc(config.pool, sizeof(hac_ip_t));
		cur->routes = switch_core_alloc(config.pool, sizeof(hac_route_t));
		if(config_ips(&cur->ips, x_list, dev) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
		if(config_routes(&cur->routes, x_list, dev) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_interfaces(hac_interface_t **ints, switch_xml_t root)
{
	switch_xml_t x_list = NULL;
	hac_interface_t *cur;
	cur = *ints;

	for(x_list = switch_xml_child(root, "interface"); x_list; x_list = x_list->next) {
		const char *name = switch_xml_attr(x_list, "name");
		const char *desc = switch_xml_attr(x_list, "description");
		const char *duplex = switch_xml_attr(x_list, "duplex");
		const char *speed = switch_xml_attr(x_list, "speed");
		if(switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: interface missing name\n");
			goto fail;
		}
		/* TODO: Validate duplex */
		/* TODO: Validate speed */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: interface discovered: %s (%s)\n", name, desc);
		if(!switch_strlen_zero(cur->name)) {
			cur->next = switch_core_alloc(config.pool, sizeof(hac_interface_t));
			cur = cur->next;
		}
		cur->name = name;
		cur->description = desc;
		cur->duplex = duplex;
		cur->speed = speed;
		cur->next = NULL;
		cur->cips = switch_core_alloc(config.pool, sizeof(hac_cluster_ips_t));
		cur->fips = switch_core_alloc(config.pool, sizeof(hac_failover_ips_t));
		cur->ips = switch_core_alloc(config.pool, sizeof(hac_ip_t));
		cur->routes = switch_core_alloc(config.pool, sizeof(hac_route_t));
		cur->arps = switch_core_alloc(config.pool, sizeof(hac_arp_t));
		if(config_cluster_ips(&cur->cips, root, name) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
		if(config_failover_ips(&cur->fips, root, name) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
		if(config_ips(&cur->ips, root, name) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
		if(config_routes(&cur->routes, root, name) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
		if(config_arp(&cur->arps, root, name) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_rules(hac_rule_t** rules, switch_xml_t root)
{
	switch_xml_t x_list = NULL;
	hac_rule_t *cur;

	cur = *rules;
	for(x_list = switch_xml_child(root, "rule"); x_list; x_list = x_list->next) {
		const char* name = switch_xml_attr(x_list, "name");
		const char* type = switch_xml_attr(x_list, "type");
		const char* string = switch_xml_attr(x_list, "string");
		const char* rate = switch_xml_attr(x_list, "rate");
		const char* burst = switch_xml_attr(x_list, "burst");
		const char* mode = switch_xml_attr(x_list, "mode");
		const char* proto = switch_xml_attr(x_list, "proto");
		const char* sport = switch_xml_attr(x_list, "sport");
		const char* dport = switch_xml_attr(x_list, "dport");
		const char* src = switch_xml_attr(x_list, "src");
		const char* srcrange = switch_xml_attr(x_list, "srcrange");
		const char* length = switch_xml_attr(x_list, "length");
		const char* mports = switch_xml_attr(x_list, "mports");
		const char* srcmports = switch_xml_attr(x_list, "srcmports");
		const char* dstmports = switch_xml_attr(x_list, "dstmports");
		const char* state = switch_xml_attr(x_list, "state");
		const char* rejectwith = switch_xml_attr(x_list, "rejectwith");
		const char* mark = switch_xml_attr(x_list, "mark");
		if(switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: rule missing name attribute.\n");
			goto fail;
		}
		if(switch_strlen_zero(type)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: rule missing type attribute.\n");
			goto fail;
		}
		if(strlen(name) > 14) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: rule name must not exceed 14 characters; sorry!\n");
			goto fail;
		}
		if(strlen(string) > 96) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: rule string match must not exceed 96 characters; sorry!\n");
			goto fail;
		}

		/* TODO: Validate rule type */
		/* TODO: Validate rule rate format */
		/* TODO: Validate sanity of burst */
		/* TODO: Validate rule mode */
		/* TODO: Validate rule proto */
		/* TODO: Validate rule sport */
		/* TODO: Validate rule dport */
		/* TODO: Validate src format (DNS or IP) */
		/* TODO: Validate srcrange format */
		/* TODO: Validate sanity of packet length */
		/* TODO: Validate mports format and range */
		/* TODO: Validate srcmports format and ranges */
		/* TODO: Validate dstmports format and ranges */
		/* TODO: Validate state */
		/* TODO: Validate rejectwith */
		/* TODO: Validate sanity of mark */
		/* TODO: Validate we have enough info to generate a full rule */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: rule discovered: %s\n", type);
		if(!switch_strlen_zero(cur->name)) {
			cur->next = switch_core_alloc(config.pool, sizeof(hac_rule_t));
			cur = cur->next;
		}
		cur->name = name;
		cur->type = type;
		cur->string = string;
		cur->rate = rate;
		cur->burst = burst;
		cur->mode = mode;
		cur->proto = proto;
		cur->sport = sport;
		cur->dport = dport;
		cur->src = src;
		cur->srcrange = srcrange;
		cur->length = length;
		cur->mports = mports;
		cur->srcmports = srcmports;
		cur->dstmports = dstmports;
		cur->state = state;
		cur->rejectwith = rejectwith;
		cur->mark = mark;
		cur->next = NULL;
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_rule_groups(hac_rule_group_t** rgroups, switch_xml_t root)
{
	switch_xml_t x_list = NULL;
	hac_rule_group_t *cur;

	cur = *rgroups;
	for(x_list = switch_xml_child(root, "rule-group"); x_list; x_list = x_list->next) {
		const char* name = switch_xml_attr(x_list, "name");
		if(switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: rule group missing name attribute.\n");
			goto fail;
		}
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: rule group discovered: %s\n", name);
		if(!switch_strlen_zero(cur->name)) {
			cur->next = switch_core_alloc(config.pool, sizeof(hac_rule_group_t));
			cur = cur->next;
		}
		cur->name = name;
		cur->next = NULL;
		cur->rules = switch_core_alloc(config.pool, sizeof(hac_rule_t));
		if(config_rules(&cur->rules, x_list) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_failover_groups(hac_failover_group_t** fover, switch_xml_t root)
{
	switch_xml_t x_list = NULL, x_list2 = NULL;
	int cur_inc = 0;
	hac_failover_group_t *cur;

	cur = *fover;
	for(x_list = switch_xml_child(root, "group"); x_list; x_list = x_list->next) {
		const char* name = switch_xml_attr(x_list, "name");
		const char* type = switch_xml_attr(x_list, "type");
		if(switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: failover group missing name attribute.\n");
			goto fail;
		}
		if(switch_strlen_zero(type)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: failover group missing type attribute.\n");
			goto fail;
		}
		/* TODO: Validate failover type */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: failover group discovered: %s (%s)\n", name, type);
		if(!switch_strlen_zero(cur->name)) {
			cur->next = switch_core_alloc(config.pool, sizeof(hac_failover_group_t));
			cur = cur->next;
		}
		cur->name = name;
		cur->include_type = type;
		cur->next = NULL;
		cur->num_includes = 0;
		for(x_list2 = switch_xml_child(x_list, "include"); x_list2; x_list2 = x_list2->next) {
			cur->num_includes++;
		}
		cur->include_names = switch_core_alloc(config.pool, sizeof(const char*) * cur->num_includes);
		for(x_list2 = switch_xml_child(x_list, "include"); x_list2; x_list2 = x_list2->next) {
			cur->include_names[cur_inc] = switch_xml_attr(x_list2, "name");
			if(switch_strlen_zero(cur->include_names[cur_inc])) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: failover group include missing name attribute.\n");
				goto fail;
			}
			cur_inc++;
		}
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_network(switch_xml_t root)
{
	switch_xml_t x_list = NULL;

	if((x_list = switch_xml_child(root, "dos-rules"))) {
		config.network.rgroups = switch_core_alloc(config.pool, sizeof(hac_rule_group_t));
		if(config_rule_groups(&config.network.rgroups, x_list) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
	}
	config.network.interfaces = NULL;
	if((x_list = switch_xml_child(root, "interfaces"))) {
		config.network.interfaces = switch_core_alloc(config.pool, sizeof(hac_interface_t));
		if(config_interfaces(&config.network.interfaces, x_list) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
	}
	if(config.network.interfaces == NULL) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: missing interfaces section in config file\n");
		goto fail;
	}
	if((x_list = switch_xml_child(root, "failover-groups"))) {
		config.network.fgroups = switch_core_alloc(config.pool, sizeof(hac_failover_group_t));
		if(config_failover_groups(&config.network.fgroups, x_list) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_hb(hac_hb_t** hb, switch_xml_t root)
{
	hac_hb_t *cur = NULL;
	const char *interval = NULL, *network = NULL, *address = NULL, *ttl = NULL, *port = NULL;

	cur = *hb;
	interval = switch_xml_attr(root, "interval");
	network = switch_xml_attr(root, "bind-network");
	port = switch_xml_attr(root, "port");
	address = switch_xml_attr(root, "multicast-address");
	ttl = switch_xml_attr(root, "ttl");
	if(switch_strlen_zero(interval)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: Missing interval attributes for heartbeat element\n");
		goto fail;
	}
	if(switch_strlen_zero(network)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: Missing bind-network attributes for heartbeat element\n");
		goto fail;
	}
	if(switch_strlen_zero(address)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: Missing multicast-address attributes for heartbeat element\n");
		goto fail;
	}
	if(switch_strlen_zero(port)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: Missing port attributes for heartbeat element\n");
		goto fail;
	}
	/* TODO: Validate heartbeat interval sanity */
	/* TODO: Validate bind-network address format and existence */
	/* TODO: Sanity check on port */
	/* TODO: Validate multicast-address */
	/* TODO: TTL Sanity check */
	if(!switch_strlen_zero(cur->address)) {
		cur->next = switch_core_alloc(config.pool, sizeof(hac_hb_t));
		cur = cur->next;
	}
	cur->interval = interval;
	cur->network = network;
	cur->address = address;
	cur->port = port;
	cur->lport = atoi(port);
	cur->ttl = ttl;
	cur->next = NULL;
	switch_thread_rwlock_create(&cur->recv_lock, config.pool);
	switch_thread_rwlock_create(&cur->send_lock, config.pool);
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_events(hac_event_match_t** events, switch_xml_t root)
{
	switch_xml_t x_list = NULL;
	hac_event_match_t *cur;

	cur = *events;
	for(x_list = switch_xml_child(root, "match"); x_list; x_list = x_list->next) {
		const char* name = switch_xml_attr(x_list, "name");
		const char* value = switch_xml_attr(x_list, "value");
		if(switch_strlen_zero(name)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: event name is missing.\n");
			goto fail;
		} else if(switch_strlen_zero(value)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: event value is missing.\n");
			goto fail;
		}
		if(!switch_strlen_zero(cur->name)) {
			cur->next = switch_core_alloc(config.pool, sizeof(hac_event_match_t));
			cur = cur->next;
		}
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: failover event match on: %s = %s\n", name, value);
		cur->name = name;
		cur->value = value;
		cur->next = NULL;
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_failover(hac_failover_t** fover, switch_xml_t root)
{
	switch_xml_t x_list = NULL;
	hac_failover_t *cur;

	cur = *fover;
	for(x_list = switch_xml_child(root, "condition"); x_list; x_list = x_list->next) {
		const char* event = switch_xml_attr(x_list, "event");
		if(switch_strlen_zero(event)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: failover event missing in condition.\n");
			goto fail;
		}
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: failover event discovered: %s\n", event);
		if(!switch_strlen_zero(cur->event)) {
			cur->next = switch_core_alloc(config.pool, sizeof(hac_failover_t));
			cur = cur->next;
		}
		cur->event = event;
		cur->next = NULL;
		cur->match_rules = switch_core_alloc(config.pool, sizeof(hac_event_match_t));
		if(config_events(&cur->match_rules, x_list) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t config_general(switch_xml_t root)
{
	switch_xml_t x_list = NULL;
	hac_hb_t *cur = NULL;

	if((x_list = switch_xml_child(root, "nodeid"))) {
		const char* value = NULL;
		value = switch_xml_attr(x_list, "value");
		if(!switch_strlen_zero(value)) {
			config.general.nodeid = value;
			switch_atomic_set(&hac_globals.nodeid, (switch_strlen_zero(value) ? -1 : atoi(value)));
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: Missing nodeid value in general section.\n");
			goto fail;
		}
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: Missing nodeid element in general section.\n");
		goto fail;
	}
	
	if((x_list = switch_xml_child(root, "timer-a"))) {
		const char* value = NULL;
		value = switch_xml_attr(x_list, "value");
		if(!switch_strlen_zero(value)) {
			config.general.timer_a = value;
			switch_atomic_set(&hac_globals.timer_a_dur, (switch_strlen_zero(value) ? 60 : atoi(value)));
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: Missing timer-a value in general section.\n");
			goto fail;
		}
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: Missing timer-a element in general section.\n");
		goto fail;
	}
	
	if((x_list = switch_xml_child(root, "timer-b"))) {
		const char* value = NULL;
		value = switch_xml_attr(x_list, "value");
		if(!switch_strlen_zero(value)) {
			config.general.timer_b = value;
			switch_atomic_set(&hac_globals.timer_b_dur, (switch_strlen_zero(value) ? 60 : atoi(value)));
		}
	}
	
	if((x_list = switch_xml_child(root, "failure-threshold"))) {
		const char* value = NULL;
		value = switch_xml_attr(x_list, "value");
		if(!switch_strlen_zero(value)) {
			config.general.failure_threshold = value;
			switch_atomic_set(&hac_globals.failure_threshold, (switch_strlen_zero(value) ? 5000 : atoi(value)));
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "HAC: Missing failure-threshold value in general section; defaulted to 5000ms.\n");
		}
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "HAC: Missing failure-threshold element in general section; defaulting to 5000ms.\n");
		switch_atomic_set(&hac_globals.failure_threshold, 5000);
	}
	
	config.general.heartbeat = switch_core_alloc(config.pool, sizeof(hac_hb_t));
	cur = config.general.heartbeat;
	for(x_list = switch_xml_child(root, "heartbeat"); x_list; x_list = x_list->next) {
		if(config_hb(&cur, x_list) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
		switch_thread_rwlock_wrlock(hac_globals.hb_lock);
		hac_globals.hb = config.general.heartbeat;
		switch_thread_rwlock_unlock(hac_globals.hb_lock);
	}
	if(switch_strlen_zero(config.general.heartbeat->address)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: Missing heartbeat element in general section.\n");
		goto fail;
	}
	
	if((x_list = switch_xml_child(root, "failover"))) {
		config.general.failover = switch_core_alloc(config.pool, sizeof(hac_failover_t));
		if(config_failover(&config.general.failover, x_list) != SWITCH_STATUS_SUCCESS) {
			goto fail;
		}
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: Missing failover section in general section.\n");
		goto fail;
	}
	return SWITCH_STATUS_SUCCESS;
fail:
	return SWITCH_STATUS_FALSE;
}

switch_status_t do_config(switch_bool_t reload)
{
	switch_xml_t xml = NULL;
	volatile switch_xml_t x_lists = NULL;
	switch_xml_t cfg = NULL;

	if(reload == SWITCH_TRUE) {
		switch_core_destroy_memory_pool(&config.pool);
	}
	switch_core_new_memory_pool(&config.pool);

	if((xml = switch_xml_open_cfg("ha_cluster.conf", &cfg, NULL))) {
		if((x_lists = switch_xml_child(cfg, "network"))) {
			if(config_network(x_lists) != SWITCH_STATUS_SUCCESS) {
				goto failconfig;
			}
		}
		if((x_lists = switch_xml_child(cfg, "general"))) {
			if(config_general(x_lists) != SWITCH_STATUS_SUCCESS) {
				goto failconfig;
			}
		}
	} else {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: Could not open ha_cluster.conf\n");
		goto failconfig;
	}
	switch_xml_free(xml);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: Configuration loaded.\n");
	switch_atomic_set(&hac_globals.state, HAC_PEER_STARTING);
	return SWITCH_STATUS_SUCCESS;

failconfig:
	switch_xml_free(xml);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: Configuration failed to load.\n");
	return SWITCH_STATUS_FALSE;
}

