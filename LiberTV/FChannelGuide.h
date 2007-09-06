#ifndef __FCHANNELGUIDE_H__
#define __FCHANNELGUIDE_H__

#include "FWebWindow.h"
#include "Utils.h"
#include "AppSettings.h"



#define TOOLBAR_IDC_BACK 1
#define TOOLBAR_IDC_FORWARD 2
#define TOOLBAR_IDC_HOME 3
#define TOOLBAR_IDC_REFRESH 4

#define NOTFOUND_HTML "data/notfound.html"

class FChannelGuide : public FIEWindow
{
public:
	DECLARE_WND_CLASS_EX("FWindowBase",  0, COLOR_WINDOW);

protected:
	BEGIN_MSG_MAP(FChannelGuide);
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus); 
		CHAIN_MSG_MAP(FIEWindow)	
	END_MSG_MAP();
	
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 1; 
	}

protected:
	void OnDocumentComplete(DWORD dwID ,BOOL bMainFrame);
	BOOL OnNavigateError(DWORD dwID, BOOL bMainFrame, const tchar* pszUrl);
	BOOL OnBeforeNavigate(DWORD dwID, const tchar* pszURL);
	void OnNavigateComplete(DWORD dwID, const tchar* pszURL);
	void OnDownloadBegin(DWORD dwID);
	void OnProgressChange(DWORD dwID, long lCurrent, long lMax);
	void OnActivated(BOOL bActivated);

public:
	void	SetToolbarAddr(const tchar* pToolbar); 
	void	OnToolbarCommand(UINT nCmd); 
	void	SetStation(const tchar* pszStation); 
	void    Refresh(DWORD dwFlag = 0); 
	void	ShowEpisodeDetails(ULONG ulEpisodeID); 
	void	ShowLoading(BOOL bShow); 
	void	ShowConnectibleWarning(BOOL bShow); 
	HRESULT	Navigate(const tchar* pszURL, DWORD dwNavFlags = 0); 
	void	SetWBFocus(); 
	int		ShowContextMenu();
	
	FString	m_StationURL; 
	bool	m_bNotFound;
	BOOL	m_bNavigating; 
};



class FToolbar : public FIEWindow
{
	BEGIN_MSG_MAP(FChannelGuide);
		MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus); 
		CHAIN_MSG_MAP(FIEWindow)	
	END_MSG_MAP()


protected:
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0; 
	}
	
};
#endif //__FCHANNELGUIDE_H__