#pragma once

#include "resource.h"

class FDlgAddFeed : public CSimpleDialog<IDD_ADD_FEED>
{                 
public:           
	enum {IDD=IDD_ADD_FEED};
	BEGIN_MSG_MAP(FDlgAddFeed)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_GET_FROM_CHANNEL, OnCheckBox)
		COMMAND_ID_HANDLER(IDC_FEED_ADDRESS, OnFeedAddress)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled); 
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled); 
	LRESULT OnCheckBox(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled); 
	LRESULT OnFeedAddress(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled); 
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 

	int Open(HWND hWndParent, const tchar* pszUrl = "", const tchar* pszFeedName = "");

public:
	FString m_StrName; 
	FString m_StrUrl; 

protected:
	BOOL	m_bIgnoreEdit; 

};
