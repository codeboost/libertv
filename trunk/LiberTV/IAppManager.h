#pragma once


struct IAppSettings
{
	//Nothing here
};

class IAppManager
{
public:
	virtual ~IAppManager() {}
	virtual void RestartApp(BOOL bSilent = FALSE) = 0; 
	virtual BOOL IsConnectable() = 0; 
	virtual int	 OnSettingsChanged(IAppSettings* pSettings) = 0; 
	virtual BOOL ShowBalloon(const char* title, const char * szMsg, const UINT timeout,  const DWORD flags) = 0; 
	virtual void ShowSpaceWarning(fsize_type aSpaceReq, fsize_type aSpaceAvail, BOOL Ballon) = 0; 

	virtual void Conf_OnLoad(long lPageId) = 0;
	virtual void Conf_OnUnload(long lPageId) = 0; 
	virtual void Conf_OnCancel() = 0; 
	virtual void Conf_OnSave() = 0; 
	virtual void OutputDebugString(const char* pStr) = 0;
	virtual int	 Conf_BrowseForFolder(const char* pszCurrent, FString& StrOut) = 0; 

	virtual BOOL OpenSettings(HWND hWndParent) = 0; 
	virtual void OnSetServer() = 0; 
	virtual void SelectServer() = 0; 

	virtual BOOL OpenMediaFromURL(const char* pszPathOrUrl) = 0; 
	virtual BOOL OpenAbout(HWND hWndParent) = 0; 
	virtual LRESULT PostMsg(UINT uMsg, WPARAM wParam, LPARAM lParam) = 0; 
	virtual void OnPortsMapped(BOOL bSuccess) = 0; 
};

extern IAppManager* g_pAppManager;