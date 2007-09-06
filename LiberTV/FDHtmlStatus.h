#ifndef __FDHTMLSTATUS_H__
#define __FDHTMLSTATUS_H__
#pragma once

#include "FWebWindow.h"

class FDHtmlStatus : public FWebWindow
{
	IMPLEMENT_HTML_MESSAGE_MAP();
	BEGIN_HTML_MSG_MAP(FDHtmlStatus,m_pBrowser)
		COMMAND_HTML_HANDLER(ID_HTML_CLICK, "logoButton", OnLogoClick); 
	END_HTML_MSG_MAP()

	BEGIN_MSG_MAP(FDHtmlStatus)
		MESSAGE_HANDLER(WM_SIZE, OnSize);
		MESSAGE_HANDLER(WM_CREATE, OnCreate); 
		MESSAGE_HANDLER(WM_DOCUMENT_COMPLETE, OnHTMLSetHandlers); 
	END_MSG_MAP()

	BOOL	m_Initialized; 


	FDHtmlStatus()
	{
		m_Initialized = FALSE; 
	}

	LRESULT OnHTMLSetHandlers(UINT, WPARAM, LPARAM, BOOL&)
	{
		_DefineHtmlMsgMap();	
		OnBrowserReady(); 
		return 0; 
	}
	bool BuildStatusHTML(FString& a); 
	bool InitStatus(); 
	LRESULT OnLogoClick(BOOL&)
	{
		return 0; 
	}

	void OnBrowserReady()
	{
		if (!m_Initialized)
			InitStatus();
		m_Initialized = TRUE; 
	}

	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
	{
		RECT rcWindow; GetClientRect( &rcWindow );
		if (CreateBrowser(rcWindow))
		{
			NavigateBrowser(g_AppSettings.AppDir("data/test.html")); 
		}
		return 1; 
	}

	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&)
	{
		RECT rcWindow; GetClientRect( &rcWindow );
		m_pBrowser.MoveWindow( &rcWindow, TRUE );
		return 1; 
	}

};

#endif //__FDHTMLSTATUS_H__