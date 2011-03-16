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
 * mod_ha_cluster.c 
 *
 * The HA Cluster module implements a dynamic, self-regulating cluster. It
 * will auto-detect the nodes that are part of the cluster, automatically
 * assign master and slave statuses based on a configured master to slave
 * ratio, automatically assign node IDs, propogate registrations and call
 * limiting information between cluster nodes, and automatically transfer
 * calls from failed nodes to slave nodes. If the cluster reaches a state
 * where no slaves are present and a slave can be safely designated from
 * an existing master node, that master will relinquish its master status
 * and become a slave.
 *
 * Most of the cluster operations are contention-free due to pre-defined
 * rules on which node takes what actions based on randomly chosen node
 * weights and IDs. The only contention period is during start-up when
 * choosing random node weights. Because each node tracks the state of all
 * other nodes and maintains a list of their weights, each node can
 * independently determine which node will be taking what actions based on
 * received events. Because they all share the same information, they will
 * all come to the same decisions. 
 *
 * The cluster algorithms require that all nodes are always able to see all
 * other nodes in the cluster. If a network split occurs, it could cause
 * something similar to a "split-brain" condition. This would be a very
 * bad thing.
 *
 */
#include <mod_ha_cluster.h>

/* Prototypes */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_ha_cluster_shutdown);
SWITCH_MODULE_RUNTIME_FUNCTION(mod_ha_cluster_runtime);
SWITCH_MODULE_LOAD_FUNCTION(mod_ha_cluster_load);

/* SWITCH_MODULE_DEFINITION(name, load, shutdown, runtime) 
 * Defines a switch_loadable_module_function_table_t and a static const char[] modname
 */
SWITCH_MODULE_DEFINITION(mod_ha_cluster, mod_ha_cluster_load, mod_ha_cluster_shutdown, mod_ha_cluster_runtime);
/*SWITCH_MODULE_DEFINITION(mod_ha_cluster, mod_ha_cluster_load, mod_ha_cluster_shutdown, NULL); */

unsigned char MAGIC[] = { 226, 132, 177, 197, 152, 198, 142, 211, 172, 197, 158, 208, 169, 208, 135, 197, 166, 207, 154, 196, 166 };
hac_config_t config;
struct s_global hac_globals;

/* TODO: Make this notify the admin of the system */
static void hac_notify_admin(const char* mesg)
{

}

static switch_status_t send_event(switch_event_t* event, switch_bool_t hb)
{
	size_t len;
	char *packet = NULL, *buf = NULL;
	hac_hb_t *cur_hb = NULL;
	struct timeval cur_time;
	size_t tstamp = 0, tstamp_hb = 0;
	switch_uuid_t uuid;
	char uuid_str[SWITCH_UUID_FORMATTED_LENGTH + 1];

	if(hb == SWITCH_TRUE) {
		gettimeofday(&cur_time, NULL);
		tstamp = cur_time.tv_sec * 1000;
		tstamp += cur_time.tv_usec / 1000;
	}

	switch_uuid_get(&uuid);
	switch_uuid_format(uuid_str, &uuid);
	switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "HAC-Event-UUID", uuid_str);

	if(switch_event_serialize(event, &packet, SWITCH_TRUE) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "HAC: Failed to serialize event for sending!\n");
		goto failsend;
	}
	len = strlen(packet) + strlen((char*) MAGIC);
	buf = malloc(len + 1);
	memset(buf, 0, len + 1);
	switch_assert(buf);
	switch_copy_string(buf, packet, len);
	switch_copy_string(buf + strlen(packet), (char*)MAGIC, strlen((char*)MAGIC) + 1);

	/* Only one thread can write to the socket at one time, and we don't want
	   them changing in the middle of writes */
	switch_thread_rwlock_wrlock(hac_globals.lock);
	cur_hb = hac_globals.hb;
	while(cur_hb != NULL) {
		if(hb == SWITCH_TRUE) {
			tstamp_hb = cur_hb->last_hb.tv_sec * 1000;
			tstamp_hb += cur_hb->last_hb.tv_usec / 1000;
			if(tstamp > tstamp_hb + atoi(cur_hb->interval)) {
				switch_socket_sendto(cur_hb->udp_sock, cur_hb->addr, 0, buf, &len);
				cur_hb->last_hb.tv_sec = cur_time.tv_sec;
				cur_hb->last_hb.tv_usec = cur_time.tv_usec;
			}
		} else {
			switch_socket_sendto(cur_hb->udp_sock, cur_hb->addr, 0, buf, &len);
		}
		cur_hb = cur_hb->next;
	}
	switch_thread_rwlock_unlock(hac_globals.lock);

	switch_safe_free(packet);
	switch_safe_free(buf);

	return SWITCH_STATUS_SUCCESS;
failsend:
	return SWITCH_STATUS_FALSE;
}
/*
static void clear_event_hash()
{
	switch_hash_index_t *hi = NULL;
	void *val;
	hac_event_t *hevent;

	for(hi = switch_hash_first(NULL, hac_globals.event_hash); hi; hi = switch_hash_next(hi)) {
		switch_hash_this(hi, NULL, NULL, &val);
		if(val) {
			hevent = (hac_event_t*)val;
			
		}
		switch_safe_free(val);
	}
	
}

static void receive_events()
{
	//switch_event_t *event = NULL;
	char *buf = NULL;//, *myaddr = NULL, *packet = NULL;
	switch_sockaddr_t *addr;
	size_t len = HAC_BUFFSIZE;
	switch_status_t status;
	hac_hb_t *cur_hb = NULL;

	buf = (char*) malloc(HAC_BUFFSIZE);
	switch_assert(buf);
	memset(buf, 0, len);

	switch_sockaddr_info_get(&addr, NULL, SWITCH_UNSPEC, 0, 0, hac_globals.pool);
	
	cur_hb = hac_globals.hb;
	while(cur_hb != NULL && hac_globals.mod_state == HAC_LOOPING) {
		/ * Read up to 1000 events from each multicast group at a time */
/*		size_t count = 0;
		while((status = switch_socket_recvfrom(addr, cur_hb->udp_sock, 0, buf, &len)) == SWITCH_STATUS_SUCCESS && len && count < 1000) {
			count++;
		}
		cur_hb = cur_hb->next;
	}
}
*/
static void send_hb()
{
	switch_event_t *local_event;
	if (switch_event_create_subclass(&local_event, SWITCH_EVENT_CUSTOM, HAC_HEARTBEAT) == SWITCH_STATUS_SUCCESS) {
		char buf[256];
		memset(buf, 0, 256);

		switch_thread_rwlock_rdlock(hac_globals.lock);
		switch_event_add_header_string(local_event, SWITCH_STACK_BOTTOM, "HAC-Peer", hac_globals.hostname);
		sprintf(buf, "%d", hac_globals.nodeid);
		switch_event_add_header_string(local_event, SWITCH_STACK_BOTTOM, "HAC-Node-ID", buf);
		memset(buf, 0, 256);
		sprintf(buf, "%ld", hac_globals.rand_val);
		switch_event_add_header_string(local_event, SWITCH_STACK_BOTTOM, "HAC-Priority", buf);
		switch(hac_globals.state) {
			case HAC_PEER_STARTING:
				switch_event_add_header_string(local_event, SWITCH_STACK_BOTTOM, "State", "STARTING");
				break;
			case HAC_PEER_MASTER:
				switch_event_add_header_string(local_event, SWITCH_STACK_BOTTOM, "State", "MASTER");
				break;
			case HAC_PEER_SLAVE:
				switch_event_add_header_string(local_event, SWITCH_STACK_BOTTOM, "State", "SLAVE");
				break;
			case HAC_PEER_FAILED_MASTER:
				switch_event_add_header_string(local_event, SWITCH_STACK_BOTTOM, "State", "FAILED_MASTER");
				break;
			case HAC_PEER_FAILED_SLAVE:
				switch_event_add_header_string(local_event, SWITCH_STACK_BOTTOM, "State", "FAILED_SLAVE");
				break;
			case HAC_PEER_STOPPING_MASTER:
				switch_event_add_header_string(local_event, SWITCH_STACK_BOTTOM, "State", "STOPPING_MASTER");
				break;
			case HAC_PEER_STOPPING_SLAVE:
				switch_event_add_header_string(local_event, SWITCH_STACK_BOTTOM, "State", "STOPPING_SLAVE");
				break;
			case HAC_PEER_UNKNOWN:
			default:
				switch_event_add_header_string(local_event, SWITCH_STACK_BOTTOM, "State", "UNKNOWN");
				break;
		}
		switch_thread_rwlock_unlock(hac_globals.lock);

		if(send_event(local_event, SWITCH_TRUE) != SWITCH_STATUS_SUCCESS) {
			hac_notify_admin("Failed to send heartbeat.");
		}
	}
}

static void starting_event_handler(switch_event_t *event)
{

}

static void master_event_handler(switch_event_t *event)
{

}

static void slave_event_handler(switch_event_t *event)
{

	/* on a takover, load sofia */
/*	if (switch_loadable_module_load_module((char *) SWITCH_GLOBAL_dirs.mod_dir, "mod_sofia", SWITCH_TRUE, &err) != SWITCH_STATUS_SUCCESS) {

	}*/
}

static void failed_master_event_handler(switch_event_t *event)
{

}

static void failed_slave_event_handler(switch_event_t *event)
{

}

static void stopping_master_event_handler(switch_event_t *event)
{

}

static void stopping_slave_event_handler(switch_event_t *event)
{

}

static void event_handler(switch_event_t *event)
{
	hac_peer_state_t state;
	switch_thread_rwlock_rdlock(hac_globals.lock);
	state = hac_globals.state;
	switch_thread_rwlock_unlock(hac_globals.lock);

	switch(state) {
		case HAC_PEER_STARTING:
			starting_event_handler(event);
			break;
		case HAC_PEER_MASTER:
			master_event_handler(event);
			break;
		case HAC_PEER_SLAVE:
			slave_event_handler(event);
			break;
		case HAC_PEER_FAILED_MASTER:
			failed_master_event_handler(event);
			break;
		case HAC_PEER_FAILED_SLAVE:
			failed_slave_event_handler(event);
			break;
		case HAC_PEER_STOPPING_MASTER:
			stopping_master_event_handler(event);
			break;
		case HAC_PEER_STOPPING_SLAVE:
			stopping_slave_event_handler(event);
			break;
		default:
			break;
	}
}

SWITCH_STANDARD_API(ha_cluster_function)
{
	do_config(SWITCH_TRUE);

	return SWITCH_STATUS_SUCCESS;
}

/* Macro expands to: switch_status_t mod_ha_cluster_load(switch_loadable_module_interface_t **module_interface, switch_memory_pool_t *pool) */
SWITCH_MODULE_LOAD_FUNCTION(mod_ha_cluster_load)
{
	switch_api_interface_t *api_interface;
	switch_status_t status;
	sigset_t set, oset;
	hac_hb_t *cur_hb;

	sigemptyset(&set);
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGPIPE);
	sigprocmask(SIG_BLOCK, &set, &oset);

	memset(&hac_globals, 0, sizeof(hac_globals));
	hac_globals.state = HAC_PEER_STARTING;
	hac_globals.mod_state = HAC_LOOPING;
	hac_globals.pool = pool;
	gethostname(hac_globals.hostname, sizeof(hac_globals.hostname));

	switch_thread_rwlock_create(&hac_globals.lock, pool);
	switch_core_new_memory_pool(&hac_globals.event_pool);
	switch_core_hash_init(&hac_globals.event_hash, pool);
	switch_core_hash_init(&hac_globals.peer_hash, pool);

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "HAC: Loading configuration...\n");

	if(do_config(SWITCH_FALSE) == SWITCH_STATUS_FALSE) {
		switch_goto_status(SWITCH_STATUS_TERM, fail);
	}

	cur_hb = hac_globals.hb;
	while(cur_hb != NULL) {
		char *ip;

		if (switch_sockaddr_info_get(&cur_hb->addr, cur_hb->address, SWITCH_UNSPEC, cur_hb->lport, 0, hac_globals.pool) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Cannot find address.\n");
			switch_goto_status(SWITCH_STATUS_TERM, fail);
		}

		if (switch_sockaddr_info_get(&cur_hb->iface, cur_hb->network, SWITCH_UNSPEC, cur_hb->lport, 0, hac_globals.pool) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Cannot find address.\n");
			switch_goto_status(SWITCH_STATUS_TERM, fail);
		}
		switch_sockaddr_ip_get(&ip, cur_hb->iface);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "HAC: IP of Multicast Interface: %s\n", ip);

		if(switch_socket_create(&cur_hb->udp_sock, AF_INET, SOCK_DGRAM, 0, hac_globals.pool) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Socket Error\n");
			switch_goto_status(SWITCH_STATUS_TERM, fail);
		}
	
		if(switch_socket_opt_set(cur_hb->udp_sock, SWITCH_SO_REUSEADDR, 1) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Socket Option Error\n");
			switch_goto_status(SWITCH_STATUS_TERM, fail);
		}

		if(switch_mcast_join(cur_hb->udp_sock, cur_hb->addr, cur_hb->iface, NULL) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Multicast Error\n");
			switch_goto_status(SWITCH_STATUS_TERM, fail);
		}

		if(switch_mcast_hops(cur_hb->udp_sock, (switch_strlen_zero(cur_hb->ttl) ? 1 : atoi(cur_hb->ttl))) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Failed to set ttl to '%s'\n", cur_hb->ttl);
			switch_goto_status(SWITCH_STATUS_TERM, fail);
		}
	
		if(switch_socket_bind(cur_hb->udp_sock, cur_hb->iface) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Bind Error\n");
			switch_goto_status(SWITCH_STATUS_TERM, fail);
		}

		cur_hb = cur_hb->next;
	}

	if(switch_event_reserve_subclass(HAC_HEARTBEAT) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't register subclass %s!\n", HAC_HEARTBEAT);
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}
	if(switch_event_reserve_subclass(HAC_LIMIT_UP) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't register subclass %s!\n", HAC_LIMIT_UP);
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}
	if(switch_event_reserve_subclass(HAC_LIMIT_DOWN) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't register subclass %s!\n", HAC_LIMIT_DOWN);
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}
	if(switch_event_reserve_subclass(HAC_REGISTER) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't register subclass %s!\n", HAC_REGISTER);
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}
	if(switch_event_reserve_subclass(HAC_UNREGISTER) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't register subclass %s!\n", HAC_UNREGISTER);
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}
	if(switch_event_reserve_subclass(HAC_STARTUP) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't register subclass %s!\n", HAC_STARTUP);
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}
	if(switch_event_reserve_subclass(HAC_SHUTDOWN) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't register subclass %s!\n", HAC_SHUTDOWN);
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}
	if(switch_event_reserve_subclass(HAC_FAILURE) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't register subclass %s!\n", HAC_FAILURE);
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}
	if(switch_event_reserve_subclass(HAC_CHOSE_RANDOM) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't register subclass %s!\n", HAC_CHOSE_RANDOM);
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}
	if(switch_event_reserve_subclass(HAC_TAKEOVER) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't register subclass %s!\n", HAC_TAKEOVER);
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}
	if(switch_event_reserve_subclass(HAC_RELINQUISH) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't register subclass %s!\n", HAC_RELINQUISH);
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}
	if (switch_event_bind(modname, SWITCH_EVENT_ALL, SWITCH_EVENT_SUBCLASS_ANY, event_handler, NULL) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "HAC: Couldn't bind to events.\n");
		switch_goto_status(SWITCH_STATUS_GENERR, fail);
	}

	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_API(api_interface, "ha_cluster", "HA Cluster API", ha_cluster_function, "syntax");

	sigprocmask(SIG_SETMASK, &oset, &set);

	return SWITCH_STATUS_SUCCESS;

fail:
	sigprocmask(SIG_SETMASK, &oset, &set);

	cur_hb = hac_globals.hb;
	while(cur_hb != NULL) {
		if (cur_hb->udp_sock) {
			switch_socket_close(cur_hb->udp_sock);
		}
		cur_hb = cur_hb->next;
	}

	switch_event_free_subclass(HAC_LIMIT_UP);
	switch_event_free_subclass(HAC_LIMIT_DOWN);
	switch_event_free_subclass(HAC_REGISTER);
	switch_event_free_subclass(HAC_UNREGISTER);
	switch_event_free_subclass(HAC_STARTUP);
	switch_event_free_subclass(HAC_SHUTDOWN);
	switch_event_free_subclass(HAC_FAILURE);
	switch_event_free_subclass(HAC_CHOSE_RANDOM);
	switch_event_free_subclass(HAC_TAKEOVER);
	switch_event_free_subclass(HAC_RELINQUISH);

	return status;
}

/*
  Called when the system shuts down
  Macro expands to: switch_status_t mod_ha_cluster_shutdown() */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_ha_cluster_shutdown)
{
	/* Cleanup dynamically allocated config settings */
	/*switch_xml_config_cleanup(instructions);*/
	hac_hb_t *cur_hb = NULL;

	hac_globals.mod_state = HAC_SHUTDOWN_RUNTIME;
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Waiting for runtime thread...\n");
	while(hac_globals.mod_state == HAC_SHUTDOWN_RUNTIME) switch_yield(1000);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Commencing module cleanup...\n");

	switch_core_hash_destroy(&hac_globals.event_hash);
	switch_core_hash_destroy(&hac_globals.peer_hash);

	cur_hb = hac_globals.hb;
	while(cur_hb != NULL) {
		if (cur_hb->udp_sock) {
			switch_socket_close(cur_hb->udp_sock);
		}
		cur_hb = cur_hb->next;
	}

	switch_event_free_subclass(HAC_LIMIT_UP);
	switch_event_free_subclass(HAC_LIMIT_DOWN);
	switch_event_free_subclass(HAC_REGISTER);
	switch_event_free_subclass(HAC_UNREGISTER);
	switch_event_free_subclass(HAC_STARTUP);
	switch_event_free_subclass(HAC_SHUTDOWN);
	switch_event_free_subclass(HAC_FAILURE);
	switch_event_free_subclass(HAC_CHOSE_RANDOM);
	switch_event_free_subclass(HAC_TAKEOVER);
	switch_event_free_subclass(HAC_RELINQUISH);


	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Unloading module...\n");
	return SWITCH_STATUS_SUCCESS;
}


/*
  If it exists, this is called in it's own thread when the module-load completes
  If it returns anything but SWITCH_STATUS_TERM it will be called again automatically
  Macro expands to: switch_status_t mod_ha_cluster_runtime()
*/
SWITCH_MODULE_RUNTIME_FUNCTION(mod_ha_cluster_runtime)
{
	int arg = 0;
	switch_yield(500000);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Starting processing...\n");
	hac_globals.mod_state = HAC_LOOPING;
	if(initialize_interfaces() != SWITCH_STATUS_SUCCESS) {
		switch_core_session_ctl(SCSC_SHUTDOWN_NOW, &arg);
	}
	if(initialize_firewall() != SWITCH_STATUS_SUCCESS) {
		switch_core_session_ctl(SCSC_SHUTDOWN_NOW, &arg);
	}
	while(hac_globals.mod_state == HAC_LOOPING)
	{
		/*switch_cond_next();*/
		switch_yield(1000);
		send_hb();
	}
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Shutting down...\n");
	hac_globals.mod_state = HAC_UNLOAD;
	return SWITCH_STATUS_TERM;
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
