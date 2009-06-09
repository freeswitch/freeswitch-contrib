#if !defined KLTYPEDEFS_H
#define KLTYPEDEFS_H

#if defined( _WINDOWS ) || defined( _Windows ) || defined( _WIN32 )
	#ifndef KWIN32
	#define KWIN32 1
	#endif
#endif

// Khomp defined types
#ifdef KWIN32
	typedef __int64				int64;
	typedef unsigned __int64	uint64;
	#define Kstdcall __stdcall
#else
	typedef long long				int64;
	typedef unsigned long long		uint64;
	#define Kstdcall 
#endif

typedef int					int32;
typedef unsigned int		uint32;

typedef uint64			    intptr;
typedef intptr              stackint;

typedef short int			int16;
typedef unsigned short int	uint16;

typedef char				int8;
typedef unsigned char		uint8;

typedef unsigned char		byte;
typedef char				sbyte;

typedef double				float64;
typedef float				float32;

typedef int32				stt_code;


enum KLibraryStatus 
{
	ksSuccess =			0,
	ksFail =			1,
	ksTimeOut =			2,
	ksBusy =			3,
	ksLocked =			4,
	ksInvalidParams =	5,
	ksEndOfFile =		6,
	ksInvalidState =	7,
	ksServerCommFail =	8,
	ksOverflow =		9,
    ksUnderrun =        10,
	ksNotFound =		11,
    ksNotAvailable =    12
};	

enum KTxRx
{
	kNoTxRx	= 0x0,
	kTx		= 0x1,
	kRx		= 0x2,
	kBoth	= 0x3
}; 


#define KMAX_SERIAL_NUMBER			12 
#define KMAX_E1_CHANNELS			30
#define KMAX_DIAL_NUMBER	        20
#define KMAX_ADDRESS				60
#define KMAX_DSP_NAME		        8
#define KMAX_STR_VERSION	        80
#define KMAX_BUFFER_ADDRESSES		16
#define KMAX_LOG                    1024
#define KMAX_SIP_DATA               248

#endif

