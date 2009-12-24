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

#ifndef CI_FREE_SWITCH_H
#define CI_FREE_SWITCH_H

#include "CICTIHandler.h"
#include "CISession.h"
#include "CSTAEvent.h"
#include <map>
#include <list>
#include <string>


using namespace std;



class FreeSwitchCall;

enum eDirection
{
	direction_incoming,	//!< attached device is the called (on internal subscriber) or call is incoming (on network interface)
	direction_outgoing, //!< attached device is the caller (on internal subscriber) or call is outgoing (on network interface)
};

class FreeSwitchConnection
{
public:
	FreeSwitchCall* pCall;
	string strUUID;					//<! Unique ID of the connection (could be different from the call's unique ID)
	eLocalConnectionState state;	//!< Local connection state
	eDirection	direction;			//!< direction of the call	
	string strChannel;				//!< channel name
};

typedef map<FreeSwitchCall*,FreeSwitchConnection*> MapCall2Connection;
typedef map<string, CISession*> MapStr2Session;

enum FreeSwitchDeviceType
{
	type_internal_subscriber=0,
	type_network_interface,
	type_queue,
	type_unknown,
};

class FreeSwitchDevice
{
protected:
	string m_strDevice;	
	MapStr2Session m_MonitoringSessions;				//!< Sessions monitoring this device

	FreeSwitchDeviceType m_deviceType;

	bool m_bIsMakeCallOriginator;					
public:
	static std::string Channel2Device(const char* channel);	
	
	static char * Type2Prefix(FreeSwitchDeviceType type);
	
	//! finds out FreeSWITCH device type of CSTA DeviceID object
	/*!
		CSTA devices with type "dialingNumber" will mapped to internal subscribers (Note, that in some cases, 
		the implementation will ignore the type field in CSTA devices and interprets the CallNr as a dial string, 
		e.g in MakeCall request the device type is checked for callingDevice but not for calledDirectoryNumber).

		CSTA devices with type "deviceNumber" are used to represent non subscriber type entities. 
		For FreeSWITCH implementation, the number string begins with 2 byte long device 
		type identifier and a device identifier. The device identifier must only contain numeric characters

		Current device type prefixes:
		"02" - Queue

		\sa queue_function
	*/
	static FreeSwitchDeviceType Device2Type(DeviceID device);
public:
	MapCall2Connection connections;	//!< Active connections of this device

	FreeSwitchDevice(const char* pcDevice, FreeSwitchDeviceType deviceType=type_internal_subscriber);

	string& GetDeviceID() {return m_strDevice;}

	bool IsMonitoredWithID(string crossRefID);
	
	bool AddMonitor(CISession* session, string crossRefID);

	bool RemoveMonitor(CISession* session);

	bool RemoveMonitor(CISession* session, string crossRefID);

	bool IsMonitored() {return m_MonitoringSessions.size()>0?true:false;}

	FreeSwitchConnection* FindConnection(const char* uuid);

	FreeSwitchConnection* FindConnection(FreeSwitchCall* pCall);

	FreeSwitchConnection* AddConnection(const char* uuid, const char * channel, FreeSwitchCall* pCall, eDirection direction);

	void DeleteConnection(FreeSwitchCall* pCall);

	//! Sends CSTA event to all sessions monitoring the device
	void SendEvent(CSTAEvent* pEvent);

	FreeSwitchDeviceType GetDeviceType() {return m_deviceType;}

	int GetConnectionCount() {return connections.size();}

	bool IsMakeCallOriginator() {return m_bIsMakeCallOriginator;}

	void SetMakeCallOriginator (bool bMakeCallOriginator) {m_bIsMakeCallOriginator=bMakeCallOriginator;}
};

typedef map<string,FreeSwitchDevice*> MapStr2Device;
typedef map<string,FreeSwitchCall*> MapStr2Call;

class FreeSwitchCall
{
public:
	string strUUID;					//!< Unique ID (used for CSTA CallID) - the FreeSWITCH unique ID of the first connection in the call.
	MapStr2Device devices;	

	FreeSwitchDevice* GetDevice(std::string strDeviceID);
};




//! CSTA Inside CICTIHandler interface realization for FreeSWITCH.
/*!
Provides FreeSWITCH specific telephony methods to used by CSTA Inside.
*/
class FreeSWITCH : public CICTIHandler
{
protected:
	 MapStr2Device m_DeviceMap;			//!< Stores the known devices. Map key is the device id (usually the dialing number)
	 MapStr2Call m_CallMap;				//!< Stores the active calls. Map key is the call ID (1st FreeSWITCH UUID)
	 switch_mutex_t *m_mutex;			//!< To protect the device and call map
	 switch_memory_pool_t * m_pool;

	 int m_nLastXRefID;

	 bool FindCallerCalled(const char* uuid, string* pstrCaller, string* pstrCalled);

	 bool PrepareCallData(const char* uuid, const char* uuid2, const char* channel, FreeSwitchDevice** ppDevice, FreeSwitchCall** ppCall, FreeSwitchConnection** ppConnection, bool bCreateCall, FreeSwitchDevice** ppQueue, FreeSwitchDevice** ppInboundNID, FreeSwitchDevice** ppOutboundNID, string* pstrCaller, string* pstrCalled);

public:
	FreeSWITCH(switch_memory_pool_t *pool);

	virtual ~FreeSWITCH();

	FreeSwitchDevice * PrepareDevice(const char * pcDevice, FreeSwitchDeviceType type=type_internal_subscriber);

	FreeSwitchCall* GetCall(const char * pcUUID);

	bool IsXRefIDUsed(string crossRefID);	//!< Returns true if given XRefID already used in the current CTI handler

	string GetNextXRefID();	//!< Returns the next available monitor cross reference ID	

//Switch event handlers:
	void OnChannelCreate(switch_event_t *event);	//!< Handling CHANNEL_CREATE FreeSwitch event

	void OnChannelHangup(switch_event_t *event);	//!< Handling CHANNEL_HANGUP FreeSwitch event

	void OnChannelAnswer(switch_event_t *event);	//!< Handling CHANNEL_ANSWER FreeSwitch event

	void OnProgress(switch_event_t *event);			//!< Handling CHANNEL_PROGRESS_MEDIA and CHANNEL_PROGRESS events

	void OnDtmf(switch_event_t *event);				//!< Handling DTMF FreeSWITCH event
	
	void OnCallQueued(const char* queue, switch_core_session_t *session);		//!< Handling incoming call on queue

	void OnCallNetworkIncoming(const char* trunk, switch_core_session_t *session);		//!< Handling incoming call on a gateway

	void OnCallNetworkOutgoing(const char* trunk, switch_core_session_t *session);		//!< Handling outgoing call on a gateway

	void OnPlayback(switch_event_t *event);			//!< Handling playback application start or stop

	void OnRecord(switch_event_t *event);			//!< Handling record application start or stop

//Session event handlers:

	void OnSessionFinished(CISession* sender);		//!< Called if given session is disconnected

	void MonitorStart(CISession* sender, MonitorStartRequest* pRequest);	//!< called if a Monitor Start request should be processed

	void SnapshotDevice(CISession* sender, SnapshotDeviceRequest* pRequest);	//!< Called if Snapshot Device request is received

	void SnapshotCall(CISession* sender, SnapshotCallRequest* pRequest);	//!< Called if Snapshot Call request is received

	
	void MakeCall(CISession* sender, MakeCallRequest* pRequest);		//!< Called if Make Call request is received

	void AnswerCall(CISession* sender, AnswerCallRequest* pRequest);	//!< Called if Answer Call request is received

	void PlayMessage(CISession* sender, PlayMessageRequest* pRequest);	//!< Called if Play Message request is received

	void RecordMessage(CISession* sender, RecordMessageRequest* pRequest);	//!< Called if Record Message request is received

	void StopMessage(CISession* sender, StopRequest* pRequest);				//!< Called if Stop request is received

	void SingleStepTransferCall(CISession* sender,SingleStepTransferCallRequest* pRequest);	//!< Called if Single Step Transfer Call Message request is received

	void ClearConnection(CISession* sender,ClearConnectionRequest* pRequest);

	void DeflectCall(CISession* sender,DeflectCallRequest* pRequest);	//!< Called if Deflect Call Message request is received
};

#endif	//CI_FREE_SWITCH_H