/* 
 * FreeSWITCH Modular Media Switching Software Library / Soft-Switch Application
 * Copyright (C) 2005-2009, Anthony Minessale II <anthm@freeswitch.org>
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
 * Leon de Rooij <leon@toyos.nl>
 *
 *
 * mod_benchmark.c
 *
 * Run an API n times and print how long it took to finish it
 *
 */
#include <switch.h>


SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_benchmark_shutdown);
SWITCH_MODULE_LOAD_FUNCTION(mod_benchmark_load);
SWITCH_MODULE_DEFINITION(mod_benchmark, mod_benchmark_load, mod_benchmark_shutdown, NULL);


#define BENCHMARK_API_FUNCTION_SYNTAX ""
SWITCH_STANDARD_API(benchmark_api_function)
{
  switch_time_t start_time = switch_micro_time_now();
  int elapsed_ms;

  elapsed_ms = (int) (switch_micro_time_now() - start_time) / 1000;

  stream->write_function(stream, "+OK Finished in %d ms.\n", elapsed_ms);

  return SWITCH_STATUS_SUCCESS;
}


/* Macro expands to: switch_status_t mod_benchmark_load(switch_loadable_module_interface_t **module_interface, switch_memory_pool_t *pool) */
SWITCH_MODULE_LOAD_FUNCTION(mod_benchmark_load)
{
  switch_api_interface_t *api_interface;
  switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Benchmark module loading...\n");

  /* connect my internal structure to the blank pointer passed to me */
  *module_interface = switch_loadable_module_create_module_interface(pool, modname);

  SWITCH_ADD_API(api_interface, "benchmark", "Perform an ODBC query", benchmark_api_function, BENCHMARK_API_FUNCTION_SYNTAX);

  switch_console_set_complete("add benchmark");

  /* indicate that the module should continue to be loaded */
  return SWITCH_STATUS_SUCCESS;
}


/*
  Called when the system shuts down
  Macro expands to: switch_status_t mod_benchmark_shutdown() */
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_benchmark_shutdown)
{
  switch_console_set_complete("del benchmark");
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
 * vim:set softtabstop=4 shiftwidth=4 tabstop=4
 */
