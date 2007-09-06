#pragma once
#include "Resource.h"

#define DLG_RESULT_LEAVE 1
#define DLG_RESULT_DELETE 2


class FStorageConfirmDlg : public CSimpleDialog<IDD_STORAGE_CHANGE>{
public:
    BEGIN_MSG_MAP(FStorageConfirmDlg)
        COMMAND_ID_HANDLER(IDLEAVE, OnLeave)
        COMMAND_ID_HANDLER(IDDELETE, OnDelete)
        COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    END_MSG_MAP()
    int  m_DlgResut; 

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        m_DlgResut = 0; 
        CenterWindow(GetParent()); 
        return 0;
    }

    LRESULT OnLeave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        m_DlgResut = DLG_RESULT_LEAVE;             
        OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
        return 0;
    }

    LRESULT OnDelete(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
    {
        m_DlgResut = DLG_RESULT_DELETE;             
        OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
        return 0;
    }

    

};