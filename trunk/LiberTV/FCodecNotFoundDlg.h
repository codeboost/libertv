#pragma once

#include "FIEDialog.h"
#include "Utils.h"


class FCodecNotFoundDlg : public FIEDialog
{
public:
	BEGIN_MSG_MAP(FCodecNotFoundDlg)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy); 
		CHAIN_MSG_MAP(FIEDialog); 
	END_MSG_MAP(); 

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 

	FString m_NavURL; 
	void OnCreated();
	void OnLoadComplete();
};
