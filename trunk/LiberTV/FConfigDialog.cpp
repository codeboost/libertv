#include "stdafx.h"
#include "resource.h"
#include "FConfigDialog.h"
#include "windowsx.h"

#define SECTION_GENERAL 0
#define SECTION_NETWORK 1
#define SECTION_PLAYER	2

LRESULT FConfigDialog::OnInitDialog(UINT , WPARAM ,LPARAM , BOOL& )
{
	m_AppSettings = g_AppSettings;
	m_ActiveDlg = NULL; 

	m_TreeView.Attach(GetDlgItem(IDC_CATEGORY));

	m_TreeView.InsertItem("General", 0, 0, 0, 0); 
	m_TreeView.InsertItem("Network", 0, 0, 0, 0); 
	m_TreeView.InsertItem("Player", 0, 0, 0, 0); 

	WTL::CStatic aStatic; 
	aStatic.Attach(GetDlgItem(IDC_DETAIL));
	aStatic.GetClientRect(&m_rDetail); 
	aStatic.MapWindowPoints(m_hWnd, &m_rDetail);
	aStatic.ShowWindow(SW_HIDE); 

	SetActiveSection(SECTION_GENERAL); 
	CenterWindow();
	return TRUE; 
}

void FConfigDialog::SetActiveSection(int nSection)
{

	HWND hWndNow = m_ActiveDlg; 
	switch(nSection)
	{
	case SECTION_GENERAL:
		{
			m_ActiveDlg = m_DlgGeneral.Create(m_hWnd); 
		}
		break; 
	}
	if (m_ActiveDlg)
	{
		::ShowWindow(hWndNow, SW_HIDE); 
		::MoveWindow(m_ActiveDlg, m_rDetail.left, m_rDetail.top, m_rDetail.Width(), m_rDetail.Height(), FALSE); 
		::ShowWindow(m_ActiveDlg, SW_SHOW);  
	}

}

LRESULT FConfigDialog::OnCancel(WORD, WORD, HWND, BOOL&)
{
	EndDialog(0);	  
	return TRUE; 
}

LRESULT FConfigDialog::OnOK(WORD, WORD, HWND, BOOL&)
{
	return TRUE; 
}