#ifndef __FWINDOWBASE_H__
#define __FWINDOWBASE_H__

#define WINVER 0x0501

#include <atlbase.h>
#include <atlapp.h>
extern CMyServerAppModule _Module;
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atldlgs.h>
#include "mz_Inc.h"
#include "Utils.h"
#include "FControls.h"
#include "CaptionPainter.h"
#include "resource.h"

template <class _T>
class FWindowBase : public CFrameWindowImpl<_T>, 
					public CFlatCaptionPainter<_T>
{
public:
	DECLARE_WND_CLASS_EX("FWindowBase",  0 , COLOR_WINDOW);
};

#endif //__FWINDOWBASE_H__