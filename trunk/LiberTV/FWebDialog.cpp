#include "stdafx.h"
#include "FWebDialog.h"


BOOL FWebDialog::OpenDialog(HWND hWndParent, const tchar* pStrUrl, const tchar* pStrCaption)
{
	Create(hWndParent, rcDefault, pStrCaption, WS_POPUP | WS_THICKFRAME); 
	return (m_hWnd != NULL);
}

LRESULT FWebDialog::OnDocumentComplete(UINT, WPARAM, LPARAM, BOOL&)
{
	ShowWindow(SW_SHOW); 
	SET_HTML_MESSAGE_MAP();
	return 0; 
}

LRESULT FWebDialog::OnSubmit(BOOL& bHandled)
{


	return 0; 
}