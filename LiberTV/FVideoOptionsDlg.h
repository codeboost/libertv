#ifndef __FVIDEOOPTIONSDLG_H__
#define __FVIDEOOPTIONSDLG_H__

#pragma once
#include "FIEDialog.h"
#include "FSubManager.h"
#include "Utils.h"


class FVideoOptionsDlg : public FIEDialog
{
public:
	BEGIN_MSG_MAP(FVideoOptionsDlg)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy); 
		CHAIN_MSG_MAP(FIEDialog); 
	END_MSG_MAP(); 

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 

    FSubManager *m_SubManager; 
	void OnCreated();

	void FindExistingSubtitles(); 
 	void OnLoadComplete();
};

#endif //__FVIDEOOPTIONSDLG_H__