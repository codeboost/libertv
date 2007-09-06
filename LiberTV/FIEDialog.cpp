#include "stdafx.h"
#include "FIEDialog.h"


BOOL FIEDialog::Open(HWND hWndParent, FRect& rcCreate)
{
	return NULL != 
		Create(hWndParent, rcCreate, "IEDlg", WS_DLGFRAME | WS_VISIBLE | WS_CLIPCHILDREN, WS_EX_STATICEDGE); 
}

LRESULT FIEDialog::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
	CMessageLoop* pLoop = _Module.GetMessageLoop();
	if (NULL != pLoop)
	{
		//pLoop->AddMessageFilter(this);
	}
	m_pW.m_hWndNotify = m_hWnd; 
	m_pW.Create(m_hWnd, rcDefault, "about:blank", WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

	OnCreated(); 
	return 0; 
}

LRESULT FIEDialog::OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
{
	//CMessageLoop* pLoop = _Module.GetMessageLoop();
	//ATLASSERT(pLoop != NULL);
	//pLoop->RemoveMessageFilter(this);
    m_pW.DestroyWindow();
	return 0; 
}

LRESULT FIEDialog::OnSize(UINT, WPARAM, LPARAM, BOOL&)
{
	FRect r; 
	GetClientRect(&r);
	m_pW.MoveWindow(&r, TRUE); 
	return 0; 
}

LRESULT FIEDialog::OnLoadComplete(UINT, WPARAM, LPARAM, BOOL&)
{
	OnLoadComplete(); 
	return 0; 
}

void FIEDialog::SetMinSize(int cx, int cy)
{
	m_MinSize.cx = cx; 
	m_MinSize.cy = cy; 

	FRect r; 
	GetWindowRect(&r); 
	if (r.Width() < cx || r.Height() < cy)
	{
		r.right = r.left+cx; 
		r.bottom = r.top + cy; 
		MoveWindow(&r, TRUE); 
	}
}

void FIEDialog::SetMaxSize(int cx, int cy)
{
	m_MaxSize.cx = cx; 
	m_MaxSize.cy = cy; 
}

LRESULT FIEDialog::OnSizing(UINT uMsg, WPARAM wSide, LPARAM lParam, BOOL& bHandled)
{
	WTL::CSize &aSize = m_MinSize; 
	LimitMinWndSize(aSize.cx, aSize.cy, wSide, lParam); 
	bHandled = TRUE; 
	return 1; 
}