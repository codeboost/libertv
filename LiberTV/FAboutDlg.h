#ifndef __FABOUTDLG_H__
#define __FABOUTDLG_H__

#include "FIEDialog.h"

class FAboutDialog : public FIEDialog
{
public:
	BEGIN_MSG_MAP(FAboutDialog)
		CHAIN_MSG_MAP(FIEDialog); 
	END_MSG_MAP(); 
	void OnCreated()
	{
		SetMinSize(320, 320); 
		SetIcon(LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_MAIN)));
		if (FAILED(m_pW.Navigate(g_AppSettings.AppDir("data/about.html"))))
		{
			MessageBox("Sorry, no about dialog today. Cannot load about.html. Please reinstall.", "Error loading about page", MB_OK | MB_ICONSTOP); 
		}
	}

	void OnLoadComplete()
	{
		m_pW.m_pBrowser.CallJScript("setPlayerVersion", g_AppSettings.m_AppVersion);
	}
};

class FLogFDialog : public FIEDialog
{
public:
	BEGIN_MSG_MAP(FLogFDialog)
		CHAIN_MSG_MAP(FIEDialog); 
	END_MSG_MAP(); 
	void OnCreated()
	{
		//SetMinSize(500, 480); 
		SetMaxSize(500, 480); 
		SetIcon(LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_MAIN)));
		if (FAILED(m_pW.Navigate(g_AppSettings.AppDir("data/logwnd.html"))))
		{
			MessageBox("Sorry, no log window today. Cannot load logwnd.html. Please reinstall.", "Error loading about page", MB_OK | MB_ICONSTOP); 
		}
	}

	void DebugPrintMessage(const tchar* pStrMsg)
	{
		m_pW.m_pBrowser.CallJScript("DebugPrintMessage", pStrMsg); 
	}

	void OnLoadComplete()
	{
	}
};
#endif //__FABOUTDLG_H__