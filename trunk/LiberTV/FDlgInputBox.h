#ifndef __FDLGINPUTBOX_H__
#define __FDLGINPUTBOX_H__

#include "Utils.h"
#include "resource.h"

//The "Add Label" Dialog from My Collection
class FDlgInputBox : public CSimpleDialog<IDD_LABEL>
{                 
public:           
	enum {IDD=IDD_LABEL};
	BEGIN_MSG_MAP(FDlgInputBox)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog);
	END_MSG_MAP();

	FString	m_StrCaption; 
	FString	m_StrPrompt;
	FString m_StrDlgResult;

	LRESULT OnInitDialog(UINT , WPARAM , LPARAM , BOOL& )
	{
		if (m_StrDlgResult.GetLength() != 0)
			SetDlgItemText(IDC_LABEL, m_StrDlgResult);

		SetWindowText(m_StrCaption); 
		SetDlgItemText(IDC_STATIC, m_StrPrompt); 

		CenterWindow(GetParent());
		return 0; 
	}

	LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		tchar* pszBuf = m_StrDlgResult.GetBufferSetLength(256);
		GetDlgItemText(IDC_LABEL, pszBuf, 256);

		m_StrDlgResult.Trim();
		m_StrDlgResult.Replace(",","");
		m_StrDlgResult.Replace("\\", ""); 
		m_StrDlgResult.Replace("<", "");
		m_StrDlgResult.Replace(">", ""); 

		OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
		return 0;
	}

	LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
	{
		m_StrDlgResult = "";
		OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
		return 0;
	}
};
#endif //__FDLGINPUTBOX_H__