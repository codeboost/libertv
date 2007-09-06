// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif



//#define USE_XML 

#pragma warning (disable: 4311) //pointer truncation from 'type' to 'type'
#pragma warning (disable: 4312) //convertion from 'type' to 'type' of greater size  
#pragma warning (disable: 4099)	//type name first seen using 'struct' now seen using 'class'
#pragma warning (disable: 4995) //name was marked as #pragma deprecated
#pragma warning (disable: 4996) //'X' was declared deprecated
#pragma warning (disable: 4251)	//Needs to have DLL interface
#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE



//#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit

// turns off ATL's hiding of some common and often safely ignored warning messages
//#define _ATL_ALL_WARNINGS

#define _WINVER 0x0501
#define _WIN32_WINDOWS 0x0501
#define _WIN32_WINNT 0x0501
#define WIN32_LEAN_AND_MEAN


//_INC_WINDOWSX is defined here so that windowsx.h is not included, because
//windowsx's macros confuse the compiler and break WTL's GDI code (eg. SelectBitmap)
#define _INC_WINDOWSX	   

//Libtorrent version used
#define LT_VERSION 0x11



#include <asio/detail/socket_types.hpp>
//Std includes
#include "Ws2tcpip.h"
#include "Wspiapi.h"
#include <atlbase.h>
#include <atlstr.h>
#include <atlapp.h>
#include <atlgdi.h>
#include <atlmisc.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atldlgs.h>
#include <windows.h>
#include "shellapi.h"
#include <atlcoll.h>
#include <sstream>
#include <iostream>
#include <vector>
#include <deque>
#include <comdef.h>
#include <strsafe.h>

//LTV Includes
#include "ServerModule.h"
#include "LTVUtil.dll.h"
#include "Utils.h"
#include "winutils.h"
#include "FGlobals.h"
#include "resource.h"
#include <wininet.h>


extern HINSTANCE g_Instance;

#define __AFX_H__	 //prevent redifinition of the POSITION structure in wxlist.h (DirectShow)




