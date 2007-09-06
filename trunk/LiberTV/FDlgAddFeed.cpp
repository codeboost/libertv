#include "stdafx.h"
#include "FDlgAddFeed.h"
#include "GlobalObjects.h"

int FDlgAddFeed::Open(HWND hWndParent, const tchar* pszUrl, const tchar* pszFeedName)
{
	m_StrUrl = pszUrl; 
	m_StrName = pszFeedName; 
	m_StrUrl.Trim(); 
	m_StrName.Trim();
	return (int)DoModal(hWndParent); 
}

LRESULT FDlgAddFeed::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_bIgnoreEdit = TRUE; 
	SetDlgItemText(IDC_FEED_NAME, m_StrName); 
	SetDlgItemText(IDC_FEED_ADDRESS, m_StrUrl); 
	m_bIgnoreEdit = FALSE; 

	if (m_StrName.GetLength() == 0)
		CheckDlgButton(IDC_GET_FROM_CHANNEL, BST_CHECKED);
	else
	{
		CheckDlgButton(IDC_GET_FROM_CHANNEL, BST_UNCHECKED);
		::EnableWindow(GetDlgItem(IDC_FEED_NAME), TRUE);
	}

	CenterWindow(GetParent()); 
	::SetFocus(GetDlgItem(IDC_FEED_ADDRESS));
	return 0; 
}

LRESULT FDlgAddFeed::OnFeedAddress(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled)
{
	if (wNotifyCode == EN_CHANGE && !m_bIgnoreEdit)
	{
		FString StrAddr; 
		GetDlgItemText(IDC_FEED_ADDRESS, StrAddr); 
		if (!UrlIs(StrAddr, URLIS_URL))
			::ShowWindow(GetDlgItem(IDC_INVALID_URL), SW_SHOW);
		else
			::ShowWindow(GetDlgItem(IDC_INVALID_URL), SW_HIDE);
	}
	return 0; 
}

LRESULT FDlgAddFeed::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled)
{
	FString StrAddr;
	GetDlgItemText(IDC_FEED_ADDRESS, StrAddr); 
	if (!PathIsURL(StrAddr))
	{
		MessageBox("Channel URL is not valid. Please check your input", "LiberTV: Invalid URL", MB_OK | MB_ICONWARNING); 
		bHandled = TRUE; 
		return 1; 
	}

	GetDlgItemText(IDC_FEED_NAME, m_StrName);
	m_StrUrl = StrAddr; 
	OnCloseCmd(wNotifyCode, wID, hWndCtrl, bHandled);
	return 0; 
}

LRESULT FDlgAddFeed::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled)
{
	m_StrName = ""; 
	m_StrUrl = ""; 
	OnCloseCmd(wNotifyCode, wID, hWndCtrl, bHandled);
	return 0; 
}

LRESULT FDlgAddFeed::OnCheckBox(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled)
{
	int nCheck = IsDlgButtonChecked(IDC_GET_FROM_CHANNEL);
	::EnableWindow(GetDlgItem(IDC_FEED_NAME), nCheck == BST_UNCHECKED); 
	if (nCheck == BST_UNCHECKED)
		::SetFocus(GetDlgItem(IDC_FEED_NAME));

	return 0; 
}


