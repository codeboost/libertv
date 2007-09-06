#ifndef __FCONFIGDIALOG_H__
#define __FCONFIGDIALOG_H__

#include "mz_Inc.h"
#include "FFunctions.h"
#include "AppSettings.h"



class FDlgGeneral : public CDialogImpl<FDlgGeneral>
{
public:
	enum { IDD = IDD_CONF_GENERAL };
	BEGIN_MSG_MAP(FConfigDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
	END_MSG_MAP()
	LRESULT OnInitDialog(UINT, WPARAM, LPARAM, BOOL&);

};

class FConfigDialog : public CDialogImpl<FConfigDialog>
{
public:
	enum { IDD = IDD_CONFIG };
	BEGIN_MSG_MAP(FConfigDialog)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

protected:
	WTL::CTreeViewCtrlEx m_TreeView; 
	AppSettings			 m_AppSettings; 
	HWND				 m_ActiveDlg; 

	FDlgGeneral			 m_DlgGeneral;
	FRect				 m_rDetail; 
	

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/,LPARAM /*lParam*/, BOOL& /*bHandled*/);
	LRESULT OnOK(WORD, WORD, HWND, BOOL&);
	LRESULT OnCancel(WORD, WORD, HWND, BOOL&);
	
	void	SetActiveSection(int nSection); 

};


#endif //__FCONFIGDIALOG_H__