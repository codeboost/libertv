#include "stdafx.h"
#include "FMediaContainer.h"
#include "FAdManager.h"
#include "GlobalObjects.h"
#include "FIEControlBar.h"
#include "atlhttp.h"
#include "FMainFrame.h"

IControlBar *g_ControlBar = NULL; 
extern FAutoWindow<FMainFrame>		g_MainFrame; 

FMediaContainer::FMediaContainer()
{
	m_AdManager.Attach(new FVideoAdManager);
	m_bFullScreenAd = FALSE; 
	m_CurVideoID = 0; 
	m_pStatusBar = NULL; 
	m_bFullscreen = FALSE; 
}

FMediaContainer::~FMediaContainer()
{
	m_bPlaying = FALSE; 
}

LRESULT FMediaContainer::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_CBarHeight = 0; 
	m_bFullscreen = FALSE; 
	m_ControlBar.Create(m_hWnd, rcDefault, "FControlBar",  WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS); 
	m_ControlBar.m_hWndNotify = m_hWnd; 
	m_ControlBar.Navigate(g_AppSettings.SkinPath("navbar.html")); 

	g_ControlBar = &m_ControlBar; 

	m_pMediaPlayer.Create(m_hWnd, rcDefault, "FMediaPlayer",  WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS); 
	m_pMediaPlayer.SetStatusBar(m_pStatusBar); 
	m_AdRequests.SetSize(16); 
	m_AdThread.Create(this); 
	return 0; 
}

LRESULT FMediaContainer::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_AdRequests.Clear();
	m_AdThread.StopThread(); 
	Stop(); 
	return 0; 
}

LRESULT FMediaContainer::OnAdClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam >=0 && wParam <= 4)
	{
		OnAdFinished((DWORD)wParam); 
	}
	return 0; 
}

void FMediaContainer::UpdateLayout()
{
	FRect r; 
	GetClientRect(&r); 

	if (m_ControlBar.IsWindowVisible())
	{
		FRect r1 = r; 
		r1.bottom = m_CBarHeight; 
		m_ControlBar.MoveWindow(&r1, TRUE); 
		r.top = r1.bottom; 
		m_ControlBar.UpdateWindow(); 
	}


	if (m_pAdWindows[0] && m_pAdWindows[0]->IsAdShowing())
	{
		Pause(); 
		m_pMediaPlayer.ShowWindow(SW_HIDE); 
		//Full screen ad
		m_pAdWindows[0]->MoveWindow(&r); 

		return; 
	}

	for (size_t k = 1; k < MAX_AD_WINDOWS; k++)
	{
		if (m_pAdWindows[k] && m_pAdWindows[k]->IsAdShowing())
		{
			FRect rWnd; 
			long lAdHeight = m_pAdWindows[k]->GetPreferedHeight();
			long lAdWidth  = m_pAdWindows[k]->GetPreferedWidth();

			AdTypes adType = (AdTypes) (k + 1);
			switch(adType)
			{
			case AdHTop:
				rWnd.SetRect(r.left, r.top, r.right, r.top + lAdHeight);
				break;
			case AdHBottom:
				rWnd.SetRect(r.left, r.bottom - lAdHeight, r.right, r.bottom); 
				break; 
			case AdVRight:
				rWnd.SetRect(r.right - lAdWidth, r.top, r.right, r.bottom); 
				break; 
			case AdVLeft:
				rWnd.SetRect(r.left, r.top, r.right - lAdWidth, r.bottom); 
				break;
			}

			//TODO: Check for reasonable width / height
			m_pAdWindows[k]->MoveWindow(&rWnd); 

			if (!m_pAdWindows[k]->SetZPosition())
			{
				//returns TRUE if window is positioned on top of the Z-order
				//returns FALSE if window z-position is not changed. must resize the video window in this case
				::SubtractRect(&r, &r, &rWnd); 
			}
		}
	}
	m_pMediaPlayer.MoveWindow(&r, TRUE); 
	
}

LRESULT FMediaContainer::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UpdateLayout();
	return 0; 
}

LRESULT FMediaContainer::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == 1)
	{
		if (!m_bFullScreenAd)
		{
			IAdPosition pPos; 
			MEDIA_TIME rtPos = m_pMediaPlayer.GetPosition();

			pPos.m_PlaybackPosMS = rtPos / DURATION_MULT;
			pPos.m_TotalDurationMS = m_pMediaPlayer.GetDuration() / DURATION_MULT;
			if (pPos.m_PlaybackPosMS == pPos.m_TotalDurationMS && pPos.m_TotalDurationMS > 0)
				pPos.m_dwFlags = FLAG_PLAYBACK_ENDED;
			else
				pPos.m_dwFlags = m_pMediaPlayer.IsPlaying() ? FLAG_PLAYBACK_RUNNING : FLAG_PLAYBACK_PAUSED;

			if (pPos.m_dwFlags & FLAG_PLAYBACK_RUNNING)
			{
				DWORD dwCurTick = GetTickCount(); 
				if (m_MSLast > 0)
				{
					m_MSPlayed+= dwCurTick - m_MSLast; 
					pPos.m_MSPlayed = m_MSPlayed; 
				}
				m_MSLast = dwCurTick; 
			}

			//Update each ad window's current playback position
			//It will automatically load any ad that must be shown based on current pos
			for (size_t k = 0; k < MAX_AD_WINDOWS; k++)
				m_pAdWindows[k]->UpdatePosition(pPos); 
		}
	} 
	return 0; 
}

//IHttpNotify
void FMediaContainer::OnDocumentComplete(DWORD dwID, BOOL bMainFrame)
{
	ATLASSERT(dwID < MAX_AD_WINDOWS); 
	ATLASSERT(m_pAdWindows[dwID].m_p != NULL); 
	
	if (dwID == 0)
	{
		//Full screen ad
		m_bFullScreenAd = TRUE; 
		Pause(); 
		for (size_t k = 1; k < MAX_AD_WINDOWS; k++)
		{
			if (m_pAdWindows[k])
				m_pAdWindows[k]->ShowWindow(SW_HIDE); 
		}
		m_pMediaPlayer.ShowWindow(SW_HIDE); 

	}
	m_pAdWindows[dwID]->ShowWindow(SW_SHOW);
	UpdateLayout();
}

void FMediaContainer::OnAdFinished(DWORD dwID)
{
	ATLASSERT(dwID < MAX_AD_WINDOWS); 
	ATLASSERT(m_pAdWindows[dwID].m_p != NULL); 

	m_pAdWindows[dwID]->ShowWindow(SW_HIDE);

	if (dwID == 0)	//Full screen
	{
		m_pMediaPlayer.ShowWindow(SW_SHOW); 
		if (m_bPlaying)
			Play(); 

		//restore small ad windows, which have been hidden when the full screen ad started
		for (size_t k = 1; k < MAX_AD_WINDOWS; k++)
		{
			if (m_pAdWindows[k] && m_pAdWindows[k]->IsAdLoaded() && !m_pAdWindows[k]->IsAdShowing())
				m_pAdWindows[k]->ShowWindow(SW_SHOW); 
		}
		m_bFullScreenAd = FALSE; 
	}
	UpdateLayout();
}

BOOL FMediaContainer::OnNavigateError(DWORD dwID, BOOL bMainFrame, const tchar* pszUrl)
{
	ATLASSERT(dwID < MAX_AD_WINDOWS); 
	ATLASSERT(m_pAdWindows[dwID].m_p != NULL); 
	{

		m_pAdWindows[dwID]->ShowWindow(SW_HIDE);
		if (dwID == 0)
		{
			m_bFullScreenAd = FALSE; 
			m_pMediaPlayer.ShowWindow(SW_SHOW); 
			if (m_bPlaying)
				Play(); 
		}
		UpdateLayout();
	}
	return false; 
}

bool WINAPI mCallback(DWORD bytes, DWORD objPtr)
{
	FMediaContainer* pThis = (FMediaContainer*)objPtr; 
	return true; //continue
}

void FMediaContainer::FAdLoader::Thread(void* p)
{
	FMediaContainer* pContainer = (FMediaContainer*)p; 

	for (;;)
	{
		bool bPopped = false; 
		vidtype videoID = 0; 
		int nRes = pContainer->m_AdRequests.PopWait(videoID, bPopped);
		if (nRes == WAIT_OBJECT_0 && bPopped)
		{
			pContainer->DownloadAdpoints(videoID); 
		}
		else
		{
			if (nRes == WAIT_IO_COMPLETION)
				break; 
		}
	}
}

BOOL FMediaContainer::DownloadAdpoints(vidtype videoID) 
{
	CAtlHttpClient Client; 
	FString AdRequest; 
	AdRequest.Format("/guide/ads.php?videoID=%d", videoID); 
	FString pszUrl = g_AppSettings.ChannelGuideUrl(AdRequest);
	
	CAtlNavigateData Data; 
	
	Data.SetReadStatusCallback(mCallback, (DWORD_PTR)this); 
	Data.dwTimeout = 15000; //15 seconds good enough ?
	if (Client.Navigate(pszUrl, &Data) && Client.GetStatus() != ATL_INVALID_STATUS)
	{
		if (Client.GetStatus() == 200) // 200 = successful HTTP transaction
		{
			DWORD size = Client.GetBodyLength();
			if (size > 0)
			{
				CAutoPtr<char> Buffer (new char[size + 1]);
				memcpy(Buffer.m_p, Client.GetBody(), size); 
				FIniConfig aConf;
				if (aConf.LoadFromBuffer(Buffer, size))
				{
					if (m_CurVideoID == videoID)
						return m_AdManager->LoadAds(aConf, TRUE); 
				}
			}
		}
	}
	return FALSE; 
}

HRESULT FMediaContainer::PlayMT(ulong videoID, BOOL bPaused /* = FALSE */)
{
	if (m_pStatusBar)
		m_pStatusBar->SetStatusText(0, "Loading video plug-in...", VIEW_PLAYER);

	m_bFullScreenAd = FALSE; 

	m_MSLast = 0; 
	m_MSPlayed = 0; 

	HRESULT hr = m_pMediaPlayer.PlayMT(videoID, bPaused); 
	BOOL bFullscreenAd = FALSE; 
	
	for (size_t k =0 ; k < MAX_AD_WINDOWS; k++)
	{
		if (m_pAdWindows[k])
		{
			m_pAdWindows[k]->ShowWindow(SW_HIDE); 
			m_pAdWindows[k]->Clear(); 
		}
	}

	m_CurVideoID = videoID; //No sync, let it burn!

	m_AdManager->Clear();
	if (SUCCEEDED(hr) || hr == E_PENDING)
	{
		m_AdRequests.Push(videoID); 
		m_bPlaying = !bPaused; 
		FDownload &pDown = m_pMediaPlayer.m_pVideo; 

		FIniConfig aConf = g_Objects._DownloadManager.GetDownloadConf(videoID); 
		
		if (m_AdManager->LoadAds(aConf, TRUE))
		{
			IAdWindowInit InitData; 
			InitData.m_hWndParent = m_hWnd; 
			InitData.m_pAdManager = m_AdManager.m_p; 
			InitData.m_pNotify = this;

			IAdPosition pPos; 
			pPos.m_PlaybackPosMS = pDown.m_Detail.m_PlaybackPosMS / DURATION_MULT;
			pPos.m_TotalDurationMS = pDown.m_Detail.m_TotalDurationMS; 
			pPos.m_dwFlags = FLAG_PLAYBACK_START; 

			for (size_t k = 0; k < MAX_AD_WINDOWS; k++)
			{
				InitData.m_AdType = (AdTypes)(k + 1);
				InitData.m_dwWinID = (DWORD)k; 

				if (m_pAdWindows[k] == NULL)
					m_pAdWindows[k].Attach(new FAdWindow);

				if (m_pAdWindows[k]->Init(InitData))
				{
					BOOL bHasAd = m_pAdWindows[k]->UpdatePosition(pPos); 
					if (k == 0 && bHasAd)
						bFullscreenAd = TRUE; 
				}
			}	
			SetTimer(1, 500); 
		}
	}
	
	m_pMediaPlayer.ShowWindow(SW_SHOW); 
	if (bFullscreenAd)
	{
		Pause(); 
	}
	UpdateLayout(); 

	if (FAILED(hr))
	{
		if (hr != E_PENDING)
		{
			if (m_pStatusBar)
				m_pStatusBar->SetStatusText(0, "Cannot play video.", VIEW_PLAYER);
		}
	}
	else
	{
		if (m_pStatusBar)
		{
			if (IsPlaying())
				m_pStatusBar->SetStatusText(0, "Playing", VIEW_PLAYER); 
			else
				m_pStatusBar->SetStatusText(0, "Paused", VIEW_PLAYER); 
		}
	}
	return hr; 
}


HRESULT	FMediaContainer::SetVolume(long lVol)
{
	if (!m_bFullScreenAd)
		return m_pMediaPlayer.SetVolume(lVol);
	return E_PENDING; 
}
HRESULT	FMediaContainer::SetPosition(MEDIA_TIME lPos)
{
	if (!m_bFullScreenAd)
		return m_pMediaPlayer.SetPosition(lPos); 

	return E_PENDING; 
}

HRESULT	FMediaContainer::Play()
{
	HRESULT hr = E_PENDING; 
	if (!m_bFullScreenAd)
	{
		m_bPlaying = TRUE; 
		m_MSLast = GetTickCount(); 
		hr = m_pMediaPlayer.Play();
		if (SUCCEEDED(hr) && m_pStatusBar)
			m_pStatusBar->SetStatusText(0, "Playing", VIEW_PLAYER); 
			
	}
	return hr; 
}

HRESULT	FMediaContainer::Pause()
{
	m_bPlaying = FALSE; 
	m_MSLast = 0; 
	HRESULT hr = m_pMediaPlayer.Pause(); 
	if (SUCCEEDED(hr) && m_pStatusBar)
		m_pStatusBar->SetStatusText(0, "Paused", VIEW_PLAYER); 
	return hr; 

}
HRESULT	FMediaContainer::Stop()
{
	m_MSPlayed = 0; 
	m_MSLast = 0; 
	HRESULT hr = m_pMediaPlayer.Stop(); 

	if (m_pStatusBar)
		m_pStatusBar->SetStatusText(0, "Stopped", VIEW_PLAYER); 
	return hr; 
}

BOOL FMediaContainer::IsPlaying()
{
	return m_pMediaPlayer.IsPlaying(); 
}

HRESULT	FMediaContainer::PlayNext(BOOL bCycle)
{
	if (!m_bFullScreenAd)
		return m_pMediaPlayer.PlayNext(bCycle);
	return E_PENDING; 
}

HRESULT	FMediaContainer::PlayPrev(BOOL bCycle)
{
	if (!m_bFullScreenAd)
		return m_pMediaPlayer.PlayPrev(bCycle); 
	return E_PENDING; 
}

ulong FMediaContainer::GetVideoID()
{
	return m_pMediaPlayer.GetVideoID();
}

int	FMediaContainer::GetCurrentClipId()
{
	return m_pMediaPlayer.GetCurrentClipId(); 
}

BOOL FMediaContainer::PlayClip(int nClipId)
{
	if (!m_bFullScreenAd)
		return m_pMediaPlayer.PlayClip(nClipId); 
	return E_PENDING; 
}

HRESULT	FMediaContainer::SelectSubtitle(const tchar* pszLang)
{
	return m_pMediaPlayer.SelectSubtitle(pszLang); 
}


HRESULT FMediaContainer::GetSubtitleFilename(char* pszSubFilename, DWORD* dwLen)
{
	return m_pMediaPlayer.GetSubtitleFilename(pszSubFilename, dwLen); 
}

HRESULT FMediaContainer::GetFocusBrowser(CComQIPtr<IWebBrowser2>& rpBrowser)
{
	if (m_bFullScreenAd && m_pAdWindows[0] != NULL)
	{
		rpBrowser = *(FWebWindow*)m_pAdWindows[0].m_p;
		return S_OK; 
	}
	return E_FAIL; 
	/*
	FIEControlBar* pBar = (FIEControlBar*)m_pMediaPlayer.m_pControlBar;
	rpBrowser = pBar->m_pBrowser;
	return S_OK; 
	*/
}

LRESULT FMediaContainer::OnPlayerNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch(wParam)
	{
	case PluginPlaybackFinished:
		{	
			KillTimer(1); 
			IAdPosition pPos; 
			pPos.m_PlaybackPosMS = 0;
			pPos.m_TotalDurationMS = m_pMediaPlayer.m_pVideo.m_Detail.m_TotalDurationMS; 
			pPos.m_dwFlags = FLAG_PLAYBACK_ENDED;
			for (size_t k = 0; k < MAX_AD_WINDOWS; k++)
			{
				if (m_pAdWindows[k])
					m_pAdWindows[k]->UpdatePosition(pPos); 
			}

			
		}
		break; 
	}
	return 0; 
}

LRESULT FMediaContainer::OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == TRUE)
	{
		if (!m_bFullscreen)
		{
			m_ControlBar.ShowWindow(SW_SHOW);
			m_CBarHeight = m_ControlBar.m_pBrowser.CallScriptLng("ltv_getHeight", ""); 
		}
		else
		{
			m_ControlBar.ShowWindow(SW_HIDE);
		}

		if (m_pStatusBar)
		{
			if (m_pMediaPlayer.m_pVideo.IsValid())
			{
				if (IsPlaying())
					m_pStatusBar->SetStatusText(0, "Playing", VIEW_PLAYER);
				else
					m_pStatusBar->SetStatusText(0, "Paused", VIEW_PLAYER); 
			}
			else
				m_pStatusBar->SetStatusText(0, "Ready", VIEW_PLAYER); 
		}
	}
	else
	{
		m_ControlBar.ShowWindow(SW_HIDE);
	}
	UpdateLayout();
	return 0; 
}

LRESULT FMediaContainer::OnFocus( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	return 0;
}

LRESULT FMediaContainer::OnDocumentComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	_DBGAlert("FMediaContainer: Document complete\n");
	
	long lHeight = m_ControlBar.m_pBrowser.CallScriptLng("ltv_getHeight", ""); 
	if (lHeight > 0)
		m_CBarHeight = lHeight; 
	m_ControlBar.ShowWindow(SW_SHOW);
	

	//adjust volume
	m_pMediaPlayer.SetVolume(g_AppSettings.m_Volume);
	
	UpdateLayout();
	
	return 0; 
}

LRESULT FMediaContainer::OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	bool bShift = ( GetKeyState(VK_SHIFT) & 0x8000) != 0;
	bool bCtrl  = ( GetKeyState(VK_CONTROL) & 0x8000) != 0;


	switch(wParam)
	{
	case 'F':
		g_MainFrame->SetFullScreen(!g_MainFrame->IsFullScreen());
		break; 
	case 'I':
		{
			g_MainFrame->ShowQuickBar(); 
		}
		break; 
	case 'S':
		{
			BOOL bVisible = m_pMediaPlayer.m_pInfoBar.IsWindow() && m_pMediaPlayer.m_pInfoBar.IsWindowVisible(); 
			m_pMediaPlayer.ShowInfoBar(!bVisible); 
			if (m_pMediaPlayer.m_pInfoBar.IsWindow())
				m_pMediaPlayer.m_pInfoBar.m_pBrowser.CallJScript("showSubs"); 
		}
		break; 
	case 'O':
		{
			m_pMediaPlayer.ShowInfoBar(TRUE); 
			g_MainFrame->OpenVideoOptions();
		}
		break; 
	case VK_UP:
		m_pMediaPlayer.m_pControlBar->IncreaseVolume();
		break; 
	case VK_DOWN:
		m_pMediaPlayer.m_pControlBar->DecreaseVolume();
		break; 
	case VK_SPACE:
		m_pMediaPlayer.PauseResume();
		break; 
	case VK_LEFT:
		{
			dword dwMultiply = bShift ? 5 : 1;
			if (bCtrl)
				dwMultiply = 30; 

			MEDIA_TIME rtPos = m_pMediaPlayer.GetPosition(); 
			MEDIA_TIME OneSecond = 1000 * DURATION_MULT * dwMultiply; 
			if (rtPos < OneSecond)
				OneSecond = rtPos;
			m_pMediaPlayer.SeekPosition(rtPos - OneSecond);
		}
		break; 
	case VK_RIGHT:
		{
			dword dwMultiply = bShift ? 5 : 1; 
			if (bCtrl)
				dwMultiply = 30; 

			MEDIA_TIME rtPos = m_pMediaPlayer.GetPosition(); 
			MEDIA_TIME OneSecond = 1000 * DURATION_MULT * dwMultiply; 
			m_pMediaPlayer.SeekPosition(rtPos + OneSecond); 
		}
		break; 
	case VK_NEXT: //page down
		{
			PlayNext(TRUE); 
		}
		break; 
	case VK_PRIOR:
		{
			PlayPrev(TRUE); 
		}
		break; 
	}

	return 0; 
}

BOOL FMediaContainer::ProcessMessage( MSG* pMsg )
{
	FRect r;
	BOOL bInPlayer = FALSE; 
	
	m_pMediaPlayer.GetClientRect(&r);

	POINT pt = pMsg->pt; 
	::ScreenToClient(m_pMediaPlayer, &pt);
	bInPlayer = PtInRect(&r, pt);

	if (pMsg->message == WM_MOUSEWHEEL)
	{
		if (GET_WHEEL_DELTA_WPARAM(pMsg->wParam) > 0)
			m_pMediaPlayer.m_pControlBar->IncreaseVolume();
		else
			m_pMediaPlayer.m_pControlBar->DecreaseVolume();
		return TRUE; 
	}


	if (pMsg->hwnd == m_pMediaPlayer || m_pMediaPlayer.IsChild(pMsg->hwnd))
		return m_pMediaPlayer.ProcessMessage(pMsg); 
	return FALSE;
}

void FMediaContainer::SetStatusBar( IStatusBar* pStatusBar )
{
	m_pStatusBar = pStatusBar;
	m_pMediaPlayer.SetStatusBar(m_pStatusBar);
}

HRESULT FMediaContainer::GetNotify( IMediaPlayerNotify** pNotify )
{
	return m_pMediaPlayer.GetNotify(pNotify);
}

void FMediaContainer::SetFullScreen( BOOL bFullScreen )
{
	m_bFullscreen = bFullScreen;
	m_ControlBar.ShowWindow(bFullScreen ? SW_HIDE : SW_SHOW);
	if (!bFullScreen)
		m_CBarHeight = m_ControlBar.m_pBrowser.CallScriptLng("ltv_getHeight", ""); 
	UpdateLayout();
}