#if !defined K3L_H
#define K3L_H

#include "k3lVersion.h"
#include "KDefs.h"
#include "KTypeDefs.h"
#include "KVoIP.h"
#include "KISDN.h"
#include "KGSM.h"

enum KDeviceType
{
	kdtE1     = 0,
	kdtFXO    = 1,
	kdtConf   = 2,
	kdtPR     = 3,
	kdtE1GW   = 4,
	kdtFXOVoIP = 5,
	kdtE1IP	  = 6,
	kdtE1Spx  = 7,
    kdtGWIP   = 8,
    kdtFXS    = 9,
    kdtFXSSpx = 10,
    kdtGSM    = 11,
    kdtGSMSpx = 12
};

enum KSignaling
{
	ksigInactive		= 0,
	ksigR2Digital		= 1,
	ksigContinuousEM	= 2,
	ksigPulsedEM		= 3,
	ksigUserR2Digital	= 4,
	ksigAnalog			= 5,
	ksigOpenCAS			= 6,
	ksigOpenR2			= 7,
	ksigSIP 			= 8,
    ksigOpenCCS         = 9,
    ksigPRI_EndPoint    = 10,
    ksigAnalogTerminal  = 11,
    ksigPRI_Network     = 12,
    ksigPRI_Passive     = 13,
	ksigLineSide  		= 14,
	ksigCAS_EL7			= 15,
    ksigGSM             = 16,
	ksigE1LC			= 17,
};

enum KE1DeviceModel
{
	kdmE1600	= 0,
	kdmE1600E	= 1,
	kdmE1600EX  = 2
};

enum KE1GWDeviceModel //Mirror of KE1DeviceModel
{
	kdmE1GW640   = 1,
    kdmE1GW640EX = 2
};

enum KE1IPDeviceModel //Mirror of KE1DeviceModel
{
	kdmE1IP   = 1,
    kdmE1IPEX = 2
};

enum KGWIPDeviceModel //Mirror of KE1DeviceModel
{
	kdmGWIP   = 1,
	kdmGWIPEX = 2
};

enum KFXODeviceModel
{
	kdmFXO80    = 0,
	kdmFXOHI    = 1,
	kdmFXO160HI = 2
};

enum KFXOVoIPDeviceModel //Mirror of KFXDeviceModel
{
	kdmFXGW180 = kdmFXO80
};

enum KConfDeviceModel
{
	kdmConf240   = 0,
	kdmConf120   = 1,
	kdmConf240EX = 2,
	kdmConf120EX = 3
};

enum KPRDeviceModel
{
	kdmPR300v1         = 0,
    kdmPR300           = 1,
	kdmPR300SpxBased   = 2,
    kdmPR300EX         = 3
};

enum KFXSDeviceModel
{
    kdmFXS300   = 1,
    kdmFXS300EX = 2
};

enum KFXSSpxDeviceModel
{
    kdmFXSSpx300      = 0,
    kdmFXSSpx2E1Based = 1,
    kdmFXSSpx300EX    = 2
};

enum KE1SpxDeviceModel
{
	kdmE1Spx    = 0,
	kdm2E1Based = 1,
    kdmE1SpxEX  = 2
};

enum KGSMDeviceModel
{
    kdmGSM      = 0
};

enum KGSMSpxDeviceModel
{
    kdmGSMSpx   = 0
};

enum KSystemObject
{
	ksoLink		= 0x00,		// K3L_LINK_STATUS, K3L_LINK_CONFIG (ksoLink + LinkNumber).
	ksoLinkMon	= 0x20,		// K3L_LINK_ERROR_COUNTER.
	ksoChannel	= 0x1000,	// K3L_CHANNEL_STATUS, K3L_CHANNEL_CONFIG (ksoChannel + ChannelNumber).
	ksoH100		= 0x200,	// H100 individual configs. Used by adding the same value passed to CM_SETUP_H100.
	ksoFirmware = 0x80,		// Structures described below (ksoFirmware + KFirmwareId).
	ksoDevice	= 0x100,	// K3L_DEVICE_CONFIG
	ksoAPI		= 0x150		// K3L_API_CONFIG
};

enum KFirmwareId
{
	kfiE1600A,				// K3L_E1600A_FW_CONFIG
	kfiE1600B,				// K3L_E1600B_FW_CONFIG
	kfiFXO80,				// K3L_FX80_FW_CONFIG
    kfiGSM40                // K3L_GSM40_FW_CONFIG
};

enum KE1Status
{
	kesOk					= 0x00,		// Link OK
	kesSignalLost			= 0x01,		// Lost of signal
	kesNetworkAlarm			= 0x02,		// Central office reporting a fail in some point
	kesFrameSyncLost		= 0x04,		// Frame is out of sync
	kesMultiframeSyncLost	= 0x08,		// Multiframe alingment fail
	kesRemoteAlarm			= 0x10,		// Central office reporting fail
	kesHighErrorRate		= 0x20,		// Excessive error rate
	kesUnknownAlarm			= 0x40,		// Unknown alarm
	kesE1Error				= 0x80,		// E1 controller damaged
    kesNotInitialized       = 0xFF      // Framer not initilialized
};

enum KE1ChannelStatus
{
	kecsFree			= 0x00,			// Channel is free for use
	kecsBusy			= 0x01,			// Channel busy (not free)
	kecsOutgoing		= 0x02,			// Channel busy by outgoing call
	kecsIncoming		= 0x04,			// Channel busy by incoming call
	kecsLocked			= 0x06,			// Channel locked
	kecsOutgoingLock	= 0x10,			// Channel locked for outgoing calls
	kecsLocalFail		= 0x20,			// Channel fail
	kecsIncomingLock	= 0x40,			// Channel locked for incoming calls
	kecsRemoteLock		= 0x80			// Channel remotelly locked
};

enum KVoIPChannelStatus
{
	kipcsFree			= kecsFree,		
	kipcsOutgoingLock	= kecsOutgoingLock,
	kipcsIncomingLock	= kecsIncomingLock
};

enum KFXOChannelStatus
{
	kfcsDisabled	= 0x00,		// Channel is not activated
	kfcsEnabled		= 0x01		// Channel is actived		
};

enum KFXSChannelStatus
{
    kfxsOnHook,
    kfxsOffHook,
    kfxsRinging,
    kfxsFail
};

enum KGsmChannelStatus
{
    kgsmIdle,
    kgsmCallInProgress,
    kgsmSMSInProgress,
    kgsmModemError,
    kgsmSIMCardError,
    kgsmNetworkError,
    kgsmNotReady
};

enum KCallStatus
{
	kcsFree		= 0x00,				// Channel is free for use
	kcsIncoming = 0x01,				// Channel in incoming call
	kcsOutgoing = 0x02,				// Channel in outgoing call
	kcsFail		= 0x04				// Channel fail
};

enum KCallStartInfo
{
    kcsiHumanAnswer,
    kcsiAnsweringMachine,
    kcsiCellPhoneMessageBox,
    kcsiUnknown,
    kcsiCarrierMessage
};

enum KChannelFeatures
{
	kcfDtmfSuppression	= 0x0001,
	kcfCallProgress		= 0x0002,
	kcfPulseDetection	= 0x0004,
	kcfAudioNotification= 0x0008,
	kcfEchoCanceller	= 0x0010,
	kcfAutoGainControl	= 0x0020,
	kcfHighImpEvents	= 0x0080,
    kcfCallAnswerInfo   = 0x0100,
    kcfOutputVolume     = 0x0200,
    kcfPlayerAGC        = 0x0400
};

enum KMixerSource
{
	kmsChannel,
	kmsPlay,
	kmsGenerator,
	kmsCTbus,
    kmsNoDelayChannel
};

struct KMixerCommand
{
    int32 Track;
    int32 Source;        // KMixerSource
    int32 SourceIndex;
};

struct KPlayFromStreamCommand
{
	void *Buffer;
	uint32 BufferSize;
	int32 CodecIndex;
};

struct KBufferParam
{
	void *Buffer;
	uint32 BufferSize;
};

enum KMixerTone
{
	kmtSilence	= 0x00,	// geracao/deteccao
	kmtDial		= 0x01, // geracao/deteccao
	kmtBusy		= 0x02, // geracao
	kmtFax		= 0x03, // geracao/deteccao
	kmtVoice	= 0x04, // deteccao
	kmtEndOf425 = 0x05,	// deteccao
    kmtCollect  = 0x06, // deteccao
    kmtEndOfDtmf= 0x07, // deteccao
};

struct K3L_CHANNEL_STATUS
{
 	KCallStatus CallStatus;
	KMixerTone AudioStatus;
	int32 AddInfo;			// KE1ChannelStatus for E1 Channels, KFXOChannelStatus for Analog...
	int32 EnabledFeatures;	// KChannelFeatures bitwise
};

struct K3L_LINK_STATUS
{
	int16 E1; // (KE1Status) int16 used for compatibility between different memory aligniments
	byte Channels[ KMAX_E1_CHANNELS ];	// (KE1ChannelStatus) Channel number indexed vector with firmware information.
};

enum KPllErrors
{
	kpeClockAError    = 0x01,
	kpeClockBError    = 0x02,
	kpeSCbusError     = 0x04,
	kpeMVIPError	  = 0x08,
	kpeMasterPllError = 0x10,
	kpeModeError      = 0x20,
	kpeLocalRefError  = 0x40,
	kpeInternalError  = 0x80

};

struct K3L_H100_STATUS
{
	int32 Mode;
	int32 MasterClock;
	int32 Enable;
	int32 Reference;

	int32 SCbus;
	int32 HMVIP;
	int32 MVIP90;
	int32 CT_NETREF;
	int32 PllLocalRef;

	int32 PllErrors;		// bitwise, defined in KPllErrors
};

enum KEchoLocation
{
	kelNetwork	= 0x0,
	kelCtBus	= 0x1
};

enum KCodecIndex
{
	kci8kHzALAW	= 0x00,
	kci8kHzPCM	= 0x01,  // not suitable for CM_RECORD_TO_FILE_EX
	kci11kHzPCM	= 0x02,  // not suitable for CM_RECORD_TO_FILE_EX
	kci8kHzGSM	= 0x03, 
	kci8kHzADPCM= 0x04, 
	kci8kHzULAW	= 0x05,
	kciLastCodecEntryMark
};

enum KEchoCancellerConfig
{
    keccNotPresent,
    keccOneSingleBank,
    keccOneDoubleBank,
    keccTwoSingleBank,
    keccTwoDoubleBank,
    keccFail
};

struct K3L_DEVICE_CONFIG
{
	int32 LinkCount;
	int32 ChannelCount;
	int32 EnabledChannelCount;
	int32 MixerCount;
	int32 MixerCapacity;
	int32 WorkStatus;
	int32 DeviceModel;		// KE1DeviceModel, KFXODeviceModel...
	int32 H100_Mode;		// KH100Mode
	int32 PciBus;
	int32 PciSlot;
	int32 PlayerCount;
	int32 VoIPChannelCount;
    int32 CTbusCount;
    KEchoCancellerConfig EchoConfig;
    KEchoLocation EchoLocation;
	sbyte SerialNumber[ KMAX_SERIAL_NUMBER ];
};

// Compatibility
typedef K3L_DEVICE_CONFIG K3L_E1_DEVICE_CONFIG;
typedef K3L_DEVICE_CONFIG K3L_FX_DEVICE_CONFIG;

struct K3L_API_CONFIG
{
	int32 MajorVersion;
	int32 MinorVersion;
	int32 BuildVersion;
    int32 SvnRevision;
	int32 RawCmdLogging;
	int32 VpdVersionNeeded;
	sbyte StrVersion[ KMAX_STR_VERSION ];
};

struct K3L_LINK_CONFIG
{
	KSignaling Signaling;
	int32 IncomingDigitsRequest;
	int32 IdentificationRequestPos;
	int32 ChannelCount;
	int32 ReceivingClock;
	sbyte NumberA[ KMAX_DIAL_NUMBER + 1 + 3 ]; // +3 = compatibility between different memory alignments
};

struct K3L_CHANNEL_CONFIG
{
	KSignaling Signaling;
    int32 AutoEnableFeatures;   // KChannelFeatures bitwise
    int32 CapableFeatures;      // future implementation
};

struct K3L_E1600A_FW_CONFIG
{
	int32 MfcExchangersCount;
	int32 MonitorBufferSize;
	sbyte FwVersion[ KMAX_STR_VERSION ];
	sbyte DspVersion[ KMAX_DSP_NAME ];
};

struct K3L_E1600B_FW_CONFIG
{
	int32 AudioBufferSize;
	int32 FilterCount;
	int32 MixerCount;
	int32 MixerCapacity;
	sbyte FwVersion[ KMAX_STR_VERSION ];
	sbyte DspVersion[ KMAX_DSP_NAME ];
};

typedef K3L_E1600B_FW_CONFIG K3L_GSM40_FW_CONFIG;
typedef K3L_E1600B_FW_CONFIG K3L_FXO80_FW_CONFIG;

struct K3L_COMMAND
{
	int32 Object;			// Object index, dependent of the context
	int32 Cmd;				// Command code
	byte *Params;			// pointer to parameters
};

// Used to specify the object passed in the first parameter when executing the
// event handler callback function.
enum KEventObjectId
{
	koiDevice		= 0x00,
	koiChannel		= 0x01,
	koiPlayer		= 0x02,
	koiLink			= 0x03
};


struct K3L_EVENT
{
	int32 Code;				// API code
	int32 AddInfo;			// Parameter 1
	int32 DeviceId;			// Hardware information
	int32 ObjectInfo;		// Additional object information
	void *Params;			// Pointer to the parameter buffer
	int32 ParamSize;		// Size of parameter buffer
	int32 ObjectId;			// KEventObjectId: Event thrower object id
};

struct KIncomingSeizeParams
{
	sbyte NumberB[ KMAX_DIAL_NUMBER + 1 ];
	sbyte NumberA[ KMAX_DIAL_NUMBER + 1 ];

	sbyte Padding[2];  // for compatibility between different memory alignments
};

struct KCtbusCommand
{
	int32 Stream;
	int32 TimeSlot;
	int32 Enable;
};

struct KUserInformation
{
    int32 ProtocolDescriptor;
    int32 UserInfoLength;
    byte  UserInfo[ KMAX_USER_USER_LEN ];
};

struct KISDNSubaddressInformation
{
    KQ931TypeOfSubaddress TypeOfSubaddress;
    bool  OddNumberOfSignals;
    int32 InformationLength;
    byte  Information[ KMAX_SUBADRESS_INFORMATION_LEN ];
};

struct KISDNSubaddresses
{
    KISDNSubaddressInformation Called;
    KISDNSubaddressInformation Calling;
};

enum KLinkErrorCounter
{
	klecChangesToLock		=  0,
	klecLostOfSignal		=  1,
	klecAlarmNotification   =  2,
	klecLostOfFrame         =  3,
	klecLostOfMultiframe    =  4,
	klecRemoteAlarm			=  5,
	klecUnknowAlarm			=  6,
	klecPRBS				=  7,
	klecWrogrBits			=  8,
	klecJitterVariation		=  9,
	klecFramesWithoutSync	= 10,
	klecMultiframeSignal	= 11,
	klecFrameError			= 12,
	klecBipolarViolation	= 13,
	klecCRC4				= 14,
	klecCount				= 15
};

struct K3L_LINK_ERROR_COUNTER
{
	int32 ErrorCounters[ klecCount ];
};

struct KSipData
{
    int32 DataLength;
    byte  Data[ KMAX_SIP_DATA ];
};


enum KLibParams
{
	klpDebugFirmware,           // starts low level depuration
    klpResetFwOnStartup,		// force firmware (re)boot on startup
    klpResetFwOnShutdown,       // force board reset on exit
    klpSeizureEventCompat,      // keep compatibility of EV_SEIZURE/EV_VOIP_SEIZURE instead of the new EV_NEW_CALL
    klpDisableTDMBufferWarnings,// don't log TDM buffer warning messages.
    klpDisableInternalVoIP,     // disables ALL VoIP channels!
    klpLogCallControl,        // starts k3l's call control logging
	klpMaxParams
};

// Definition of K3L_CALLBACK
typedef stt_code ( Kstdcall K3L_CALLBACK )();
typedef stt_code ( Kstdcall K3L_MONITOR_CALLBACK )( byte *, byte );
typedef stt_code ( Kstdcall K3L_EVENT_CALLBACK )( int32 Object, K3L_EVENT * );
typedef stt_code ( Kstdcall *K3L_THREAD_CALLBACK )( void * );
typedef void	 ( Kstdcall K3L_AUDIO_CALLBACK )( int32 DevId, int32 Channel, byte *Buffer, int32 Size );

#if __GNUC__ >= 4
#pragma GCC visibility push(default)
#endif

extern "C"
{

sbyte *Kstdcall k3lStart( int32 Major, int32 Minor, int32 Build );

void Kstdcall k3lStop();

void Kstdcall k3lRegisterEventHandler( K3L_EVENT_CALLBACK *Function );

void Kstdcall k3lRegisterAudioListener( K3L_AUDIO_CALLBACK *Player, K3L_AUDIO_CALLBACK *Recorder );

int32 Kstdcall k3lSendCommand( int32 DeviceId, K3L_COMMAND *Cmd );

int32 Kstdcall k3lSendRawCommand( int32 DeviceId, int32 IntfId, void *Command, int32 CmdSize );

int32 Kstdcall k3lRegisterMonitor( K3L_MONITOR_CALLBACK *EventMonitor, K3L_MONITOR_CALLBACK *CommandMonitor, K3L_MONITOR_CALLBACK *BufferMonitor );

int32 Kstdcall k3lGetDeviceConfig( int32 DeviceId, int32 Object, void *Data, int32 DataSize );

int32 Kstdcall k3lGetDeviceStatus( int32 DeviceId, int32 Object, void *Data, int32 DataSize );

int32 Kstdcall k3lGetDeviceCount();

int32 Kstdcall k3lGetDeviceType( int32 DeviceId );

int32 Kstdcall k3lGetEventParam( K3L_EVENT *Evt, const sbyte *Name, sbyte *Buffer, int32 BufferSize );

int32 Kstdcall k3lSetGlobalParam( int32 ParamIndex, int32 ParamValue );

}

#if __GNUC__ >= 4
#pragma GCC visibility pop
#endif

#endif


