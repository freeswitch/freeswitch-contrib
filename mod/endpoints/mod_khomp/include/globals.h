#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include "k3lapi.hpp"

#include <config_options.hpp>

#include <vector>

extern "C"
{
    #include <switch.h>
}


struct Globals
{

    static K3LAPI _k3lapi;

    /* Config options class */
    static config_options _options;



    static switch_endpoint_interface_t *_khomp_endpoint_interface;
    static switch_api_interface_t *_api_interface;
    static switch_memory_pool_t *_module_pool;
    static int _running;

    static unsigned int _flags;
    static int _calls;
    static switch_mutex_t *_mutex;

};


#endif /* _GLOBALS_H_ */
