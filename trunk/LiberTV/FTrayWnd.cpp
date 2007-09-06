#include "stdafx.h"
#include "FTrayWnd.h"
#include "GlobalObjects.h"
#include "FMainFrame.h"
#include "strsafe.h"
#include "FVideoConv.h"
#include "FDlgNewVersion.h"
#include "AppCheck.h"
#include "CpuUsage.h"


extern FAutoWindow<FMainFrame> g_MainFrame; 

LRESULT FTrayWindow::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
	m_Connectable = TRUE; 
	m_HasNewVersion = FALSE; 
	m_VersionNotified = FALSE; 
	m_bShuttingDown = FALSE; 
	m_LastIcon = IconNone;
	m_bTrayIconInstalled = FALSE; 
	WM_TASKBARCREATED = ::RegisterWindowMessage(_T("TaskbarCreated"));

	SetTrayIcon(IconNormal, "LiberTV Player"); 

	BOOL bOpenMainFrame = TRUE; 

	if (bOpenMainFrame && !g_AppSettings.m_CmdLine.HasParam("/silent"))
	{
		OpenMainFrame();
	}
	else
	{
		g_Objects._DownloadManager.LoadStorageAsync();
	}

	if (g_AppSettings.m_CmdLine.HasParam("/devel"))
	{
		g_AppSettings.bDeveloperMode = true; 
	}


	_DBGAlert("FTrayWindow::OnCreate() Done\n"); 

	ProcessCmdLine(g_AppSettings.m_CmdLine);

	SetTimer(VERSION_CHECK_TIMER, 15000);
	SetTimer(FEED_UPDATE_TIMER, 5000); 
	SetTimer(TRAY_TOOLTIP_TIMER, 1000); 


	//SetTimer(TEST_TIMER, 5000); 

	return 0; 
}

LRESULT FTrayWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	
	SetTrayIcon(IconShuttingDown, "LiberTV is stopping..."); 
	m_bShuttingDown = TRUE; //prevents tray icon/tooltip change
	if (g_MainFrame.IsObjectAndWindow())
		g_MainFrame->DestroyWindow(); 

	KillTimer(FEED_UPDATE_TIMER);
	KillTimer(VERSION_CHECK_TIMER);
	KillTimer(SPACE_LEFT_WARNING);

	m_Updater.CheckNewVersionSync(UPDATE_EVENT_STOPPING);

	WSACleanup();

	
	m_Updater.Stop();
	g_Objects._DownloadManager.Stop();
	g_Objects.Stop(); 
	g_AppSettings.SaveSettings();

	
	PostQuitMessage(0); 
	return 0; 
}

void FTrayWindow::SetTrayIcon(TrayIcons aIcon, const tchar* pszTooltip)
{
	if (m_bShuttingDown)
		return; 

	m_CurrentTooltip = pszTooltip; 

	int TheIcon = IDI_MAIN; 
	switch(aIcon)
	{
	case IconNotConnectable:
	case IconShuttingDown:
		TheIcon = IDI_NOT_CONNECTABLE; 
		break; 
	}
	
	HICON hIconSmall = NULL; 
		hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(TheIcon), 
			IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	if (hIconSmall)
	{
		if (!m_bTrayIconInstalled)
			InstallIcon(pszTooltip, hIconSmall, IDR_TRAY); 
		else
			ModifyIcon(pszTooltip, hIconSmall, IDR_TRAY); 
		DestroyIcon(hIconSmall); 
	}
	m_bTrayIconInstalled = TRUE;
	m_CurrentIcon = aIcon; 
}

LRESULT FTrayWindow::OnTaskbarCreated(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RemoveIcon();
	m_bTrayIconInstalled = FALSE; 
	SetTrayIcon(m_CurrentIcon, m_CurrentTooltip); 
	return 0; 
}

void FTrayWindow::ProcessCmdLine(FCmdLine& pCmdLine)
{
	FString FullPath; 
	for (int k = 1; k < pCmdLine.GetArgc(); k++)
	{
		
		const char* pArg = pCmdLine.GetAt(k); 

		if (pArg && *pArg != '-' && *pArg != '/')
		{

			if (FullPath.GetLength() > 0)
				FullPath+=" ";

			FullPath+=pArg; 
			const char* pFileName = FullPath; 

			if (PathIsURL(pFileName))
			{
				OpenMediaFromURL(pFileName); 
			}
			else
			{
				FIniConfig aConf; 
				if (aConf.Load(pFileName))
				{
					if (aConf.SectionExists("Subscribe"))
					{
						FString FeedUrl = aConf.GetValue("Subscribe", "FeedURL");
						FString FeedName = aConf.GetValue("Subscribe", "FeedName"); 
						if (PathIsURL(FeedUrl))
						{
							OpenMainFrame();
							g_MainFrame->m_Container->SwitchActiveView(VIEW_FEEDS); 
							g_MainFrame->AddFeedDialog(FeedUrl, FeedName); 
							g_MainFrame->m_Container->SwitchActiveView(VIEW_FEEDS); 
							return; 
						}
						//Subscribe to FEED
					}
				
					vidtype videoID = aConf.GetValueDWORD("Video", "videoID", 10); 
					if (g_Objects._DownloadManager.IsDownload(videoID))
					{
						OpenMainFrame();
						g_MainFrame->SetActiveView(VIEW_STATUS); 
						return; 
					}
				}
				vidtype videoID = LoadMetatorrent(pFileName);
				if (videoID != 0)
				{
					if (g_MainFrame.IsObjectAndWindow())
					{
						BOOL bStartPlayback = g_Objects._DownloadManager.IsDownloadFinished(videoID); 

						if (bStartPlayback)
							g_MainFrame->PlayMediaFile(videoID);
						else
							g_MainFrame->SetActiveView(VIEW_STATUS);
					}
				}
			}
		}
		else
		{
			//handle the case when a multi-word argument is passed without quotes
			//
			if (*pArg != '-' && *pArg != '/')
			{
				if (FullPath.GetLength() > 0)
					FullPath+=" ";
				FullPath+=pArg;					
			}
		}
	}
}

LRESULT FTrayWindow::OnCopyData(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bProcessed)
{
	COPYDATASTRUCT* cds = (COPYDATASTRUCT*)lParam; 

	if (cds->cbData > 0)
	{
		char* pxdata = new char[cds->cbData + 1];
		memcpy(pxdata, cds->lpData, cds->cbData); 
		pxdata[cds->cbData] = 0; 
		FCmdLine pCmdLine;
		pCmdLine.Set(pxdata);

		ProcessCmdLine(pCmdLine); 
		delete[] pxdata; 
	}
	bProcessed = TRUE; 

	return 1; 
}

LRESULT FTrayWindow::OnSettings(WORD, WORD, HWND, BOOL&)
{
	OpenSettings(); 
	return 0; 
}

BOOL FTrayWindow::OpenSettings(HWND hWndParent)
{
	if (NULL == hWndParent)
    {
        if (g_MainFrame.IsObjectAndWindow())
        {
            //OpenMainFrame(); //Restore, BringToTop, etc
            hWndParent = g_MainFrame;
        }
    }

	FRect rc = ScreenCenteredRect(640, 400); 
	return NULL != m_pConfDialog.Create(hWndParent, rc, "LiberTV: Settings", 
		WS_DLGFRAME | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SYSMENU, 
		WS_EX_STATICEDGE);
}

void FTrayWindow::AddEpisodeComment(dword dwVideoID, dword dwEpisodeID)
{
	if (!g_MainFrame.IsObjectAndWindow())
	{
		OpenMainFrame();
	}
	//if (g_MainFrame.IsObjectAndWindow())
	//	g_MainFrame->AddEpisodeComment(dwVideoID, dwEpisodeID); 
	//ATLASSERT(0); 
}

BOOL FTrayWindow::OpenAbout(HWND hWndParent /* = NULL */)
{
	if (NULL == hWndParent)
		hWndParent = m_hWnd; 
	FRect rc = ScreenCenteredRect(320, 200); 
	return NULL != m_pAboutDlg.Create(hWndParent, rc, "LiberTV: About", WS_DLGFRAME | WS_SYSMENU | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_STATICEDGE);
}

LRESULT FTrayWindow::OnEndSession(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SendMessage(WM_COMMAND, IDC_EXIT, 0); 
	bHandled = TRUE; 
	return 1; 
}

void FTrayWindow::OnPortsMapped(BOOL bMapped)
{
	m_Updater.CheckConnectable(m_hWnd);
}

LRESULT FTrayWindow::OnConnectable(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == 1)
	{
		m_Connectable = FALSE; 
	//	SetTrayIcon(IconNotConnectable, "LiberTV - You are not connectible!"); 
		if (g_MainFrame.IsObjectAndWindow())
			g_MainFrame->ShowUnconnectableWarning(); 
	}
	else
	{
		m_Connectable = TRUE; 
		//SetTrayIcon(IconNormal, "LiberTV Player"); 
	}

	return 0; 
}

LRESULT FTrayWindow::OnVersion(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == 1)
	{
		/*
			If main frame is opened, and user must be notified about the update, 
			ask him what to do - install update now or later.
			else
			if main frame is not opened and user has 'automatically install updates' checked, then we
			must restart the app
		*/
		HWND hWndParent = m_hWnd; 
		m_HasNewVersion = TRUE; 
		if (g_AppSettings.m_Flags & PFLAG_AUTO_UPDATE)
			m_VersionNotified = TRUE; 

		
		if (g_MainFrame.IsObjectAndWindow())
		{

			if (!m_VersionNotified && 
				g_MainFrame->GetCurrentView() == VIEW_BROWSER || 
				g_MainFrame->GetCurrentView() == VIEW_STATUS)
			{
				hWndParent = g_MainFrame; 
				FDlgNewVersion NewVersionDlg; 
				int nRes = (int)NewVersionDlg.DoModal(g_MainFrame);
				if (nRes == IDOK)
					RestartApp(FALSE); 
				else
					m_VersionNotified = TRUE; 
			}
		}
		else
		{
			//Install update automatically
			if (m_VersionNotified)
				RestartApp(TRUE);
		}
	}
	return 0; 
}

LRESULT FTrayWindow::OnParentNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == WM_DESTROY && lParam == 0xabcdef)	//Main Frame has been destroyed
	{
		if (m_VersionNotified)
		{
			if (g_AppSettings.m_Flags & PFLAG_AUTO_UPDATE)
			{
				Sleep(2000); 
				RestartApp(TRUE); 
			}
		}
	}
	return 0; 
}

LRESULT FTrayWindow::OnTimer(UINT, WPARAM nTimer, LPARAM, BOOL&)
{
	switch(nTimer)
	{
	case VERSION_CHECK_TIMER:
		m_Updater.CheckNewVersion(m_hWnd); 
		KillTimer(VERSION_CHECK_TIMER); 
        break; 
		
	case SPACE_LEFT_WARNING:
		{
			KillTimer(SPACE_LEFT_WARNING); 
			m_MsgSync.Lock(); 
			FString WarningMsg = m_StrMsgWarning; 
			m_MsgSync.Unlock(); 
			ShowBalloon("LiberTV: Low disk space", WarningMsg, 2000, NIIF_ERROR);
		}
		break; 
	case FEED_UPDATE_TIMER:
		{
			g_Objects._RSSManager->CheckForUpdates();
			//KillTimer(FEED_UPDATE_TIMER);
			//SetTimer(FEED_UPDATE_TIMER, 600000);	//10 minutes
		}
		break; 
	case TRAY_TOOLTIP_TIMER:
		if (m_CurrentIcon == IconNormal)
		{

			FString ToolTipText = "LiberTV Player";
			FString DownloadSpeed; 
			FString StrCpu; 
			CCpuUsage usage;
			int iUsage = usage.GetCpuUsage(GetCurrentProcessId());

			if (iUsage > 0)
			{
				StrCpu.Format("\r\nCPU Usage: %3d%%", iUsage);
			}

			if (!g_MainFrame.IsObjectAndWindow())
			{
				FDownloadProgress aProgress; 
				g_Objects._ClipDownloader->GetSessionProgress(aProgress); 
				if (aProgress.m_upBytesPerSec > 1024 || aProgress.m_bytesPerSec > 1024)
				{
					DownloadSpeed.Format("\r\nUp: %.1fKb/s | Down: %.1f Kb/s", aProgress.m_upBytesPerSec / 1024, aProgress.m_bytesPerSec / 1024);
				}
			}

			if (DownloadSpeed.GetLength() > 0)
				ToolTipText.Append(DownloadSpeed);
			
			if (StrCpu.GetLength() > 0)
				ToolTipText.Append(StrCpu); 

			SetTrayIconTooltip(ToolTipText); 
		}
		break; 
	case UPDATE_INSTALL_TIMER:
		{
			KillTimer(UPDATE_INSTALL_TIMER); 
			SendMessage(WM_LTV_VERSION_UPDATE, 1, 1); 
		}
	}
	return 0; 
}

LRESULT FTrayWindow::OnTrayTest(WORD, WORD, HWND, BOOL&)
{
    return 0; 
}

LRESULT FTrayWindow::OnLogWindow(WORD, WORD, HWND, BOOL&)
{
	FRect rc = ScreenCenteredRect(500, 480); 
	m_pLogWnd.Create(m_hWnd, rc, "LiberTV: LogWindow", 
		WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
		WS_EX_STATICEDGE | WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
	return 0; 
}

LRESULT FTrayWindow::OnLTAnnounce(WORD, WORD, HWND, BOOL&)
{
	return 0; 
}

LRESULT FTrayWindow::OnClose(UINT, WPARAM, LPARAM, BOOL&)
{
	DestroyWindow();
	return 0; 
}

LRESULT FTrayWindow::OnExit(WORD,WORD,HWND,BOOL&)
{
	DestroyWindow();
	return 0; 
}


LRESULT FTrayWindow::OnMyTrayIcon(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (lParam == NIN_BALLOONUSERCLICK)
	{
		OpenMainFrame();
		g_MainFrame->m_Container->SwitchActiveView(VIEW_STATUS);
		return 0; 
	}

	switch (LOWORD(lParam))
	{
	case WM_LBUTTONDOWN:
		{
			OpenMainFrame(); 
		}
		break; 
	case WM_RBUTTONUP:
		{
			//Pause/resume
			const tchar* pPauseAll = "Pause &Transfers";
			if (g_Objects._DownloadManager.IsAllPaused())
				pPauseAll = "Resume &Transfers";

			m_TrayMenu.ModifyMenu(ID_TRAY_PAUSEDOWNLOADS, MF_STRING | MF_BYCOMMAND, ID_TRAY_PAUSEDOWNLOADS, pPauseAll);

			if (!g_AppSettings.bDeveloperMode)
			{
				m_TrayMenu.DeleteMenu(ID_TRAY_LOGWINDOW, MF_BYCOMMAND);
			}
			OpenTrayMenu();
		}
		break; 
	}
	return 0; 
}



LRESULT FTrayWindow::OnPauseResume(WORD, WORD, HWND, BOOL&)
{
	
	if (g_Objects._DownloadManager.IsAllPaused())
		g_Objects._DownloadManager.ResumeAll();
	else
		g_Objects._DownloadManager.PauseAll();

	return 0; 
}

LRESULT FTrayWindow::OnTestCommand(WORD, WORD, HWND, BOOL&)
{

	return 0; 
}

void FTrayWindow::OnSetServer()
{
	m_pSelectServer->DestroyWindow(); 

	if (g_MainFrame.IsObjectAndWindow())
	{
		g_MainFrame->NavigateGuide(_RegStr("ChannelGuideURL"), 0); 
	}
	else
		OpenMainFrame(); 
}

void FTrayWindow::SelectServer()
{
	HWND hWndParent = NULL; 
	if (g_MainFrame.IsObjectAndWindow())
		hWndParent = g_MainFrame;

	FRect rc = ScreenCenteredRect(555, 315); 
	const tchar* pszPath = "data/lang_select.html";
	m_pSelectServer.Create(hWndParent, rc, pszPath, WS_SYSMENU | WS_OVERLAPPED | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
		WS_EX_STATICEDGE);
}

bool FTrayWindow::OpenMainFrame()
{

	FString SiteURL = _RegStr("ChannelGuideURL"); 
	if (!PathIsURL(SiteURL))
	{
		SelectServer();
		return true; 
	}

	if (g_MainFrame.IsObjectAndWindow())
	{
		g_MainFrame->RestoreWindow();
	}
	else
	{
		FRect rc = ScreenCenteredRect(1130, 640); 
		
		dword dwStyles = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN ;

		g_MainFrame.Create(NULL, &rc, "LiberTV Player", 
			dwStyles ,   WS_EX_CONTROLPARENT);
	}

	if (m_HasNewVersion && !m_VersionNotified)
	{
		BOOL bHandled = FALSE;
		OnVersion(WM_LTV_VERSION_UPDATE, 1, 0, bHandled); 
	}

	return g_MainFrame.IsObjectAndWindow();
}

bool FTrayWindow::PlayMediaFile(vidtype videoID)
{
	if (OpenMainFrame())
	{
		g_MainFrame->PlayMediaFile(videoID);
		return true; 
	}
	return false; 
}


vidtype  FTrayWindow::LoadMetatorrent(const tchar* FileName)
{
	OpenMainFrame();
	if (g_MainFrame.IsObjectAndWindow())
	{
		FDownloadInfo aInfo; 
		srand((uint)time(NULL)); 		
		aInfo.m_DownloadFile = FileName; 
		return g_Objects._DownloadDispatcher.LoadDownload(aInfo); 
	}
	 return 0; 
}

vidtype  FTrayWindow::OpenSomeMetatorrent()
{
	CFileDialog FileDlg(True, "mtt", NULL, 0, "Metatorrent files(*.mtt)\0*.mtt\0\0", m_hWnd);
	if (IDOK == FileDlg.DoModal())
	{
		return LoadMetatorrent(FileDlg.m_szFileName);
	}
	return 0; 
}

HRESULT FTrayWindow::PlayVideo(vidtype videoID)
{
	if (PlayMediaFile(videoID))
		return S_OK; 
	return E_FAIL; 
}

BOOL FTrayWindow::OpenMediaFromURL(const tchar* szMtUrl)
{
	if (OpenMainFrame())
	{
		FDownloadInfo aInfo; 
		aInfo.m_DownloadUrl = szMtUrl; 
		g_Objects._DownloadDispatcher.OpenMTTFromHTTP(aInfo);
		return TRUE; 
	}
    return FALSE; 
}


int FTrayWindow::OnSettingsChanged(IAppSettings* pNewSettings)
{
	AppSettings& NewSettings = *(AppSettings*)pNewSettings; 
	BOOL bMustRestart = FALSE; 
	if (g_AppSettings.m_IndexPath != NewSettings.m_IndexPath)
	{
		bMustRestart = TRUE; 
	}
	g_AppSettings = NewSettings; 
	g_AppSettings.SaveSettings();

	if (!bMustRestart)
	{
		FClipDownloadConfig Config; 
		g_AppSettings.FillConf(Config); 
		g_Objects._ClipDownloader->UpdateConfig(Config);
	}
	else
	{
		return 1; 
	}
	return 0; 
}

void FTrayWindow::RestartApp(BOOL bSilent)
{
	
	if (bSilent)
		g_Objects.dwRestartOnClose = 2;
	else
		g_Objects.dwRestartOnClose = 1; 

	DestroyWindow(); 
}

void FTrayWindow::ShowSpaceWarning(fsize_type aSpaceReq, fsize_type aSpaceAvail, BOOL bBalloon)
{

	m_MsgSync.Lock(); 

	m_bSpaceBalloon = TRUE; 
	tchar pszsSpaceAvail[64]={0};
	tchar pszSpaceReq[64] = {0};
	
	StrFormatByteSize64(aSpaceAvail, pszsSpaceAvail, 64); 
	StrFormatByteSize64(aSpaceReq, pszSpaceReq, 64);

	m_StrMsgWarning.Format("Cannot start download, because it would leave less than %d MB left on your storage disk.\n"\
				   "Either free some space on disk, or set the warning size setting to a lower value.\n"\
				   "Download size is %s, you currently have %s free on disk.", g_AppSettings.m_MinSpaceOnDriveMB, pszSpaceReq, pszsSpaceAvail);
	m_MsgSync.Unlock(); 
	SetTimer(SPACE_LEFT_WARNING, 100); 
}

void FTrayWindow::PrepareMenu(HMENU hMenu)
{
	CMenu aMenu; 
	aMenu.Attach(hMenu); 
	aMenu.Detach(); 
}


BOOL FTrayWindow::IsConnectable()
{
	return m_Connectable;
}

void FTrayWindow::Conf_OnLoad( LONG lPage )
{
	m_pConfDialog->OnLoad(lPage);
}

void FTrayWindow::Conf_OnUnload( long lPageId )
{
	m_pConfDialog->OnUnload(lPageId);
}

void FTrayWindow::Conf_OnCancel()
{
	m_pConfDialog->OnCancel();
}

void FTrayWindow::Conf_OnSave()
{
	m_pConfDialog->OnSave();
}

int FTrayWindow::Conf_BrowseForFolder( const char* pszCurrent, FString& StrOut )
{
	return m_pConfDialog->BrowseForFolder(pszCurrent, StrOut);
}

BOOL FTrayWindow::ShowBalloon( const char* title, const char * szMsg, const UINT timeout, const DWORD flags )
{
	return CTrayIconImpl<FTrayWindow>::ShowBalloon(title, szMsg, timeout, flags);
}

LRESULT FTrayWindow::PostMsg( UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	return PostMessage(uMsg, wParam, lParam);
}
//////////////////////////////////////////////////////////////////////////

mz_Sync g_DbgAlertSync; 
















