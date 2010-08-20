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


#define BENCHMARK_API_FUNCTION_SYNTAX "<repetition> <child_api_cmd_with_optional_arguments>"
SWITCH_STANDARD_API(benchmark_api_function)
{
  int repetition = 1;
  char *s_repetition;
  char *mycmd = NULL, *childcmd = NULL, *childargs = NULL;
  switch_stream_handle_t mystream = { 0 };
  switch_time_t outer_start_time, inner_start_time;
  int inner_elapsed_ms, outer_elapsed_ms;
  int inner_min_elapsed_ms = -1, inner_max_elapsed_ms = -1;
  switch_status_t status;

  /* did we get any cmd ? */
  if (zstr(cmd)) {
    stream->write_function(stream, "No paramaters supplied !\n\n-USAGE: %s\n", BENCHMARK_API_FUNCTION_SYNTAX);
    return SWITCH_STATUS_SUCCESS;
  }

  mycmd = strdup(cmd);
  s_repetition = mycmd;

  /* get the child cmd */
  if (!((childcmd = strchr(mycmd, ' ')))) {
    stream->write_function(stream, "No child api cmd supplied !\n\n-USAGE: %s\n", BENCHMARK_API_FUNCTION_SYNTAX);
    switch_safe_free(mycmd);
    return SWITCH_STATUS_SUCCESS;
  }
  *childcmd++ = '\0';
  repetition = atoi(s_repetition);

  /* get the child arguments (optional) */
  if ((childargs = strchr(childcmd, ' ')))
    *childargs++ = '\0';

  outer_start_time = switch_micro_time_now();

  for (int i = 0; i < repetition; i++) {
    inner_start_time = switch_micro_time_now();

    SWITCH_STANDARD_STREAM(mystream);

    if ((status = switch_api_execute(childcmd, childargs, session, &mystream)) != SWITCH_STATUS_SUCCESS) {
      stream->write_function(stream, "-ERR, error executing command\n");
    }

    switch_safe_free(mystream.data);

    inner_elapsed_ms = (int) (switch_micro_time_now() - inner_start_time) / 1000;

    if (inner_min_elapsed_ms == -1)
      inner_min_elapsed_ms = inner_elapsed_ms;
    else
      if (inner_elapsed_ms < inner_min_elapsed_ms)
        inner_min_elapsed_ms = inner_elapsed_ms;

    if (inner_elapsed_ms > inner_max_elapsed_ms)
      inner_max_elapsed_ms = inner_elapsed_ms;
  }

  outer_elapsed_ms = (int) (switch_micro_time_now() - outer_start_time) / 1000;

  stream->write_function(stream, "+OK Finished in %d ms (min %d ms, max %d ms)\n",
    outer_elapsed_ms, inner_min_elapsed_ms, inner_max_elapsed_ms);

  switch_safe_free(mycmd);

  return SWITCH_STATUS_SUCCESS;
}


SWITCH_MODULE_LOAD_FUNCTION(mod_benchmark_load)
{
  switch_api_interface_t *api_interface;
  switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO, "Benchmark module loading...\n");
  *module_interface = switch_loadable_module_create_module_interface(pool, modname);
  SWITCH_ADD_API(api_interface, "benchmark", "Perform an ODBC query", benchmark_api_function, BENCHMARK_API_FUNCTION_SYNTAX);
  switch_console_set_complete("add benchmark");
  return SWITCH_STATUS_SUCCESS;
}


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
