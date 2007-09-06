#pragma once

class FPluginDownload : public CSimpleDialog<IDD_PLUGIN_DOWNLOAD>
{                 
public:           
	enum {IDD=IDD_PLUGIN_DOWNLOAD};
	BEGIN_MSG_MAP(FDlgSaveVideo)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP();

	WTL::CProgressBarCtrl m_pProgress; 

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled); 
	LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};