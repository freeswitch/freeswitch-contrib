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

/*! \file mod_csta_socket.cpp
\brief PBX module interface implementation for FreeSWITCH.

Provides interface functions to allow integrate the module to FreeSWITCH 
*/


#include <switch.h>

#include "FreeSwitch.h"
#include "FreeSwitchPlatform.h"
#include "FreeSwitchSocket.h"
#include "XMLSession.h"
#include "config.h"
#include "UniversalErrorResponse.h"

SWITCH_MODULE_LOAD_FUNCTION(mod_csta_socket_load);
SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_csta_socket_shutdown);
SWITCH_MODULE_RUNTIME_FUNCTION(mod_csta_socket_runtime);
SWITCH_MODULE_DEFINITION(mod_csta_socket, mod_csta_socket_load, mod_csta_socket_shutdown, mod_csta_socket_runtime);

static volatile bool bRunning=true;

FreeSWITCH* fs=NULL;
FreeSWITCHPlatform* platform=NULL;


struct THREAD_INFO
{
	switch_memory_pool_t *pool;
	switch_socket_t* socket;
};


switch_socket_t* listen_socket=NULL;
switch_event_node_t *node=NULL;


//! Called if a call hits a queue
/*!
	Called if call enters into a queue. Queues should be defined in dialplan 
	default section with csta_trigger_queue application triggered.
	\param session The FreeSWITCH session
	\param data	The queue name set in dialplan (data parameter of csta_trigger_queue application)
	
	Example configuration:	

   <extension name="example_queue">
      <condition field="destination_number" expression="^9500$">  
          <action application="ring_ready"/>
          <action application="csta_trigger_queue" data="9500"/>
          <action application="sleep" data="600000"/>
      </condition>      
   </extension> 

   Note that queue name must only contain numeric characters to fit CSTA standard
*/
SWITCH_STANDARD_APP(queue_function)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Queue triggered, queue: %s, call:%s!\n", data, switch_core_session_get_uuid(session));
	fs->OnCallQueued(data, session); 
}


//! Called if a call enters FreeSWITCH from outside
/*!
	Called if a call enters FreeSWITCH from outside. Incoming CSTA Network interfaces should be defined in dialplan 
	public section with csta_trigger_trunk_inbound application triggered.
	\param session The FreeSWITCH session
	\param data	The NID name set in dialplan (data parameter of csta_trigger_trunk_inbound application)
	
	Example configuration:	
   
	  <extension name="inbound_example">
		<condition field="destination_number" expression="^193190$">       
		  <action application="csta_trigger_trunk_inbound" data="20"/>
		  <action application="set" data="domain_name=$${domain}"/>      
		  <action application="transfer" data="9500 XML default"/>
		</condition>
	  </extension>  

   Note that NID name must only contain numeric characters to fit CSTA standard
*/
SWITCH_STANDARD_APP(trunk_inbound_function)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Inbound call on trunk %s, call:%s!\n", data, switch_core_session_get_uuid(session));
	fs->OnCallNetworkIncoming(data, session);
}

//! Called if a call leaves FreeSWITCH via a gateway
/*!
	Called if a call leaves FreeSWITCH via a gateway. Outgoing CSTA Network interfaces should be defined in dialplan 
	default section with csta_trigger_trunk_outbound application triggered.
	\param session The FreeSWITCH session
	\param data	The NID name set in dialplan (data parameter of csta_trigger_trunk_outbound application)
	
	Example configuration:	
   
	  <extension name="outbound_example">
		<condition field="destination_number" expression="^20(.*)$">
			<action application="csta_trigger_trunk_outbound" data="20"/>
			<action application="set" data="effective_caller_id_number=193190"/>
			<action application="bridge" data="sofia/gateway/example_gw/$1"/>
		</condition>
	  </extension> 

   Note that NID name must only contain numeric characters to fit CSTA standard
*/
SWITCH_STANDARD_APP(trunk_outbound_function)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Outbound call on trunk %s, call:%s!\n", data, switch_core_session_get_uuid(session));
	fs->OnCallNetworkOutgoing(data, session);
}

static void event_handler(switch_event_t *event)
{
	
	switch(event->event_id)
	{
	case SWITCH_EVENT_CUSTOM:
		break;
	case SWITCH_EVENT_CLONE:
		break;
	case SWITCH_EVENT_CHANNEL_CREATE:
		fs->OnChannelCreate(event);
		break;
	case SWITCH_EVENT_CHANNEL_DESTROY:
		break;
	case SWITCH_EVENT_CHANNEL_STATE:
		break;
	case SWITCH_EVENT_CHANNEL_ANSWER:
		fs->OnChannelAnswer(event);
		break;
	case SWITCH_EVENT_CHANNEL_HANGUP:
		fs->OnChannelHangup(event);
		break;
	case SWITCH_EVENT_CHANNEL_EXECUTE:		
	case SWITCH_EVENT_CHANNEL_EXECUTE_COMPLETE:
		{
			const char* app=switch_event_get_header(event, "Application");
			if(app) 
			{
				if(strcmp(app,"playback")==0)
					fs->OnPlayback(event);
				else
				if(strcmp(app,"record")==0)
					fs->OnRecord(event);
			}
		}
		break;
	case SWITCH_EVENT_CHANNEL_BRIDGE:
		break;
	case SWITCH_EVENT_CHANNEL_UNBRIDGE:
		break;
	case SWITCH_EVENT_CHANNEL_PROGRESS:		
	case SWITCH_EVENT_CHANNEL_PROGRESS_MEDIA:
		fs->OnProgress(event);
		break;
	case SWITCH_EVENT_CHANNEL_OUTGOING:
		break;
	case SWITCH_EVENT_CHANNEL_PARK:
		break;
	case SWITCH_EVENT_CHANNEL_UNPARK:
		break;
	case SWITCH_EVENT_CHANNEL_APPLICATION:
		break;
	case SWITCH_EVENT_CHANNEL_ORIGINATE:
		break;
	case SWITCH_EVENT_CHANNEL_UUID:
		break;
	case SWITCH_EVENT_API:
		break;
	case SWITCH_EVENT_LOG:
		break;
	case SWITCH_EVENT_INBOUND_CHAN:
		break;
	case SWITCH_EVENT_OUTBOUND_CHAN:
		break;
	case SWITCH_EVENT_STARTUP:
		break;
	case SWITCH_EVENT_SHUTDOWN:
		break;
	case SWITCH_EVENT_PUBLISH:
		break;
	case SWITCH_EVENT_UNPUBLISH:
		break;
	case SWITCH_EVENT_TALK:
		break;
	case SWITCH_EVENT_NOTALK:
		break;
	case SWITCH_EVENT_SESSION_CRASH:
		break;
	case SWITCH_EVENT_MODULE_LOAD:
		break;
	case SWITCH_EVENT_MODULE_UNLOAD:
		break;
	case SWITCH_EVENT_DTMF:
		fs->OnDtmf(event);
		break;
	case SWITCH_EVENT_MESSAGE:
		break;
	case SWITCH_EVENT_PRESENCE_IN:
		break;
	case SWITCH_EVENT_NOTIFY_IN:
		break;
	case SWITCH_EVENT_PRESENCE_OUT:
		break;
	case SWITCH_EVENT_PRESENCE_PROBE:
		break;
	case SWITCH_EVENT_MESSAGE_WAITING:
		break;
	case SWITCH_EVENT_MESSAGE_QUERY:
		break;
	case SWITCH_EVENT_ROSTER:
		break;
	case SWITCH_EVENT_CODEC:
		break;
	case SWITCH_EVENT_BACKGROUND_JOB:
		break;
	case SWITCH_EVENT_DETECTED_SPEECH:
		break;
	case SWITCH_EVENT_DETECTED_TONE:
		break;
	case SWITCH_EVENT_PRIVATE_COMMAND:
		break;
	case SWITCH_EVENT_HEARTBEAT:
		break;
	case SWITCH_EVENT_TRAP:
		break;
	case SWITCH_EVENT_ADD_SCHEDULE:
		break;
	case SWITCH_EVENT_DEL_SCHEDULE:
		break;
	case SWITCH_EVENT_EXE_SCHEDULE:
		break;
	case SWITCH_EVENT_RE_SCHEDULE:
		break;
	case SWITCH_EVENT_RELOADXML:
		break;
	case SWITCH_EVENT_NOTIFY:
		break;
	case SWITCH_EVENT_SEND_MESSAGE:
		break;
	case SWITCH_EVENT_RECV_MESSAGE:
		break;
	case SWITCH_EVENT_REQUEST_PARAMS:
		break;
	case SWITCH_EVENT_CHANNEL_DATA:
		break;
	case SWITCH_EVENT_GENERAL:
		break;
	case SWITCH_EVENT_COMMAND:
		break;
	case SWITCH_EVENT_SESSION_HEARTBEAT:
		break;
	case SWITCH_EVENT_CLIENT_DISCONNECTED:
		break;
	case SWITCH_EVENT_SERVER_DISCONNECTED:
		break;
	case SWITCH_EVENT_SEND_INFO:
		break;
	case SWITCH_EVENT_RECV_INFO:
		break;
	}
	
}


static void *SWITCH_THREAD_FUNC listener_run(switch_thread_t *thread, void *obj)
{
	THREAD_INFO* thread_info=(THREAD_INFO*)obj;
	
	FreeSWITCHSocket socket(thread_info->socket,thread_info->pool );
	XMLSession csta_session(platform, &socket, fs);
	
	char remote_ip[50];
	switch_sockaddr_t *sa;
	switch_socket_addr_get(&sa, SWITCH_TRUE, thread_info->socket);
	switch_get_addr(remote_ip, sizeof(remote_ip), sa);	
	if(		(Config::Instance()->IsACLEnabled())
		&&	(!switch_check_network_list_ip(remote_ip, Config::Instance()->GetACLName())) )
	{		
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "IP %s Rejected by acl \"%s\"\n", remote_ip, Config::Instance()->GetACLName());		
		UniversalErrorResponse err(0,error_type_security,genericError);
		csta_session.SendMsg(&err);
		socket.Close();		
	}
	else
		csta_session.Run();

	delete thread_info;
	return NULL;
}


SWITCH_MODULE_LOAD_FUNCTION(mod_csta_socket_load)
{	
	*module_interface = switch_loadable_module_create_module_interface(pool, modname);
	switch_application_interface_t *app_interface;
	
	fs=new FreeSWITCH(pool);
	platform=new FreeSWITCHPlatform();

	bRunning=true;
	Config::Instance()->ReadConfig();

	SWITCH_ADD_APP(app_interface, "csta_trigger_queue", "Notifies about call entered a CSTA queue", "Notifies CSTA module about a call entered a CSAT queue.", queue_function, "", SAF_SUPPORT_NOMEDIA);
	SWITCH_ADD_APP(app_interface, "csta_trigger_trunk_inbound", "Notifies about call entering FreeSWITCH from outside", "Notifies CSTA module about about call entering FreeSWITCH from outside.", trunk_inbound_function, "", SAF_SUPPORT_NOMEDIA);
	SWITCH_ADD_APP(app_interface, "csta_trigger_trunk_outbound", "Notifies about call leaving FreeSWITCH", "Notifies CSTA module about call leaving FreeSWITCH.", trunk_outbound_function, "", SAF_SUPPORT_NOMEDIA);

	//TODO: filter events
	if (switch_event_bind_removable(modname, SWITCH_EVENT_ALL, SWITCH_EVENT_SUBCLASS_ANY, event_handler, NULL, &node) != SWITCH_STATUS_SUCCESS) 
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Couldn't bind to events!\n");
		return SWITCH_STATUS_GENERR;
	}

	return SWITCH_STATUS_SUCCESS;
}

SWITCH_MODULE_SHUTDOWN_FUNCTION(mod_csta_socket_shutdown)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Shutting down..\n");
	bRunning=false;
	if(listen_socket)
	{
		switch_socket_shutdown(listen_socket, SWITCH_SHUTDOWN_READWRITE);
		switch_socket_close(listen_socket);
		listen_socket=NULL;
	}

	switch_event_unbind(&node);

	delete fs;
	delete platform;
	fs=NULL;
	platform=NULL;
	
	return SWITCH_STATUS_SUCCESS;
}



SWITCH_MODULE_RUNTIME_FUNCTION(mod_csta_socket_runtime)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "Starting..\n");

	switch_status_t rv;
	
	switch_sockaddr_t *sa;
	switch_memory_pool_t *pool = NULL, *listener_pool = NULL;
	switch_socket_t *inbound_socket = NULL;	

	if (switch_core_new_memory_pool(&pool) != SWITCH_STATUS_SUCCESS) 
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "switch_core_new_memory_pool failed.\n");
		return SWITCH_STATUS_TERM;
	}

	rv = switch_sockaddr_info_get(&sa, Config::Instance()->GetListenAddress(), SWITCH_INET, Config::Instance()->GetXmlPort(), 0, pool);
	if(rv)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "switch_sockaddr_info_get failed.\n");
		return SWITCH_STATUS_TERM;
	}
	rv = switch_socket_create(&listen_socket, switch_sockaddr_get_family(sa), SOCK_STREAM, SWITCH_PROTO_TCP, pool);
	if(rv)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "switch_socket_create failed.\n");
		return SWITCH_STATUS_TERM;
	}

	rv = switch_socket_opt_set(listen_socket, SWITCH_SO_REUSEADDR, 1);
	if (rv)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "switch_socket_opt_set failed.\n");
		return SWITCH_STATUS_TERM;
	}
	rv = switch_socket_bind(listen_socket, sa);
	if (rv)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "switch_socket_bind failed.\n");
		return SWITCH_STATUS_TERM;
	}
	rv = switch_socket_listen(listen_socket, 5);
	if (rv)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "switch_socket_listen failed.\n");
		return SWITCH_STATUS_TERM;
	}
			
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "CSTA-XML socket up listening on %s:%u\n", Config::Instance()->GetListenAddress(),Config::Instance()->GetXmlPort());

	while(bRunning)
	{
		if (switch_core_new_memory_pool(&listener_pool) != SWITCH_STATUS_SUCCESS) 
		{
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "OH OH no pool\n");
			return SWITCH_STATUS_TERM;
		}
		
		if ((rv = switch_socket_accept(&inbound_socket, listen_socket, listener_pool))) {
			if (!bRunning) {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_NOTICE, "Shutting Down\n");
			} else {
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "Socket Error\n");
			}
			break;
		}
		

		switch_thread_t *thread;
		switch_threadattr_t *thd_attr = NULL;
		THREAD_INFO * thread_info=new THREAD_INFO;
		thread_info->socket=inbound_socket;
		thread_info->pool=listener_pool;

		switch_threadattr_create(&thd_attr, listener_pool);
		switch_threadattr_detach_set(thd_attr, 1);
		switch_threadattr_stacksize_set(thd_attr, SWITCH_THREAD_STACKSIZE);
		switch_thread_create(&thread, thd_attr, listener_run, thread_info, listener_pool);		

	}

	if(listen_socket)
	{
		switch_socket_shutdown(listen_socket, SWITCH_SHUTDOWN_READWRITE);
		//switch_socket_close(listen_socket);
		listen_socket=NULL;
	}

	return SWITCH_STATUS_TERM;
}


