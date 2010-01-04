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
#include <QString>
#include <QtGui>
#include <QDir>
#include "mod_qsettings/mod_qsettings.h"

static struct {
    switch_memory_pool_t* pool;
} globals;

switch_xml_t XMLBinding::getConfigXML(QString tmpl)
{
    switch_event_t *e;
    switch_event_create_plain(&e, SWITCH_EVENT_REQUEST_PARAMS);
    switch_assert(e);

    QFile tmplFile(QString("%1/templates/%2.xml").arg(QApplication::applicationDirPath(),tmpl));

    if (!tmplFile.exists()) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_INFO,
                          "Template %s.xml, doesn't exist on directory, falling back to embedded template.\n",
                          tmpl.toAscii().constData());
        tmplFile.setFileName(QString(":/confs/%1.xml").arg(tmpl));
        return NULL;
    }

    if (tmplFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Template %s could not be read.!\n", tmpl.toAscii().constData());
        return NULL;
    }

    /* Open template file and expand all strings based on QSettings */
    QString tmplContents(tmplFile.readAll());
    tmplFile.close();
    _settings->beginGroup("FreeSIWTCH/conf");
    _settings->beginGroup(tmpl);
    foreach(QString k, _settings->childKeys())
    {
        switch_event_add_header_string(e, SWITCH_STACK_BOTTOM, k.toAscii().constData(), _settings->value(k).toByteArray().constData());
    }

    char *res = switch_event_expand_headers(e, tmplContents.toAscii().constData());
    switch_safe_free(e);
    return switch_xml_parse_str(res, strlen(res));
}

static switch_xml_t xml_url_fetch(const char *section, const char *tag_name, const char *key_name, const char *key_value, switch_event_t *params,
                                  void *user_data)
{
    XMLBinding *binding = (XMLBinding *) user_data;

    if (!binding) {
        return NULL;
    }

    return binding->getConfigXML(key_value);
}

static switch_status_t do_config(void)
{
    char *cf = "qsettings.conf";
    switch_xml_t cfg, xml, bindings_tag;
    XMLBinding *binding = NULL;

    if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "open of %s failed\n", cf);
        return SWITCH_STATUS_TERM;
    }

    if (!(bindings_tag = switch_xml_child(cfg, "bindings"))) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Missing <bindings> tag!\n");
        switch_xml_free(xml);
        return SWITCH_STATUS_FALSE;
    }

    QString bind_mask = switch_xml_attr_soft(bindings_tag, "value");
    if (!bind_mask.isEmpty())
    {
        binding = new XMLBinding(bind_mask);
    }

    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Binding XML Fetch Function [%s]\n",
                      binding->getBinding().isEmpty() ? "all" : binding->getBinding().toAscii().constData());
    switch_xml_bind_search_function(xml_url_fetch, switch_xml_parse_section_string(binding->getBinding().toAscii().constData()), binding);
    binding = NULL;

    switch_xml_free(xml);
    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_LOAD_FUNCTION(mod_qsettings_load)
{
    /*switch_api_interface_t *qsettings_api_interface;*/

    /* connect my internal structure to the blank pointer passed to me */
    *module_interface = switch_loadable_module_create_module_interface(pool, "mod_qsettings");

    memset(&globals,0,sizeof(globals));
    globals.pool = pool;

    if (do_config() == SWITCH_STATUS_SUCCESS) {
        switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Sucessfully configured.\n");
    } else {
        return SWITCH_STATUS_FALSE;
    }


    switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "We loaded mod_qsettings.\n");
    /* indicate that the module should continue to be loaded */
    return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_qsettings_shutdown)
{
    switch_xml_unbind_search_function_ptr(xml_url_fetch);
    return SWITCH_STATUS_SUCCESS;
}
