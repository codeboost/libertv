#ifndef __FSESSIONSTATUS_H__
#define __FSESSIONSTATUS_H__

#include "Utils.h"
#include "resource.h"

//The Session Status Dialog
class FDlgSessionStatus : public CSimpleDialog<IDD_SESSION_STATUS>, public CDialogResize<FDlgSessionStatus>
{                 
public:           
	enum {IDD=IDD_SESSION_STATUS};
	BEGIN_MSG_MAP(FDlgInputBox)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
		MESSAGE_HANDLER(WM_TIMER, OnTimer); 
		//MESSAGE_HANDLER(WM_SIZE, OnSize); 
		CHAIN_MSG_MAP(CDialogResize<FDlgSessionStatus>)
	END_MSG_MAP();

	BEGIN_DLGRESIZE_MAP(FDlgSessionStatus)
		DLGRESIZE_CONTROL(IDC_SESSION, DLSZ_SIZE_X)
		DLGRESIZE_CONTROL(IDC_PEERS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
		DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
		DLGRESIZE_CONTROL(IDC_VIDEO_NAME, DLSZ_SIZE_X)
	END_DLGRESIZE_MAP()
	FString m_StrDlgResult;
	WTL::CListViewCtrl m_lvSessionStatus; 
	WTL::CListViewCtrl m_lvPeers; 

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 

	void UpdateSessionStatus();
	void UpdatePeers(); 

	LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		KillTimer(0); 
		OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_StrDlgResult = "";
		OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
		return 0;
	}

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
protected:
	DWORD m_LastVideo; 


};

#endif //__FSESSIONSTATUS_H__