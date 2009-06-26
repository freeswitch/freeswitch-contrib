#include "globals.h"


K3LAPI Globals::_k3lapi;

config_options Globals::_options;

switch_endpoint_interface_t * Globals::_khomp_endpoint_interface;
switch_api_interface_t * Globals::_api_interface;
switch_memory_pool_t * Globals::_module_pool = NULL;
int Globals::_running = 1;

unsigned int Globals::_flags;
int Globals::_calls;
switch_mutex_t * Globals::_mutex;


