#include "stdafx.h"
#include "resource.h"
#include "FPluginDownload.h"
#include "GlobalObjects.h"

LRESULT FPluginDownload::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0; 
}

LRESULT FPluginDownload::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (!EnsureDirExists(g_AppSettings.AppDir("vlc")))
	{
		FString Message; 
		if (GetLastError() == ERROR_ACCESS_DENIED)
			Message = "Cannot create plugin directory. Acess Denied.";
	}
	FString FileName = g_AppSettings.AppDir("vlc"); 
	return 0; 
}