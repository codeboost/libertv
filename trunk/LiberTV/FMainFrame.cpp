#include "stdafx.h"
#include "FMainFrame.h"
#include "GlobalObjects.h"
#include "FMainFrame.h"
#include "windowsx.h"
#include "FClipboard.h"
#include "FDlgInputBox.h"
#include "FDlgSaveVideo.h"
#include "FDlgAddFeed.h"
#include "FDlgRemoveVideo.h"
#include "FDlgShareVideo.h"

int TopBarH = 54;

#define FLAG_FULLSCREEN		0x01
#define FLAG_QUICKBAR_ON	0x02
#define FLAG_CURSOR_HIDDEN	0x04
#define FLAG_TOPBAR_LOADED	0x08


SIZE FMinSize ={320, 200};
SIZE FDefSize = {1024, 768};

FStatusBarObject FMainFrame::m_iStatusBar; 

FMainFrame::FMainFrame()
{
	m_dwFlags = 0; 
	m_hMenu = NULL;
	m_ExStyle = 0; 
	m_Style = 0; 
	m_Container = NULL; 
	m_TopBar = NULL; 
	m_hConnectable = NULL; 
	m_hNotConnectable = NULL;
}



LRESULT FMainFrame::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{						

	m_dwFlags &= ~(FLAG_TOPBAR_LOADED); 

	SetCurrentDirectory(g_AppSettings.m_AppDirectory); 

	FIniConfig aConf; 

	m_Container = new FMainFrameContainer;
	m_TopBar = new FToolbar; 
    
	if (g_AppSettings.m_Flags & PFLAG_SHOW_LAST_VIDEO)
		FMainFrameContainer::m_CurrentView = g_AppSettings.m_LastSection; 

	m_TopBar->m_hWndNotify = m_hWnd; 
	m_TopBar->Create(m_hWnd, rcDefault, _T("TopBar"), WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0 );
	m_Container->Create	(m_hWnd,	rcDefault,	_T("FMainFrameContainer"),	WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0); 

	
	SetIcon(LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_MAIN)));

	m_hWndStatusBar = m_pStatusBar.Create(m_hWnd, rcDefault, "", SBARS_TOOLTIPS | SBARS_SIZEGRIP | WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, (HMENU)IDC_STATUS_BAR);
	
	int Parts[3] = {350, 100, -1};
	m_pStatusBar.SetParts(3, Parts);
	

	m_FntStatus.CreateFont( 12, 0, 0, 0,
		FW_BOLD, 0, 0, 0,DEFAULT_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
		DEFAULT_PITCH|FF_DONTCARE, "Tahoma");

	m_pStatusBar.SetFont(m_FntStatus); 
	m_iStatusBar.SetBarWnd(m_pStatusBar); 
	m_hConnectable = (HICON)LoadImage(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_MAIN), IMAGE_ICON, 16, 16, 0);
	m_hNotConnectable = (HICON)LoadImage(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_NOT_CONNECTABLE), IMAGE_ICON, 16, 16, 0);
	
	g_Objects._DownloadManager.SetStatus(&m_iStatusBar); 

    m_Style = GetWindowLong(GWL_STYLE); 
    m_ExStyle = GetWindowLong(GWL_EXSTYLE); 

	int mx = GetSystemMetrics(SM_CXSCREEN);
	int my = GetSystemMetrics(SM_CYSCREEN); 
	
	if (mx < FDefSize.cx)
		FDefSize.cx = mx; 
	
	if (my < FDefSize.cy)
		FDefSize.cy = my; 

	FRect rcWindow = ScreenCenteredRect(FDefSize.cx, FDefSize.cy);
	
	int nRes = _LoadWindowPosition("MainFrame", rcWindow);
	if (nRes == 1)
    {
        if (rcWindow.Width() < FMinSize.cx)
            rcWindow.right = rcWindow.left + FMinSize.cx; 

        if (rcWindow.Height() < FMinSize.cy)
            rcWindow.bottom = rcWindow.top + FMinSize.cy; 
    }

	m_dwFlags = 0; 
	SetWindowPos(NULL, rcWindow.left, rcWindow.top, rcWindow.Width(), rcWindow.Height(), SWP_NOZORDER);

	if (nRes == 2)	//Maximized
	{
		DWORD dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE); 
		dwStyle |= WS_MAXIMIZE;
		::SetWindowLong(m_hWnd, GWL_STYLE, dwStyle); 
	}
                                                                                                          

	_Module.GetMessageLoop()->AddMessageFilter(this);
	
	m_TopBar->Navigate(g_AppSettings.SkinPath("toolbar.html"), ""); 
	UpdateQuickbarVisibility(); 
	UpdateCtrlLayout();
	g_Objects._DownloadManager.m_hWndNotify = m_hWnd; 
	m_dwLastAction = GetTickCount();	
	SetTimer(0, 1000); 

	CheckProxyServer(); 


	SetTimer(2, 1000); //Status bar update
	return 0; 
}

void FMainFrame::CheckProxyServer()
{
	DWORD dwIgnoreProxy = _RegDword("IgnoreGlobalProxy");

	if (g_AppSettings.m_Proxy.GetLength() == 0 && dwIgnoreProxy == 0)
	{
		FString aProxy, aUser, aPass, aBypass; 
		if (g_AppSettings.QueryUserProxy(aProxy, aUser, aPass, aBypass))
		{
			FString StrMsg; 
			StrMsg.Format("LiberTV has determined that you use a proxy to access the Internet:\n%s\nWould you like to use this proxy for LiberTV downloads?\nNote: You can change the proxy later in the LiberTV settings dialog.", aProxy);
			int nReply = MessageBox(StrMsg, "LiberTV: Proxy Detected", MB_YESNO | MB_ICONQUESTION);
			if (nReply == IDNO)
			{
				_RegSetDword("IgnoreGlobalProxy", 1);
			}
			else
			{
				g_AppSettings.m_Proxy = aProxy; 
				if (aUser.GetLength() > 0)
					g_AppSettings.m_ProxyA = aUser + FString(":") + aPass;

				g_AppSettings.SaveSettings();
			}
		}
	}
}

LRESULT FMainFrame::OnDocumentComplete(UINT uMsg, WPARAM wWnd, LPARAM lParam, BOOL& bHandled)
{
	if ((HWND)wWnd == *m_TopBar)
	{
		m_dwFlags|=FLAG_TOPBAR_LOADED; 
		long lHeight = m_TopBar->m_pBrowser.CallScriptLng("ltv_getHeight", ""); 
		if (lHeight > 0) TopBarH = lHeight; 

		FString StrActive; StrActive.Format("%d", m_Container->m_CurrentView);

		m_TopBar->m_pBrowser.CallJScript("set_active_section", StrActive); 

		ShowWindow(SW_SHOW); 

		m_iStatusBar.SetStatusText(0, "Ready", VIEW_BROWSER); 

		if (!g_Objects._DownloadManager.IsStorageLoaded())
			g_Objects._DownloadManager.LoadStorageAsync(); 
	}
	else
	if (m_QuickBar == (HWND)wWnd)
	{
		UpdateQuickbarVisibility(); 
	}
	return 0; 
}

void FMainFrame::OnViewChanged(int nCurrentView)
{
	UpdateCtrlLayout();
	m_iStatusBar.OnViewChanged(nCurrentView); 
}

void FMainFrame::UpdateCtrlLayout()
{
	UpdateLayout(); 
	
	FRect r; 
	GetClientRect(&r); 

	int Top = 0; 

	if (m_TopBar->IsWindowVisible())
	{
		FRect rTopBar = r; 
		rTopBar.bottom = TopBarH; 
		Top+=TopBarH; 
		m_TopBar->MoveWindow(&rTopBar, TRUE); 
	}

	int ContainerHeight = r.Height() - Top;

	if (m_pStatusBar.IsWindow() && m_pStatusBar.IsWindowVisible())
	{
		FRect rStatusBar; 
		m_pStatusBar.GetWindowRect(&rStatusBar); 
		ContainerHeight-=rStatusBar.Height(); 
	}

	m_Container->SetWindowPos(NULL, 0, Top, r.Width(), ContainerHeight, SWP_NOZORDER); 



	int Parts[4];

	Parts[3] = r.Width();
	Parts[2] = r.Width() - 200;
	Parts[1] = r.Width() - 200 - 22;
	Parts[0] = r.Width() - 200 - 150 - 22;
	if (m_pStatusBar.IsWindow())
	{
		m_pStatusBar.SetParts(4, Parts);
	}
}

LRESULT FMainFrame::OnSize(UINT , WPARAM wParam, LPARAM , BOOL& bHandled)
{
	UpdateCtrlLayout();
	return 1; 
}

LRESULT FMainFrame::OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
{
	if (!IsWindow())
		return 0; 

	g_Objects._DownloadManager.SetStatus(NULL); 
	m_Container->HideActiveView();
	_Module.GetMessageLoop()->RemoveMessageFilter(this); 

	_SaveWindowPosition(m_hWnd, "MainFrame"); 

	if (m_Container->m_pMediaPlayer.IsObjectAndWindow())
		m_Container->m_pMediaPlayer->Stop();

	g_AppSettings.m_LastSection = m_Container->m_CurrentView;

	
	m_Container->DestroyWindow(); 
	m_TopBar->DestroyWindow();

	delete m_Container;
	delete m_TopBar;

	SetProcessWorkingSetSize(GetCurrentProcess(), -1, -1); 

	if (m_hConnectable != NULL)
		DestroyIcon(m_hConnectable);
	if (m_hNotConnectable)
		DestroyIcon(m_hNotConnectable); 

	g_Objects._RSSManager->ClearChannels();
	g_AppSettings.SaveSettings(); 
	g_pAppManager->PostMsg(WM_MAINFRAME_CLOSED, WM_DESTROY, 0xabcdef);
	return 0; 
}

LRESULT FMainFrame::OnSizing(UINT uMsg, WPARAM wSide, LPARAM lParam, BOOL& bHandled)
{

	int MinHeight = FMinSize.cy; 
	LimitMinWndSize(FMinSize.cx, MinHeight, wSide, lParam); 
	bHandled = TRUE; 
	return 1; 
}

LRESULT FMainFrame::OnKeyDown(UINT uMsg, WPARAM wCode, LPARAM lParam, BOOL& b)
{
	CComQIPtr<IWebBrowser2> pBrowser; 

	eBrowsers eBrowser = GetFocusBrowser(pBrowser); 

	switch(wCode)
	{
	case VK_ESCAPE:
		{
			if (IsFullScreen())
				SetFullScreen(FALSE); 
			else
			{
				if (g_AppSettings.m_Flags & PFLAG_ESC_FULLSCREEN)
					SetFullScreen(TRUE); 
			}
			return TRUE;
		}
		break; 
	case VK_RETURN:
		{
			bool bCtrl  = ( GetKeyState(VK_MENU) & 0x8000) != 0;
			if (bCtrl)
			{
				if (IsFullScreen())
					SetFullScreen(FALSE); 
				else
				{
					if (g_AppSettings.m_Flags & PFLAG_ESC_FULLSCREEN)
						SetFullScreen(TRUE); 
				}
				return TRUE;
			}
		}
		break; 
	case VK_F4:
    	OnCommand(WM_COMMAND, MAKEWPARAM(ID_VIEW_QUICKBAR, 0), 0, b);
		break; 
    case VK_F11:
        OnCommand(WM_COMMAND, MAKEWPARAM(ID_VIEW_FULLSCREEN, 0), 0, b); 
        break;
    case VK_F1:
        OnCommand(WM_COMMAND, MAKEWPARAM(ID_HELP_HELP, 0), 0, b); 
        break; 
    case VK_F7:
    case VK_LAUNCH_MEDIA_SELECT:
        OnCommand(WM_COMMAND, MAKEWPARAM(ID_VIEW_CHANNELGUIDE, 0), 0, b); 
        break; 
    case VK_F8:
        OnCommand(WM_COMMAND, MAKEWPARAM(ID_VIEW_MYCOLLECTION, 0), 0, b); 
        break; 
	case VK_F9:
		OnCommand(WM_COMMAND, MAKEWPARAM(ID_VIEW_SUBSCRIPTIONS, 0), 0, b); 
		break; 
	case VK_F10:
        OnCommand(WM_COMMAND, MAKEWPARAM(ID_VIEW_PLAYER, 0), 0, b); 
        break; 
	case VK_F2:
		m_Container->SwitchActiveView(VIEW_STATUS); 
		m_Container->m_pStatus->CycleView();
		break; 
    case VK_MEDIA_PLAY_PAUSE:
		{
			IVideoPlayer* pPlayer = GetPlayer(); 
			if (pPlayer)
			{
				if (pPlayer->GetVideoID() > 0)
				{
					if (pPlayer->IsPlaying())
						pPlayer->Pause();
					else
						pPlayer->Play();
					return 0; 
				}
			}
			else
			{
				m_Container->SwitchActiveView(VIEW_PLAYER, FALSE); 
				pPlayer = GetPlayer(); 
				if (pPlayer)
				{
					vidtype videoID = g_AppSettings.GetLastVideoID();
					if (videoID != 0)
						pPlayer->PlayMT(videoID); 
				}
			}
		}
        break; 
    case VK_F5:
        {
            if (m_Container->m_CurrentView == VIEW_BROWSER)
            {
                m_Container->m_pBrowser->Refresh(REFRESH_COMPLETELY);
            }
        }
        break;
	case 'V':
		{
			if (m_Container->m_CurrentView == VIEW_STATUS )
			{
				bool bCtrl  = ( GetKeyState(VK_CONTROL) & 0x8000) != 0;
				if (bCtrl)
				{
					CClipboard Clipboard; 
					if (Clipboard.Open(m_hWnd))
					{
						FString sStr; 
						if (Clipboard.GetTextData(sStr))
						{
							_DBGAlert("Pasted: %s\n", sStr); 
							if (PathIsURL(sStr))
							{
								FDownloadInfo pInfo; 
								pInfo.m_DownloadUrl = sStr; 
								g_Objects._DownloadDispatcher.OpenMTTFromHTTP(pInfo);
							}
							else
							{/*
								FDownloadInfo pInfo;
								pInfo.m_DownloadFile = sStr; 
								g_Objects._DownloadDispatcher.LoadDownloadAsync(pInfo); 
								*/
							}
						}
					}
				}
			}
		}
		break; 
	default:
		break; 
	}

	
	//if (m_Container->m_CurrentView == VIEW_PLAYER)
	//	return m_Container->m_pMediaPlayer->SendMessage(WM_KEYDOWN, wCode, lParam);
	
	return 0; 
}
long FMainFrame::ShowMenu()
{
	CMenu aMenu; 
	aMenu.LoadMenu(IDR_MAINFRAME);

	
	CMenu aPopupMenu; 
	aPopupMenu.CreatePopupMenu();

	for (size_t k = 0; k < (size_t)aMenu.GetMenuItemCount(); k++)
	{
		FString StrMenuName;
		aMenu.GetMenuString((UINT)k, StrMenuName, MF_BYPOSITION);
		aPopupMenu.InsertMenu((UINT)k, MF_STRING | MF_POPUP | MF_BYPOSITION, aMenu.GetSubMenu((int)k), StrMenuName);
		
	}
	POINT pt;
	GetCursorPos(&pt);
//	::ScreenToClient(m_hWnd, &pt); 


	int nRes = aPopupMenu.TrackPopupMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd, NULL); 

	if (nRes > 0)
	{
		BOOL bHandled = FALSE; 
		OnCommand(WM_COMMAND, MAKEWPARAM(nRes, 0), 0, bHandled); 

		return nRes;
	}
	return 0; 
}

LRESULT FMainFrame::OnCommand(UINT, WPARAM wCmd, LPARAM, BOOL&)
{
	BOOL b; 
	switch (LOWORD(wCmd))
	{
	case ID_FILE_CLOSE:
		DestroyWindow();
		break; 
	case ID_FILE_SETTINGS:
		g_pAppManager->OpenSettings(m_hWnd); 
		break; 
	case ID_FILE_SELECTSERVER:
		{
			g_pAppManager->SelectServer(); 
		}
		break; 
	case ID_VIEW_FULLSCREEN:
		OnPlayerMaximize(0, 0, 0, b); 
		break; 

	case ID_VIEW_CHANNELGUIDE:
		m_Container->SwitchActiveView(VIEW_BROWSER); 
		break; 
	case ID_VIEW_MYCOLLECTION:
		m_Container->SwitchActiveView(VIEW_STATUS); 
		break; 
	case ID_VIEW_PLAYER:
		m_Container->SwitchActiveView(VIEW_PLAYER);
		break; 
	case ID_VIEW_SUBSCRIPTIONS:
		m_Container->SwitchActiveView(VIEW_FEEDS);
		break; 
	case ID_VIEW_QUICKBAR:
		{
			ShowQuickBar();
		}
		break; 
	case ID_VIEW_SESSIONSTATUS:
		{
			ShowStatusWindow();
		}
		break; 
	case ID_HELP_HELP:
		{
			FString HelpURL = _RegStr("HelpURL"); 
			if (PathIsURL(HelpURL))
				ShellExecute(NULL, "open", HelpURL, "",  "", SW_SHOW); 
		}
		break; 
	case ID_HELP_ABOUT:
		g_pAppManager->OpenAbout(m_hWnd);
		break;
	case ID_ASPECTRATIO_AUTOMATIC:
		{
			//SetAspectRatio(0, 0); 
		}
		break; 
	case ID_ASPECTRATIO_4:
		{
			//SetAspectRatio(4, 3); 
		}
		break; 
	case ID_ASPECTRATIO_16:
		{
			//SetAspectRatio(16, 9); 
		}
		break; 
	case ID_UPLOAD_UPLOADCONTENT:
		{
			//ShellExecute(NULL, "open", "http://admin.libertv.ro/", "", "", SW_SHOW); 
			m_Container->SwitchActiveView(VIEW_BROWSER); 
			m_Container->m_pBrowser->Navigate("http://admin.libertv.ro/"); 
		}
		break; 

	case ID_FILE_OPENSTATION:
		{
			FDlgInputBox DlgStation; 
			DlgStation.m_StrCaption = "Open Station";
			DlgStation.m_StrPrompt = "Enter station URL:";

			int nRes = (int)DlgStation.DoModal(m_hWnd); 
			if (nRes == IDOK)
			{
				if (PathIsURL(DlgStation.m_StrDlgResult))
				{
					m_Container->SwitchActiveView(VIEW_BROWSER); 
					m_Container->m_pBrowser->SetStation(DlgStation.m_StrDlgResult);
				}
				else
				{
				}
			}
		}
		break; 
	case ID_CHANGE_COLLECTION:
		{
			FDlgSelectIndexFolder DlgIndexFolder; 
			int nRes = DlgIndexFolder.DoModal(m_hWnd); 
			if (nRes == IDOK)
				g_pAppManager->RestartApp(); 
		}
		break; 

	case ID_FILE_SHARE:
		{
			FDlgShareVideo Dlg; 
			Dlg.DoModal(m_hWnd); 
		}
		break; 
	}

	return 0; 
}

LRESULT FMainFrame::OnSysCommand(UINT, WPARAM wCommand, LPARAM, BOOL& bHandled)
{
	switch(wCommand)
	{
	case SC_SCREENSAVE:
	case SC_MONITORPOWER:
		{
			//Prevent screensaver from starting
			if (m_Container->m_CurrentView == VIEW_PLAYER)
			{
				bHandled = TRUE; 
				return 0; 
			}
		}
		break; 
	case SC_MINIMIZE:
		{
			m_Container->OnMinMax(SC_MINIMIZE);
		}
		break;
	case SC_RESTORE:
		{
			m_Container->OnMinMax(SC_RESTORE);
		}
		break; 
	}
	bHandled = FALSE; 
	return 0; 
}

//////////////////////////////////////////////////////////////////////////

void FMainFrame::UpdateQuickbarVisibility()
{
	if (g_AppSettings.m_Flags & PFLAG_QUICKBAR_ON)
	{
		CheckMenuItem(m_hMenu, ID_VIEW_QUICKBAR, MF_CHECKED);
	}
	else
	{
		CheckMenuItem(m_hMenu, ID_VIEW_QUICKBAR, MF_UNCHECKED);
	}
}

bool FMainFrame::PlayMediaFile(vidtype videoID)
{

	FDownload pDown = g_Objects._DownloadManager.GetDownloadInfo(videoID); 
	if (pDown.IsValid() && pDown.m_Clips.GetCount() > 0)
	{
		FDownloadItem* pClip = pDown.m_Clips[0];
		if (pClip->m_ContentType.Find("html") != -1 &&
			pClip->m_DownloadType == "http")
		{
			ShellExecute(NULL, "open", pClip->m_Href, "", "", SW_SHOW); 
			return true; 
		}
	}

	if (m_Container->m_CurrentView != VIEW_PLAYER)
		m_Container->SwitchActiveView(VIEW_PLAYER, TRUE); 

	if (m_Container->m_pMediaPlayer.IsObjectAndWindow())
	{
		m_Container->m_pMediaPlayer->PlayMT(videoID, FALSE);
	}
    return true;
}

bool FMainFrame::RemoveVideo(vidtype videoID)
{
	int nViewNow = m_Container->m_CurrentView; 
	bool bRemove = false; 

	IVideoPlayer* pPlayer = GetPlayer();
	if (pPlayer && pPlayer->GetVideoID() == videoID)
	{
			//Switch to player
			//if (m_Container->m_CurrentView != VIEW_PLAYER)
			//	m_Container->SwitchActiveView(VIEW_PLAYER); 

			if (IDYES == MessageBox("You are currently playing this video. Are you sure you want to remove it ?", "Confirm remove", MB_YESNO | MB_ICONQUESTION))
			{
				bRemove = true; 
				pPlayer->Stop(); 
			}
	}
	else
	{
		if (g_AppSettings.m_Flags & PFLAG_CONFIRM_REMOVE)
		{
			FDownload pDown = g_Objects._DownloadManager.GetDownloadInfo(videoID); 

			FString MessageText; 
			//MessageText.Format("Are you sure you want to remove '%s' from your collection?", pDown.m_Detail.m_VideoName);
			//bRemove = (MessageBox(MessageText, "Confirm remove", MB_YESNO | MB_ICONQUESTION) == IDYES);
			FDlgRemoveVideo DlgRemove; 
			int nRes = DlgRemove.Open(m_hWnd, pDown.m_Detail.m_VideoName);
			bRemove = (nRes == IDOK);
		}
		else
			bRemove = true; 
	}
	

	if (bRemove)
    {
		g_Objects._DownloadManager.RemoveDownload(videoID); 
    }

	//Restore View
	if (nViewNow != m_Container->m_CurrentView)
		m_Container->SwitchActiveView(nViewNow); 


	vidtype qvid = bRemove?0:videoID;

	return bRemove; 
}

void FMainFrame::SetFullScreen(BOOL bFullScreen)
{
	int xMax = GetSystemMetrics(SM_CXSCREEN); 
	int yMax = GetSystemMetrics(SM_CYSCREEN); 

	DWORD dwStyle = GetWindowLong(GWL_STYLE); 
	DWORD dwExStyle = GetWindowLong(GWL_EXSTYLE); 

	if (bFullScreen)
	{
		if (!IsFullScreen())
		{
			GetWindowRect(&m_OrgRect); 
			m_Style = dwStyle; 
			m_ExStyle = dwExStyle; 

			dwStyle &= ~(WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU  | WS_MAXIMIZE | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME | WS_BORDER);
			dwStyle|=WS_MAXIMIZE;
			dwExStyle&=~ (WS_EX_WINDOWEDGE);
			m_pStatusBar.ShowWindow(SW_HIDE); 

			SetWindowLong(GWL_STYLE, dwStyle);
			SetWindowLong(GWL_EXSTYLE, dwExStyle); 
			int border = 2; 
			int yborder = 2; 

			SetWindowPos(HWND_TOP, -border, -yborder, xMax + border*2, yMax + yborder*2, 0);
			m_dwFlags|=FLAG_FULLSCREEN; 

			m_hMenu = GetMenu(); 
			CheckMenuItem(m_hMenu, ID_VIEW_FULLSCREEN, MF_CHECKED);
			m_TopBar->ShowWindow(SW_HIDE); 
			if (m_Container->m_CurrentView == VIEW_PLAYER)
				m_Container->m_pMediaPlayer->ShowNavbar(FALSE); 
		}
	}
	else		//bFullScreen == FALSE; 
	{
		if (IsFullScreen())
		{
			m_pStatusBar.ShowWindow(SW_SHOW); 
			SetWindowLong(GWL_STYLE, m_Style);
			SetWindowLong(GWL_EXSTYLE, m_ExStyle);
			SetWindowPos(HWND_NOTOPMOST, m_OrgRect.left, m_OrgRect.top, m_OrgRect.Width(), m_OrgRect.Height(), 0);         
			m_dwFlags&=~FLAG_FULLSCREEN; 
			m_TopBar->ShowWindow(SW_SHOW); 
			CheckMenuItem(m_hMenu, ID_VIEW_FULLSCREEN, MF_UNCHECKED);
			if (m_Container->m_CurrentView == VIEW_PLAYER)
				m_Container->m_pMediaPlayer->ShowNavbar(TRUE); 

		}
	}

	UpdateCtrlLayout();
}

BOOL FMainFrame::IsFullScreen()
{
	return (m_dwFlags & FLAG_FULLSCREEN); 
}

LRESULT FMainFrame::OnPlayerMaximize(UINT, WPARAM, LPARAM, BOOL&)
{
	SetFullScreen(!IsFullScreen());
    return 0; 
}


BOOL MustForwardKeyDown(MSG* pMsg)
{
	char disallowedKeys[] = {
		VK_LEFT, VK_RIGHT, VK_RETURN, VK_BACK, VK_UP, VK_DOWN, 0
	};

	//Only forward TAB messages
	if (pMsg->message == WM_KEYDOWN)
	{
		for (int k = 0 ;disallowedKeys[k] != 0; k++)
			if (pMsg->wParam == disallowedKeys[k])
				return FALSE; 
	}
	return TRUE; 
}

eBrowsers FMainFrame::GetFocusBrowser(CComQIPtr<IWebBrowser2>& rpBrowser)
{
	HWND hFocus = GetFocus();  

	/*
		Check the following browsers:
		- Top Bar
		- Channel Guide
		- NavBar
		- Collection
		- MediaPlayer InfoBar
		- MediaPlayer
	*/

	if (m_TopBar->IsChild(hFocus))						//Topbar
	{
		rpBrowser = m_TopBar->m_pBrowser; 
		return brTopBar;
	}
	else
	if (m_CommentDialog.IsWindow() && m_CommentDialog.IsChild(hFocus) || m_CommentDialog == hFocus)
	{
		rpBrowser = m_CommentDialog.m_pW.m_pBrowser;
		return brCommentDlg;
	}
	else
	if (m_Container->m_pBrowser.IsObjectAndWindow() &&	//Chnannelguide
		m_Container->m_pBrowser->IsChild(hFocus))
	{
		rpBrowser = m_Container->m_pBrowser->m_pBrowser; 
		return brGuide; 
	}
	else
	if (m_Container->m_pStatus.IsObjectAndWindow() && m_Container->m_pStatus->IsChild(hFocus))		//Collection
	{
		rpBrowser = m_Container->m_pStatus->m_pBrowser;
		return brCollection; 
	}
	else
	if (m_Container->m_pMediaPlayer.IsObjectAndWindow() && m_Container->m_CurrentView == VIEW_PLAYER)
	{


		if (SUCCEEDED(m_Container->m_pMediaPlayer->GetFocusBrowser(rpBrowser)))
			return brMPlayer;
	}
	else
	if (m_Container->m_CurrentView == VIEW_FEEDS && m_Container->m_pFeeds.IsObjectAndWindow())
	{
		rpBrowser = m_Container->m_pFeeds->m_pBrowser; 
		return brFeeds; 
	}
	//	if (m_ControlBar->IsWindow() && m_ControlBar->IsChild(hFocus))					//NavBar
	//	{
	//		rpBrowser = m_ControlBar->m_pBrowser;
	//		return brNavBar;
	//	}
	//	else

	return brNone; 
}

BOOL FMainFrame::ForwardAccelerators(MSG* pMsg)
{
	if((pMsg->message >= WM_KEYFIRST && pMsg->message <= WM_KEYLAST) ||
		(pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST))
	{

		//Remember last mouse action
		m_dwLastAction = GetTickCount(); 

		if (m_dwFlags & FLAG_CURSOR_HIDDEN)
		{
			ShowCursor(TRUE); 
			m_dwFlags &=~FLAG_CURSOR_HIDDEN; 
		}

		CComQIPtr<IWebBrowser2> pBrowser; 
		eBrowsers theBrowser = (eBrowsers)GetFocusBrowser(pBrowser); 
		BOOL bForward = TRUE; 
		if (theBrowser != brNone)
		{
			if (pMsg->message == WM_KEYDOWN)
				bForward = MustForwardKeyDown(pMsg);

			if (bForward)
			{
				CComQIPtr<IOleInPlaceActiveObject> pOLEObject = pBrowser; 
				if (pOLEObject)
					pOLEObject->TranslateAccelerator(pMsg); 
			}
		}
	}
	return FALSE; 
}

BOOL FMainFrame::PreTranslateMessage(MSG* pMsg)
{

	ForwardAccelerators(pMsg);

/*	if (pMsg->message == WM_MOUSEWHEEL)
	{
		if (m_Container->m_CurrentView == VIEW_PLAYER)
			m_Container->m_pMediaPlayer->SendMessage(WM_MOUSEWHEEL, pMsg->wParam, pMsg->lParam);
		
	}
*/
	if (pMsg->message == WM_KEYDOWN && ((pMsg->lParam & 0x40000000) == 0))
	{
		BOOL b = FALSE; 
		OnKeyDown(WM_KEYDOWN, pMsg->wParam, pMsg->lParam, b);
		return FALSE; 
	}

	BOOL bIsFullScreen = m_dwFlags & FLAG_FULLSCREEN; 
	BOOL bProcessed = FALSE; 

	HWND hWndFocus = GetFocus(); 
	if ((pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST))
	{
		
		if (m_Container->m_CurrentView == VIEW_PLAYER && m_Container->m_pMediaPlayer.IsObjectAndWindow())
		{
			if (m_CommentDialog.IsWindow() && (hWndFocus == m_CommentDialog || m_CommentDialog.IsChild(hWndFocus)))
				;
			else
				if (m_Container->m_pMediaPlayer->ProcessMessage(pMsg))
					return TRUE; 
		}
	}
	

	//Process fullscreen mouse messages...
	if (m_dwFlags & FLAG_FULLSCREEN)
	{
		int MouseHeight = TopBarH; 

		if (m_Container->m_CurrentView == VIEW_PLAYER)
			MouseHeight += m_Container->m_pMediaPlayer->GetNavBarHeight();


		if (pMsg->message == WM_MOUSEMOVE)
		{
			POINT pt = {GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam)};
			::MapWindowPoints(pMsg->hwnd, m_hWnd, &pt, 1); 
			FRect rw; GetClientRect(&rw); 
            if (!m_TopBar->IsWindowVisible())
            {
                if (pt.y < 5)
                    SetTimer(1, g_AppSettings.m_dwAnimTime, NULL); 
            }
            else if (pt.y > MouseHeight)
            {
                KillTimer(1); 
                m_TopBar->ShowWindow(SW_HIDE); 
				if (m_Container->m_CurrentView == VIEW_PLAYER)
					m_Container->m_pMediaPlayer->ShowNavbar(FALSE); 
				UpdateCtrlLayout();
            }
		}	//IsFullScreen
	}  //WM_MOUSEMOVE

	return FALSE; 
}

LRESULT FMainFrame::OnTimer(UINT, WPARAM wID, LPARAM, BOOL&)
{
	POINT pt; GetCursorPos(&pt); 

	if (wID == 0)	//Cursor Poll timer
	{				   
		if (GetActiveWindow() == m_hWnd && 
			m_Container->m_CurrentView == VIEW_PLAYER && 
			m_Container->m_pMediaPlayer->IsPlaying())
		{
			if (GetTickCount() - m_dwLastAction > g_AppSettings.m_dwHideMouse * 1000)
			{
				if (!(m_dwFlags & FLAG_CURSOR_HIDDEN))
				{
					FRect rPlayer; 
					m_Container->m_pMediaPlayer->GetWindowRect(&rPlayer); 

					if (PtInRect(&rPlayer, pt))
					{
						m_dwFlags|=FLAG_CURSOR_HIDDEN;
						ShowCursor(FALSE); 
					}
				}
			}
		}
	}
	else
	if (wID == 1)	//control bar
	{
		if (!m_TopBar->IsWindowVisible())
		{
			FRect rw; 
			GetClientRect(&rw); 
			ScreenToClient(&pt); 
			if (pt.y > 0 && pt.y < TopBarH)
			{
				m_TopBar->ShowWindow(SW_SHOW); 

				if (m_Container->m_CurrentView == VIEW_PLAYER)
					m_Container->m_pMediaPlayer->ShowNavbar(TRUE); 

				UpdateCtrlLayout();
			}
			KillTimer(1);
		}
	} 
	else
	if (wID == 2) //Status bar update timer
	{
		m_pStatusBar.SetText(1, ""); 

		if (g_pAppManager->IsConnectable())
		{
			if (m_pStatusBar.GetIcon(2) != m_hConnectable)
			{
				m_pStatusBar.SetIcon(2, m_hConnectable); 
				m_pStatusBar.SetTipText(2, "You are connectable");
			}
		}
		else
		{
			if (m_pStatusBar.GetIcon(2) != m_hNotConnectable)
			{
				m_pStatusBar.SetIcon(2, m_hNotConnectable); 
				m_pStatusBar.SetTipText(2, "You are not connectable");
			}
		}

		FDownloadProgress aProgress; 
		g_Objects._ClipDownloader->GetSessionProgress(aProgress); 
		FString StatusText; 
		StatusText.Format("Up: %.1fKb/s | Down: %.1f Kb/s", aProgress.m_upBytesPerSec / 1024, aProgress.m_bytesPerSec / 1024);
		m_pStatusBar.SetText(3, StatusText);
	}
	return 0; 
}

void FMainFrame::RestoreWindow()
{
	if (IsIconic())
		SendMessage(WM_SYSCOMMAND, SC_RESTORE, 0); 

	BringWindowToTop(); 
	SetForegroundWindow(m_hWnd); 
}

void FMainFrame::Search(const char* szSearchString, LONG flag)
{
    if (flag == 0)
    {
        if (m_Container->m_CurrentView == VIEW_STATUS)
            m_Container->m_pStatus->m_pBrowser.CallJScript("applyFilter", szSearchString);
    }
    else
    {
        m_Container->SwitchActiveView(VIEW_BROWSER); 
        FString SearchString;
        SearchString.Format("code/front/search.php?q=%s&rand=%d", szSearchString, GetTickCount()); 
        m_Container->m_pBrowser->Navigate(g_AppSettings.ChannelGuideUrl(SearchString)); 
    }
}

HRESULT FMainFrame::NavigateGuide(const tchar* pszURL, ULONG dwFlags)
{
	HRESULT hr = S_OK; 

	if (dwFlags == 1)
	{
		ShellExecute(m_hWnd, "open", pszURL, "", "", SW_SHOW); 
	}
	else
	{
		m_Container->SwitchActiveView(VIEW_BROWSER); 
		
		FString UrlRel;
		
		if (PathIsURL(pszURL))
			UrlRel = pszURL;
		else
			UrlRel = g_AppSettings.ChannelGuideUrl(pszURL); 

		//m_Container->m_pBrowser->ShowWindow(SW_HIDE); 
		hr = m_Container->m_pBrowser->Navigate(UrlRel);

		if (SUCCEEDED(hr)){}
		else
		{
			//m_Container->m_pBrowser->ShowWindow(SW_SHOW); 
		}
	}
	return hr; 
}

void FMainFrame::EpisodeDetails(ULONG ulVideoID)
{
	FDownload pDown = g_Objects._DownloadManager.GetDownloadInfo(ulVideoID);
	if (pDown.IsValid())
	{
		if (PathIsURL(pDown.m_Detail.m_DetailURL) || pDown.m_Detail.m_EpisodeID > 0)
		{
			m_Container->SwitchActiveView(VIEW_BROWSER); 
			if (PathIsURL(pDown.m_Detail.m_DetailURL))
				m_Container->m_pBrowser->Navigate(pDown.m_Detail.m_DetailURL);
			else
				m_Container->m_pBrowser->ShowEpisodeDetails(pDown.m_Detail.m_EpisodeID); 
		}
		else
		{
			if (pDown.m_RSSInfo.m_RSSURL.GetLength() > 0)
				GoToRSS(ulVideoID);
		}
	}
}

void FMainFrame::NavigateToDetails(const tchar* pszDetailsURL)
{
	m_Container->SwitchActiveView(VIEW_BROWSER); 
	m_Container->m_pBrowser->Navigate(pszDetailsURL);
}

IVideoPlayer* FMainFrame::GetPlayer()
{
	if (m_Container->m_pMediaPlayer.IsObjectAndWindow())
		return dynamic_cast<IVideoPlayer*>(m_Container->m_pMediaPlayer.pWindow);

	return NULL; 
}

BOOL FMainFrame::OpenVideoOptions()
{
	FRect r = ScreenCenteredRect(320, 200); 
	if (!m_pVideoOptions.IsObjectAndWindow())
		m_pVideoOptions.Create(m_hWnd, r, "LiberTV: Video Options", WS_DLGFRAME | WS_SYSMENU | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_DLGFRAME, 
		WS_EX_STATICEDGE | WS_EX_TOPMOST);
	return TRUE;
}

void FMainFrame::ShowLoading(BOOL bShow)
{
	m_TopBar->m_pBrowser.CallJScript("showLoading", bShow ? "1" : "0"); 
	if (!bShow)
	{
		m_iStatusBar.SetStatusText(0, "Done", VIEW_BROWSER); 
	}
}

vidtype FMainFrame::GetActiveVideo()
{
	IVideoPlayer* pPlayer = GetPlayer(); 
	if (pPlayer)
	{
		return pPlayer->GetVideoID();
	}
	return 0; 
}


BOOL FMainFrame::ShowQuickBar()
{
	FRect rc = ScreenCenteredRect(320, 480); 
	m_QuickBar.Create(m_hWnd, rc, "data\\quickbar.html", 
		WS_OVERLAPPED | WS_SYSMENU  | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SIZEBOX , 
		WS_EX_TOOLWINDOW);
	return TRUE; 
}

BOOL FMainFrame::ShowStatusWindow()
{
	FRect rc = ScreenCenteredRect(640, 480); 
	m_StatusWindow.Create(m_hWnd, rc, "data\\session_status.html", 
		WS_OVERLAPPED | WS_SYSMENU  | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_SIZEBOX , 
		WS_EX_TOOLWINDOW);
	return TRUE; 

	return TRUE; 
}


//////////////////////////////////////////////////////////////////////////

LRESULT FMainFrame::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0; 
}

LRESULT FMainFrame::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_Container->SetFocus(); 
	return 0; 
}

void FMainFrame::ShowUnconnectableWarning()
{
	//MessageBox("YOU ARE NOT CONNECTABLE!", "LIBERTV", MB_OK | MB_ICONERROR); 
	if (m_Container->m_pBrowser.IsObjectAndWindow())
	{
		m_Container->m_pBrowser->ShowConnectibleWarning(TRUE);
	}

/*
	FClipDownloadConfig aConf; 
	g_AppSettings.FillConf(aConf);
	//aConf.m_MaxKBDown = 200; 
	g_Objects._ClipDownloader->UpdateConfig(aConf); 
*/
}

void FMainFrame::ShowMissingCodecDialog(const tchar* pszPath)
{
	FRect rc = ScreenCenteredRect(640, 480); 
	m_CodecNotFound.Create(m_hWnd, rc, pszPath, WS_DLGFRAME | WS_SYSMENU | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 
		WS_EX_STATICEDGE);
}

DWORD FMainFrame::AddFeedDialog(const tchar* pszUrl, const tchar* pszName)
{
	FDlgAddFeed Dialog; 
	if (IDOK == Dialog.Open(m_hWnd, pszUrl, pszName))
	{
		return g_Objects._RSSManager->AddFeed(Dialog.m_StrUrl, Dialog.m_StrName, 0);
	}
	return FALSE; 
}


LRESULT FMainFrame::OnDownloadLoaded(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	vidtype videoID = (vidtype)wParam; 

	if (videoID == 0)
	{
		MessageBox("LiberTV was unable to download requested file. The format is not recognized.", "LiberTV: Download error", MB_OK | MB_ICONERROR); 
	}
	else
	{
		FDownload pDown = g_Objects._DownloadManager.GetDownloadInfo(videoID); 

		FString StrVideoID; 
		StrVideoID.Format("%u", videoID); 
		if (m_Container->m_CurrentView == VIEW_STATUS)
		{
			m_Container->m_pStatus->m_pBrowser.CallJScript("refreshVideos"); 
			m_Container->m_pStatus->m_pBrowser.CallJScript("onNewDownload", StrVideoID);
		}
		else
		{
			m_TopBar->m_pBrowser.CallJScript("showDownloading");
			if (m_Container->m_CurrentView == VIEW_FEEDS)
			{
				_DBGAlert("Calling onDownloadStarted: %d - videID %d\n", pDown.m_RSSInfo.m_Guid, videoID);
				char szGuid[16]; 
				StringCbPrintf(szGuid, 16, "%d", pDown.m_RSSInfo.m_Guid); 
				m_Container->m_pFeeds->m_pBrowser.CallJScript("onDownloadStarted", szGuid, StrVideoID);
			}

			if (!(g_AppSettings.m_Flags & PFLAG_DONT_SHOW_INFO_DLG) && !(pDown.m_RSSInfo.m_dwFlags & RSS_FLAG_FROM_AUTODOWNLOAD))
			{
				FDlgInfoDownload aDlg; 
				aDlg.DoModal(m_hWnd); 
			}
		}

		if (pDown.m_Clips.GetCount() > 0 && pDown.m_Clips[0]->m_DownloadType == "stream")
		{
			PlayMediaFile(videoID); 
		}
	}
	return 0; 
}

LRESULT FMainFrame::OnMTTDownload(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//wParam = 0 (download starting); wParam = 1 (download finished)
	//lParam = result code (cast to HRESULT);

	if (wParam == 0)
	{
		//m_WaitDlg.Open(m_hWnd, "Retrieving download info"); 
		if (m_Container->m_CurrentView == VIEW_BROWSER || m_Container->m_CurrentView == VIEW_FEEDS)
			SetStatusText(0, "Retrieving download info...", m_Container->m_CurrentView );
	}
	else
	{
		HRESULT hr = (HRESULT)lParam; 
		if (FAILED(hr))
		{
			FString MsgInfo; 
			if (hr == E_NOINTERFACE)
				MsgInfo.Format("LiberTV was unable to download requested file. The format is not recognized.");
			else
				MsgInfo.Format("LiberTV was unable to download requested file. Error code = 0x%x", hr);

			MessageBox(MsgInfo, "LiberTV: Download error", MB_OK | MB_ICONERROR);
		}
		else
		{
			SetStatusText(0, "Done", m_Container->m_CurrentView);
			if (m_WaitDlg.IsWindow())
			{
				m_WaitDlg.DestroyWindow();
				m_WaitDlg.m_hWnd = NULL; 
			}
		}

	}
	return 0; 
}

LRESULT FMainFrame::OnActivate( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	bHandled = TRUE; 
	return 0;
}

LRESULT FMainFrame::OnEnable( UINT, WPARAM w, LPARAM l, BOOL& bHandled )
{
	if (w == TRUE)
		_Module.GetMessageLoop()->AddMessageFilter(this);
	else
		_Module.GetMessageLoop()->RemoveMessageFilter(this); 
	return 0;
}

FMainFrameContainer& FMainFrame::GetContainer()
{
	return *m_Container;
}

void FMainFrame::SetStatusIcon( int nPane, HICON hIcon )
{
	if (m_pStatusBar.IsWindow())
		m_pStatusBar.SetIcon(nPane, hIcon);
}

void FMainFrame::SetStatusText( int nPane, const tchar* pszText, int nObject )
{
	if (m_pStatusBar.IsWindow())
		m_pStatusBar.SetText(nPane, pszText, nObject);
}


void FMainFrame::ShowCommentsWnd()
{
	IVideoPlayer* pPlayer = GetPlayer();
	if (pPlayer)
	{
		vidtype curId = pPlayer->GetVideoID();
		FDownload pDown = g_Objects._DownloadManager.GetDownloadInfo(curId);
		if (pDown.IsValid())
		{
			FString URL;
			URL.Format("%s&display=commentWnd", pDown.m_Detail.m_DetailURL);

			m_CommentDialog.Open(m_hWnd, URL);
		}
	}
}

void FMainFrame::GoToRSS(vidtype videoID)
{
	USES_CONVERSION;

	FDownload pDown = g_Objects._DownloadManager.GetDownloadInfo(videoID);

	if (!pDown.IsValid())
		return; 

	DWORD dwChannelId = pDown.m_RSSInfo.m_RSSGuid;

	if (g_Objects._RSSManager->FeedExists(dwChannelId))
	{
		g_AppSettings.m_dwLastChannelId = dwChannelId; 		
		m_Container->SwitchActiveView(VIEW_FEEDS, TRUE); 
		m_Container->m_pFeeds->GoToChannel(dwChannelId);
	}
	else
	{
		if (PathIsURL(pDown.m_RSSInfo.m_RSSURL))
		{
			FString Message;
			Message.Format("You are no longer subscribed to this channel:\n%s\nWould you like to subscribe to it now?", pDown.m_RSSInfo.m_RSSName);
			if (IDYES == MessageBox(Message, "LiberTV: No channel", MB_YESNO | MB_ICONQUESTION))
			{
				DWORD dwChannelId = AddFeedDialog(pDown.m_RSSInfo.m_RSSURL, pDown.m_RSSInfo.m_RSSName);
				g_AppSettings.m_dwLastChannelId = dwChannelId; 
				m_Container->SwitchActiveView(VIEW_FEEDS, TRUE); 
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

extern FAutoWindow<FMainFrame> g_MainFrame; 

void FStatusBarObject::SetStatusText( int nPane, const tchar* pszText, int nObject )
{
	SynchronizeThread(m_Sync); 
	if (FMainFrameContainer::m_CurrentView == nObject)
	{
		::SendMessage(m_hWndBar, SB_SETTEXT, (WPARAM)nPane, (LPARAM)pszText);
	}
	if (nObject >=0 && nObject < 8)
	{
		m_SectionStr[nObject] = pszText;
	}
}

void FStatusBarObject::SetStatusIcon( int nPane, HICON hIcon )
{
	SynchronizeThread(m_Sync); 
	::SendMessage(m_hWndBar, SB_SETICON, (WPARAM)nPane, (LPARAM)hIcon);
}

void FStatusBarObject::SetBarWnd( HWND hWndBar )
{
	m_hWndBar = hWndBar;
}

void FStatusBarObject::OnViewChanged(int nSection)
{
	SetStatusText(0, m_SectionStr[nSection], nSection); 
}

LRESULT FMainFrame::OnStatusBarClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	LPNMMOUSE pMous = (LPNMMOUSE)pnmh; 
	if (pMous->dwItemSpec == 2)
	{
		//Click on the 'Connectable' icon.
		if (!g_pAppManager->IsConnectable())
		{
			ShellExecute(NULL, "open", g_AppSettings.GetSiteURL("connectible.php"), "", "", SW_SHOW);
		}
	}
	return 0; 
}