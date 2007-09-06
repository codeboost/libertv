#ifndef __MZ_INC_H__
#define __MZ_INC_H__

#include "LTVUtil.dll.h"
#define IOA_API

typedef unsigned int uint; 
typedef unsigned long ulong; 
typedef unsigned long dword; 
typedef char MZCHAR;
typedef void* MZHANDLE;
typedef long MZBOOL;

#ifdef NULL
#undef NULL
#endif
#define NULL 0

#define MZIO_API
#define MZIO_CAPI


//I'm specifically defining these constants here, so that the implementation doesn't use any 
//native Win32 API calls (no windows.h, for portability reasons)
//Future, near-complete version will have to include windows.h and re-define the constants
//eg. #define MZ_WAIT_FAILED WAIT_FAILED.., etc

//Generally, these shouldn't be available in the major library code (everything should be done using the corresponding objects)


#define MZ_WAIT_FAILED  	(dword)0xFFFFFFFF
#define MZ_WAIT_OBJECT_0	(dword)0x00000000L
#define MZ_WAIT_TIMEOUT 	(dword)0x00000102L

#define mzmemcpy	CopyMemory
#define mzstrcpy	lstrcpy
#define mzstrncpy	lstrcpyn
#define mzstrcat	lstrcat
#define mzstrlen	lstrlen
#define mzstrcmp	lstrcmp
#define mzstricmp	lstrcmpi
#define mztolower(x) CharLower((char*)(x))
#define mztoupper(x) CharUpper((char*)(x))
#define mzisalpha	IsCharAlpha
#define mzisalnum	IsCharAlphaNumeric
#define mzsprintf	wsprintf
#define mzvsprintf	wvsprintf

#if defined(IGNORE_STDC)
#define time(x)		timeGetTime()/1000
#define mzstdout
#define	mzisdigit(x) ((x)>=48 && (x)<57)
#else
#include <time.h>
#define	mzstdout stdout
#define mzisdigit isdigit
#endif


#endif //__MZ_INC_H__
