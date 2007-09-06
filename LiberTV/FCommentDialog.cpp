#include "stdafx.h"
#include "FCommendDialog.h"


void FCommentDialog::OnCreated()
{
	SetMinSize(600, 480); 
	SetIcon(LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_MAIN)));

	if (FAILED(m_pW.Navigate(m_URL)))
	{
		MessageBox("Error loading comment window. Server might be down. Please retry later", "Error", MB_OK | MB_ICONERROR);
		DestroyWindow();
	}
}

int FCommentDialog::Open(HWND hWndParent, const char* pszURL)
{
	if (!PathIsURL(pszURL))
	{
		::MessageBox(hWndParent, "This episode does not have comments", "No Comments", MB_OK | MB_ICONINFORMATION);
		return FALSE; 
	}

	m_URL = pszURL;
	if (IsWindow())
	{
		OnCreated(); 
	}
	else
	{
		FRect rc = ScreenCenteredRect(640, 480); 
		Create(hWndParent, rc, "LiberTV: Comments", 
			WS_OVERLAPPED | WS_SYSMENU  | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SIZEBOX);
	}
	return TRUE; 
}

LRESULT FCommentDialog::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return FIEDialog::OnDestroy(uMsg, wParam, lParam, bHandled); 
}

LRESULT FCommentDialog::OnDocumentComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	ShowWindow(SW_SHOW);
	SetFocus();
	return 0; 
}

LRESULT FCommentDialog::OnLoadError(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	MessageBox("This episode does not have comments", "No Comments", MB_OK | MB_ICONINFORMATION);
	DestroyWindow();
	return 0; 
}
