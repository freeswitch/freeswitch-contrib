#ifndef _KISDN_H_
#define _KISDN_H_

#define KMAX_USER_USER_LEN              32
#define KMAX_SUBADRESS_INFORMATION_LEN  20 

/**
    KQ931Cause: Causes for disconnection and/or fails.
 */
enum KQ931Cause
{
    kq931cNone                    			= 0,
    kq931cUnallocatedNumber       			= 1,
    kq931cNoRouteToTransitNet     			= 2,
    kq931cNoRouteToDest           			= 3,
	kq931cSendSpecialInfoTone				= 4,
	kq931cMisdialedTrunkPrefix				= 5,
    kq931cChannelUnacceptable     			= 6,
    kq931cCallAwarded             			= 7,
	kq931cPreemption						= 8,
	kq931cPreemptionCircuitReuse			= 9,
	kq931cQoR_PortedNumber					= 14,
    kq931cNormalCallClear         			= 16,
    kq931cUserBusy                			= 17,
    kq931cNoUserResponding        			= 18,
    kq931cNoAnswerFromUser        			= 19,
	kq931cSubscriberAbsent					= 20,
    kq931cCallRejected            			= 21,
    kq931cNumberChanged           			= 22,
	kq931cRedirectionToNewDest				= 23,
	kq931cCallRejectedFeatureDest			= 24,
	kq931cExchangeRoutingError				= 25,
    kq931cNonSelectedUserClear    			= 26,
    kq931cDestinationOutOfOrder   			= 27,
    kq931cInvalidNumberFormat     			= 28,
    kq931cFacilityRejected        			= 29,
    kq931cRespStatusEnquiry       			= 30,
    kq931cNormalUnspecified       			= 31,
    kq931cNoCircuitChannelAvail   			= 34,
    kq931cNetworkOutOfOrder       			= 38,
	kq931cPermanentFrameConnOutOfService	= 39,
	kq931cPermanentFrameConnOperational		= 40,
    kq931cTemporaryFailure          		= 41,
    kq931cSwitchCongestion          		= 42,
    kq931cAccessInfoDiscarded       		= 43,
    kq931cRequestedChannelUnav      		= 44,
	kq931cPrecedenceCallBlocked				= 46,
    kq931cResourceUnavailable       		= 47,
    kq931cQosUnavailable            		= 49,
    kq931cReqFacilityNotSubsc       		= 50,
	kq931cOutCallsBarredWithinCUG			= 53,
	kq931cInCallsBarredWithinCUG			= 55,
    kq931cBearerCapabNotAuthor      		= 57,
    kq931cBearerCapabNotAvail       		= 58,
	kq931cInconsistency						= 62,
    kq931cServiceNotAvailable       		= 63,
    kq931cBcNotImplemented          		= 65,
    kq931cChannelTypeNotImplem      		= 66,
    kq931cReqFacilityNotImplem      		= 69,
    kq931cOnlyRestrictedBcAvail     		= 70,
    kq931cServiceNotImplemented     		= 79,
    kq931cInvalidCrv                		= 81,
    kq931cChannelDoesNotExist       		= 82,
    kq931cCallIdDoesNotExist        		= 83,
    kq931cCallIdInUse               		= 84,
    kq931cNoCallSuspended           		= 85,
    kq931cCallIdCleared             		= 86,
	kq931cUserNotMemberofCUG				= 87,
    kq931cIncompatibleDestination   		= 88,
    kq931cInvalidTransitNetSel      		= 91,
    kq931cInvalidMessage            		= 95,
    kq931cMissingMandatoryIe        		= 96,
    kq931cMsgTypeNotImplemented     		= 97,
    kq931cMsgIncompatWithState      		= 98,
    kq931cIeNotImplemented          		= 99,
    kq931cInvalidIe                 		= 100,
    kq931cMsgIncompatWithState2     		= 101,
    kq931cRecoveryOnTimerExpiry     		= 102,
    kq931cProtocolError             		= 103,
	kq931cMessageWithUnrecognizedParam		= 110,
	kq931cProtocolErrorUnspecified			= 111,
    kq931cInterworking              		= 127,
    kq931cCallConnected             		= 128,
    kq931cCallTimedOut              		= 129,
    kq931cCallNotFound              		= 130,
    kq931cCantReleaseCall           		= 131,
    kq931cNetworkFailure            		= 132,
    kq931cNetworkRestart            		= 133,
    kq931cLastValidCause            		= kq931cNetworkRestart,

};

/**
    KQ931ProgressIndication: Used internally to indicate and to be informed of the 
    presence of call control tones in band.
 */
enum KQ931ProgressIndication
{
    kq931pTonesMaybeAvailable       = 1,
    kq931pDestinationIsNonIsdn      = 2,
    kq931pOriginationIsNonIsdn      = 3,
    kq931pCallReturnedToIsdn        = 4,
    kq931pTonesAvailable            = 8,
};

/**
    KQ931Hlc: High Layer compatibility: Used internally to indicate the call content
    ('kq931hTelefony' is always sent).
 */
enum KQ931Hlc
{
    kq931hTelefony                  = 0x81,
    k1931hFaxGroup23                = 0x84,
    k1931hFaxGroup4                 = 0xa1,
    kq931hTeletexF184               = 0xa4,
    kq931hTeletexF220               = 0xa8,
    kq931hTeletexf200               = 0xb1,
    kq931hVideotex                  = 0xb2,
    kq931hTelexF60                  = 0xb5,
    kq931hMhs                       = 0xb8,
    kq931hOsiApp                    = 0xc1,
    kq931hMaintenance               = 0xde,
    kq931hManagement                = 0xdf,
};


/**
    KQ931BearerCapability: Used internally to indicate the call bearer's capability
    (can be changed in KConfig).
 */
enum KQ931BearerCapability
{
    kq931bSpeech                    = 0x00,
    kq931bUnrestrictedDigital       = 0x08,
    kq931bAudio31kHz                = 0x10,
    kq931bAudio7kHz                 = 0x11,
    kq931bVideo                     = 0x18,
};


/**
    KQ931TypeOfNumber: Indicates the caller/called party type of number. Defaults to
    'kq931tUnknownNumber', but can be changed by a parameter in CM_MAKE_CALL.
*/
enum KQ931TypeOfNumber
{
    kq931tUnknownNumber             = 0x00,
    kq931tInternationalNumber       = 0x10,
    kq931tNationalNumber            = 0x20,
    kq931tNetworkSpecificNumber     = 0x30,
    kq931tSubscriberNumber          = 0x40,
    kq931tAbbreviatedNumber         = 0x60,
    kq931tReservedNumber            = 0x70,
    kq931tDefaultNumber             = kq931tUnknownNumber,
};

/**
    KQ931NumberingPlan: Indicates the caller/called party numbering plan. Defaults to
    'kq931pUnknownPlan', but can be changed by a parameter in CM_MAKE_CALL.
 */
enum KQ931NumberingPlan
{
    kq931pUnknownPlan               = 0x00,
    kq931pIsdnPlan                  = 0x01,
    kq931pDataPlan                  = 0x03,
    kq931pTelexPlan                 = 0x04,
    kq931pNationalPlan              = 0x08,
    kq931pPrivatePlan               = 0x09,
    kq931pReservedPlan              = 0x0F,
    kq931pDefaultPlan               = kq931pUnknownPlan,
};

/**
    KQ931UserInfoProtocolDescriptor: Used in 'struct KUserInformation' (declared in k3l.h) to indicate
    the protocol that is being used with the CM_USER_INFORMATION command.
 */
enum KQ931UserInfoProtocolDescriptor
{
    kq931uuUserSpecific             = 0x00,
    kq931uuOSI_HighLayer            = 0x01,
    kq931uuX244                     = 0x02,
    kq931uuIA5_Chars                = 0x04,
    kq931uuX208_X209                = 0x05,
    kq931uuV120                     = 0x07,
    kq931uuQ931_CallControl         = 0x08,
    kq931uuNational                 = 0x40 //MASK
};

enum KQ931PresentationIndicator
{
	kq931piPresentationAllowed					= 0x00,
	kq931piPresentationRestricted				= 0x01,
	kq931piNumberNotAvailableDueToInterworking	= 0x02,
};

enum KQ931ScreeningIndicator
{
	kq931siUserProvidedNotScreened				= 0x00,
	kq931siUserProvidedVerifiedAndPassed		= 0x01,
	kq931siUserProvidedVerifiedAndFailed		= 0x02,
	kq931siNetworkProvided						= 0x03,
};

enum KQ931TypeOfSubaddress
{
    kq931tsNSAP                                 = 0x00,
    kq931tsUserSpecified                        = 0x01,
};

#endif //_KISDN_H_

