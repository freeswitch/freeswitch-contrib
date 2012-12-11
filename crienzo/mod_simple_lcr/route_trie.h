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
 *
 * Maintainer: Christopher M. Rienzo <chris@rienzo.com>
 *
 * route_trie.h -- prefix tree of routes ordered by least cost ascending.
 *
 */
#ifndef ROUTE_TRIE_H
#define ROUTE_TRIE_H

#include <stdint.h>

struct route_trie;
struct route_array;
struct route;

typedef uint16_t route_value_t;
typedef uint8_t route_type_t;
typedef uint16_t route_cost_t;
typedef uint8_t route_size_t;

/**
 * Get the route value
 * @param route the route
 * @return the value
 */
route_value_t route_get_value(struct route *route);

/**
 * Get the user-defined value
 * @param route the route
 * @return type the type of route
 */
route_type_t route_get_type(struct route *route);

/**
 * Get the route cost
 * @param route the route
 * @return the cost
 */
route_cost_t route_get_cost(struct route *route);

/**
 * Create an array to store lookup results
 *
 * @param max_size the maximum number of results to store
 * @return the array
 */
struct route_array *route_array_create(route_size_t max_size);

/**
 * Add a route to the array preserving the order of cost, ascending.  The highest
 * cost route is removed from the array to make room for a lower cost route.
 * @param array the array
 * @param route the route to add
 */
void route_array_add(struct route_array *array, struct route *route);

/**
 * Get the size of the array
 * @param array the array
 * @return the array size
 */
route_size_t route_array_get_size(struct route_array *array);

/**
 * Get a route from the array
 * @param array the array
 * @param index the route index
 * @return the route or NULL if out of bounds
 */
struct route *route_array_get_route(struct route_array *array, route_size_t index);

/**
 * Reset the array for a new query
 * @param array
 */
void route_array_reset(struct route_array *array);

/**
 * Destroy the route array
 * @param array the array
 */
void route_array_destroy(struct route_array *array);

/**
 * Search array for matching value
 * @param value to match
 * @return 1 if array contains value
 */
int route_array_contains(struct route_array *array, route_value_t value);

/**
 * Create a new route_trie
 * @param max_routes the maximum number of routes to store per node
 * @return the trie
 */
struct route_trie *route_trie_create(route_size_t max_routes);

/**
 * Add a route to the trie
 * @param trie the trie
 * @param prefix the route prefix
 * @param cost the route cost
 * @param value the routing information
 * @param type the type of route
 * @return 1 if successful
 */
int route_trie_add(struct route_trie *trie, const char *prefix, route_cost_t cost, route_value_t value, route_type_t type);

/**
 * Search for least cost routes.
 * @param trie the trie
 * @param key the number to search
 * @param array the array to fill with routes, ordered by cost ascending
 * @return the number of routes found
 */
route_size_t route_trie_find(struct route_trie *trie, const char *key, struct route_array *array);

/**
 * Get count of routes
 * @param trie the trie
 * @param key the number to search
 * @return the number of routes
 */
int route_trie_find_count(struct route_trie *trie, const char *key);

/**
 * Destroy the trie
 * @param trie the trie
 */
void route_trie_destroy(struct route_trie *trie);

#endif /* ROUTE_TRIE_H */

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
