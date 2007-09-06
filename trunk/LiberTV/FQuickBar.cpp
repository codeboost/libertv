#include "stdafx.h"
#include "FQuickBar.h"
#include "AppSettings.h"

void FQuickBar::OnCreated()
{
	m_dwFlags = 0; 
	FString Str;
	GetWindowText(Str);
	if (FAILED(m_pW.Navigate(g_AppSettings.AppDir(Str))))
	{
		MessageBox("Cannot load html. Please reinstall.", "Error", MB_OK | MB_ICONERROR);
		DestroyWindow();
	}

	SetMinSize(200, 200);
	//These are detected by calling getCloseBtnWidth() and getCaptionHeight()
	m_CaptionHeight = 23; 
	m_CloseBtnWidth = 14; 
	SetWindowText("LiberTV Quickbar");
}

void FQuickBar::OnLoadComplete()
{
	FString StrWidth = m_pW.m_pBrowser.CallScriptStr("getCloseBtnWidth", ""); 
	FString StrHeight = m_pW.m_pBrowser.CallScriptStr("getCaptionHeight", ""); 

	int height = strtol(StrWidth, NULL, 10); 
	if (height > 0)
		m_CaptionHeight = height; 
	int width = strtol(StrHeight, NULL, 10); 
	if (width > 0)
		m_CloseBtnWidth = width; 
	SetFocus();
}

LRESULT FQuickBar::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return FIEDialog::OnDestroy(uMsg, wParam, lParam, bHandled); 
}

static int offsetx = 0; 
static int offsety = 0; 



BOOL FQuickBar::PreTranslateMessage(MSG* pMsg)
{
	if((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
		(pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
		return FALSE;

	if (pMsg->message == WM_MOUSEMOVE)
	{
		POINT ptMouse = {GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam)};
		if (m_dwFlags & 1)
		{
			POINT ptMouse = {GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam)};
			FRect rc;
			GetClientRect(&rc); 
			ClientToScreen(&ptMouse); 

			//MoveWindow(ptMouse.x - offsetx, ptMouse.y - offsety, rc.Width(), rc.Height(), FALSE);
			SetWindowPos(GetParent(), ptMouse.x - offsetx, ptMouse.y - offsety, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			return TRUE; 
		}
	}
	else
	if (pMsg->message == WM_LBUTTONDOWN)
	{
		POINT ptMouse = {GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam)};
		FRect rc; 
		GetClientRect(&rc);
		rc.bottom = rc.top + m_CaptionHeight;
		if (PtInRect(&rc, ptMouse))
		{

			rc.left = rc.right - m_CloseBtnWidth; 
			if (PtInRect(&rc, ptMouse))
			{
				DestroyWindow();
				return FALSE; 
			}

			m_dwFlags |= 1;
			offsetx = ptMouse.x; 
			offsety = ptMouse.y; 
			SetCapture(); 
			return TRUE; 
		}
	}
	else
	if (pMsg->message == WM_LBUTTONUP)
	{
		if (m_dwFlags & 1)
		{
			ReleaseCapture();
			m_dwFlags&=~1;
			return TRUE; 
		}
	}

	return FIEDialog::PreTranslateMessage(pMsg); 
}