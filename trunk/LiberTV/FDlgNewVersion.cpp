#include "stdafx.h"
#include "FDlgNewVersion.h"
#include "AppSettings.h"


LRESULT FDlgNewVersion::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent()); 
	return 0; 
}

LRESULT FDlgNewVersion::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (IsDlgButtonChecked(IDC_AUTO_UPDATE))
	{
		g_AppSettings.m_Flags |= PFLAG_AUTO_UPDATE;
		g_AppSettings.SaveSettings();
	}
	OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
	return 0; 
}

LRESULT FDlgNewVersion::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (IsDlgButtonChecked(IDC_AUTO_UPDATE))
	{
		g_AppSettings.m_Flags |= PFLAG_AUTO_UPDATE;
		g_AppSettings.SaveSettings();
	}
	OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
	return 0; 
}
