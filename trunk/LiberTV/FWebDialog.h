#ifndef __FWEBDIALOG_H__
#define __FWEBDIALOG_H__

#include "mz_Inc.h"
#include "FWebWindow.h"

class FWebDialog : public FWebWindow
{


public:
	BEGIN_HTML_MSG_MAP(FWebDialog, m_pBrowser)
		// Set an event handler on buttonSumbmit.onclick 
		COMMAND_HTML_HANDLER(ID_HTML_CLICK, "buttonSubmit", OnSubmit )
	END_HTML_MSG_MAP()


	BOOL		OpenDialog(HWND hWndParent, const tchar* pStrFormURL, const char* pStrCaption); 
	LRESULT		OnDocumentComplete(UINT, WPARAM, LPARAM, BOOL&); 

	LRESULT		OnSubmit(BOOL& bHandled);
	
};

#endif //__FWEBDIALOG_H__