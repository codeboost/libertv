#ifndef FTrayWnd_Dot_h
#define FTrayWnd_Dot_h
#pragma once
#include <atlbase.h>
#include <atlapp.h>
extern CMyServerAppModule _Module;
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atldlgs.h>
#include "mz_Inc.h"
#include "trayiconimpl.h"
#include "resource.h"
#include "FDownloadStatus.h"
#include "FAutoWindow.h"
#include "FMainFrame.h"
#include "TestWindow.h"
#include "FUpdater.h"
#include "FConfigDialog2.h"
#include "FAboutDlg.h"
#include "mzTrayIcon.h"
#include "FSelectServer.h"
#include "IAppManager.h"

#define VERSION_CHECK_TIMER		1
#define SPACE_LEFT_WARNING		2
#define FEED_UPDATE_TIMER		3
#define TRAY_TOOLTIP_TIMER		4
#define UPDATE_INSTALL_TIMER	5

#define UPDATE_CHECK_TIMEOUT 30 * 1000 * 60		//30 minutes

#define LTV_CLASSNAME LTV_APP_NAME"_Class"



enum TrayIcons{
	IconNone, 
	IconNormal, 
	IconNotConnectable, 
	IconShuttingDown
};

enum BallonOptions{
	boShowAlways,			//Always show balloon
	boShowIfClosed,			//Only show if main frame is closed
	boShowIfClosedOrMsgBox	//Show balloon if main frame is closed, or show message box
};

class FTrayWindow : public CWindowImpl<FTrayWindow, CWindow, CWinTraits < 0, 0> >, 
					public CTrayIconImpl<FTrayWindow>,
					public IAppManager
{
public:

	DECLARE_WND_CLASS_EX(LTV_CLASSNAME, 0, COLOR_WINDOW); 
	UINT						 WM_TASKBARCREATED; //handle explorer crash

	BEGIN_MSG_MAP(FTrayWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_TRAYICON, OnMyTrayIcon); 
		MESSAGE_HANDLER(WM_CLOSE, OnClose); 
		MESSAGE_HANDLER(WM_TIMER, OnTimer); 
		MESSAGE_HANDLER(WM_LTV_VERSION_UPDATE, OnVersion); 
		MESSAGE_HANDLER(WM_LTV_CONNECTABLE, OnConnectable);
		MESSAGE_HANDLER(WM_COPYDATA, OnCopyData); 
		MESSAGE_HANDLER(WM_QUERYENDSESSION, OnEndSession);
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy);
		MESSAGE_HANDLER(WM_TASKBARCREATED, OnTaskbarCreated);		//Handles explorer crash
		MESSAGE_HANDLER(WM_MAINFRAME_CLOSED, OnParentNotify)
		COMMAND_ID_HANDLER(IDC_EXIT, OnExit);
		COMMAND_ID_HANDLER(IDC_TEST, OnTestCommand); 
		COMMAND_ID_HANDLER(IDC_SETTINGS, OnSettings); 
		COMMAND_ID_HANDLER(ID_TRAY_OPENPLAYER, OnOpenPlayer);
		COMMAND_ID_HANDLER(ID_TRAY_LOGWINDOW, OnLogWindow); 
		COMMAND_ID_HANDLER(IDC_OPEN_METATORRENT, OnOpenMetatorrent);
		COMMAND_ID_HANDLER(ID_TRAY_PAUSEDOWNLOADS, OnPauseResume); 
		COMMAND_ID_HANDLER(ID_TRAY_FEEDBACK, OnTrayFeedback); 
        COMMAND_ID_HANDLER(ID_TRAY_TEST, OnTrayTest); 
		CHAIN_MSG_MAP(CTrayIconImpl<FTrayWindow>)
	END_MSG_MAP()

public:
	FAutoWindow<FConfigDialog2>	 m_pConfDialog; 
	FAutoWindow<FAboutDialog>	 m_pAboutDlg;
	FAutoWindow<FLogFDialog>	 m_pLogWnd; 
	FAutoWindow<FSelectServer>	 m_pSelectServer; 
	FUpdater					 m_Updater; 
	mzTrayIcon					 *m_TrayIcon; 
	BOOL						m_bTrayIconInstalled; 

	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnMyTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/);
	LRESULT OnTimer(UINT, WPARAM, LPARAM, BOOL&); 
	LRESULT OnVersion(UINT, WPARAM, LPARAM, BOOL&); 
	LRESULT OnCopyData(UINT, WPARAM, LPARAM, BOOL&); 
	LRESULT OnEndSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnConnectable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnParentNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 


	//Cmds
	LRESULT OnExit(WORD,WORD,HWND,BOOL&);
	LRESULT OnAnnounce(WORD, WORD, HWND, BOOL&); 
	LRESULT OnLTAnnounce(WORD, WORD, HWND, BOOL&); 
	LRESULT OnLogWindow(WORD, WORD, HWND, BOOL&); 
	LRESULT OnOpenPlayer(WORD,WORD,HWND,BOOL&){OpenMainFrame();return 0;}
	LRESULT OnOpenMetatorrent(WORD, WORD, HWND, BOOL&){OpenSomeMetatorrent();return 0;}
	LRESULT OnPauseResume(WORD, WORD, HWND, BOOL&); 
    LRESULT OnTrayTest(WORD, WORD, HWND, BOOL&); 

	LRESULT OnTrayFeedback(WORD, WORD, HWND, BOOL&){
        ShellExecute(NULL, "open", "http://forum.libertv.ro/", "", "", SW_SHOW); 
        return 0; 
    }

public:
	FTrayWindow(){}
	~FTrayWindow(){}
	bool		OpenMainFrame();
	BOOL		OpenSettings(HWND hWndParent = NULL); 
	BOOL		OpenAbout(HWND hWndParent = NULL); 
	bool		PlayMediaFile(vidtype videoID); 
	vidtype		LoadMetatorrent(const tchar* FileName); 
	vidtype		OpenSomeMetatorrent(); 
	
	//IAppManager
	void		RestartApp(BOOL bSilent); 
	BOOL		IsConnectable();
	void		ProcessCmdLine(FCmdLine& pCmdLine); 

	LRESULT OnTestCommand(WORD, WORD, HWND, BOOL&);
	LRESULT	OnSettings(WORD, WORD, HWND, BOOL&); 

	HRESULT	PlayVideo(vidtype videoID); 
	BOOL	OpenMediaFromURL(const tchar* szMtUrl);
	int		OnSettingsChanged(IAppSettings* pNewSettings); 
	void	ShowSpaceWarning(fsize_type aSpaceReq, fsize_type aSpaceAvail, BOOL Ballon); 
	void	AddEpisodeComment(dword dwVideoID, dword dwEpisodeID); 

	void	SetTrayIcon(TrayIcons aIcon, const tchar* pszTooltip); 

	void	OutputDebugString(const char* pStrOutStr)
	{
		if (m_pLogWnd.IsObjectAndWindow())
		{
			m_pLogWnd->DebugPrintMessage(pStrOutStr); 
		}
	}

	void	OnPortsMapped(BOOL bMapped); 
	void	OnSetServer(); 
	void	SelectServer(); 

	void	PrepareMenu(HMENU hMenu); 

	FString		m_StrMsgWarning; 
	BOOL		m_bSpaceBalloon; 
	mz_Sync		m_MsgSync; 
	BOOL		m_Connectable; 
	TrayIcons	m_CurrentIcon; 
	FString		m_CurrentTooltip; 
	BOOL		m_HasNewVersion; 
	BOOL		m_VersionNotified; 
	BOOL		m_bShuttingDown; 
	TrayIcons	m_LastIcon; 

	void Conf_OnLoad(LONG lPage);
	void Conf_OnUnload(long lPageId);
	void Conf_OnCancel();
	void Conf_OnSave();
	int  Conf_BrowseForFolder(const char* pszCurrent, FString& StrOut);
	BOOL ShowBalloon(const char* title, const char * szMsg, const UINT timeout,  const DWORD flags);

	LRESULT PostMsg(UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif