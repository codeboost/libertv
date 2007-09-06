#pragma once

class FDlgRemoveVideo : public CSimpleDialog<IDD_REMOVE_VIDEO>
{                 
public:           
	enum {IDD=IDD_REMOVE_VIDEO};
	BEGIN_MSG_MAP(FDlgRemoveVideo)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()

	LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled); 
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	int		Open(HWND hWndParent, const char* pszVideoName);


	CFont m_Fnt; 
	FString m_StrVideoName; 
};