#if !defined KVOIPDEFS_H
#define KVOIPDEFS_H

enum KRejectReason
{
	UserBusy = 0,
	UserNotFound,
	NoAnswer,
	Decline,
	ServiceUnavailable,
	ServerInternalError,
	UnknownRejectReason
};

enum KSIP_Failures
{
	kveResponse_200_OK_Success             		= 200,

    kveRedirection_300_MultipleChoices			= 300,
	kveRedirection_301_MovedPermanently			= 301,
	kveRedirection_302_MovedTemporarily			= 302,
	kveRedirection_305_UseProxy					= 305,
	kveRedirection_380_AlternativeService		= 380,

	kveFailure_400_BadRequest					= 400,
	kveFailure_401_Unauthorized					= 401,
	kveFailure_402_PaymentRequired				= 402,
	kveFailure_403_Forbidden					= 403,
	kveFailure_404_NotFound						= 404,
	kveFailure_405_MethodNotAllowed				= 405,
	kveFailure_406_NotAcceptable				= 406,
	kveFailure_407_ProxyAuthenticationRequired	= 407,
	kveFailure_408_RequestTimeout				= 408,
	kveFailure_410_Gone							= 410,
	kveFailure_413_RequestEntityTooLarge		= 413,
	kveFailure_414_RequestURI_TooLong			= 414,
	kveFailure_415_UnsupportedMediaType			= 415,
	kveFailure_416_UnsupportedURI_Scheme		= 416,
	kveFailure_420_BadExtension					= 420,
	kveFailure_421_ExtensionRequired			= 421,
	kveFailure_423_IntervalTooBrief				= 423,
	kveFailure_480_TemporarilyUnavailable		= 480,
	kveFailure_481_CallDoesNotExist				= 481,
	kveFailure_482_LoopDetected					= 482,
	kveFailure_483_TooManyHops					= 483,
	kveFailure_484_AddressIncomplete			= 484,
	kveFailure_485_Ambiguous					= 485,
	kveFailure_486_BusyHere						= 486,
	kveFailure_487_RequestTerminated			= 487,
	kveFailure_488_NotAcceptableHere			= 488,
	kveFailure_491_RequestPending				= 491,
	kveFailure_493_Undecipherable				= 493,

	kveServer_500_InternalError					= 500,
	kveServer_501_NotImplemented				= 501,
	kveServer_502_BadGateway					= 502,
	kveServer_503_ServiceUnavailable			= 503,
	kveServer_504_TimeOut						= 504,
	kveServer_505_VersionNotSupported			= 505,
	kveServer_513_MessageTooLarge				= 513,

	kveGlobalFailure_600_BusyEverywhere			= 600,
	kveGlobalFailure_603_Decline				= 603,
	kveGlobalFailure_604_DoesNotExistAnywhere	= 604,
	kveGlobalFailure_606_NotAcceptable			= 606
};

enum KVoIPRegTypes
{
   kvrtRegister    =   0,
   kvrtUnregister  =   1
};

// Below structures are deprecated
struct KVoIPCallParams
{
	sbyte ToUser[ KMAX_ADDRESS + 1 ];
	sbyte FromUser[ KMAX_ADDRESS + 1 ];
    sbyte FromUserIP[ KMAX_ADDRESS + 1 ];
};

struct KVoIPEvRegisterParams
{
    KVoIPRegTypes Register;
	sbyte User[ KMAX_ADDRESS + 1 ];
	sbyte ProxyIP[ KMAX_ADDRESS + 1 ];
};

struct KVoIPSeize 
{
	sbyte FromUser[ KMAX_ADDRESS + 1 ];
	sbyte ToUser[ KMAX_ADDRESS + 1 ];
    sbyte ProxyIP[ KMAX_ADDRESS + 1 ];
};

#endif
