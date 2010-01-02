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
 * Joao Mesquita <jmesquita@freeswitch.org>
 *
 *
 * Description:
 * Module to load configurations from Qt preference system QSettings
 *
 */
#include <switch.h>

SWITCH_MODULE_LOAD_FUNCTION(mod_qsettings_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_qsettings_shutdown);
SWITCH_MODULE_DEFINITION(mod_qsettings, mod_qsettings_load, mod_qsettings_shutdown, NULL);


struct xml_binding {
	char *bindings;
};
typedef struct xml_binding xml_binding_t;


static struct {
    switch_memory_pool_t* pool;
} globals;



static switch_xml_t xml_url_fetch(const char *section, const char *tag_name, const char *key_name, const char *key_value, switch_event_t *params,
								  void *user_data)
{
	xml_binding_t *binding = (xml_binding_t *) user_data;

	if (!binding) {
		return NULL;
	}
	/* All we have to do here is check the section requested and maybe use a
	   templating system to provide the corresponding xml */

	return NULL;
}

#define ENABLE_PARAM_VALUE "enabled"
static switch_status_t do_config(void)
{
        char *cf = "qsettings.conf";
	switch_xml_t cfg, xml, bindings_tag, binding_tag, param;
	xml_binding_t *binding = NULL;
	int x = 0;

	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
		return SWITCH_STATUS_TERM;
	}

	if (!(bindings_tag = switch_xml_child(cfg, "bindings"))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing <bindings> tag!\n");
		goto done;
	}

	for (binding_tag = switch_xml_child(bindings_tag, "binding"); binding_tag; binding_tag = binding_tag->next) {
		char *bname = (char *) switch_xml_attr_soft(binding_tag, "name");
		char *bind_mask = NULL;

		for (param = switch_xml_child(binding_tag, "param"); param; param = param->next) {
			bind_mask = (char *) switch_xml_attr_soft(param, "value");
		}

                binding = new xml_binding_t();

		if (bind_mask) {
			binding->bindings = strdup(bind_mask);
		}


		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Binding [%s] XML Fetch Function [%s]\n",
						  zstr(bname) ? "N/A" : bname, binding->bindings ? binding->bindings : "all");
		switch_xml_bind_search_function(xml_url_fetch, switch_xml_parse_section_string(binding->bindings), binding);
		x++;
		binding = NULL;
	}

  done:
	switch_xml_free(xml);

	return x ? SWITCH_STATUS_SUCCESS : SWITCH_STATUS_FALSE;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_qsettings_load)
{
        /*switch_api_interface_t *qsettings_api_interface;*/

	/* connect my internal structure to the blank pointer passed to me */
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);

	memset(&globals,0,sizeof(globals));
	globals.pool = pool;

	if (do_config() == SWITCH_STATUS_SUCCESS) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Binding worked!");
	} else {
		return SWITCH_STATUS_FALSE;
	}


	/* indicate that the module should continue to be loaded */
	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_qsettings_shutdown)
{
	switch_xml_unbind_search_function_ptr(xml_url_fetch);
	return SWITCH_STATUS_SUCCESS;
}
