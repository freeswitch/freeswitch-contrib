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
 * route_trie.c -- Implementation of an uncompressed trie for fast least cost routing
 *                 lookups.
 *
 */
#include "route_trie.h"

#include <stdlib.h>
#include <string.h>

/**
 * A route in the database
 */
struct route {
	/** cost of route */
	route_cost_t cost;
	/** routing information */
	route_value_t value;
	/** route type */
	route_type_t type;
};

/**
 * An array of routes
 */
struct route_array {
    /** maximum size of this array */
    route_size_t max_size;
    /** Array size */
    route_size_t size;
    /** The routes */
    struct route route[];
};

/**
 * A trie node.  Each node represents a digit in a dial string (0-9).
 */
struct route_trie_node {
	/** child nodes, indexed by dial string digit */
	struct route_trie_node *child[10];
	/** array of routes stored in this node (nullable) */
	struct route_array *array;
};

/**
 * route trie instance
 */
struct route_trie {
	/** max routes per node */
	int max_routes;
	/** root of the trie */
	struct route_trie_node *root;
};

route_value_t route_get_value(struct route *route)
{
	return route->value;
}

route_type_t route_get_type(struct route *route)
{
	return route->type;
}

route_cost_t route_get_cost(struct route *route)
{
	return route->cost;
}

struct route_array *route_array_create(route_size_t max_size)
{
	struct route_array *array = calloc(1, sizeof(struct route_array) + max_size * sizeof(struct route));
	array->max_size = max_size;
	return array;
}

route_size_t route_array_get_size(struct route_array *array)
{
	return array->size;
}

struct route *route_array_get_route(struct route_array *array, route_size_t index)
{
	if (index < array->size) {
		return &array->route[index];
	}
	return NULL;
}

void route_array_reset(struct route_array *array)
{
	array->size = 0;
}

void route_array_destroy(struct route_array *array)
{
	if (array) {
		free(array);
	}
}

int route_array_contains(struct route_array *array, route_value_t value)
{
	int i;
	for (i = 0; i < array->size; i++) {
		if (array->route[i].value == value) {
			return 1;
		}
	}
	return 0;
}

/**
 * Find the index in the array that contains the same value and type
 * @param array the array to search
 * @param value the value to search for
 * @param type the type to search for
 * @return the index
 */
static int route_array_find_by_value_and_type(struct route_array *array, route_value_t value, route_type_t type)
{
	int i;
	for (i = 0; i < array->size; i++) {
		if (array->route[i].value == value && array->route[i].type == type) {
			return i;
		}
	}

	/* no match */
	return array->max_size;
}

/**
 * Find the index in the array to insert a new route
 * @param array the array to search
 * @param cost the route cost
 * @return the index
 */
static int route_array_find_by_cost(struct route_array *array, route_cost_t cost)
{
	int i;

	/* TODO implement heap or do binary search if max_size needs to be larger than 3 */

	/* linear search for first index where rte can be placed */
	for (i = 0; i < array->max_size; i++) {
		if (!array->route[i].value || array->route[i].cost > cost) {
			return i;
		}
	}

	/* no room in array */
	return array->max_size;
}

/**
 * Remove a route from the array
 * @param array the array
 * @param index the route index to remove 
 */
void route_array_remove(struct route_array *array, int index)
{
	if (index >= 0 && index < array->size && array->size > 0) {
		/* remove this route from the array */
		for (; index >= 0 && index < array->size - 1; index++) {
			array->route[index] = array->route[index + 1];
		}
		array->size--;
	}
}

/**
 * Add a route to the array preserving the order of cost, ascending.  The highest
 * cost route is removed from the array to make room for a lower cost route.
 * @param array the array
 * @param route the route to add
 */
void route_array_add(struct route_array *array, struct route *route)
{
	struct route cur = *route;
	struct route swap;
	int i;

	/* check if route is already in the array */
	if ((i = route_array_find_by_value_and_type(array, route->value, route->type)) != array->max_size) {
 		/* don't add route if existing route is cheaper than the route to add */
		if (array->route[i].cost < route->cost) {
			return;
		}
		/* remove more expensive existing route */
		route_array_remove(array, i);
	}

	/* insert route into array if it costs less than the most expensive route or
	   the array is not filled */
	for (i = route_array_find_by_cost(array, route->cost); cur.value && i < array->max_size; i++) {
		swap = array->route[i];
		array->route[i] = cur;
		cur = swap;
	}

	/* check for discarded route */
	array->size++;
	if (array->size > array->max_size) {
		array->size = array->max_size;
	}
}

/**
 * Create a route trie node
 * @return the node
 */
struct route_trie_node *route_trie_node_create()
{
	return calloc(1, sizeof(struct route_trie_node));
}

/**
 * Add a route to the route trie
 * @param node the route trie node
 * @param prefix the prefix string
 * @param route the route to add
 * @param max_routes the maximum # of routes allowed per node
 * @return 1 if successful
 */
static int route_trie_node_add(struct route_trie_node *node, const char *prefix, struct route *route, int max_routes)
{
	struct route_trie_node *cur_node, *child_node;

	/* Find the node to store the route in.  Create nodes as needed. */
	for (cur_node = node; prefix && prefix[0] != '\0'; cur_node = child_node) {
		int i = (int)(prefix[0] - '0'); /* convert char to digit */
		child_node = NULL;
		if (i >= 0 && i <= 9) {
			if (!cur_node->child[i]) {
				cur_node->child[i] = route_trie_node_create();
			}
			child_node = cur_node->child[i];
			prefix++; /* advance to next character in prefix */
		} else {
			return 0; /* bad input */
		}
	}

	/* Store the route.  Create route arrays as needed. */
	if (cur_node) {
		if (!cur_node->array) {
			cur_node->array = route_array_create(max_routes);
		}
		route_array_add(cur_node->array, route);
		return 1;
	}
	return 0;
}

/**
 * Search trie for lowest cost routes
 * @param node the trie to search
 * @param prefix the route prefix
 * @param array the array to fill
 * @return the number of routes returned
 */
static route_size_t route_trie_node_find(struct route_trie_node *node, const char *prefix, struct route_array *array)
{
	struct route_trie_node *cur_node, *child_node;
	for (cur_node = node; cur_node; cur_node = child_node) {
		/* check if any routes at this node */
		if (cur_node->array) {
			int i;
			for (i = 0; i < cur_node->array->size; i++) {
				route_array_add(array, &cur_node->array->route[i]);
			}
		}

		/* advance to child node if prefix still has more digits */
		child_node = NULL;
		if (prefix && prefix[0] != '\0') {
			int i = (int)(prefix[0] - '0');
			if (i >= 0 && i <= 9) {
				if (cur_node->child[i]) {
					child_node = cur_node->child[i];
					prefix++; /* advance to next character in prefix */
				}
			} else {
				return 0; /* bad input */
			}
		}
	}
	return array->size;
}

/**
 * Search trie for count of routes
 * @param node the trie to search
 * @param prefix the route prefix
 * @return the number of routes
 */
static int route_trie_node_find_count(struct route_trie_node *node, const char *prefix)
{
	int count = 0;
	struct route_trie_node *cur_node, *child_node;
	for (cur_node = node; cur_node; cur_node = child_node) {
		/* check if any routes at this node */
		if (cur_node->array) {
			count += cur_node->array->size;
		}

		/* advance to child node if prefix still has more digits */
		child_node = NULL;
		if (prefix && prefix[0] != '\0') {
			int i = (int)(prefix[0] - '0');
			if (i >= 0 && i <= 9) {
				if (cur_node->child[i]) {
					child_node = cur_node->child[i];
					prefix++; /* advance to next character in prefix */
				}
			} else {
				return 0; /* bad input */
			}
		}
	}
	return count;
}

/**
 * Destroy the route trie node and its children
 * @param node the trie node
 */
static void route_trie_node_destroy(struct route_trie_node *node)
{
	if (node) {
		int i;
		for (i = 0; i < 10; i++) {
			struct route_trie_node *child = node->child[i];
			route_trie_node_destroy(child);
		}
		if (node->array) {
			free(node->array);
		}
		free(node);
	}
}

struct route_trie *route_trie_create(route_size_t max_routes)
{
	struct route_trie *trie = malloc(sizeof(*trie));
	trie->root = route_trie_node_create();
	trie->max_routes = max_routes;
	return trie;
}

int route_trie_add(struct route_trie *trie, const char *prefix, route_cost_t cost, route_value_t value, route_type_t type)
{
	struct route route = { cost, value, type };
	return route_trie_node_add(trie->root, prefix, &route, trie->max_routes);
}

route_size_t route_trie_find(struct route_trie *trie, const char *prefix, struct route_array *array)
{
	return route_trie_node_find(trie->root, prefix, array);
}

int route_trie_find_count(struct route_trie *trie, const char *key)
{
	return route_trie_node_find_count(trie->root, key);
}

void route_trie_destroy(struct route_trie *trie)
{
	if (trie) {
		route_trie_node_destroy(trie->root);
		free(trie);
	}
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
