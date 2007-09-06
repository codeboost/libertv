#pragma once
#include "resource.h"
class FDlgNewVersion : public CSimpleDialog<IDD_NEW_VERSION>
{                 
public:           
	enum {IDD=IDD_NEW_VERSION};
	BEGIN_MSG_MAP(FDlgNewVersion)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP();

	LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled); 
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled); 
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
};



