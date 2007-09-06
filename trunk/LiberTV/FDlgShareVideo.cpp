#include "stdafx.h"
#include "FDlgShareVideo.h"

LRESULT FDlgShareVideo::OnAdd( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	CFileDialog Dlg (TRUE); 
	int nRes = Dlg.DoModal();
	if (nRes == IDOK)
	{
		FString FileName = Dlg.m_szFileTitle;
		int nIns = m_ListView.InsertItem(0, FileName.GetBuffer()); 
		m_ListView.SetItem(nIns, 1, LVIF_TEXT, SizeString(GetFileSize(FileName)),0, 0, 0, 0); 
	}
	return 0; 
}

LRESULT FDlgShareVideo::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	m_ListView.Attach(GetDlgItem(IDC_FILES));
	m_ListView.AddColumn("Filename", 0); 
	m_ListView.AddColumn("Size", 1); 
	m_ListView.SetColumnWidth(0, 250); 
	m_ListView.SetColumnWidth(1, 100); 
	return 0;
}

LRESULT FDlgShareVideo::OnOk( WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled )
{
	OnCloseCmd(wNotifyCode, wID, hWndCtrl, bHandled);
	return 0;
}