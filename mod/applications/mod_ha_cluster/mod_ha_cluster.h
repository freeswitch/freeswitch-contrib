
#ifndef _HA_CLUSTER_H_
#define _HA_CLUSTER_H_

#include <switch.h>
#include <sys/time.h>
#include <errno.h>

#define HAC_BUFFSIZE 65536

#define HAC_HEARTBEAT     "ha_cluster::heartbeat"    /* Call limit was just set up */
#define HAC_LIMIT_UP      "ha_cluster::limit_up"     /* Call limit was just set up */
#define HAC_LIMIT_DOWN    "ha_cluster::limit_down"   /* Call limit was just removed */
#define HAC_REGISTER      "ha_cluster::register"     /* Register event occured here */
#define HAC_UNREGISTER    "ha_cluster::unregister"   /* De-register event occured here */
#define HAC_STARTUP       "ha_cluster::startup"      /* This node just started */
#define HAC_SHUTDOWN      "ha_cluster::shutdown"     /* This node is shutting down */
#define HAC_FAILURE       "ha_cluster::failure"      /* This node just failed in some way */
#define HAC_CHOSE_RANDOM  "ha_cluster::random"       /* This node just picked a random number */
#define HAC_TAKEOVER      "ha_cluster::takeover"     /* This node is taking over another node */
#define HAC_RELINQUISH    "ha_cluster::relinquish"   /* This node is relinquishing master state to ensure we have a slave */

extern unsigned char MAGIC[];
/*static char *MARKER = "1"; */

typedef enum {
	HAC_PEER_UNKNOWN = 0,
	HAC_PEER_STARTING = 1,
	HAC_PEER_MASTER = 2,
	HAC_PEER_SLAVE = 3,
	HAC_PEER_FAILED_MASTER = 4,
	HAC_PEER_FAILED_SLAVE = 5,
	HAC_PEER_STOPPING_MASTER = 6,
	HAC_PEER_STOPPING_SLAVE = 7
} hac_peer_state_t;

struct s_hac_rule {
	const char* name;
	const char* type;
	const char* string;
	const char* rate;
	const char* burst;
	const char* mode;
	const char* proto;
	const char* sport;
	const char* dport;
	const char* src;
	const char* srcrange;
	const char* length;
	const char* mports;
	const char* srcmports;
	const char* dstmports;
	const char* state;
	const char* rejectwith;
	const char* mark;
	struct s_hac_rule* next;
};
typedef struct s_hac_rule hac_rule_t;

struct s_hac_rule_group {
	const char* name;
	hac_rule_t* rules;
	struct s_hac_rule_group* next;
};
typedef struct s_hac_rule_group hac_rule_group_t;

struct s_hac_route {
	const char* name;
	const char* destination;
	const char* gateway;
	const char* metric;
	const char* dev;
	struct s_hac_route* next;
};
typedef struct s_hac_route hac_route_t;

struct s_hac_arp {
	const char* ip;
	const char* mac;
	const char* dev;
	struct s_hac_arp* next;
};
typedef struct s_hac_arp hac_arp_t;

struct s_hac_ip {
	const char* name;
	const char* cidr;
	const char* var;
	const char* dev;
	const char* rgroup;
	hac_route_t* routes;
	struct s_hac_ip* next;
};
typedef struct s_hac_ip hac_ip_t;

struct s_hac_cluster_ips {
	const char* name;
	const char* vmac;
	const char* dev;
	hac_ip_t* ips;
	hac_route_t* routes;
	struct s_hac_cluster_ips* next;
};
typedef struct s_hac_cluster_ips hac_cluster_ips_t;

struct s_hac_failover_ips {
	const char* name;
	const char* nodes;
	const char* dev;
	hac_ip_t* ips;
	hac_route_t* routes;
	struct s_hac_failover_ips* next;
};
typedef struct s_hac_failover_ips hac_failover_ips_t;

struct s_hac_interface {
	const char* name;
	const char* description;
	const char* duplex;
	const char* speed;
	hac_cluster_ips_t* cips;
	hac_failover_ips_t* fips;
	hac_ip_t* ips;
	hac_route_t* routes;
	hac_arp_t* arps;
	struct s_hac_interface* next;
};
typedef struct s_hac_interface hac_interface_t;

struct s_hac_failover_group {
	const char* name;
	const char* include_type;
	const char** include_names;
	int num_includes;
	struct s_hac_failover_group *next;
};
typedef struct s_hac_failover_group hac_failover_group_t;

struct s_hac_network {
	hac_rule_group_t* rgroups;
	hac_interface_t* interfaces;
	hac_failover_group_t* fgroups;
	int num_interfaces;
	int num_groups;
};
typedef struct s_hac_network hac_network_t;

struct s_hac_hb {
	const char* interval;
	const char* network;
	const char* address;
	const char* port;
	const char* ttl;
	switch_port_t lport;
	switch_sockaddr_t *addr;
	switch_sockaddr_t *iface;
	switch_socket_t *udp_sock;
	struct timeval last_hb;
	struct s_hac_hb* next;
};
typedef struct s_hac_hb hac_hb_t;

struct s_hac_event_match {
	const char* name;
	const char* value;
	struct s_hac_event_match* next;
};
typedef struct s_hac_event_match hac_event_match_t;

struct s_hac_failover {
	const char* event;
	hac_event_match_t* match_rules;
	struct s_hac_failover* next;
};
typedef struct s_hac_failover hac_failover_t;

struct s_hac_general {
	const char* nodeid;
	const char* msr;
	const char* timer_a;
	const char* timer_b;
	const char* failure_threshold;
	hac_hb_t* heartbeat;
	hac_failover_t* failover;
	size_t num_failover_conditions;
};
typedef struct s_hac_general hac_general_t;

struct s_hac_config {
	hac_network_t network;
	hac_general_t general;
	switch_memory_pool_t* pool;
};
typedef struct s_hac_config hac_config_t;

typedef enum {
	HAC_LOOPING = 0,
	HAC_SHUTDOWN_RUNTIME = 1,
	HAC_UNLOAD = 2
} hac_state_t;


extern hac_config_t config;

struct s_hac_event {
	switch_event_t *event;
	switch_bool_t processed;
	struct timeval arrived;
};
typedef struct s_hac_event hac_event_t;

struct s_hac_chain_list {
	char *chain_name;
	int chain_len;
	char chain_flush[64];
	char chain_delete[64];
	struct s_hac_chain_list *next;
};
typedef struct s_hac_chain_list hac_chain_list_t;

struct s_hac_peer {
	char hostname[256];
	char* ip;
	int nodeid;
	uint64_t rand_val;
	struct timeval first_seen;
	struct timeval last_seen;
	hac_peer_state_t state;
	uint64_t missed_beats;
};
typedef struct s_hac_peer hac_peer_t;

struct s_global {
	switch_memory_pool_t* pool;
	char hostname[256];
	uint64_t host_hash;
	switch_memory_pool_t* event_pool;
	switch_hash_t *event_hash;
	switch_hash_t *peer_hash;
	hac_peer_state_t state;
	hac_state_t mod_state;
	hac_hb_t* hb;
	int nodeid;
	uint64_t rand_val;
	const char* msr;
	int req_masters;
	int masters;
	int slaves;
	int timer_a_dur;
	int timer_b_dur;
	struct timeval timer_a;
	struct timeval timer_b;
	int failure_threshold;
	switch_thread_rwlock_t *lock;
};
extern struct s_global hac_globals;

/* ha_cluster_util.c */
switch_status_t execute_command(const char *cmd, char **out);


/* ha_cluster_config.c */
switch_status_t config_routes(hac_route_t **routes, switch_xml_t root, const char *dev);
switch_status_t config_arp(hac_arp_t **arps, switch_xml_t root, const char *dev);
switch_status_t config_ips(hac_ip_t** ips, switch_xml_t root, const char *dev);
switch_status_t config_cluster_ips(hac_cluster_ips_t **cips, switch_xml_t root, const char *dev);
switch_status_t config_failover_ips(hac_failover_ips_t **fips, switch_xml_t root, const char *dev);
switch_status_t config_interfaces(hac_interface_t **ints, switch_xml_t root);
switch_status_t config_rules(hac_rule_t** rules, switch_xml_t root);
switch_status_t config_rule_groups(hac_rule_group_t** rgroups, switch_xml_t root);
switch_status_t config_failover_groups(hac_failover_group_t** fover, switch_xml_t root);
switch_status_t config_network(switch_xml_t root);
switch_status_t config_hb(hac_hb_t** hb, switch_xml_t root);
switch_status_t config_events(hac_event_match_t** events, switch_xml_t root);
switch_status_t config_failover(hac_failover_t** fover, switch_xml_t root);
switch_status_t config_general(switch_xml_t root);
switch_status_t do_config(switch_bool_t reload);

/* ha_cluster_interfaces.c */
switch_status_t initialize_interfaces();
switch_status_t initialize_firewall();
switch_status_t verify_interfaces();
switch_status_t routes_up(const hac_route_t *routes);
switch_status_t routes_down(const hac_route_t *routes);
switch_status_t ips_up(const hac_ip_t *ips);
switch_status_t ips_down(const hac_ip_t *ips);
char *generate_rule_cmd(const hac_rule_t *rule, const hac_ip_t *ip, const int rule_num);
switch_status_t rules_up(const hac_rule_t *rules, const hac_ip_t *ip);



#endif
