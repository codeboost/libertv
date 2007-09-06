#pragma once


#include "FIEDialog.h"

class FQuickBar :  public FIEDialog
{
	BEGIN_MSG_MAP(FQuickBar)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy);
		CHAIN_MSG_MAP(FIEDialog);
	END_MSG_MAP()

	DWORD m_dwFlags;
	int	  m_CaptionHeight; 
	int	  m_CloseBtnWidth; 
public:
	FQuickBar()
	{
		m_dwFlags = 0; 
	}

	void OnCreated(); 
	void	OnLoadComplete();
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	BOOL PreTranslateMessage(MSG* pMsg);
};