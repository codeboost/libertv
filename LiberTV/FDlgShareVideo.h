#pragma once

#include "resource.h"
class FDlgShareVideo: public CSimpleDialog<IDD_SHARE_MOVIE>
{                 
public:           
	enum {IDD=IDD_SHARE_MOVIE};
	BEGIN_MSG_MAP(FDlgShareVideo)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_ADD, OnAdd)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP();

	WTL::CListViewCtrl m_ListView; 
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled)
	{
		OnCloseCmd(wNotifyCode, wID, hWndCtrl, bHandled);
		return 0;
	}
	LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled);

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);



	LRESULT OnAdd(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};