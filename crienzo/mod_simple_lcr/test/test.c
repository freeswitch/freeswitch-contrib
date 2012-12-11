/*
 * test.c
 *
 * Christopher M. Rienzo <chris@rienzo.com>
 *
 * test application for route_trie
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "../route_trie.h"

void test_lookups()
{
	char *search[] = { "", "0", "00", "000", "0000", "23", "234", "2345", "23456", "234567", "2345678", "111111111",
		"222222222", "333333333", "444444444", "555555555", "666666666", "777777777", "888888888", "99999999", "0000000000", "abcd", "+12345",
		"61723+", "aasdgs", "!@#$%^&*", "+1234", NULL };
	char *routes[] = { "null", "P1", "CV", "IP", "FR", "FW", "L3", "GX", "IC", "ERROR" };
	char *types[] = { "intra", "inter" };
	int i;
	int success = 0;
	struct route_array *results = route_array_create(3);
	struct route_trie *trie = route_trie_create(3);
	success += route_trie_add(trie, "781897", 0.001f * 10000, 1, 0);
	success += route_trie_add(trie, "774266", 0.0005f * 10000, 1, 1);
	success += route_trie_add(trie, "234567", 0.01f * 10000, 2, 0);
	success += route_trie_add(trie, "234567", 0.09f * 10000, 3, 1);
	success += route_trie_add(trie, "234567", 0.07f * 10000, 4, 0);
	success += route_trie_add(trie, "234567", 0.02f * 10000, 5, 1);
	success += route_trie_add(trie, "234567", 0.025f * 10000, 6, 0);
	success += route_trie_add(trie, "23", 0.05f * 10000, 7, 1);
	success += route_trie_add(trie, "234", 0.02f * 10000, 8, 0);
	success += route_trie_add(trie, "1234+", 0.1f * 10000, 9, 0);
	success += route_trie_add(trie, "+1234", 0.1f * 10000, 9, 0);
	success += route_trie_add(trie, "asdvasvn3q8a-vw9", 0.1f * 10000, 9, 0);
	success += route_trie_add(trie, "!@#$%^&*()m", 0.1f * 10000, 9, 0);
	if (success > 9) {
		printf("Bad data inserted into trie!\n");
	} else if (success < 9) {
		printf("Not enough data inserted into trie!\n");
	}
	for (i = 0; search[i]; i++) {
		route_array_reset(results);
		route_size_t num_routes = route_trie_find(trie, search[i], results);
		if (num_routes) {
			int j;
			for (j = 0; j < num_routes; j++) {
				struct route *route = route_array_get_route(results, j);
				printf("%s, route %s: type = %s cost = %f\n", search[i], routes[route_get_value(route)], types[route_get_type(route)], (float)route_get_cost(route) / 10000.0f);
			}
		} else {
			printf("%s: NO MATCH\n", search[i]);
		}
	}
	route_array_destroy(results);
	route_trie_destroy(trie);
}

void test_performance()
{
	int i, z;
	struct route_array *results = route_array_create(4);
	char dialstring[11] = { 0 };
	char *routes[] = { "null", "A", "B", "C", "D", "E" };
	struct timespec ts, te;

	struct route_trie *trie = route_trie_create(5);
	printf("building trie of 1 mil nodes...\n");
	for (i = 0; i <= 999999; i++) {
		char prefix[7] = { 0 };
		sprintf(prefix, "%06d", i);
		route_trie_add(trie, prefix, 0.01f * 10000, 1, 0);
		route_trie_add(trie, prefix, 0.005f * 10000, 2, 0);
		route_trie_add(trie, prefix, 0.003f * 10000, 3, 0);
		route_trie_add(trie, prefix, 0.002f * 10000, 4, 0);
		route_trie_add(trie, prefix, 0.015f * 10000, 5, 0);
	}
	printf("executing 30 million lookups...\n");
	clock_gettime(CLOCK_MONOTONIC, &ts);
	for (z = 0; z <= 30000000; z++) {
		char prefix[7] = { 0 };
		sprintf(prefix, "%06d", z % 1000000);
		route_array_reset(results);
		route_trie_find(trie, dialstring, results);
	}
	clock_gettime(CLOCK_MONOTONIC, &te);
	printf("destroying trie\n");
	route_array_destroy(results);
	route_trie_destroy(trie);
	printf("Start time = %lu sec\n", ts.tv_sec);
	printf("End time   = %lu sec\n", te.tv_sec);
	printf("Lookups per sec = %d\n", 30000000 / (te.tv_sec - ts.tv_sec));
	printf("Mean lookup time = %f nsec\n", (te.tv_sec - ts.tv_sec) / (30000000 * 1e-9));
}

int main(int argc, char **argv)
{
	test_lookups();
	test_performance();
	return 0;
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
