#ifndef __FCONFIGDIALOG2_H__
#define __FCONFIGDIALOG2_H__

#include "FIEDialog.h"
#include "AppSettings.h"


class FConfigDialog2 : public FIEDialog
{
public:

	BEGIN_MSG_MAP(FConfigDialog2)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy); 
		CHAIN_MSG_MAP(FIEDialog); 
    END_MSG_MAP();


	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&); 

	void	OnCreated(); 
	void	OnLoadComplete()
	{

	}
	void	OnLoad(long lSection); 
	void	OnUnload(long lSection); 
	void	OnCancel(); 
	void	OnSave(); 

	AppSettings	m_AppSettings; 
	long		m_CurrentSection; 
	int	BrowseForFolder(const tchar* pStrCurrent, FString& StrOut);

    FConfigDialog2()
    {
        m_CurrentSection = 0; 
    }
};




#endif //__FCONFIGDIALOG2_H__