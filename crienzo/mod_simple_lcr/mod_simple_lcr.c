/*
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2012, Anthony Minessale II <anthm@freeswitch.org>
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
 * Christopher M. Rienzo <chris@rienzo.com>
 * Ken Rice <krice@rmktek.com> (dialplan and LCR ideas based on mod_biwara)
 *
 * Maintainer: Christopher M. Rienzo <chris@rienzo.com>
 *
 * mod_simple_lcr.c -- Simple in-memory LCR.
 *
 */
#include <switch.h>
#include "route_trie.h"


SWITCH_MODULE_LOAD_FUNCTION(mod_simple_lcr_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_simple_lcr_shutdown);
SWITCH_MODULE_DEFINITION(mod_simple_lcr, mod_simple_lcr_load, mod_simple_lcr_shutdown, NULL);

#define MAX_CARRIERS 256
#define MAX_GATEWAYS 8
#define MAX_ROUTES 8
#define MAX_INTER_AREAS 16
#define DEFAULT_MAX_ROUTES 3


/**
 * A carrier gateway
 */
struct gateway {
	/** prefix to prepend to dial string */
	char *prefix;
	/** suffix to append to dial string */
	char *suffix;
};

/**
 * A carrier to route calls to
 */
struct carrier {
	/** ID of this carrier */
	route_value_t id;
	/** The name of the carrier */
	char *name;
	/** The number of gateways */
	int num_gateways;
	/** The carrier gateway addresses */
	struct gateway *gateway[MAX_GATEWAYS];
	/** Load balancing synchronization */
	switch_mutex_t *mutex;
	/** Next gateway to select */
	int next_gateway;
};

/**
 * LCR database
 */
struct lcr {
	/** memory pool */
	switch_memory_pool_t *pool;
	/** The carrier inter-area route prefix tree */
	struct route_trie *inter_trie;
	/** The carrier intra-area route prefix tree */
	struct route_trie *intra_trie;
	/** map of prefixes to area */
	struct route_trie *areas;
	/** Caller ID prefixes that will always be rated as inter calls */
	struct route_trie *inter_areas;
	/** The carriers. 1-based index */
	struct carrier *carriers[MAX_CARRIERS + 1];
	/** The number of carriers stored in this database */
	int num_carriers;
	/** Maximum number of routes to return on queries */
	int max_routes;
	/** Digits to strip from caller ID when looking up area */
	int areas_caller_id_strip;
	/** Digits to prefix to caller ID when looking up area */
	char *areas_caller_id_prefix;
	/** Digits to strip from dialed number when looking up area */
	int areas_dest_strip;
	/** Digits to prefix to caller ID when looking up area */
	char *areas_dest_prefix;
	/** prevents destruction of database while readers exist */
	switch_thread_rwlock_t *mutex;
};

/**
 * Module global variables
 */
static struct {
	/** synchronizes access to the LCR database pointer */
	switch_mutex_t *update_lock;
	/** lcr database */
	struct lcr *lcr;
} globals;

/**
 * Create a new carrier gateway
 * @param pool the memory pool to use
 * @param prefix the prefix to prepend to dial strings
 * @param suffix the suffix to append to dial strings
 * @return the gateway
 */
static struct gateway *gateway_create(switch_memory_pool_t *pool, const char *prefix, const char *suffix)
{
	struct gateway *gateway = switch_core_alloc(pool, sizeof(*gateway));
	gateway->prefix = switch_core_strdup(pool, prefix);
	gateway->suffix = switch_core_strdup(pool, suffix);
	return gateway;
}

/**
 * Build a dial string for this gateway
 * @param gateway the gateway
 * @param buf the string to fill
 * @param max_len the buf length
 * @param dial_digits the digits to dial
 * @return SWITCH_STATUS_SUCCESS if succesful
 */
static switch_status_t gateway_get_dialstring(struct gateway *gateway, char *buf, size_t max_len, const char *dial_digits)
{
	if (snprintf(buf, max_len, "%s%s%s", gateway->prefix, dial_digits, gateway->suffix) > max_len) {
		return SWITCH_STATUS_FALSE;
	}
	return SWITCH_STATUS_SUCCESS;
}

/**
 * Create a new carrier to route calls to
 * @param pool the memory pool to use
 * @param name the name of the carrier
 * @param id the carrier ID
 * @return the carrier
 */
static struct carrier *carrier_create(switch_memory_pool_t *pool, const char *name, route_value_t id)
{
	struct carrier *carrier = switch_core_alloc(pool, sizeof(*carrier));
	carrier->name = switch_core_strdup(pool, name);
	carrier->id = id;
	if (switch_mutex_init(&carrier->mutex, SWITCH_MUTEX_UNNESTED, pool) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to create carrier mutex!\n");
		return NULL;
	}
	return carrier;
}

/**
 * Add a gateway to a carrier
 * @param pool the memory pool to use
 * @param carrier the carrier
 * @param prefix the gateway prefix
 * @param suffix the gateway suffix
 * @return SWITCH_STATUS_SUCCESS if successful
 */
static switch_status_t carrier_add_gateway(switch_memory_pool_t *pool, struct carrier *carrier, const char *prefix, const char *suffix)
{
	if (carrier->num_gateways < MAX_GATEWAYS) {
		carrier->gateway[carrier->num_gateways] = gateway_create(pool, prefix, suffix);
		carrier->num_gateways++;
		return SWITCH_STATUS_SUCCESS;
	}
	return SWITCH_STATUS_FALSE;
}

/**
 * Pick a gateway round-robin from a carrier
 * @param carrier the carrier to pick a gateway from 
 * @return gateway the gateway
 */
static struct gateway *carrier_select_gateway(struct carrier *carrier)
{
	struct gateway *gateway = NULL;
	if (carrier->num_gateways > 0) {
		switch_mutex_lock(carrier->mutex);
		gateway = carrier->gateway[carrier->next_gateway];
		carrier->next_gateway = (carrier->next_gateway + 1) % carrier->num_gateways;
		switch_mutex_unlock(carrier->mutex);
	}
	return gateway;
}

/**
 * Build a dial string for this carrier
 * @param carrier the carrier
 * @param buf the string to fill
 * @param max_len the buf length
 * @param dial_digits the digits to dial
 * @return SWITCH_STATUS_SUCCESS if succesful
 */
static switch_status_t carrier_get_dialstring(struct carrier *carrier, char *buf, size_t max_len, const char *dial_digits)
{
	struct gateway *gateway = carrier_select_gateway(carrier);
	if (gateway) {
		return gateway_get_dialstring(gateway, buf, max_len, dial_digits);
	}
	return SWITCH_STATUS_FALSE;
}

/**
 * Get read access to LCR database
 * @return LCR database
 */
static struct lcr *lcr_rdlock(void)
{
	struct lcr *lcr;
	switch_mutex_lock(globals.update_lock);
	lcr = globals.lcr;
	switch_thread_rwlock_rdlock(lcr->mutex);
	switch_mutex_unlock(globals.update_lock);
	return lcr;
}

/**
 * Release access to LCR database
 */
static void lcr_unlock(struct lcr *lcr)
{
	if (lcr) {
		switch_thread_rwlock_unlock(lcr->mutex);
	}
}

/**
 * Replace the LCR database
 * @param new_lcr the new LCR database
 * @return lcr the old LCR database
 */
static struct lcr *lcr_replace(struct lcr *new_lcr)
{
	struct lcr *old_lcr;
	switch_mutex_lock(globals.update_lock);
	old_lcr = globals.lcr;
	globals.lcr = new_lcr;
	switch_mutex_unlock(globals.update_lock);
	return old_lcr;
}

/**
 * Create a new LCR database
 * @param num_carriers the maximum number of carriers to store
 * @return the LCR database
 */
static struct lcr *lcr_create(int num_carriers)
{
	switch_memory_pool_t *pool = NULL;
	struct lcr *lcr = NULL;
	if (switch_core_new_memory_pool(&pool) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to create LCR db memory pool!\n");
		return NULL;
	}
	lcr = switch_core_alloc(pool, sizeof(*lcr));
	lcr->pool = pool;
	if (switch_thread_rwlock_create(&lcr->mutex, lcr->pool) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to create LCR mutex!\n");
		return NULL;
	}
	lcr->max_routes = DEFAULT_MAX_ROUTES;
	lcr->num_carriers = num_carriers;
	lcr->inter_trie = route_trie_create(num_carriers);
	lcr->intra_trie = route_trie_create(num_carriers);
	lcr->areas = route_trie_create(1);
	lcr->areas_caller_id_strip = 0;
	lcr->areas_caller_id_prefix = "";
	lcr->areas_dest_strip = 1;
	lcr->areas_dest_prefix = "";
	lcr->inter_areas = route_trie_create(num_carriers);
	return lcr;
}

/**
 * Set the maximum number of routes to return on lookup
 * @param lcr the LCR database
 * @param max_routes the maximum number of routes to return
 */
static void lcr_set_max_routes(struct lcr *lcr, int max_routes)
{
	if (max_routes >= MAX_ROUTES) {
		lcr->max_routes = MAX_ROUTES;
	} else if (max_routes >= 1) {
		lcr->max_routes = max_routes;
	}
}

/**
 * Search for call area
 * @param lcr the LCR database
 * @param lookup_number the number to check
 * @return area ID or -1
 */
static int lcr_find_area(struct lcr *lcr, const char *lookup_number)
{
	int area_id = -1;
	struct route_array *result = NULL;
	
	if (zstr(lookup_number)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Skipping due to empty lookup_number\n");
		return -1;
	}

	/* search prefix tree for area ID */
	result = route_array_create(1);
	if (route_trie_find(lcr->areas, lookup_number, result)) {
		struct route *route = route_array_get_route(result, 0);
		area_id = route_get_value(route);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG1, "Found area %d\n", area_id);
	}
	route_array_destroy(result);

	return area_id;
}

/**
 * Check if call falls in an inter or intra area
 * @param lcr the LCR database
 * @param caller_id the caller ID
 * @param dest_number the number to dial
 * @return 1 if inter-area, 0 if intra-area
 */
static int lcr_is_inter_area(struct lcr *lcr, const char *caller_id, const char *dest_number)
{
	char *lcaller_id = NULL;
	char *ldest_number = NULL;
	int src_area = -1;
	int dest_area = -1;

	/* check input and settings */
	if (zstr(caller_id) || lcr->areas_caller_id_strip >= strlen(caller_id) || lcr->areas_caller_id_strip < 0) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG1, "Skipping due to bad caller-id input\n");
		return 0;
	}
	if (zstr(dest_number) || lcr->areas_dest_strip >= strlen(dest_number) || lcr->areas_dest_strip < 0) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG1, "Skipping due to bad dest input\n");
		return 0;
	}

	/* strip/add prefix to caller ID */
	caller_id += lcr->areas_caller_id_strip;
	if (!zstr(lcr->areas_caller_id_prefix)) {
		lcaller_id = switch_mprintf("%s%s", lcr->areas_caller_id_prefix, caller_id);
		caller_id = lcaller_id;
	}
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG1, "Caller ID = %s\n", caller_id);

	/* strip/add prefix to dest number */
	dest_number += lcr->areas_dest_strip;
	if (!zstr(lcr->areas_dest_prefix)) {
		ldest_number = switch_mprintf("%s%s", lcr->areas_dest_prefix, dest_number);
		dest_number = ldest_number;
	}
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG1, "Dest = %s\n", dest_number);

	/* get areas from caller ID and destination number.  If they are different, it is inter-area */
	src_area = lcr_find_area(lcr, caller_id);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG1, "Src area = %d\n", src_area);
	dest_area = lcr_find_area(lcr, dest_number);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG1, "Dest area = %d\n", dest_area);
	switch_safe_free(lcaller_id);
	switch_safe_free(ldest_number);
	return src_area >= 0 && dest_area >= 0 && src_area != dest_area;
}

/**
 * Search an LCR database for best routes
 * @param lcr the LCR database
 * @param caller_id the caller ID
 * @param dest_number the number to dial
 * @param results the search results
 * @return the number of matches
 */
static int lcr_find(struct lcr *lcr, const char *caller_id, const char *dest_number, struct route_array *results)
{
	int num_inter_carriers = 0;

	if (zstr(dest_number)) {
		return 0;
	}

	/* strip leading + */
	if (*dest_number == '+') {
		dest_number++;
	}

	/* strip leading + */
	if (!zstr(caller_id) && *caller_id == '+') {
		caller_id++;
	}

	/* do lookup, determining if inter/intra then searching the respective trie */
	if (lcr_is_inter_area(lcr, caller_id, dest_number)) {
		/* inter-routing */
		route_trie_find(lcr->inter_trie, dest_number, results);
	} else if ((num_inter_carriers = route_trie_find_count(lcr->inter_areas, caller_id)) >= lcr->num_carriers) {
		/* inter-routing (all carriers treat caller ID as inter) */
		route_trie_find(lcr->inter_trie, dest_number, results);
	} else if (num_inter_carriers == 0) {
		/* intra-routing */
		route_trie_find(lcr->intra_trie, dest_number, results);
	} else {
		int num_routes = 0;
		/* mixed routing */
		struct route_array *inter_carriers = route_array_create(num_inter_carriers);
		struct route_array *inter_results = route_array_create(lcr->num_carriers);
		struct route_array *intra_results = route_array_create(lcr->num_carriers);

		/* which carriers are inter? */
		route_trie_find(lcr->inter_areas, caller_id, inter_carriers);

		/* add intra results */
		num_routes = route_trie_find(lcr->intra_trie, dest_number, intra_results);
		for (int i = 0; i < num_routes; i++) {
			struct route *route = route_array_get_route(intra_results, i);
			/* if route is intra and carrier is intra, add it */
			if (!route_get_type(route) && !route_array_contains(inter_carriers, route_get_value(route))) {
				route_array_add(results, route);
			}
		}

		/* add inter results */
		num_routes = route_trie_find(lcr->inter_trie, dest_number, inter_results);
		for (int i = 0; i < num_routes; i++) {
			struct route *route = route_array_get_route(inter_results, i);
			/* if route is inter and carrier is inter, add it */
			if (route_get_type(route) && route_array_contains(inter_carriers, route_get_value(route))) {
				route_array_add(results, route);
			}
		}

		route_array_destroy(inter_carriers);
		route_array_destroy(intra_results);
		route_array_destroy(inter_results);
	}
	return route_array_get_size(results);
}

/**
 * Destroy an LCR database
 * @param lcr the LCR database
 */
static void lcr_destroy(struct lcr *lcr)
{
	if (lcr) {
		/* wait for all readers to finish */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Waiting to destroy LCR db\n");
		switch_thread_rwlock_wrlock(lcr->mutex);

		/* destroy routes */
		route_trie_destroy(lcr->inter_trie);
		route_trie_destroy(lcr->intra_trie);

		/* destroy area map */
		route_trie_destroy(lcr->areas);

		/* destroy inter-area map */
		route_trie_destroy(lcr->inter_areas);

		/* destroy LCR carriers/gateways */
		switch_core_destroy_memory_pool(&lcr->pool);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "LCR db destroyed\n");
	}
}

/**
 * Concatenate LCR routes into a dial string
 * @param buf the buffer to hold the dial string
 * @param buf_len the buffer length
 * @param sep the dial string separator ',', '|', etc.
 * @param routes the routes
 * @param dial_digits the digits to dial
 * @return SWITCH_STATUS_SUCCESS if there is enough buffer space for the dial string
 */
static switch_status_t build_dialstring_list(char *buf, size_t buf_len, char sep, struct lcr *lcr, struct route_array *routes, const char *dial_digits)
{
	int offset = 0;
	int i;
	struct route *route = route_array_get_route(routes, 0);
	route_value_t carrier_id = route_get_value(route);
	struct carrier *carrier = lcr->carriers[carrier_id];
	
	if (carrier_get_dialstring(carrier, buf, buf_len, dial_digits) > buf_len) {
		return SWITCH_STATUS_FALSE;
	}
	for (i = 1; i < route_array_get_size(routes); i++) {
		struct route *route = route_array_get_route(routes, i);
		carrier_id = route_get_value(route);
		carrier = lcr->carriers[carrier_id];
		offset += strlen(buf + offset);
		if (buf_len - offset > 0) {
			buf[offset++] = sep;
		} else {
			return SWITCH_STATUS_FALSE;
		}
		if (carrier_get_dialstring(carrier, buf + offset, buf_len - offset, dial_digits) > buf_len - offset) {
			return SWITCH_STATUS_FALSE;
		}
	}
	return SWITCH_STATUS_SUCCESS;
}

/**
 * Read line from file, trimming the newline character
 * @param line the buffer to fill with the line
 * @param size the size of the buffer
 * @param in the FILE to read
 * @return the line or NULL if end of file
 */
static char *fgets_trimmed(char *line, int size, FILE *in)
{
	char *retval = fgets(line, 255, in);
	if (retval) {
		/* trim newline */
		char *pos;
		if ((pos = strchr(line, '\n')) != NULL) {
			*pos = '\0';
		}
	}
	return retval;
}

/**
 * Load a area CSV file
 * @param area_trie the area database to add to
 * @param area the area file name
 * @return SWITCH_STATUS_SUCCESS if successful
 */
static switch_status_t load_area_file(struct route_trie *area_trie, const char *area)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	route_value_t next_id = 0;
	switch_hash_t *area_hash = NULL;
	switch_memory_pool_t *pool = NULL;
	int line_num = 0;
	char line[256];
	FILE *in = fopen(area, "r");
	if (!in) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to open area file %s\n", area);
		status = SWITCH_STATUS_FALSE;
		goto done;
	}

	if (switch_core_new_memory_pool(&pool) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to create area ID memory pool!\n");
		status = SWITCH_STATUS_FALSE;
		goto done;
	}

	/* maps area external IDs to internal IDs */
	switch_core_hash_init_case(&area_hash, pool, SWITCH_TRUE);

	line[255] = '\0';
	while (fgets_trimmed(line, 255, in)) {
		char *field[3];
		line_num++;
		if (switch_separate_string(line, ',', field, 3) >= 2) {
			char *digits = field[0];
			char *ext_id = field[1];
			/* find internal ID for the external ID */
			route_value_t *id_ptr = switch_core_hash_find(area_hash, ext_id);
			if (!id_ptr) {
				/* assign new internal ID to this area external ID */
				id_ptr = switch_core_alloc(pool, sizeof(*id_ptr));
				*id_ptr = ++next_id;
				switch_core_hash_insert(area_hash, switch_core_strdup(pool, ext_id), id_ptr);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "%s: new area %s => %d\n", area, ext_id, *id_ptr);
			}

			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "%s: mapping prefix %s, %s => %d\n", area, digits, ext_id, *id_ptr);
			/* map digits to internal ID */
			if (!route_trie_add(area_trie, digits, 0, *id_ptr, 0)) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "%s: Bad input on line %i!\n", area, line_num);
				status = SWITCH_STATUS_FALSE;
				goto done;
			}
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "%s: Bad input on line %i!\n", area, line_num);
			status = SWITCH_STATUS_FALSE;
			goto done;
		}
	}

done:

	if (area_hash) {
		switch_core_hash_destroy(&area_hash);
	}
	if (pool) {
		switch_core_destroy_memory_pool(&pool);
	}
	if (in) {
		fclose(in);
	}
	return status;
}

#define COST_NO_INPUT -1
#define COST_BAD_INPUT -2

/**
 * Convert the string representation of route cost 
 * to the internal cost
 * @param cost string cost
 * @return internal cost value, or maximum cost if NULL
 */
static int to_route_cost(char *cost)
{
	if (zstr(cost)) {
		return COST_NO_INPUT;
	}

	if (!zstr(cost) && switch_is_number(cost)) {
		double costf = atof(cost);
		if (costf >= 0 && costf < 6.5535f) {
			return costf * 10000;
		}
		if (costf >= 6.5535f) {
			return 6.5535f;
		}
	}

	return COST_BAD_INPUT;
}

/**
 * Load a ratedeck CSV file
 * @param inter_trie the inter-area database to add routes to
 * @param intra_trie the intra-area database to add routes to
 * @param carrier the carrier the ratedeck belongs to
 * @param ratedeck the ratedeck file name
 * @return SWITCH_STATUS_SUCCESS if successful
 */
static switch_status_t load_ratedeck_file(struct route_trie *inter_trie, struct route_trie *intra_trie, struct carrier *carrier, const char *ratedeck)
{
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	int line_num = 0;
	char line[256];
	FILE *in = fopen(ratedeck, "r");
	if (!in) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to open ratedeck %s\n", ratedeck);
		return SWITCH_STATUS_FALSE;
	}
	line[255] = '\0';
	while (fgets_trimmed(line, 255, in)) {
		char *field[4] = { 0 };
		line_num++;
		if (switch_separate_string(line, ',', field, 4) > 2) {
			char *digits = field[0];
			int inter_cost = to_route_cost(field[1]);
			int intra_cost = to_route_cost(field[2]);
			if (inter_cost == COST_BAD_INPUT || (inter_cost >= 0 && !route_trie_add(inter_trie, digits, inter_cost, carrier->id, 1))) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "%s: Bad inter cost on line %i column 1\n", ratedeck, line_num);
				status = SWITCH_STATUS_FALSE;
				goto done;
			} else if (inter_cost == COST_NO_INPUT) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "%s: Missing inter cost on line %i column 1\n", ratedeck, line_num);
			}
			if (intra_cost == COST_BAD_INPUT || (intra_cost >= 0 && !route_trie_add(intra_trie, digits, intra_cost, carrier->id, 0))) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "%s: Bad intra cost on line %i column 2\n", ratedeck, line_num);
				status = SWITCH_STATUS_FALSE;
				goto done;
			} else if (intra_cost == COST_NO_INPUT) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "%s: Missing intra cost on line %i column 1\n", ratedeck, line_num);
			}
		} else {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "%s: Missing field on line %i\n", ratedeck, line_num);
			status = SWITCH_STATUS_FALSE;
			goto done;
		}
	}

done:
	fclose(in);
	return status;
}

/**
 * Configure module from configuration file
 */
static switch_status_t load_config(void)
{
	switch_xml_t cfg, xml = NULL, carrier, settings;
	switch_status_t status = SWITCH_STATUS_SUCCESS;
	struct lcr *new_lcr = NULL;
	route_value_t carrier_id = 0;
	int num_carriers = 0;

	if (!(xml = switch_xml_open_cfg("simple_lcr.conf", &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of simple_lcr.conf failed\n");
		status = SWITCH_STATUS_FALSE;
		goto done;
	}

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "Loading LCR configuration\n");

	/* size the LCR database based on the number of carriers defined */
	for (carrier = switch_xml_child(cfg, "carrier"); carrier; carrier = carrier->next) {
		num_carriers++;
		if (num_carriers > MAX_CARRIERS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Too many carriers!\n");
			status = SWITCH_STATUS_FALSE;
			goto done;
		}
	}
	new_lcr = lcr_create(num_carriers);

	if ((settings = switch_xml_child(cfg, "settings"))) {
		switch_xml_t param;
		for (param = switch_xml_child(settings, "param"); param; param = param->next) {
			const char *name = switch_xml_attr_soft(param, "name");
			const char *value = switch_xml_attr_soft(param, "value");
			if (zstr(name)) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Param name must be specified\n");
				status = SWITCH_STATUS_FALSE;
				goto done;
			}
			if (!strcasecmp("max-routes", name)) {
				int val = atoi(value);
				if (switch_is_number(value) && val > 0) {
					if (val > MAX_ROUTES) {
						val = MAX_ROUTES;
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "reducing max-routes to %d\n", MAX_ROUTES);
						lcr_set_max_routes(new_lcr, val);
					} else {
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "setting max-routes to %d\n", val);
						lcr_set_max_routes(new_lcr, val);
					}
				} else {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Ignoring invalid value %s for %s\n", value, name);
				}
			} else if (!strcasecmp("areas", name)) {
				if (!zstr(value)) {
					/* load the area prefix map */
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "Loading areas from: %s\n", value);
					if (load_area_file(new_lcr->areas, value) != SWITCH_STATUS_SUCCESS) {
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error loading areas from: %s\n", value);
						status = SWITCH_STATUS_FALSE;
						goto done;
					}
				}
			} else if (!strcasecmp("areas-cid-strip", name)) {
				int val = atoi(value);
				if (switch_is_number(value) && val >= 0) {
					new_lcr->areas_caller_id_strip = val;
				} else {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "ignoring invalid areas-cid-strip value of %s\n", value);
				}
			} else if (!strcasecmp("areas-cid-prefix", name)) {
				if (!zstr(value)) {
					new_lcr->areas_caller_id_prefix = switch_core_strdup(new_lcr->pool, value);
				}
			} else if (!strcasecmp("areas-dest-strip", name)) {
				int val = atoi(value);
				if (switch_is_number(value) && val >= 0) {
					new_lcr->areas_dest_strip = val;
				} else {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "ignoring invalid areas-dest-strip value of %s\n", value);
				}
			} else if (!strcasecmp("areas-dest-prefix", name)) {
				if (!zstr(value)) {
					new_lcr->areas_dest_prefix = switch_core_strdup(new_lcr->pool, value);
				}
			} else {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Ignoring unknown parameter: %s\n", name);
			}
		}
	}

	for (carrier = switch_xml_child(cfg, "carrier"); carrier; carrier = carrier->next) {
		switch_xml_t gateway;
		const char *name = switch_xml_attr_soft(carrier, "name");
		const char *ratedeck_file = switch_xml_attr_soft(carrier, "ratedeck");
		const char *inter_areas = switch_xml_attr_soft(carrier, "interAreas");
		struct carrier *cr;

		/* add a carrier */
		if (zstr(name) || zstr(ratedeck_file)) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Carrier name and ratedeck must be specified\n");
			status = SWITCH_STATUS_FALSE;
			goto done;
		}
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "Adding carrier %s\n", name);
		carrier_id++;
		cr = carrier_create(new_lcr->pool, name, carrier_id);
		new_lcr->carriers[carrier_id] = cr;

		/* add inter-areas LCR database for this carrier */
		if (!zstr(inter_areas)) {
			int argc = 0;
			char *argv[MAX_INTER_AREAS] = { 0 };
			char *mydata = NULL;

			if (!(mydata = strdup(inter_areas))) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "Memory error!\n");
				status = SWITCH_STATUS_FALSE;
				goto done;
			}

			if ((argc = switch_separate_string(mydata, ',', argv, (sizeof(argv) / sizeof(argv[0])))) > 1) {
				for (int i = 0; i < argc; i++) {
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "Adding carrier %s interArea %s\n", name, argv[i]);
					if (!route_trie_add(new_lcr->inter_areas, argv[i], 0, carrier_id, 0)) {
						switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Bad input: non-numeric interArea %s for carrier %s\n", argv[i], name);
						switch_safe_free(mydata);
						status = SWITCH_STATUS_FALSE;
						goto done;
					}
				}
			}
			switch_safe_free(mydata);
		}

		/* add gateways to the carrier */
		for (gateway = switch_xml_child(carrier, "gateway"); gateway; gateway = gateway->next) {
			const char *gateway_prefix = switch_xml_attr_soft(gateway, "prefix");
			const char *gateway_suffix = switch_xml_attr_soft(gateway, "suffix");
			if (zstr(gateway_prefix) && zstr(gateway_suffix)) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "A gateway prefix or suffix needs to be specified!\n");
				status = SWITCH_STATUS_FALSE;
				goto done;
			}
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "Adding carrier %s gateway: (%s, %s)\n", name, gateway_prefix, gateway_suffix);
			if (carrier_add_gateway(new_lcr->pool, cr, gateway_prefix, gateway_suffix) != SWITCH_STATUS_SUCCESS) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Too many gateways!\n");
				status = SWITCH_STATUS_FALSE;
				goto done;
			}
		}

		/* load the ratedeck associated with the carrier */
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CONSOLE, "Adding carrier %s ratedeck: %s\n", name, ratedeck_file);
		if (load_ratedeck_file(new_lcr->inter_trie, new_lcr->intra_trie, cr, ratedeck_file) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Error loading ratedeck: %s\n", ratedeck_file);
			status = SWITCH_STATUS_FALSE;
			goto done;
		}
	}

  done:

	if (status != SWITCH_STATUS_SUCCESS) {
		/* configuration failed.. clean up */
		lcr_destroy(new_lcr);
	} else {
		/* swap old LCR database with new LCR database */
		struct lcr *old_lcr = lcr_replace(new_lcr);
		lcr_destroy(old_lcr);
	}

	if (xml) {
		switch_xml_free(xml);
	}

	return status;
}

/**
 * LCR lookup and redirect dialplan
 */
SWITCH_STANDARD_DIALPLAN(simple_lcr_dialplan_redirect)
{
	switch_caller_extension_t *extension = NULL;
	struct route_array *results = NULL;
	route_size_t num_routes;
	char *lookup_number = "";
	const char *caller_id = "";
	switch_time_t start = switch_time_now();
	struct lcr *lcr = NULL;

	if (!caller_profile) {
		caller_profile = switch_channel_get_caller_profile(switch_core_session_get_channel(session));
	}

	if ((extension = switch_caller_extension_new(session, caller_profile->destination_number, caller_profile->destination_number)) == 0) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_CRIT, "memory error!\n");
		goto done;
	}

	/* get dialed number */
	if (!zstr(caller_profile->destination_number)) {
		lookup_number = caller_profile->destination_number;
		if (*lookup_number == '+') {
			lookup_number++;
		}
	}

	/* get caller ID */
	if (!zstr(caller_profile->caller_id_number)) {
		caller_id = caller_profile->caller_id_number;
		if (*caller_id == '+') {
			caller_id++;
		}
	}

	/* DO LCR Lookup */
	lcr = lcr_rdlock();
	results = route_array_create(lcr->max_routes);
	num_routes = lcr_find(lcr, caller_id, lookup_number, results);
	if (num_routes) {
		char routes_str[1024];
		size_t max_len = sizeof(routes_str) - 1;
		routes_str[max_len] = '\0';
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "Number of Routes Returned[%i]\n", num_routes);
		if (build_dialstring_list(routes_str, max_len, ',', lcr, results, caller_profile->destination_number) != SWITCH_STATUS_SUCCESS) {
			switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_CRIT, "dial string buffer overflow\n");
			extension = NULL;
			goto done;
		}
		/* redirect the call to the carrier gateways */
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_NOTICE, "Redirecting call from %s to %s to %s\n",
			caller_profile->caller_id_number, caller_profile->destination_number, routes_str);
		switch_caller_extension_add_application(session, extension, "redirect", switch_core_session_strdup(session, routes_str));
	} else {
		/* no route found - reject the call */
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_NOTICE, "Rejecting call from %s to %s\n",
			caller_profile->caller_id_number, caller_profile->destination_number);
		switch_caller_extension_add_application(session, extension, "respond", "404 No route found");
	}

 done:
	lcr_unlock(lcr);
	route_array_destroy(results);
	switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_DEBUG, "processing time = %"SWITCH_TIME_T_FMT" ns\n", switch_time_now() - start);
	return extension;
}

#define LCR_API "simple_lcr"
#define LCR_API_SYNTAX "<cid> <dest> [csv]"
SWITCH_STANDARD_API(simple_lcr_function)
{
	int argc = 0;
	char *argv[4] = { 0 };
	char *mydata = NULL;
	struct route_array *results = NULL;
	struct lcr *lcr = NULL;

	if (session) {
		switch_log_printf(SWITCH_CHANNEL_SESSION_LOG(session), SWITCH_LOG_ERROR, "This function cannot be called from the dialplan.\n");
		return SWITCH_STATUS_FALSE;
	}

	if (zstr(cmd)) {
		stream->write_function(stream, "Usage: " LCR_API " " LCR_API_SYNTAX "\n");
		return SWITCH_STATUS_SUCCESS;
	}

	if (!(mydata = strdup(cmd))) {
		stream->write_function(stream, "Memory error!\n");
		return SWITCH_STATUS_SUCCESS;
	}

	if ((argc = switch_separate_string(mydata, ' ', argv, (sizeof(argv) / sizeof(argv[0])))) > 1) {
		int i = 0;
		int csv = 0;
		char *dest_arg = argv[1];
		char *lookup_number = dest_arg;
		char *caller_id = argv[0];
		const char *format;
		route_size_t num_routes;

		/* is CSV format requested? */
		if (argc > 2) {
			csv = !strcasecmp("csv", argv[2]);
		}

		/* do lookup */
		lcr = lcr_rdlock();
		results = route_array_create(lcr->num_carriers);
		num_routes = lcr_find(lcr, caller_id, lookup_number, results);
		if (num_routes == 0) {
			stream->write_function(stream, "No Match!\n");
			goto done;
		}

		/* output results of lookup in a tabular or CSV format */
		if (csv){
			stream->write_function(stream, "carrier_id,type,cost,dial string\n");
			format = "%s,%s,%f,%s\n";
		} else {
			stream->write_function(stream, "Routes to %s%s%s\n", dest_arg, caller_id ? " from " : "", caller_id ? caller_id : "");
			stream->write_function(stream, "Carrier\t\tType\t\tCost\t\tDial string\n");
			format = "%s\t\t%s\t\t%f\t%s\n";
		}
		for (i = 0; i < num_routes; i++) {
			int j;
			struct route *route = route_array_get_route(results, i);
			route_value_t carrier_id = route_get_value(route);
			struct carrier *carrier = lcr->carriers[carrier_id];
			for (j = 0; carrier->gateway[j] && j < MAX_GATEWAYS; j++) {
				char dialstring[256];
				dialstring[255] = '\0';
				gateway_get_dialstring(carrier->gateway[j], dialstring, 255, dest_arg);
				stream->write_function(stream, format, carrier->name, route_get_type(route) ? "inter" : "intra", (float)route_get_cost(route) / 10000.0f, dialstring);
			}
		}
	} else {
		stream->write_function(stream, "Usage: " LCR_API " " LCR_API_SYNTAX "\n");
	}

 done:
	lcr_unlock(lcr);
	route_array_destroy(results);
	switch_safe_free(mydata);

	return SWITCH_STATUS_SUCCESS;
}

/**
 * Reload the configuration whenever there is a "reloadxml" event
 */
static void reloadxml_event_handler(switch_event_t *event)
{
    load_config();
}

SWITCH_MODULE_LOAD_FUNCTION(mod_simple_lcr_load)
{
	switch_api_interface_t *api_interface;
	switch_dialplan_interface_t *dp_interface;

	memset(&globals, 0, sizeof(globals));

	if (switch_mutex_init(&globals.update_lock, SWITCH_MUTEX_UNNESTED, pool) != SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Failed to create update_lock mutex!\n");
		return SWITCH_STATUS_FALSE;
	}

	load_config();

	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	SWITCH_ADD_API(api_interface, LCR_API, "LCR", simple_lcr_function, LCR_API_SYNTAX);
	SWITCH_ADD_DIALPLAN(dp_interface, "simple_lcr_redirect", simple_lcr_dialplan_redirect);

	if ((switch_event_bind(modname, SWITCH_EVENT_RELOADXML, NULL, reloadxml_event_handler, NULL) != SWITCH_STATUS_SUCCESS)) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind our reloadxml handler!\n");
		return SWITCH_STATUS_FALSE;
	}

	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_simple_lcr_shutdown)
{
	switch_event_unbind_callback(reloadxml_event_handler);
	lcr_destroy(globals.lcr);
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
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4 expandtab:
 */
