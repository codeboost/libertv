#pragma once
class FWaitDlg : public CDialogImpl<FWaitDlg>
{                 
public:           
    enum {IDD=IDD_WAIT};
    BEGIN_MSG_MAP(FWaitDlg)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
    END_MSG_MAP();

    LRESULT OnInitDialog(UINT , WPARAM , LPARAM , BOOL& )
    {
        CenterWindow(GetParent());
        return 0; 
    }

    BOOL Open(HWND hWndParent, const tchar* pszText)
    {
        if (m_hWnd != NULL)
            DestroyWindow();
        m_hWnd = NULL; 

        Create(hWndParent); 
        ShowWindow(SW_SHOW); 
        SetDlgItemText(IDC_STATIC, pszText); 
        UpdateWindow();
        return (m_hWnd != NULL);
    }

};