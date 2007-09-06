// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#pragma warning (disable: 4995) //name was marked as #pragma deprecated
#pragma warning (disable: 4996) //'X' was declared deprecated
#pragma warning (disable: 4251) //class x must have DLL-interface to be used by clients...

// Modify the following defines if you have to target a platform prior to the ones specified below.
// Refer to MSDN for the latest info on corresponding values for different platforms.
#ifndef WINVER				// Allow use of features specific to Windows XP or later.
#define WINVER 0x0501		// Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT		// Allow use of features specific to Windows XP or later.                   
#define _WIN32_WINNT 0x0501	// Change this to the appropriate value to target other versions of Windows.
#endif						

#ifndef _WIN32_WINDOWS		// Allow use of features specific to Windows 98 or later.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE			// Allow use of features specific to IE 6.0 or later.
#define _WIN32_IE 0x0600	// Change this to the appropriate value to target other versions of IE.
#endif

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// some CString constructors will be explicit
#define MZ_DLL_BUILD

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
#include <wininet.h>
#include "LTVUtil.dll.h"
#include "dutil.h"

// TODO: reference additional headers your program requires here
