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
#include <direct.h>
#include "FreeSWITCH.h"

#include "MonitorStartRequest.h"
#include "UniversalErrorResponse.h"
#include "MonitorStartResponse.h"
#include "MakeCallRequest.h"
#include "MakeCallResponse.h"
#include "AnswerCallRequest.h"
#include "AnswerCallResponse.h"
#include "SingleStepTransferCallRequest.h"
#include "SingleStepTransferCallResponse.h"
#include "DeflectCallRequest.h"
#include "DeflectCallResponse.h"
#include "PlayMessageRequest.h"
#include "PlayMessageResponse.h"
#include "RecordMessageRequest.h"
#include "RecordMessageResponse.h"
#include "StopRequest.h"
#include "StopResponse.h"
#include "ServiceInitiatedEvent.h"
#include "OriginatedEvent.h"
#include "DeliveredEvent.h"
#include "EstablishedEvent.h"
#include "ConnectionClearedEvent.h"
#include "QueuedEvent.h"
#include "DtmfDetectedEvent.h"
#include "PlayEvent.h"
#include "RecordEvent.h"
#include "StopEvent.h"
#include "ClearConnectionRequest.h"
#include "ClearConnectionResponse.h"
#include "SnapshotDeviceRequest.h"
#include "SnapshotDeviceResponse.h"
#include "SnapshotCallRequest.h"
#include "SnapshotCallResponse.h"

#include <string>
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class FreeSwitchDevice

/*static*/ std::string FreeSwitchDevice::Channel2Device(const char* channel)
{
	string strDevice;
	if(strncmp(channel, "sofia/internal/", 15)==0)
	{
		if(strncmp(channel+15, "sip:", 4)==0)
			strDevice=channel+19;
		else
			strDevice=channel+15;
	}
	std::string::size_type i=strDevice.find("@");
	if(i!=strDevice.npos)
	{
		strDevice.erase(i);
	}	
	
	return strDevice;
}

/*static*/ char * FreeSwitchDevice::Type2Prefix(FreeSwitchDeviceType type)
{
	switch(type)
	{
	case type_network_interface: return "01";
	case type_queue: return "02";
	default: return "";
	};
}

/*static*/ FreeSwitchDeviceType FreeSwitchDevice::Device2Type(DeviceID device)
{
	if(device.GetTypeOfNumber()==typeOfNumberDialable)
		return type_internal_subscriber;

	if(device.GetCallNr().size()<2)
		return type_unknown;

	for(int i=type_network_interface;Type2Prefix((FreeSwitchDeviceType)i)[0]!=0;i++)
	{
		if(strncmp(Type2Prefix((FreeSwitchDeviceType)i),device.GetCallNr().c_str(),2)==0)
		{
			return (FreeSwitchDeviceType)i;
		}
	}

	return type_unknown;
}


FreeSwitchDevice::FreeSwitchDevice(const char* pcDevice, FreeSwitchDeviceType deviceType/*=type_internal_subscriber*/) : m_strDevice(pcDevice), m_deviceType(deviceType), m_bIsMakeCallOriginator(false)
{
	
}

FreeSwitchConnection* FreeSwitchDevice::FindConnection(const char* uuid)
{
	for(MapCall2Connection::iterator it=connections.begin();it!=connections.end();it++)	
	{
		if(it->first->strUUID==uuid)			
			return it->second;
	}
	return NULL;
}

FreeSwitchConnection* FreeSwitchDevice::FindConnection(FreeSwitchCall* pCall)
{
	MapCall2Connection::iterator it=connections.find(pCall);
	if(it==connections.end())
		return NULL;
	else
		return it->second;
}

FreeSwitchConnection* FreeSwitchDevice::AddConnection(const char* uuid, const char * channel, FreeSwitchCall* pCall, eDirection direction)
{
	FreeSwitchConnection* pConnection=new FreeSwitchConnection();		
	pConnection->pCall=pCall;
	pConnection->strUUID=uuid;
	pConnection->state=STATE_INITIATED;	
	pConnection->direction=direction;
	pConnection->strChannel=channel;
	pCall->devices[GetDeviceID()]=this;
	connections[pCall]=pConnection;
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSwitchDevice::AddConnection: connection %s - %s added.\n",this->m_strDevice.c_str(),pCall->strUUID.c_str());
	
	return pConnection;
}

void FreeSwitchDevice::DeleteConnection(FreeSwitchCall* pCall)
{	
	MapCall2Connection::iterator it=connections.find(pCall);
	if(it==connections.end())
		return;
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSwitchDevice::DeleteConnection: connection %s - %s deleted.\n",this->m_strDevice.c_str(),pCall->strUUID.c_str());

	delete it->second;
	connections.erase(it);

	pCall->devices.erase(this->GetDeviceID());
}


bool FreeSwitchDevice::IsMonitoredWithID(string crossRefID)
{
	return m_MonitoringSessions.find(crossRefID)==m_MonitoringSessions.end()?false:true;
}

bool FreeSwitchDevice::AddMonitor(CISession* session, string crossRefID)
{
	if(IsMonitoredWithID(crossRefID))
		return false;	//XRefID already used
	
	m_MonitoringSessions[crossRefID]=session;
	return true;
}

bool FreeSwitchDevice::RemoveMonitor(CISession* session)
{
	bool found=false;
	for(MapStr2Session::iterator it=m_MonitoringSessions.begin();it!=m_MonitoringSessions.end();it++)
		if(it->second==session)
		{
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSwitchDevice::RemoveMonitor: removing session 0x%08X from device %s\n",session, m_strDevice.c_str());
			it=m_MonitoringSessions.erase(it);
			found=true;
			if(it==m_MonitoringSessions.end())
				break;
		}
	return found;
}

bool FreeSwitchDevice::RemoveMonitor(CISession* session, string crossRefID)
{
	MapStr2Session::iterator it=m_MonitoringSessions.find(crossRefID);
	
	if(it==m_MonitoringSessions.end())
		return false;	//XRefID not found

	if(it->second!=session)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSwitchDevice::RemoveMonitor: XRefID %s mapped to device %sbut from session 0x%08X instead of 0x%08X, removal denied.\n",crossRefID.c_str(), m_strDevice.c_str(), it->second, session);
	}
	m_MonitoringSessions.erase(it);
	return true;
}

void FreeSwitchDevice::SendEvent(CSTAEvent* pEvent)
{
	for(MapStr2Session::iterator it=m_MonitoringSessions.begin();it!=m_MonitoringSessions.end();it++)
	{
		pEvent->SetCrossRefID(it->first);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSwitchDevice::SendEvent: device %s, session %X, sending event: \n%s\n",GetDeviceID().c_str(),it->second,pEvent->GetReadable().c_str());
		it->second->SendMsg(pEvent);	
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class FreeSwitchCall

FreeSwitchDevice* FreeSwitchCall::GetDevice(std::string strDeviceID)
{	
	MapStr2Device::iterator it=devices.find(strDeviceID);
	if(it!=devices.end())
		return it->second;
	else 
		return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// class FreeSwitch

FreeSWITCH::FreeSWITCH(switch_memory_pool_t *pool) : m_pool(pool), m_nLastXRefID(0)
{
	switch_mutex_init(&m_mutex, SWITCH_MUTEX_NESTED, m_pool);
}


FreeSWITCH::~FreeSWITCH()
{
	for(MapStr2Device::iterator it=m_DeviceMap.begin();it!=m_DeviceMap.end();it++)
	{
		delete it->second;
	}
	m_DeviceMap.clear();
	switch_mutex_destroy(m_mutex);
}

string FreeSWITCH::GetNextXRefID()
{
	++m_nLastXRefID;
	if(m_nLastXRefID>100000)
		m_nLastXRefID=1;
	char temp[8];	
	itoa(m_nLastXRefID,temp,10);
	return string(temp);
}


FreeSwitchDevice * FreeSWITCH::PrepareDevice(const char * pcDevice, FreeSwitchDeviceType type/*=type_internal_subscriber*/)
{
	string strFullDevName=FreeSwitchDevice::Type2Prefix(type);
	strFullDevName+=pcDevice;
	
	MapStr2Device::iterator it=m_DeviceMap.find(strFullDevName);
	FreeSwitchDevice* pDevice=NULL;
	if(it==m_DeviceMap.end())
	{
		pDevice=new FreeSwitchDevice(strFullDevName.c_str(),type);
		m_DeviceMap[strFullDevName]=pDevice;
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::PrepareDevice: device %s type %d added.", pcDevice, type);
	}
	else
	{
		pDevice=it->second;
		if(pDevice->GetDeviceType()!=type)
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::PrepareDevice: device %s found but type is different!",pcDevice);

	}

	return pDevice;
}

FreeSwitchCall* FreeSWITCH::GetCall(const char * pcUUID)
{
	if(!pcUUID)
		return NULL;
	MapStr2Call::iterator it=m_CallMap.find(pcUUID);
	if(it==m_CallMap.end())
		return NULL;
	else return it->second;	//TODO: might chech the secondary UUID
}

bool FreeSWITCH::IsXRefIDUsed(string crossRefID)
{
	switch_mutex_lock(m_mutex);
	
	for(MapStr2Device::iterator it=m_DeviceMap.begin();it!=m_DeviceMap.end();it++)
	{
		if(it->second->IsMonitoredWithID(crossRefID))
		{
			switch_mutex_unlock(m_mutex);
			return true;
		}
	}

	switch_mutex_unlock(m_mutex);	
	return false;
}

bool FreeSWITCH::FindCallerCalled(const char* uuid, string* pstrCaller, string* pstrCalled)
{
	switch_core_session_t * session=NULL;
	for(int i=0;i<3;i++)
	{
		session=switch_core_session_locate(uuid);
		if(session)
			break;
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "FreeSWITCH::FindCallerCalled: Session %s not available, waiting.\n",uuid);
		switch_yield(100000);
	}

	if(!session)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::FindCallerCalled: Session %s no longer exists.\n",uuid);
		return false;
	}
	switch_channel_t* channel=switch_core_session_get_channel(session);
	switch_caller_profile_t * caller=switch_channel_get_caller_profile(channel);
	
	if(pstrCaller)
		*pstrCaller=caller->caller_id_number;

	if(pstrCalled)
	{
		if(strncmp(caller->destination_number, "sip:", 4)==0)
			*pstrCalled=caller->destination_number+4;
		else
			*pstrCalled=caller->destination_number;

		std::string::size_type i=(*pstrCalled).find("@");
		if(i!=(*pstrCalled).npos)
		{
			(*pstrCalled).erase(i);
		}	
	}

	switch_core_session_rwunlock(session);
	return true;
}

bool FreeSWITCH::PrepareCallData(const char* uuid, const char* uuid2, const char* channel, FreeSwitchDevice** ppDevice, FreeSwitchCall** ppCall, FreeSwitchConnection** ppConnection, bool bCreateCall, FreeSwitchDevice** ppQueue, FreeSwitchDevice** ppInboundNID, FreeSwitchDevice** ppOutboundNID, string* pstrCaller, string* pstrCalled)
{
	if(!ppDevice || !ppCall)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::PrepareCallData: mandatory parameter is null.\n");
		return false;
	}
	
	//Finding call:
	if( !(*ppCall=GetCall(uuid)) && !(uuid2 && (*ppCall=GetCall(uuid2))) )
	{
		if(!bCreateCall)
		{
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_WARNING, "FreeSWITCH::PrepareCallData: call %s [or %s] not found.\n",uuid, uuid2?uuid2:"NULL");
			return false;
		}
			
		*ppCall=new FreeSwitchCall();
		(*ppCall)->strUUID=uuid;
		m_CallMap[(*ppCall)->strUUID]=*ppCall;
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::PrepareCallData: call %s created.\n",(*ppCall)->strUUID.c_str());
	}	

	//Finding the subscriber:
	if(strncmp(channel,"sofia/external",14)==0)
	{
		//Network interface - finding later
		*ppDevice=NULL;
	}
	else
	{
		//Internal subscriber
		string device=FreeSwitchDevice::Channel2Device(channel);
		if(!device.size())
		{			
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::PrepareCallData: Channel %s not recognized.\n",channel);
			return false;
		}
		*ppDevice=PrepareDevice(device.c_str());	
		if(!(*ppDevice))
		{
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::PrepareCallData: Device %s not recognized.\n",device.c_str());
			return false;
		}	
	}
	if(!*ppDevice || ppQueue || ppInboundNID || ppOutboundNID)
	{

		//Collecting special devices from the call & finding network subscriber:
		for(MapStr2Device::iterator it=(*ppCall)->devices.begin();it!=(*ppCall)->devices.end();it++)
		{
			if(ppQueue && it->second->GetDeviceType()==type_queue)
			{
				*ppQueue=it->second;			
			}
			else
			if(it->second->GetDeviceType()==type_network_interface)
			{
				FreeSwitchConnection* pTempConn=it->second->FindConnection(*ppCall);
				if(!pTempConn)
				{
					switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_CRIT, "FreeSWITCH::PrepareCallData: trunk %s is mapped to call %s without connection????\n",it->second->GetDeviceID().c_str(),(*ppCall)->strUUID.c_str());
					continue;
				}
				if(ppInboundNID && pTempConn->direction==direction_incoming)
					*ppInboundNID=it->second;
				else
				if(ppOutboundNID && pTempConn->direction==direction_outgoing)
					*ppOutboundNID=it->second;

				if( (!*ppDevice) && (pTempConn->strChannel==channel) )
					*ppDevice=it->second;	
				
				if( (!*ppDevice) && (pTempConn->strChannel.size()==0) && (pTempConn->direction==direction_outgoing) )
				{
					//External outbound call: the device was created before the FS channel, updating the data. 				
					*ppDevice=it->second;
					pTempConn->strUUID=uuid;
					pTempConn->strChannel=channel;
				}
			}
		}
	}

	if(!*ppDevice)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::PrepareCallData: Device for channel %s not recognized.\n",channel);
		return false;
	}

	//Finding connection:
	if(ppConnection)
		*ppConnection=(*ppDevice)->FindConnection(*ppCall);
	
	
	//Finding caller & called
	if(pstrCaller || pstrCalled)
	{		
		FindCallerCalled(uuid, pstrCaller, pstrCalled);		
	}

	return true;
}



void FreeSWITCH::OnChannelCreate(switch_event_t *event)
{		
	const char* channel=switch_event_get_header(event, "Channel-Name");
	if(!channel || strstr(channel,"sofia/external"))	//External inbound calls are handled in OnCallNetworkIncoming
		return;
	
	string device=FreeSwitchDevice::Channel2Device(channel);
	if(!device.size())
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnChannelCreate: Channel %s not recognized.\n",channel);
		return;
	}
	switch_mutex_lock(m_mutex);
	FreeSwitchDevice* pDevice=PrepareDevice(device.c_str());
	if(!pDevice)	
		return;
	const char* direction=switch_event_get_header(event, "Call-Direction");
	const char* uuid=switch_event_get_header(event, "Unique-ID");
	if(		!direction || !uuid 
		|| (strcmp(direction, "inbound") && !pDevice->IsMakeCallOriginator()) )	//direction= "inbound" means outgoing call, direction is wrong from some reason
	{
		switch_mutex_unlock(m_mutex);
		return;
	}

	if(GetCall(uuid))
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnChannelCreate: Call %s already exists.\n",uuid);
		switch_mutex_unlock(m_mutex);
		return;
	}

	FreeSwitchCall* pCall=new FreeSwitchCall();
	pCall->strUUID=uuid;
	m_CallMap[pCall->strUUID]=pCall;
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnChannelCreate: call %s created.\n",pCall->strUUID.c_str());

	pDevice->AddConnection(uuid, switch_event_get_header(event, "Channel-Name"), pCall,direction_outgoing);
	if(pDevice->IsMonitored())
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnChannelCreate: ServiceInitiated device: %s, call: %s.\n",pDevice->GetDeviceID().c_str(),uuid);
		ServiceInitiatedEvent si(ConnectionID(pDevice->GetDeviceID(),pCall->strUUID));
		if(pDevice->IsMakeCallOriginator())
			si.SetEventCause(CAUSE_MAKE_CALL);
		pDevice->SendEvent(&si);
	}
	switch_mutex_unlock(m_mutex);
}


void FreeSWITCH::OnProgress(switch_event_t *event)
{
	const char* channel=switch_event_get_header(event, "Channel-Name");
	const char* uuid=switch_event_get_header(event, "Unique-ID");
	const char* uuid2=switch_event_get_header(event, "Other-Leg-Unique-ID");
	if(!channel || !uuid)				
		return;	
	
	FreeSwitchCall* pCall;	
	FreeSwitchDevice* pDevice=NULL, *pInboundNIDDevice=NULL, *pOutboundNIDDevice=NULL;
	string caller, called;
	FreeSwitchConnection* pConnection=NULL;
	
	switch_mutex_lock(m_mutex);
	if(!PrepareCallData(uuid, uuid2, channel, &pDevice, &pCall, &pConnection, true, NULL, &pInboundNIDDevice, &pOutboundNIDDevice, &caller, &called))
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnProgress: PrepareCallData failed.\n");
		return;
	}
	
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnProgress: %s device: %s, call: %s [%s].\n",event->event_id==SWITCH_EVENT_CHANNEL_PROGRESS?"CHANNEL_PROGRESS":"CHANNEL_PROGRESS_MEDIA",pDevice->GetDeviceID().c_str(), uuid, uuid2?uuid2:"NULL");
		
	if(!pConnection)
	{
		pConnection=pDevice->AddConnection(uuid,channel,pCall,direction_incoming);
		pConnection->state=STATE_ALERTING;
	}
	else
		pConnection->state=STATE_CONNECTED;
			
	DeviceID alertingDevice(called);

	OriginatedEvent oe(ConnectionID(caller,pCall->strUUID), caller, called);	
	oe.SetLocalConnectionInfo(STATE_CONNECTED);
	DeliveredEvent de(ConnectionID(alertingDevice,pCall->strUUID), alertingDevice, caller, called);
	de.SetLocalConnectionInfo(pConnection->state);

	if(pInboundNIDDevice)
	{
		oe.SetNetworkCalledDevice(called);
		oe.SetNetworkCallingDevice(caller);
		oe.SetAssociatedCallingDevice(DeviceID(typeOfNumberDevice,pInboundNIDDevice->GetDeviceID()));

		de.SetNetworkCalledDevice(called);
		de.SetNetworkCallingDevice(caller);
		de.SetAssociatedCallingDevice(DeviceID(typeOfNumberDevice,pInboundNIDDevice->GetDeviceID()));
	}
	if(pOutboundNIDDevice)
	{
		de.SetAssociatedCalledDevice(DeviceID(typeOfNumberDevice,pOutboundNIDDevice->GetDeviceID()));
		de.SetConnection(ConnectionID(DeviceID(typeOfNumberDevice,pOutboundNIDDevice->GetDeviceID()),pCall->strUUID));
	}
		
	if(pDevice->IsMonitored())
	{
		if(pConnection->state==STATE_CONNECTED)
			pDevice->SendEvent(&oe);
		pDevice->SendEvent(&de);		
	}

	switch_mutex_unlock(m_mutex);
}


void FreeSWITCH::OnChannelAnswer(switch_event_t *event)
{
	const char* channel=switch_event_get_header(event, "Channel-Name");
	const char* uuid=switch_event_get_header(event, "Unique-ID");
	const char* uuid2=switch_event_get_header(event, "Other-Leg-Unique-ID");
	if(!channel || !uuid)				
		return;	
	
	FreeSwitchCall* pCall;	
	FreeSwitchDevice* pDevice=NULL, *pQueueDevice=NULL, *pInboundNIDDevice=NULL, *pOutboundNIDDevice=NULL;
	string caller, called;
	FreeSwitchConnection* pConnection=NULL;
	
	switch_mutex_lock(m_mutex);
	if(!PrepareCallData(uuid, uuid2, channel, &pDevice, &pCall, &pConnection, true, &pQueueDevice, &pInboundNIDDevice, &pOutboundNIDDevice, &caller, &called))
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnChannelAnswer: PrepareCallData failed.\n");
		return;
	}
	
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnChannelAnswer: CHANNEL_ANSWER device: %s, call: %s [%s].\n",pDevice->GetDeviceID().c_str(), uuid, uuid2?uuid2:"NULL");
		
	if(!pConnection)
	{
		pConnection=pDevice->AddConnection(uuid,channel,pCall,direction_incoming);
	}

	pConnection->state=STATE_CONNECTED;
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnChannelAnswer: Established device: %s, call: %s.\n",pDevice->GetDeviceID().c_str(),pCall->strUUID.c_str());
		
	DeviceID answeringDevice;
	if(pQueueDevice)
	{
		answeringDevice.SetCallNr(pQueueDevice->GetDeviceID().c_str());
		answeringDevice.SetTypeOfNumber(typeOfNumberDevice);
	}
	else
	{
		answeringDevice.SetCallNr(called);
	}
	EstablishedEvent ee(ConnectionID(answeringDevice,pCall->strUUID), answeringDevice, caller, called);

	if(pInboundNIDDevice)
	{
		ee.SetNetworkCalledDevice(called);
		ee.SetNetworkCallingDevice(caller);
		ee.SetAssociatedCallingDevice(DeviceID(typeOfNumberDevice,pInboundNIDDevice->GetDeviceID()));
	}
	if(pOutboundNIDDevice)
	{
		ee.SetAssociatedCalledDevice(DeviceID(typeOfNumberDevice,pOutboundNIDDevice->GetDeviceID()));
		ee.SetEstablishedConnection(ConnectionID(DeviceID(typeOfNumberDevice,pOutboundNIDDevice->GetDeviceID()),pCall->strUUID));
	}
		
	if(pDevice->IsMonitored())
		pDevice->SendEvent(&ee);	
		
	if(pQueueDevice && pQueueDevice->IsMonitored())
		pQueueDevice->SendEvent(&ee);	

	switch_mutex_unlock(m_mutex);
}

void FreeSWITCH::OnChannelHangup(switch_event_t *event)
{
	char* channel=switch_event_get_header(event, "Channel-Name");
	const char* uuid=switch_event_get_header(event, "Unique-ID");
	const char* uuid2=switch_event_get_header(event, "Other-Leg-Unique-ID");
	const char* cause=switch_event_get_header(event, "Hangup-Cause");
	if(!channel || !uuid)				
		return;	

	FreeSwitchCall* pCall=NULL;	
	FreeSwitchDevice* pDevice=NULL, *pQueueDevice=NULL;	

	switch_mutex_lock(m_mutex);
	if(!PrepareCallData(uuid, uuid2, channel, &pDevice, &pCall, NULL, false, NULL, NULL, NULL, NULL, NULL))
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnChannelHangup: PrepareCallData failed.\n");
		return;
	}	
	
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnChannelHangup: CHANNEL_HANGUP device: %s, call: %s [%s], cause: %s.\n",pDevice->GetDeviceID().c_str(),uuid, uuid2?uuid2:"NULL", cause?cause:"NULL");

	ConnectionClearedEvent cc(ConnectionID(pDevice->GetDeviceID(),pCall->strUUID));

	//Setting cause
	if(!cause || !strcmp(cause,"NORMAL_CLEARING") )
		cc.SetEventCause(CAUSE_NORMAL_CLEARING);
	else
	if(!strcmp(cause,"NO_ROUTE_DESTINATION"))
		cc.SetEventCause(CAUSE_DEST_NOT_OBTAINABLE);
	else
	if(!strcmp(cause,"USER_BUSY"))
		cc.SetEventCause(CAUSE_BUSY);
	if(!strcmp(cause,"ORIGINATOR_CANCEL"))
		cc.SetEventCause(CAUSE_CALL_NOT_ANSWERED);

	int devices_in_call=pCall->devices.size();	
	//Sending event to all devices in the call
	FreeSwitchConnection* pConnection=NULL;
	for(MapStr2Device::iterator it=pCall->devices.begin();it!=pCall->devices.end();it++)
	{		
		pConnection=it->second->FindConnection(pCall->strUUID.c_str());
		if(!pConnection)
		{
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnChannelHangup: Device %s could not have connection to %s.",it->second->GetDeviceID().c_str(),pCall->strUUID.c_str());
			continue;
		}
		
		if(it->second->GetDeviceType()==type_queue)
		{
			pConnection->state=STATE_NULL;	
			pQueueDevice=it->second;
		}
		else
		if(it->second==pDevice)			
			pConnection->state=STATE_NULL;	//The terminated device
		else
		if(devices_in_call<=2)
			pConnection->state=STATE_FAIL;	//If only one device remain, it will be in failed state
											//If more than one devices remain (conference), they will have the original state.	
		
		if(it->second->IsMonitored())
		{			
			cc.SetLocalConnectionInfo(pConnection->state);
			it->second->SendEvent(&cc);			
		}	
	}
	pDevice->DeleteConnection(pCall);
	if(pQueueDevice)
	{
		if(pQueueDevice->IsMonitored())
		{
			ConnectionClearedEvent cc2(ConnectionID(DeviceID(typeOfNumberDevice,pQueueDevice->GetDeviceID()),pCall->strUUID));
			cc2.SetLocalConnectionInfo(STATE_NULL);
			cc2.SetEventCause(cc.GetEventCause());
			pQueueDevice->SendEvent(&cc2);
		}
		pQueueDevice->DeleteConnection(pCall);
	}

	if(pCall->devices.size()==0)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnChannelHangup: call %s deleted.\n",pCall->strUUID.c_str());
		m_CallMap.erase(pCall->strUUID);
		delete pCall;
	}	
	switch_mutex_unlock(m_mutex);
}

void FreeSWITCH::OnDtmf(switch_event_t *event)
{
	char* channel=switch_event_get_header(event, "Channel-Name");
	const char* uuid=switch_event_get_header(event, "Unique-ID");	
	const char* uuid2=switch_event_get_header(event, "Other-Leg-Unique-ID");
	if(!channel || !uuid)				
		return;	

	FreeSwitchCall* pCall=NULL;	
	FreeSwitchDevice* pDevice=NULL, *pQueueDevice=NULL;	
	
	switch_mutex_lock(m_mutex);
	
	if(!PrepareCallData(uuid, uuid2, channel, &pDevice, &pCall, NULL, false, &pQueueDevice, NULL, NULL, NULL, NULL))
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnDtmf: PrepareCallData failed.\n");
		return;
	}	

	const char* digit=switch_event_get_header(event, "DTMF-Digit");
	if(!digit)	
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnDtmf: DTMF-Digit not found in event.");
		switch_mutex_unlock(m_mutex);
		return;
	}

	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnDtmf: DTMF device: %s, call: %s \n",pDevice->GetDeviceID().c_str(),pCall->strUUID.c_str());
	
	DeviceID dev(pDevice->GetDeviceType()==type_internal_subscriber?typeOfNumberDialable:typeOfNumberDevice,pDevice->GetDeviceID());
	DtmfDetectedEvent dd(ConnectionID(dev,pCall->strUUID),digit[0]);

	if(pDevice->IsMonitored())
		pDevice->SendEvent(&dd);
	
	if(pQueueDevice && pQueueDevice->IsMonitored())	
		pQueueDevice->SendEvent(&dd);

	switch_mutex_unlock(m_mutex);

}

void FreeSWITCH::OnPlayback(switch_event_t *event)
{
	char* channel=switch_event_get_header(event, "Channel-Name");
	const char* uuid=switch_event_get_header(event, "Unique-ID");	
	const char* uuid2=switch_event_get_header(event, "Other-Leg-Unique-ID");
	if(!channel || !uuid)				
		return;	

	FreeSwitchCall* pCall=NULL;	
	FreeSwitchDevice* pDevice=NULL;	
	
	switch_mutex_lock(m_mutex);
	
	if(!PrepareCallData(uuid, uuid2, channel, &pDevice, &pCall, NULL, false, NULL, NULL, NULL, NULL, NULL))
	{
		switch_mutex_unlock(m_mutex);
		if(event->event_id!=SWITCH_EVENT_CHANNEL_EXECUTE_COMPLETE)	//Could be normal if call terminates during playback.
			switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnPlayback: PrepareCallData failed.\n");
		return;
	}	

	const char* file=switch_event_get_header(event, "Application-Data");
	if(!file)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnPlayback: Filename not found in event.\n");
		switch_mutex_unlock(m_mutex);
		return;
	}
	
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnPlayback: device: %s, call: %s \n",pDevice->GetDeviceID().c_str(),pCall->strUUID.c_str());
	
	DeviceID dev(pDevice->GetDeviceType()==type_internal_subscriber?typeOfNumberDialable:typeOfNumberDevice,pDevice->GetDeviceID());
	
	CSTAEvent* pEvent=NULL;
	if(event->event_id==SWITCH_EVENT_CHANNEL_EXECUTE_COMPLETE)
	{
		pEvent=new StopEvent(ConnectionID(dev,pCall->strUUID),file);
		char* result=switch_event_get_header(event, "Application-Response");
		if(result)
		{
			if(	strcmp(result, "FILE PLAYED")==0)
				pEvent->SetEventCause(CAUSE_END_OF_MESSAGE_DETECTED);
			else
			if(strcmp(result, "FILE NOT FOUND")==0)
				pEvent->SetEventCause(CAUSE_RESOURCES_NOT_AVAILABLE);
			else
			if(strcmp(result, "PLAYBACK ERROR")==0)
				pEvent->SetEventCause(CAUSE_BLOCKED);
		}		
	}
	else
	{
		pEvent=new PlayEvent(ConnectionID(dev,pCall->strUUID),file);
	}

	for(MapStr2Device::iterator it=pCall->devices.begin();it!=pCall->devices.end();it++)
	{
		if(it->second->IsMonitored())		
			it->second->SendEvent(pEvent);		
	}

	delete pEvent;

	switch_mutex_unlock(m_mutex);
}

void FreeSWITCH::OnRecord(switch_event_t *event)
{
	char* channel=switch_event_get_header(event, "Channel-Name");
	const char* uuid=switch_event_get_header(event, "Unique-ID");	
	const char* uuid2=switch_event_get_header(event, "Other-Leg-Unique-ID");
	if(!channel || !uuid)				
		return;	

	FreeSwitchCall* pCall=NULL;	
	FreeSwitchDevice* pDevice=NULL;	
	
	switch_mutex_lock(m_mutex);
	
	if(!PrepareCallData(uuid, uuid2, channel, &pDevice, &pCall, NULL, false, NULL, NULL, NULL, NULL, NULL))
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnRecord: PrepareCallData failed.\n");
		return;
	}	
	
	char* p=switch_event_get_header(event, "Application-Data");
	if(!p)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnRecord: Filename not found in event.\n");
		switch_mutex_unlock(m_mutex);
		return;
	}

	char file[MAX_PATH];
	strcpy(file,p);	
	p=strstr(file,".wav ");
	if(p)
		p[4]=0;	//cutting length parameter
	
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnRecord: device: %s, call: %s \n",pDevice->GetDeviceID().c_str(),pCall->strUUID.c_str());
	
	DeviceID dev(pDevice->GetDeviceType()==type_internal_subscriber?typeOfNumberDialable:typeOfNumberDevice,pDevice->GetDeviceID());
	
	CSTAEvent* pEvent=NULL;
	if(event->event_id==SWITCH_EVENT_CHANNEL_EXECUTE_COMPLETE)
	{
		pEvent=new StopEvent(ConnectionID(dev,pCall->strUUID),file);
		pEvent->SetEventCause(CAUSE_MESSAGE_DURATION_EXCEEDED);		
	}
	else
	{
		pEvent=new RecordEvent(ConnectionID(dev,pCall->strUUID),file);
	}

	for(MapStr2Device::iterator it=pCall->devices.begin();it!=pCall->devices.end();it++)
	{
		if(it->second->IsMonitored())		
			it->second->SendEvent(pEvent);		
	}

	delete pEvent;

	switch_mutex_unlock(m_mutex);
}


void FreeSWITCH::OnCallQueued(const char* queue, switch_core_session_t *session)
{
	const char* uuid=switch_core_session_get_uuid(session);
	switch_channel_t* channel=switch_core_session_get_channel(session);
	switch_caller_profile_t * caller=switch_channel_get_caller_profile(channel);	

	switch_mutex_lock(m_mutex);
	
	FreeSwitchDevice* pDevice=PrepareDevice(queue,type_queue);
	if(!pDevice)	
		return;

	FreeSwitchCall* pCall=GetCall(uuid);
	if(!pCall)
	{
		pCall=new FreeSwitchCall();
		pCall->strUUID=uuid;
		m_CallMap[pCall->strUUID]=pCall;
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnCallQueued: call %s created.\n",pCall->strUUID.c_str());
	}
	
	FreeSwitchConnection* pConnection=pDevice->AddConnection(uuid, switch_channel_get_name(channel),pCall, direction_incoming);
	pConnection->state=STATE_QUEUED;
	QueuedEvent qe(ConnectionID(DeviceID(typeOfNumberDevice,pDevice->GetDeviceID()),uuid),DeviceID(typeOfNumberDevice,pDevice->GetDeviceID()),caller->caller_id_number, caller->destination_number);
	
	if(strstr(caller->chan_name,"sofia/external"))
	{		
		qe.SetNetworkCallingDevice(caller->caller_id_number);
		qe.SetNetworkCalledDevice(caller->destination_number);
		for(MapStr2Device::iterator it=pCall->devices.begin();it!=pCall->devices.end();it++)		
			if(it->second->GetDeviceType()==type_network_interface)
			{
				qe.SetAssociatedCallingDevice(DeviceID(typeOfNumberDevice,it->second->GetDeviceID()));
				break;
			}
		
	}	
	qe.SetNumberQueued(pDevice->GetConnectionCount());
	for(MapStr2Device::iterator it=pCall->devices.begin();it!=pCall->devices.end();it++)
	{
		//Sending queued event for the queue and the caller.
		if(it->second->IsMonitored())		
			it->second->SendEvent(&qe);
	}


	switch_mutex_unlock(m_mutex);
}

void FreeSWITCH::OnCallNetworkIncoming(const char* trunk, switch_core_session_t *session)
{
	const char* uuid=switch_core_session_get_uuid(session);
	switch_channel_t* channel=switch_core_session_get_channel(session);	
	//switch_caller_profile_t * caller=switch_channel_get_caller_profile(channel);

	switch_mutex_lock(m_mutex);
	
	FreeSwitchDevice* pDevice=PrepareDevice(trunk,type_network_interface);
	if(!pDevice)	
		return;

	FreeSwitchCall* pCall=GetCall(uuid);
	if(!pCall)
	{
		pCall=new FreeSwitchCall();
		pCall->strUUID=uuid;
		m_CallMap[pCall->strUUID]=pCall;
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnCallNetworkIncoming: call %s created.\n",pCall->strUUID.c_str());
	}
	
	FreeSwitchConnection* pConnection=pDevice->AddConnection(uuid, switch_channel_get_name(channel), pCall, direction_incoming);
	pConnection->strChannel=switch_channel_get_name(channel);	
	if(pDevice->IsMonitored())
	{
		ServiceInitiatedEvent si(ConnectionID(DeviceID(typeOfNumberDevice,pDevice->GetDeviceID()),uuid));
		//TODO: add networkCallingDevice, networkCalledDevice, associatedCallingDevice fields
		pDevice->SendEvent(&si);
	}
	switch_mutex_unlock(m_mutex);
}

void FreeSWITCH::OnCallNetworkOutgoing(const char* trunk, switch_core_session_t *session)
{
	const char* uuid=switch_core_session_get_uuid(session);
	//switch_channel_t* channel=switch_core_session_get_channel(session);	
	//switch_caller_profile_t * caller=switch_channel_get_caller_profile(channel);

	switch_mutex_lock(m_mutex);
	
	FreeSwitchDevice* pDevice=PrepareDevice(trunk,type_network_interface);
	if(!pDevice)	
		return;

	FreeSwitchCall* pCall=GetCall(uuid);
	if(!pCall)
	{
		pCall=new FreeSwitchCall();
		pCall->strUUID=uuid;
		m_CallMap[pCall->strUUID]=pCall;
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnCallNetworkOutgoing: call %s created.\n",pCall->strUUID.c_str());
	}
	
	//outgoing uuid and channel name is not yet known.
	pDevice->AddConnection("", "", pCall, direction_outgoing);
	
	if(pDevice->IsMonitored())
	{
		//TODO: Network Reached
	}
	switch_mutex_unlock(m_mutex);
}



void FreeSWITCH::OnSessionFinished(CISession* sender)
{
	switch_mutex_lock(m_mutex);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::OnSessionFinished: removing session 0x%08X\n",sender);
	
	try
	{
		for(MapStr2Device::iterator it=m_DeviceMap.begin();it!=m_DeviceMap.end();it++)
		{
			it->second->RemoveMonitor(sender);
		}
	}
	catch(...)
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::OnSessionFinished: Exception.");
	}

	switch_mutex_unlock(m_mutex);	
}

void FreeSWITCH::MonitorStart(CISession* sender, MonitorStartRequest* pRequest)
{	
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::MonitorStart: Request with InvokeID: %d\n:%s\n",pRequest->GetInvokeID(), pRequest->GetReadable().c_str());
	
	if(!pRequest->GetMonitorObject().GetCallNr().size())
	{
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation, invalidMonitorObject);
		sender->SendMsg(&err);
		return;
	}
	
	
	FreeSwitchDeviceType type=FreeSwitchDevice::Device2Type(pRequest->GetMonitorObject());
	if(type==type_unknown)
	{
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation, invalidMonitorObject);
		sender->SendMsg(&err);
		return;
	}

	switch_mutex_lock(m_mutex);
	FreeSwitchDevice* pDevice=PrepareDevice(pRequest->GetMonitorObject().GetCallNr().c_str()+(type==type_internal_subscriber?0:2), type);
	
	string crossRefID=pDevice->GetDeviceID().c_str();
	for(crossRefID=pDevice->GetDeviceID();IsXRefIDUsed(crossRefID);crossRefID=GetNextXRefID());
	
	if(pDevice->AddMonitor(sender, crossRefID))
	{
		MonitorStartResponse resp(pRequest->GetInvokeID(),crossRefID);
		sender->SendMsg(&resp);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::MonitorStart: Response sent with InvokeID: %d\n:%s\n",resp.GetInvokeID(), resp.GetReadable().c_str());
	}
	else
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::MonitorStart: AddMonitor failed.");
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation, invalidMonitorObject);
		sender->SendMsg(&err);
	}
	switch_mutex_unlock(m_mutex);
}


void FreeSWITCH::SnapshotDevice(CISession* sender, SnapshotDeviceRequest* pRequest)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SnapshotDevice: Request with InvokeID: %d\n:%s\n",pRequest->GetInvokeID(), pRequest->GetReadable().c_str());
	
	FreeSwitchDeviceType type=FreeSwitchDevice::Device2Type(pRequest->GetSnapshotObject());
	if(type==type_unknown)
	{
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation, invalidDeviceID);
		sender->SendMsg(&err);
		return;
	}

	switch_mutex_lock(m_mutex);
	FreeSwitchDevice* pDevice=PrepareDevice(pRequest->GetSnapshotObject().GetCallNr().c_str()+(type==type_internal_subscriber?0:2), type);
	
	SnapshotDeviceResponse resp(pRequest->GetInvokeID());
	
	for(MapCall2Connection::iterator it=pDevice->connections.begin();it!=pDevice->connections.end();it++)	
	{
		SnapshotDeviceData data(ConnectionID(pRequest->GetSnapshotObject(),it->second->pCall->strUUID),it->second->state);
		resp.AddSnapshotData(data);
	}

	sender->SendMsg(&resp);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SnapshotDevice: Response sent with InvokeID: %d\n:%s\n",resp.GetInvokeID(), resp.GetReadable().c_str());

	switch_mutex_unlock(m_mutex);
}

void FreeSWITCH::SnapshotCall(CISession* sender, SnapshotCallRequest* pRequest)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SnapshotCall: Request with InvokeID: %d\n:%s\n",pRequest->GetInvokeID(), pRequest->GetReadable().c_str());

	switch_mutex_lock(m_mutex);	
	FreeSwitchCall* pCall=GetCall(pRequest->GetSnapshotObject().GetCallID().c_str());	
	if(!pCall)
		
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::SnapshotCall: CallID %s not found.",pRequest->GetSnapshotObject().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidCallID);
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SnapshotCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	SnapshotCallResponse resp(pRequest->GetInvokeID());
	
	for(MapStr2Device::iterator it=pCall->devices.begin();it!=pCall->devices.end();it++)
	{
		FreeSwitchConnection* pConn=it->second->FindConnection(pCall); 
		SnapshotCallData data(it->second->GetDeviceID(),pConn?pConn->state:STATE_NULL);		
		resp.AddSnapshotData(data);
	}
	string caller, called;
	FindCallerCalled(pCall->strUUID.c_str(), &caller, &called);
	resp.SetCallingDevice(caller);
	resp.SetCalledDevice(called);

	sender->SendMsg(&resp);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SnapshotDevice: Response sent with InvokeID: %d\n:%s\n",resp.GetInvokeID(), resp.GetReadable().c_str());

	switch_mutex_unlock(m_mutex);

}


void FreeSWITCH::MakeCall(CISession* sender, MakeCallRequest* pRequest)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::MakeCall: Request with InvokeID: %d\n:%s\n",pRequest->GetInvokeID(), pRequest->GetReadable().c_str());

	switch_channel_t *caller_channel;
	switch_core_session_t *caller_session = NULL;
	char aleg[400], exten[400];
	uint32_t timeout = 60;
	switch_call_cause_t cause = SWITCH_CAUSE_NORMAL_CLEARING;
	FreeSwitchDevice* pDevice=NULL;
		
	if(pRequest->GetCallingDevice().GetTypeOfNumber()==typeOfNumberDialable)
	{
		sprintf(aleg,"%ssofia/internal/%s%s%s",pRequest->GetAutoOriginate()==ORIGINATE_DO_NOT_PROMPT?"{sip_auto_answer=true}":"",pRequest->GetCallingDevice().GetCallNr().c_str(),"%",switch_core_get_variable("domain"));
		pDevice=PrepareDevice(pRequest->GetCallingDevice().GetCallNr().c_str());
		pDevice->SetMakeCallOriginator(true);
	}
	else
	{
		sprintf(aleg,"loopback/%s",pRequest->GetCallingDevice().GetCallNr().c_str()+2);
	}
	sprintf(exten,"%s",pRequest->GetCalledDirectoryNumber().GetCallNr().c_str());
		
	if (switch_ivr_originate(NULL, &caller_session, &cause, aleg, timeout, NULL, "CSTA", pRequest->GetCallingDevice().GetCallNr().c_str(), NULL, NULL, SOF_NONE) != SWITCH_STATUS_SUCCESS	|| !caller_session) 
	{		
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "-ERR Cannot Create Outgoing Channel! [%s] cause: %s\n", aleg, switch_channel_cause2str(cause));
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,genericError); //TODO: refine error code
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::MakeCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		if(pDevice)
			pDevice->SetMakeCallOriginator(false);
		return;
	}

	caller_channel = switch_core_session_get_channel(caller_session);
	switch_channel_clear_state_handler(caller_channel, NULL);
	
	switch_ivr_session_transfer(caller_session, exten, "XML", "default");	
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "+OK Created Session: %s\n", switch_core_session_get_uuid(caller_session));
	
	MakeCallResponse resp(pRequest->GetInvokeID(),ConnectionID(pRequest->GetCallingDevice(),switch_core_session_get_uuid(caller_session)));
	sender->SendMsg(&resp);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::MakeCall: Response sent with InvokeID: %d\n:%s\n",resp.GetInvokeID(), resp.GetReadable().c_str());
	if(pDevice)
		pDevice->SetMakeCallOriginator(false);
	
	switch_core_session_rwunlock(caller_session);	
	
}


void FreeSWITCH::AnswerCall(CISession* sender, AnswerCallRequest* pRequest)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::AnswerCall: Request with InvokeID: %d\n:%s\n",pRequest->GetInvokeID(), pRequest->GetReadable().c_str());

	switch_mutex_lock(m_mutex);

	FreeSwitchCall* pCall=GetCall(pRequest->GetCallToBeAnswered().GetCallID().c_str());	
	if(!pCall)
		
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::AnswerCall: CallID %s not found.",pRequest->GetCallToBeAnswered().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidCallID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::AnswerCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	FreeSwitchDevice * pDevice=pCall->GetDevice(pRequest->GetCallToBeAnswered().GetDeviceID()->GetCallNr());	
	if(!pDevice)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::AnswerCall: Device %s not found.",pRequest->GetCallToBeAnswered().GetDeviceID()->GetCallNr().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::AnswerCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}
	FreeSwitchConnection* pConnection=pDevice->FindConnection(pCall);
	if(!pConnection)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::AnswerCall: Connection %s/%s not found.",pRequest->GetCallToBeAnswered().GetDeviceID()->GetCallNr().c_str(),pRequest->GetCallToBeAnswered().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::AnswerCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}
	switch_core_session_t* session=switch_core_session_locate(pConnection->strUUID.c_str());

	if(!session)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::AnswerCall: No session for CallID %s found.",pRequest->GetCallToBeAnswered().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidCallID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::AnswerCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}
	switch_channel_t * channel=switch_core_session_get_channel(session);
	switch_channel_clear_flag(channel, CF_PROXY_MEDIA);
	switch_channel_clear_flag(channel, CF_PROXY_MODE);
	if(switch_channel_answer(channel)==SWITCH_STATUS_SUCCESS)
	{
		AnswerCallResponse resp(pRequest->GetInvokeID());
		sender->SendMsg(&resp);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::AnswerCall: Response sent with InvokeID: %d\n:%s\n",resp.GetInvokeID(), resp.GetReadable().c_str());
	}
	else
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::AnswerCall: switch_channel_answer failed.");
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,genericError); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::AnswerCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());

	}
	switch_core_session_rwunlock(session);
	
	switch_mutex_unlock(m_mutex);
}

void FreeSWITCH::PlayMessage(CISession* sender, PlayMessageRequest* pRequest)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::PlayMessage: Request with InvokeID: %d\n:%s\n",pRequest->GetInvokeID(), pRequest->GetReadable().c_str());

	switch_mutex_lock(m_mutex);

	FreeSwitchCall* pCall=GetCall(pRequest->GetOverConnection().GetCallID().c_str());	
	if(!pCall)
		
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::PlayMessage: CallID %s not found.",pRequest->GetOverConnection().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidCallID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::PlayMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	FreeSwitchDevice * pDevice=pCall->GetDevice(pRequest->GetOverConnection().GetDeviceID()->GetCallNr());	
	if(!pDevice)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::PlayMessage: Device %s not found.",pRequest->GetOverConnection().GetDeviceID()->GetCallNr().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::PlayMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}
	FreeSwitchConnection* pConnection=pDevice->FindConnection(pCall);
	if(!pConnection)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::PlayMessage: Connection %s/%s not found.",pRequest->GetOverConnection().GetDeviceID()->GetCallNr().c_str(),pRequest->GetOverConnection().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::PlayMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}
	
	if(switch_ivr_broadcast(pConnection->strUUID.c_str(), pRequest->GetMessageToBePlayed().c_str(), SMF_ECHO_ALEG)==SWITCH_STATUS_SUCCESS)
	{
		PlayMessageResponse resp(pRequest->GetInvokeID());
		sender->SendMsg(&resp);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::PlayMessage: Response sent with InvokeID: %d\n:%s\n",resp.GetInvokeID(), resp.GetReadable().c_str());
	}
	else
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::PlayMessage: switch_ivr_broadcast failed.");
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,genericError); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::PlayMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
	}

	switch_mutex_unlock(m_mutex);
}

void FreeSWITCH::RecordMessage(CISession* sender, RecordMessageRequest* pRequest)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::RecordMessage: Request with InvokeID: %d\n:%s\n",pRequest->GetInvokeID(), pRequest->GetReadable().c_str());
	
	switch_mutex_lock(m_mutex);

	FreeSwitchCall* pCall=GetCall(pRequest->GetCallToBeRecorded().GetCallID().c_str());	
	if(!pCall)		
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::RecordMessage: CallID %s not found.",pRequest->GetCallToBeRecorded().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidCallID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::RecordMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	FreeSwitchDevice * pDevice=pCall->GetDevice(pRequest->GetCallToBeRecorded().GetDeviceID()->GetCallNr());	
	if(!pDevice)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::RecordMessage: Device %s not found.",pRequest->GetCallToBeRecorded().GetDeviceID()->GetCallNr().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::RecordMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}
	FreeSwitchConnection* pConnection=pDevice->FindConnection(pCall);
	if(!pConnection)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::RecordMessage: Connection %s/%s not found.",pRequest->GetCallToBeRecorded().GetDeviceID()->GetCallNr().c_str(),pRequest->GetCallToBeRecorded().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::RecordMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	switch_core_session_t *session=switch_core_session_locate(pConnection->strUUID.c_str());
	if(!session)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::RecordMessage: Session %s not found.",pConnection->strUUID.c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidCallID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::RecordMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}
	

	if ((switch_channel_test_flag(switch_core_session_get_channel(session), CF_EVENT_PARSE))) 
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::RecordMessage: Record or broadcast already in progress.");
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,genericError); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::RecordMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	char filename[MAX_PATH];
	sprintf(filename,"%s-%X.wav",pRequest->GetCallToBeRecorded().GetCallID().c_str(),clock());
	char arg[MAX_PATH+20];
	
	if(pRequest->GetMaxDuration()>1000)
	{
		sprintf(arg, "%s %d",filename,pRequest->GetMaxDuration()/1000);
	}
	else
	{
		strcpy(arg,filename);
	}
	
    char path[MAX_PATH];          
	strcat(path,switch_channel_get_variable(switch_core_session_get_channel(session), "sound_prefix"));
	strcat(path,SWITCH_PATH_SEPARATOR);
	strcat(path,filename);
	
	switch_event_t *event;
	if (switch_event_create(&event, SWITCH_EVENT_COMMAND) == SWITCH_STATUS_SUCCESS) 
	{		
		switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "call-command", "execute");
		switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "execute-app-name", "record");
		switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "execute-app-arg", arg);			
		switch_event_add_header_string(event, SWITCH_STACK_BOTTOM, "event-lock", "true");
		switch_core_session_queue_private_event(session, &event);		

		RecordMessageResponse resp(pRequest->GetInvokeID(),path);
		sender->SendMsg(&resp);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::RecordMessage: Response sent with InvokeID: %d\n:%s\n",resp.GetInvokeID(), resp.GetReadable().c_str());
	}
	else
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::RecordMessage: switch_event_create failed.");
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,genericError); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::RecordMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
	}
	switch_core_session_rwunlock(session);
	switch_mutex_unlock(m_mutex);
}

void FreeSWITCH::StopMessage(CISession* sender, StopRequest* pRequest)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::StopMessage: Request with InvokeID: %d\n:%s\n",pRequest->GetInvokeID(), pRequest->GetReadable().c_str());
	
	switch_mutex_lock(m_mutex);

	FreeSwitchCall* pCall=GetCall(pRequest->GetConnection().GetCallID().c_str());	
	if(!pCall)		
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::StopMessage: CallID %s not found.",pRequest->GetConnection().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidCallID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::StopMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	FreeSwitchDevice * pDevice=pCall->GetDevice(pRequest->GetConnection().GetDeviceID()->GetCallNr());	
	if(!pDevice)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::StopMessage: Device %s not found.",pRequest->GetConnection().GetDeviceID()->GetCallNr().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::StopMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}
	FreeSwitchConnection* pConnection=pDevice->FindConnection(pCall);
	if(!pConnection)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::StopMessage: Connection %s/%s not found.",pRequest->GetConnection().GetDeviceID()->GetCallNr().c_str(),pRequest->GetConnection().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::StopMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	switch_core_session_t *session=switch_core_session_locate(pConnection->strUUID.c_str());
	if(!session)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::StopMessage: Session %s not found.",pConnection->strUUID.c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidCallID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::StopMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}	

	if (!switch_channel_test_flag(switch_core_session_get_channel(session), CF_EVENT_PARSE)) 
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::RecordMessage:No record or broadcast running.");
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,genericError); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::RecordMessage: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}	
	
	switch_ivr_stop_record_session(session, pRequest->GetMessageToBeStopped().size()?pRequest->GetMessageToBeStopped().c_str():"all");
	switch_channel_stop_broadcast(switch_core_session_get_channel(session));

	StopResponse resp(pRequest->GetInvokeID());
	sender->SendMsg(&resp);
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::StopMessage: Response sent with InvokeID: %d\n:%s\n",resp.GetInvokeID(), resp.GetReadable().c_str());	
	
	switch_core_session_rwunlock(session);
	switch_mutex_unlock(m_mutex);
}


void FreeSWITCH::SingleStepTransferCall(CISession* sender,SingleStepTransferCallRequest* pRequest)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SingleStepTransferCall: Request with InvokeID: %d\n:%s\n",pRequest->GetInvokeID(), pRequest->GetReadable().c_str());

	switch_mutex_lock(m_mutex);

	FreeSwitchCall* pCall=GetCall(pRequest->GetActiveCall().GetCallID().c_str());	
	if(!pCall)		
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::SingleStepTransferCall: CallID %s not found.",pRequest->GetActiveCall().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidCallID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SingleStepTransferCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	//Retrieving the device that initiates the transfer
	FreeSwitchDevice * pTransferringDevice=pCall->GetDevice(pRequest->GetActiveCall().GetDeviceID()->GetCallNr());	
	if(!pTransferringDevice)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::SingleStepTransferCall: Device %s not found.",pRequest->GetActiveCall().GetDeviceID()->GetCallNr().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SingleStepTransferCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	for(MapStr2Device::iterator it=pCall->devices.begin();it!=pCall->devices.end();it++)
	{
		if(it->second!=pTransferringDevice)	
		{
			FreeSwitchConnection* pConnectionToTransfer=it->second->FindConnection(pCall);
			if(!pConnectionToTransfer)
			{
				switch_mutex_unlock(m_mutex);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::SingleStepTransferCall: Connection %s/%s not found.",it->second->GetDeviceID().c_str(),pRequest->GetActiveCall().GetCallID().c_str());
				UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
				sender->SendMsg(&err);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SingleStepTransferCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
				return;
			}

			switch_core_session_t* sessionToTransfer=switch_core_session_locate(pConnectionToTransfer->strUUID.c_str());
			if(!sessionToTransfer)
			{
				switch_mutex_unlock(m_mutex);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::SingleStepTransferCall: Session %s not found.",pConnectionToTransfer->strUUID.c_str());
				UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
				sender->SendMsg(&err);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SingleStepTransferCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
				return;
			}

			if(switch_ivr_session_transfer(sessionToTransfer,pRequest->GetTransferredTo().GetCallNr().c_str(),NULL,NULL)==SWITCH_STATUS_SUCCESS)
			{
				SingleStepTransferCallResponse resp(pRequest->GetInvokeID(),ConnectionID(pRequest->GetTransferredTo(),pCall->strUUID));
				sender->SendMsg(&resp);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SingleStepTransferCall: Response sent with InvokeID: %d\n:%s\n",resp.GetInvokeID(), resp.GetReadable().c_str());
			}
			else
			{
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::SingleStepTransferCall: switch_ivr_session_transfer failed.");
				UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,genericError); 
				sender->SendMsg(&err);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::SingleStepTransferCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
			}
			switch_core_session_rwunlock(sessionToTransfer);
		}
	}

	//TODO: send transferred event
	pTransferringDevice->DeleteConnection(pCall);
	
	switch_mutex_unlock(m_mutex);
}

void FreeSWITCH::DeflectCall(CISession* sender,DeflectCallRequest* pRequest)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::DeflectCall: Request with InvokeID: %d\n:%s\n",pRequest->GetInvokeID(), pRequest->GetReadable().c_str());

	switch_mutex_lock(m_mutex);

	FreeSwitchCall* pCall=GetCall(pRequest->GetCallToBeDiverted().GetCallID().c_str());	
	if(!pCall)		
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::DeflectCall: CallID %s not found.",pRequest->GetCallToBeDiverted().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidCallID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::DeflectCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	//Retrieving the device that initiates the transfer
	FreeSwitchDevice * pTransferringDevice=pCall->GetDevice(pRequest->GetCallToBeDiverted().GetDeviceID()->GetCallNr());	
	if(!pTransferringDevice)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::DeflectCall: Device %s not found.",pRequest->GetCallToBeDiverted().GetDeviceID()->GetCallNr().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::DeflectCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	for(MapStr2Device::iterator it=pCall->devices.begin();it!=pCall->devices.end();it++)
	{
		if(it->second!=pTransferringDevice)	
		{
			FreeSwitchConnection* pConnectionToTransfer=it->second->FindConnection(pCall);
			if(!pConnectionToTransfer)
			{
				switch_mutex_unlock(m_mutex);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::DeflectCall: Connection %s/%s not found.",it->second->GetDeviceID().c_str(),pRequest->GetCallToBeDiverted().GetCallID().c_str());
				UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
				sender->SendMsg(&err);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::DeflectCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
				return;
			}

			switch_core_session_t* sessionToTransfer=switch_core_session_locate(pConnectionToTransfer->strUUID.c_str());
			if(!sessionToTransfer)
			{
				switch_mutex_unlock(m_mutex);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::DeflectCall: Session %s not found.",pConnectionToTransfer->strUUID.c_str());
				UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
				sender->SendMsg(&err);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::DeflectCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
				return;
			}

			if(switch_ivr_session_transfer(sessionToTransfer,pRequest->GetNewDestination().GetCallNr().c_str(),NULL,NULL)==SWITCH_STATUS_SUCCESS)
			{
				DeflectCallResponse resp(pRequest->GetInvokeID());
				sender->SendMsg(&resp);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::DeflectCall: Response sent with InvokeID: %d\n:%s\n",resp.GetInvokeID(), resp.GetReadable().c_str());
			}
			else
			{
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::DeflectCall: switch_ivr_session_transfer failed.");
				UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,genericError); 
				sender->SendMsg(&err);
				switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::DeflectCall: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
			}
			switch_core_session_rwunlock(sessionToTransfer);
		}
	}

	//TODO: send transferred event
	pTransferringDevice->DeleteConnection(pCall);
	
	switch_mutex_unlock(m_mutex);
}

void FreeSWITCH::ClearConnection(CISession* sender, ClearConnectionRequest* pRequest)
{
	switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::ClearConnection: Request with InvokeID: %d\n:%s\n",pRequest->GetInvokeID(), pRequest->GetReadable().c_str());

	switch_mutex_lock(m_mutex);

	FreeSwitchCall* pCall=GetCall(pRequest->GetConnectionToBeCleared().GetCallID().c_str());	
	if(!pCall)		
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::ClearConnection: CallID %s not found.",pRequest->GetConnectionToBeCleared().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidCallID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::ClearConnection: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	//Retrieving the device that initiates the transfer
	FreeSwitchDevice * pDevice=pCall->GetDevice(pRequest->GetConnectionToBeCleared().GetDeviceID()->GetCallNr());	
	if(!pDevice)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::ClearConnection: Device %s not found.",pRequest->GetConnectionToBeCleared().GetDeviceID()->GetCallNr().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::ClearConnection: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	FreeSwitchConnection* pConnection=pDevice->FindConnection(pCall);
	if(!pConnection)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::ClearConnection: Connection %s/%s not found.",pDevice->GetDeviceID().c_str(),pRequest->GetConnectionToBeCleared().GetCallID().c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::ClearConnection: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	switch_core_session_t* session=switch_core_session_locate(pConnection->strUUID.c_str());
	if(!session)
	{
		switch_mutex_unlock(m_mutex);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::ClearConnection: Session %s not found.",pConnection->strUUID.c_str());
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,invalidDeviceID); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::ClearConnection: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
		return;
	}

	switch_channel_t *channel = switch_core_session_get_channel(session);	

	if(switch_channel_hangup(channel, SWITCH_CAUSE_NORMAL_CLEARING)==CS_HANGUP)
	{
		ClearConnectionResponse resp(pRequest->GetInvokeID());
		sender->SendMsg(&resp);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::ClearConnection: Response sent with InvokeID: %d\n:%s\n",resp.GetInvokeID(), resp.GetReadable().c_str());
	}
	else
	{
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_ERROR, "FreeSWITCH::ClearConnection: switch_ivr_session_transfer failed.");
		UniversalErrorResponse err(pRequest->GetInvokeID(),error_type_operation,genericError); 
		sender->SendMsg(&err);
		switch_log_printf(SWITCH_CHANNEL_LOG, SWITCH_LOG_DEBUG, "FreeSWITCH::ClearConnection: Response sent with InvokeID: %d\n:%s\n",err.GetInvokeID(), err.GetReadable().c_str());
	}
	switch_core_session_rwunlock(session);
	switch_mutex_unlock(m_mutex);
}