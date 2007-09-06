#pragma once


#include "FIEDialog.h"
#include "AppSettings.h"


class FCommentDialog : public FIEDialog
{
public:

	BEGIN_MSG_MAP(FCommentDialog)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy); 
		MESSAGE_HANDLER(WM_DOCUMENT_COMPLETE, OnDocumentComplete)
		MESSAGE_HANDLER(WM_FRAME_LOAD_ERROR, OnLoadError)
		CHAIN_MSG_MAP(FIEDialog); 
	END_MSG_MAP();

	FString m_URL; 

	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnDocumentComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnLoadError(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 

	void	OnCreated(); 
	void	OnLoadComplete(){}

	int Open(HWND hWndParent, const tchar* pszURL); 
};