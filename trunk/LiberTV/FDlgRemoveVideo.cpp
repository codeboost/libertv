#include "stdafx.h"
#include "resource.h"
#include "FDlgRemoveVideo.h"
#include "AppSettings.h"

int FDlgRemoveVideo::Open( HWND hWndParent, const char* pszVideoName )
{
	m_StrVideoName = pszVideoName; 

	return DoModal(hWndParent);
}

LRESULT FDlgRemoveVideo::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetDlgItemText(IDC_VIDEO_NAME, m_StrVideoName); 
	CenterWindow(GetParent()); 
	return 0; 
}

LRESULT FDlgRemoveVideo::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (IsDlgButtonChecked(IDC_DONT_SHOW))
	{
		g_AppSettings.m_Flags &=~PFLAG_CONFIRM_REMOVE;
		g_AppSettings.SaveSettings(); 
	}
	OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
	return 0; 
}

LRESULT FDlgRemoveVideo::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled)
{
	OnCloseCmd(wNotifyCode, wID, hWndCtrl, bHandled);
	return 0; 
}