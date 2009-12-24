/* 
 * CSTA interface for FreeSWICH 
 * Copyright (C) 2009, Szentesi Krisztian <sz.krisz@freemail.hu>
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
 * The Original Code is CSTA interface for FreeSWICH   
 *
 * The Initial Developer of the Original Code is
 * Szentesi Krisztian <sz.krisz@freemail.hu>
 * Portions created by the Initial Developer are Copyright (C)
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 * 
 * Szentesi Krisztian <sz.krisz@freemail.hu>
 *
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 */

#include <switch.h>
#include "config.h"


/*static*/ Config Config::_instance;

Config::Config()
{
	strcpy(listen_address,"127.0.0.1");
	xml_port=3000;
	acl_name[0]=0;
}

bool Config::ReadConfig()
{
	
	char *cf = "csta_socket.conf";
	switch_xml_t cfg, xml, settings, param;

	if (!(xml = switch_xml_open_cfg(cf, &cfg, NULL))) {
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "Open of %s failed, using defaults.\n", cf);
	} else {
		if ((settings = switch_xml_child(cfg, "settings"))) {
			for (param = switch_xml_child(settings, "param"); param; param = param->next) {
				char *var = (char *) switch_xml_attr_soft(param, "name");
				char *val = (char *) switch_xml_attr_soft(param, "value");

				if (!strcmp(var, "listen-ip"))
				{
					strncpy(listen_address,val,sizeof(listen_address));	
					listen_address[sizeof(listen_address)-1]=0;
				}
				else if (!strcmp(var, "xml-listen-port"))
					xml_port=(switch_port_t)atoi(val);
				else if (!strcasecmp(var, "apply-inbound-acl")) 
				{
					strncpy(acl_name,val,sizeof(acl_name));	
					acl_name[sizeof(acl_name)-1]=0;
				}
			}
		}
		switch_xml_free(xml);
	}
	
	return true;

}