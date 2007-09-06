#pragma once
#include "FIEDialog.h"

class FSelectServer :  public FIEDialog
{
	BEGIN_MSG_MAP(FSelectServer)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy);
		CHAIN_MSG_MAP(FIEDialog);
	END_MSG_MAP()
public:
	void OnCreated()
	{
		SetIcon(LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_MAIN)));

		FString Str;
		GetWindowText(Str);

		SetWindowText("Select server"); 

		if (FAILED(m_pW.Navigate(g_AppSettings.AppDir(Str))))
		{
			MessageBox("Load error. Please re-install.", "Error loading dialog", MB_OK | MB_ICONERROR); 
			DestroyWindow();
			return ; 
		}
	}
	void	OnLoadComplete()
	{
		m_pW.m_pBrowser.CallJScript("setCurrentServer", _RegStr("ChannelGuideURL")); 
		SetFocus(); 
		SetActiveWindow(); 
	}
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		FIEDialog::OnDestroy(uMsg, wParam, lParam, bHandled); 
		return 0; 
	}
};