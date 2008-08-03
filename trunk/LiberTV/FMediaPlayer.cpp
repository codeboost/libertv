/*

Copyright (c) 2007, Florin Braghis (florin@libertv.ro)
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the distribution.
* Neither the name of the author nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include "stdafx.h"
#include "FMediaPlayer.h"
#include "GlobalObjects.h"
#include "FMPlayerHTML.h"
#include "FMPlayerDShow.h"
#include "FMainFrame.h"
#include "FMPlayerWM.h"
#include "FMPlayerVLC.h"
#include "AppCheck.h"
#include "FCodecNotFoundDlg.h"
#include "Qedit.h"
#include "FMPlayerVLC_C.h"

			
extern IControlBar*					g_ControlBar; 
extern FAutoWindow<FMainFrame>		g_MainFrame; 

static int cInfoBarHeight = 64;
#define UPDATE_INTERFACE_TIMEOUT 500
//////////////////////////////////////////////////////////////////////////

BOOL IsQTVersionOK()
{
	LONG QTVersion = StrToLong(GetQTVersion().c_str()); 

	if (QTVersion < 7000000)
		return FALSE; 
	return TRUE; 
}

BOOL IsVLCVersionOK()
{
	return TRUE; 
	std::string vlcStr = GetVLCVersion();
	if (vlcStr.length() && (vlcStr[2] >= '8' && vlcStr[4] >= '6'))
		return TRUE; 
	return FALSE; 
}

BOOL CheckQTVersion()
{
	if (!IsQTVersionOK())
	{
		g_MainFrame->ShowMissingCodecDialog("data/qt_notfound.html"); 
		return FALSE; 
	}
	return TRUE; 
}

BOOL CheckVLCVersion()
{
	if (!IsVLCVersionOK())
	{
		g_MainFrame->ShowMissingCodecDialog("data/vlc_notfound.html"); 
		return FALSE; 
	}
	return TRUE; 
}

BOOL IsQuickTime(FString& MediaType)
{
	return MediaType.Find("mov") != -1			|| 
		   MediaType.Find("quicktime") != -1	|| 
		   MediaType.Find("m4v") != -1			||
		   MediaType.Find("mp4") != -1;
}

typedef FMediaPlayerVLC_Ex FPlayerVLC;

IFMediaPlayer* FMediaPlayerPool::CreatePlayer(const tchar* pszMediaType, BOOL bForceVLC)
{
	
	FString MediaType = pszMediaType; 
	MediaType.Trim();
	MediaType.MakeLower();
	
	IFMediaPlayer* pNewPlayer = NULL; 
	if (1)
	{
		if (MediaType.Find("avi") != -1)
		{
			if (MediaType.Find(".div3") != -1 || bForceVLC)
				pNewPlayer = new FPlayerVLC;
			else
				pNewPlayer = new FMPlayerDShow; 
		}
		else
		if (MediaType.Find("mpeg") != -1)
		{
			if (g_AppSettings.m_dwCodecFlags & CODEC_FLAG_USE_WMP)
				pNewPlayer = new FMPlayerWM;
			else
				pNewPlayer = new FPlayerVLC;
		}
		else
		if (MediaType.Find("audio") != -1)
		{
			if (g_AppSettings.m_dwCodecFlags & CODEC_FLAG_USE_WMP)
				pNewPlayer = new FMPlayerWM;
			else
				pNewPlayer = new FPlayerVLC;
		}
		else
		if (MediaType.Find("html") != -1)
		{
			pNewPlayer = new FMediaPlayerHTML; 
		} 
		else
		if (MediaType.Find("flv") != -1)
		{
			pNewPlayer = new FMediaPlayerFLV;
		} 
		else
		if (MediaType.Find("asf") != -1  || MediaType.Find("wmv") != -1)
		{
			if (g_AppSettings.m_dwCodecFlags & CODEC_FLAG_USE_WMP)
				pNewPlayer = new FMPlayerWM;
			else
				pNewPlayer = new FPlayerVLC;
		}
		else
		if (IsQuickTime(MediaType))
		{
			BOOL bIsQT = IsQTVersionOK(); 
			if (g_AppSettings.m_dwCodecFlags & CODEC_FLAG_USE_QUICKTIME && bIsQT)
				pNewPlayer = new FMediaPlayerMOV; 
			else
				pNewPlayer = new FPlayerVLC;
		}
		if (pNewPlayer == NULL)
		{
			pNewPlayer = new FPlayerVLC;
		}
	}
	else
	{
		pNewPlayer = new FPlayerVLC;
	}
	 
	if (pNewPlayer)
    {
        pNewPlayer->SetMediaType(MediaType); 
		m_MediaPlayers.Add(pNewPlayer); 
    }
	
	return pNewPlayer; 
}

IFMediaPlayer* FMediaPlayerPool::FindPlayer(const tchar* pszMediaType)
{
	for (size_t k = 0; k < m_MediaPlayers.GetCount(); k++)
	{
		if (m_MediaPlayers[k]->IsMediaType(pszMediaType))
			return m_MediaPlayers[k];
	}
	return NULL; 
}

void FMediaPlayerPool::Clear()
{
	for (size_t k = 0; k < m_MediaPlayers.GetCount(); k++)
		delete m_MediaPlayers[k];

    m_MediaPlayers.RemoveAll();
}

//////////////////////////////////////////////////////////////////////////

LRESULT FMediaPlayer::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_pPlayer = NULL; 
	m_pControlBar = g_ControlBar; 
	m_curClipIndex = -1; 
	m_PlayCount = 0; 
	SetWindowText(LTV_APP_NAME" Media Player");
	ATLASSERT(m_pControlBar != NULL); 

	m_LastState = vssMediaEnded;
	m_pNotify.Init(m_hWnd); 
	m_bShowSub = FALSE; 
	m_bPaused = FALSE; 
	m_bBuffering = FALSE;
	m_Streaming = FALSE; 
	m_bDisableUpdate = FALSE; 
	m_bForwardPluginInput = FALSE; 

	m_pControlBar->SetPlayer(this); 
	m_pControlBar->SeekBar_SetRange(0, 0); 
	m_pControlBar->SeekBar_SetPos(0); 
	m_pControlBar->SeekBar_Enable(false); 
	m_pPlayer = &m_pBlankMP; 

	CWindowDC aDC(m_hWnd); 
	m_BackgroundDC.CreateCompatibleDC(aDC);

	SetBgImage("data\\images\\player\\logo_not_loaded.bmp");
	m_pStatusBar.SetStatusText(0, "Ready"); 
    return 0; 
}

void FMediaPlayer::SetBgImage(const tchar* pszImage)
{
	m_BackgroundDC.SelectBitmap(0); 
	
	if (m_pBackgroundBmp)
		m_pBackgroundBmp.DeleteObject();
	
	HANDLE hImage = LoadImage(NULL, g_AppSettings.AppDir(pszImage), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE); 

	int err = GetLastError(); 
	if (hImage)
	{
		m_pBackgroundBmp.Attach((HBITMAP)hImage); 
		m_BackgroundDC.SelectBitmap(m_pBackgroundBmp);
	}
	Invalidate(); 
	UpdateWindow();
}

void FMediaPlayer::ReportError(HRESULT hr)
{
	SetBgImage("data\\images\\player\\error.bmp");
	m_pControlBar->Status_FormatMessage(0, "%s: Error opening video: 0x%x", FormatVideoName(), hr);
}

FString FMediaPlayer::FormatVideoName()
{
	FString VideoName;

	if (m_curClipIndex >=0 && m_curClipIndex < (int)m_MediaFiles.m_Files.size())
		if (m_MediaFiles.m_Files[m_curClipIndex].m_Description.GetLength() != 0)
			VideoName.Format("%s - %s", m_pVideo.m_Detail.m_VideoName, m_MediaFiles.m_Files[m_curClipIndex].m_Description);

	if (VideoName.GetLength() == 0)
	{
		long rtPos = (long)(GetPosition() / 10000);
		VideoName = m_pVideo.m_Detail.m_VideoName; 
	}

	return VideoName; 
}

BOOL FMediaPlayer::ShowInfoBar(BOOL bShow)
{
    if (bShow)
    {
        if (!m_pInfoBar.IsWindow())
        {
            m_pInfoBar.Create(m_hWnd, rcDefault, "about:blank", WS_CHILD); 
            m_pInfoBar.Navigate(g_AppSettings.AppDir("data/infopane.html"));
			m_pInfoBar.m_hWndNotify = m_hWnd; 
			FString StrVID, StrEID; 
			StrVID.Format("%u", m_pVideo.m_Detail.m_VideoID); 
			StrEID.Format("%u", m_pVideo.m_Detail.m_EpisodeID); 
			m_pInfoBar.m_pBrowser.CallJScript("setVideoInfo", StrVID, StrEID); 
        }
		else
			UpdateSubtitles(); 
    }

	if (m_pInfoBar.IsWindow())
		m_pInfoBar.ShowWindow(bShow ? SW_SHOW : SW_HIDE); 

    BOOL bHandled = FALSE; 

    OnSize(WM_SIZE, 0, 0, bHandled); 
	m_bShowSub = m_pInfoBar.IsWindow() && m_pInfoBar.IsWindowVisible(); 
    return m_bShowSub; 
}

LRESULT FMediaPlayer::OnInforbarDocComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	FString StrSize = _RegStr("SubFontSize", PLAYER_REG_KEY);
	m_pInfoBar.m_pBrowser.CallJScript("ltv_setFontSize", StrSize);
	long lHeight = m_pInfoBar.m_pBrowser.CallScriptLng("ltv_getHeight", "");
	if (lHeight > 0)
		cInfoBarHeight = lHeight; 
	UpdateSubtitles(); 
	UpdateLayout(); 
	return 0; 
}

LRESULT FMediaPlayer::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UpdateLayout();
    return 0; 
}

LRESULT FMediaPlayer::OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0; 
}

void FMediaPlayer::UpdateLayout()
{
	FRect rcClient; 
	GetClientRect(&rcClient);
	if (m_pInfoBar.IsWindow() && m_pInfoBar.IsWindowVisible())
	{
		FRect rcInfoBar = rcClient; 
		rcInfoBar.top = rcInfoBar.bottom - cInfoBarHeight; 
		m_pInfoBar.MoveWindow(&rcInfoBar, TRUE); 
		rcClient.bottom -= cInfoBarHeight;
	}

	if (m_pPlayer)
		m_pPlayer->SetVideoRect(rcClient); 


	Invalidate();
	UpdateWindow(); 
}

LRESULT FMediaPlayer::OnDestroy(UINT wMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	Stop(); 
	
	return 0; 
}



LRESULT FMediaPlayer::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CPaintDC PaintDC(m_hWnd); 
	FRect r; 
	GetClientRect(&r); 

	CDC dcCompat; 

	dcCompat.CreateCompatibleDC(PaintDC);
	CBitmap bmCompat; 

	bmCompat.CreateCompatibleBitmap(PaintDC, r.Width(), r.Height()); 
	dcCompat.SelectBitmap(bmCompat);

	CBrush aBrush; 
	aBrush.CreateSolidBrush(RGB(0, 0, 0)); 
	dcCompat.FillRect(&r, aBrush); 

	SIZE size; 
	m_pBackgroundBmp.GetSize(size);
	int xP = r.Width() / 2 - size.cx / 2; 
	int yP = r.Height() / 2 - size.cy / 2; 
	dcCompat.BitBlt(xP, yP, size.cx, size.cy, m_BackgroundDC, 0, 0, SRCCOPY); 

	PaintDC.BitBlt(0, 0, r.Width(), r.Height(), dcCompat, 0, 0, SRCCOPY); 

	return 1; 
}

void FMediaPlayer::UpdateBuffering()
{
	if (m_bDisableUpdate)
		return; 

	if (m_bBuffering)
	{

		if (m_pVideo.IsPaused())
		{
			m_pControlBar->Status_FormatMessage(0, "%s: Download suspended. Resume download to continue.", FormatVideoName());
			return ; 
		}

		if (!IsStreaming())
		{
			FDownloadProgress aProgress; 
			g_Objects._DownloadManager.GetDownloadProgress(m_pVideo.GetVideoID(), aProgress); 

			tchar pszBPS[64]={0};
			LONGLONG llBPS = (LONGLONG)aProgress.m_bytesPerSec; 
			StrFormatByteSize64(llBPS, pszBPS, 64); 

			FString StrLeft = ""; 
			if (aProgress.m_bytesPerSec > 10.0)
			{
				size_type bytesLeft = aProgress.m_bytesTotal - aProgress.m_bytesDownloaded;
				double dblTimeLeft = bytesLeft / aProgress.m_bytesPerSec *  1000; //in milliseconds
				char szTimeLeft[128] = {0};
				UINT dwTimeLeftLen = 128;
				if (StrFromTimeInterval(szTimeLeft, dwTimeLeftLen, (DWORD)dblTimeLeft, 3) > 0)
					StrLeft.Format("%s left (%s/s)", szTimeLeft, pszBPS);
			}

			if (StrLeft == "")
				StrLeft.Format("%s/s", pszBPS);

			m_pControlBar->Status_FormatMessage(0, "%s : Buffering - %s", FormatVideoName(), StrLeft); 
		}
		else
		{
			long lProgress; 
			if (SUCCEEDED(m_pPlayer->GetBufferingProgress(&lProgress)))
			{
				m_pControlBar->Status_FormatMessage(0, "%s - Buffering: %d%%", FormatVideoName(), lProgress);
			}
			else
				m_pControlBar->Status_FormatMessage(0, "Buffering: %s", FormatVideoName()); 
		}
		
	}
}
static BOOL bLastBuffering = FALSE; 

void FMediaPlayer::UpdateControlBar()
{
	if (m_bDisableUpdate)
		return; 
	MEDIA_TIME rtCur = GetPosition(); 


	MEDIA_TIME rtTotal = m_pPlayer->GetDuration(); 
	MEDIA_TIME rtAvail = rtTotal; 

	if (IsStreaming())
	{
		rtAvail = m_pPlayer->GetAvailDuration();
	}

	if (rtTotal == 0)
	{
		rtAvail = rtCur; 
		rtTotal = rtCur; 
	}
	else
	{
		if (rtAvail == 0 && !m_bBuffering)
			rtAvail = rtTotal; 
	}

	m_pControlBar->SeekBar_Enable(true); 
	m_pControlBar->SeekBar_SetRange(0, rtTotal); 
	m_pControlBar->SeekBar_SetAvail(rtAvail);
	if (rtCur < rtAvail)
	{
		m_pControlBar->SeekBar_SetPos(rtCur); 
		m_pControlBar->Status_SetTimer(rtCur, rtTotal, false, 0); 
	}

	if (m_bBuffering)
	{
		UpdateBuffering(); 
	}
	else
	{
		if (bLastBuffering == TRUE)
			m_pControlBar->Status_FormatMessage(0, "%s", FormatVideoName(), ""); 
	}

	if (m_bBuffering || IsStreaming())
		m_pControlBar->ShowBuffering(TRUE); 
	else
		m_pControlBar->ShowBuffering(FALSE); 

	g_ControlBar->ShowNextPrev(IsPlaylist()); 
	bLastBuffering = m_bBuffering; 

}

LRESULT FMediaPlayer::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

    m_pVideo = g_Objects._DownloadManager.GetDownloadInfo(m_pVideo.m_Detail.m_VideoID); 
	

	if (m_pVideo.GetAvailMediaCount() > m_MediaFiles.m_Files.size())
    {
        m_pVideo.GetAvailMedia(m_MediaFiles); 
		UpdateMediaTypes();

		if (m_bBuffering)
		{
			PlayNext(FALSE);
			Play(); 
		}
	}

	VideoState aState = m_pPlayer->GetVideoState(); 
	if (aState == vssPlaying)
	{
		m_bDisableUpdate = FALSE; 
		
	}

	if (m_bDisableUpdate)
		return 0; 

	if (m_bShowSub && aState == vssPlaying || aState == vssPaused)
	{
		double dblNow = (GetPosition() / 10000000.0) * m_pPlayer->GetFrameRate();
		UpdateSubtitle((ulong)dblNow); 
	}

	if (aState == vssPlaying)
	{
		if (m_bBuffering)
			m_bBuffering = FALSE;
		
		m_PlayCount = 0; 
		m_pVideo.m_Detail.m_Watched+=UPDATE_INTERFACE_TIMEOUT;
		if (m_LastState != vssPlaying)
			m_pControlBar->SetPlayState(FALSE); 
		m_pPlayer->ShowVideoWnd(SW_SHOW); 
		m_pStatusBar->SetStatusText(0, "Playing"); 
	}
	else
	if (aState == vssPaused)
	{
		m_PlayCount = 0; 
		if (m_LastState != vssPaused)
			m_pControlBar->SetPlayState(TRUE); 
		m_pPlayer->ShowVideoWnd(SW_SHOW); 
		m_pControlBar->Status_FormatMessage(0, "%s", FormatVideoName(), "Paused"); 
		m_pStatusBar->SetStatusText(0, "Paused"); 
	}
	else
	if (aState == vssMediaEnded)
	{
		m_PlayCount = 0; 
		m_bBuffering = FALSE;
		m_pNotify.NotifyPlayer(PluginPlaybackFinished, S_OK); 
		m_pStatusBar->SetStatusText(0, "Playback finished");
	}
	else
	if (aState == vssBuffering)
	{
		m_bBuffering = TRUE; 

		m_pStatusBar->SetStatusText(0, "Buffering..."); 

		MEDIA_TIME rtTimeAvail = m_pPlayer->GetAvailDuration(); 
		MEDIA_TIME rtTimeCur = m_pPlayer->GetPosition(); 
		if ((rtTimeAvail - rtTimeCur) / 10000000 > 30)
		{
			if (!m_bPaused && m_PlayCount < 1)
			{
				if (SUCCEEDED(Play()))
					m_PlayCount++; 
			}
		}

	}

	m_LastState = aState;
	UpdateControlBar();
	return 0; 
}


static bool s_bDown = false; 
static DWORD dwLastClick = 0; 

BOOL FMediaPlayer::ProcessMessage(MSG* pMsg)
{
	UINT uMsg = pMsg->message; 
	LPARAM lParam = pMsg->lParam; 
	WPARAM wParam = pMsg->wParam; 

	POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
	

	if (m_pInfoBar.IsWindow())
	{
		if (pMsg->hwnd == m_pInfoBar || m_pInfoBar.IsChild(pMsg->hwnd))
			return FALSE; 
	}

	if (uMsg == WM_LBUTTONDOWN)
	{
		if (GetTickCount() - dwLastClick < GetDoubleClickTime())
		{
			uMsg = WM_LBUTTONDBLCLK;
			dwLastClick = GetTickCount(); 
		}
	}

	switch(uMsg)
	{
	case WM_LBUTTONDBLCLK:
		{
			if (s_bDown)
			{
				PauseResume(); 
				s_bDown = false; 
			}
			g_MainFrame->SetFullScreen(!g_MainFrame->IsFullScreen());
			return TRUE; 
		}
		break; 
	case WM_LBUTTONDOWN:
		{
			dwLastClick = GetTickCount();
			s_bDown = true; 
			PauseResume();
			return TRUE; 
		}
		break; 
	case WM_MOUSEWHEEL:
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
				m_pControlBar->IncreaseVolume();
			else
				m_pControlBar->DecreaseVolume();
			return TRUE; 
		}
		break;
	}
	return FALSE; 
}

LRESULT FMediaPlayer::OnMouse(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0; 
}

LRESULT FMediaPlayer::OnClipPlaybackFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return OnPluginNotify(WM_PLUGIN_NOTIFY, (WPARAM)PluginPlaybackFinished, S_OK, bHandled);
}

LRESULT FMediaPlayer::OnPluginNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DWORD dwCode = (DWORD)wParam; 
	HRESULT hr = (HRESULT)lParam; 
	switch(dwCode)
	{
	case PluginPlaybackFinished:
		if (m_pPlayer->GetMediaEvent() == EC_COMPLETE)
		{
			PlayNext(FALSE); 
		}
		break; 
	case PluginInitComplete:
		{
			if (SUCCEEDED(hr))
			{
				m_pPlayer->ShowVideoWnd(SW_HIDE); 

				hr = LoadMedia(m_PendingPlay.m_Index, m_PendingPlay.m_Offset); 

				if (SUCCEEDED(hr))
				{
					BOOL bHandled = FALSE; 
					OnSize(WM_SIZE, 0, 0, bHandled); 
					if (!m_bPaused) 
						Play(); 
					else 
						Pause(); 
					m_PendingPlay.Reset(); 
				}
				else if (hr != E_PENDING)	//if E_PENDING, wait for PluginMediaReady notification
				{
					ReportError(hr); 
				}
			}
			else
			{
				if (hr == E_FAIL_SHOW_ERROR)
				{
					m_pPlayer->ShowVideoWnd(SW_SHOW); 
					BOOL bHandled = FALSE; 
					OnSize(WM_SIZE, 0, 0, bHandled); 
				}
				else
				{
					ReportError(hr); 
				}
			}
		}
		break; 
	case PluginMediaReady:
		{
			if (SUCCEEDED(hr))
			{
				SetVolume(g_AppSettings.m_Volume); 
				m_bBuffering = FALSE; 
				m_bDisableUpdate = FALSE; 

				if (!m_bPaused) 
					Play(); 
				else 
					Pause(); 
			}
			else
			{
				ReportError(hr); 
			}
		}
		break; 
	case PluginMediaPlaying:
		{
			::SetActiveWindow(g_MainFrame); 
		}
		break; 

	case PluginMediaBuffering:
		{
			if (hr == S_FALSE)
			{
				m_bBuffering = FALSE; 
				m_pPlayer->ShowVideoWnd(SW_SHOW); 
			}
			else
			{
				m_bBuffering = TRUE; 
				UpdateBuffering(); 
				//See what time we are at. 
				MEDIA_TIME rtCur = m_pPlayer->GetPosition(); 
				if (0 == rtCur)
				{
					m_pPlayer->ShowVideoWnd(SW_HIDE); 
					SetBgImage("data\\images\\player\\logo_loading.bmp");
				}
				else
					m_pPlayer->ShowVideoWnd(SW_SHOW); 
			}
		}
		break; 
	case PluginDownloadQT:
		{
			if (MessageBox("This video requires QuickTime or QuickTime alternative to be installed. Would you like to download and install it now ? ", LTV_APP_NAME": Quicktime required", MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				FString CodecInfo = g_AppSettings.CombineURL(_RegStr("SiteURL"), "codecs.php");
				ShellExecute(NULL, "open", CodecInfo, "", "", SW_SHOW); 
			}
		}
		break; 
	case PluginNavBarLoaded:
		{
			UpdateControlBar();
			if (IsStreaming())
				m_pControlBar->Status_FormatMessage(0, "Streaming: %s", FormatVideoName(), "Playing"); 
			else
				m_pControlBar->Status_FormatMessage(0, "%s", FormatVideoName(), "Playing"); 
		}
		break; 
	default:
		break; 
	}
	return 0; 
}

HRESULT FMediaPlayer::Stop()
{
	
	KillTimer(0); 
	m_LastState = vssStopped;
	SetBgImage("data\\images\\player\\logo_not_loaded.bmp");
	UpdateWindow();

	VideoState curState = m_pPlayer->GetVideoState(); 
	if (curState == vssPlaying || 
		curState == vssPaused)
	{
		duration_type rtCur = (duration_type)(GetPosition() / DURATION_MULT);
		m_pVideo.m_Detail.m_PlaybackPosMS = rtCur; 
		g_Objects._DownloadManager.SaveVideoDetails(m_pVideo.m_Detail.m_VideoID, m_pVideo.m_Detail); 
	}

	
	m_pControlBar->SeekBar_SetRange(0, 0); 
	m_pControlBar->SeekBar_SetPos(0); 
	m_pControlBar->SeekBar_Enable(false);
    m_pControlBar->OnStop(); 
	m_pControlBar->ShowBuffering(FALSE); 
	m_curClipIndex = -1;  
	m_pPlayer->Stop();
	m_pPlayer->Clear();
	m_pMediaPlayers.Clear();
	m_MediaFiles.Clear(); 
	m_pPlayer = &m_pBlankMP;
	m_SubParser.Clear();
	m_bHideInfoBarOnError = FALSE; 
	m_pVideo.Clear(); 
	ShowInfoBar(FALSE); 
	m_PlayCount = 0; 
	m_PluginName = "";
	return S_OK; 
}

MEDIA_TIME FMediaPlayer::GetDuration()
{
	if (m_curClipIndex == -1)
		return 0; 

	return m_pPlayer->GetDuration(); 
}

MEDIA_TIME FMediaPlayer::GetPosition()
{
	if (m_curClipIndex == -1)
		return 0; 
	return m_pPlayer->GetPosition(); 
}


HRESULT FMediaPlayer::LoadMedia(size_t nIndex, MEDIA_TIME rtOffset)
{
	ATLASSERT(nIndex < (int)m_MediaFiles.m_Files.size());

	//m_bDisableUpdate = TRUE; 
	if (nIndex >= (int)m_MediaFiles.m_Files.size())
		return E_INVALIDARG; 

	FMediaFile& curFile = m_MediaFiles.m_Files[nIndex];

	if (curFile.m_DurationMS > 0 && (duration_type)rtOffset > curFile.m_DurationMS * DURATION_MULT)
		return E_INVALIDARG; 
	
	
	IFMediaPlayer* pMP = m_pMediaPlayers.FindPlayer(curFile.m_MediaType);
	/*
	if (pMP)
	{
		pMP->Clear(); 
		m_pMediaPlayers.Clear(); 
	}
	*/
	

	HRESULT hr = E_FAIL; 

	if (NULL == pMP)
	{
		BOOL bForceVLC = TRUE; 
		
		//We need to use xvid for split videos only. The rest goes through VLC.
		if (m_pVideo.m_Clips.GetCount() > 1 && !IsPlaylist())
			bForceVLC = FALSE; 

		pMP = m_pMediaPlayers.CreatePlayer(curFile.m_MediaType, bForceVLC);
        if (pMP)
		{
		    hr = pMP->Init(m_hWnd, &m_pNotify);
			if (hr == E_PENDING)
			{
				m_PendingPlay.m_Index = nIndex; 
				m_PendingPlay.m_Offset = rtOffset; 
				m_pPlayer = pMP; 
				return E_PENDING; 
			}
		}
		else
			return E_FAIL; 
	}
	
	if (m_pPlayer)
	{
		m_pPlayer->SendCommand("GetPluginName", m_PluginName.GetBufferSetLength(256));

		if (m_pPlayer != pMP)
		{
			FRect rcMin;
			m_pPlayer->SetVideoRect(rcMin);
			m_pPlayer->ShowVideoWnd(SW_HIDE); 
			m_pPlayer->Pause(); 
			m_pPlayer = pMP; 
		}

		m_bBuffering = FALSE; 
		if (m_curClipIndex != nIndex)
		{
			m_curClipIndex = (int)nIndex; 
			IMediaOptions pOptions; 
			pOptions.m_SubPath = m_pVideo.m_Detail.m_SubURL; 
			pOptions.rtOffset = rtOffset;
			pOptions.m_Vout = g_AppSettings.m_Vout; 

			hr = m_pPlayer->LoadMedia(curFile.m_FileName, &pOptions);
			if (SUCCEEDED(hr))
            {
				SetVolume(g_AppSettings.m_Volume); 
				RefreshQuickbar();
                return S_OK; 
            }

		}
		else
			hr = m_pPlayer->SeekPosition(rtOffset); 
	}
	else
	{
		m_pPlayer = &m_pBlankMP; 
	}
	return hr; 
}

HRESULT FMediaPlayer::SeekPosition(MEDIA_TIME rPos)
{
	duration_type dtOffs = 0; 
	size_t nIndex = 0; 
	duration_type dtPos = (duration_type)(rPos / DURATION_MULT); 
	HRESULT hr = E_FAIL; 
	if (m_MediaFiles.m_Files.size() == 0)
		return hr; 

	if (m_MediaFiles.m_TotalDurationMS > 0)
	{
		if (m_MediaFiles.GetTimeIndex(dtPos, nIndex, dtOffs))
		{
			hr = LoadMedia(nIndex, dtOffs * DURATION_MULT);
			if (SUCCEEDED(hr))
			{
				if (!m_bPaused) Play(); else Pause(); 
			}
		}
	}
	else
	{
		
		hr = LoadMedia(m_curClipIndex == -1 ? 0 : m_curClipIndex, rPos); 
		if (SUCCEEDED(hr))
		{
			if (!m_bPaused) Play(); else Pause(); 
		}
	}

	return hr; 
}

void FMediaPlayer::RefreshQuickbar()
{
	if (g_MainFrame->m_QuickBar.IsObjectAndWindow())
		g_MainFrame->m_QuickBar->GetBrowser().CallJScript("refresh");
}


BOOL IsSafeFile(const tchar* pszFileName)
{
	const char* szUnsafe[6] = {".com", ".exe", ".pif", ".bat", ".cmd", ".vbs"};
	const tchar* pszExt = PathFindExtension(pszFileName);
	if (pszExt != NULL)
	{
		for (size_t k = 0; k < 6; k++)
		{
			if (stricmp(pszExt, szUnsafe[k]) == 0)
				return FALSE; 
		}
	}
	return TRUE;
}


HRESULT FMediaPlayer::PlayMT(vidtype videoID, BOOL bStartPaused)
{
	HRESULT hr = E_FAIL; 
	m_bBuffering = FALSE; 
	m_Streaming = FALSE; 
	m_bDisableUpdate = FALSE; 
	Stop(); 

	m_bPaused = bStartPaused;

	//g_ControlBar->m_pBrowser.CallJScript("showInfoBtn", "1"); 
	//g_ControlBar->ShowInfoButton(TRUE); 

	m_pVideo = g_Objects._DownloadManager.GetDownloadInfo(videoID); 

	if (!m_pVideo.IsValid())
		return E_FAIL; 

	SetBgImage("data\\images\\player\\logo_loading.bmp");

	g_AppSettings.SetLastVideoID(m_pVideo.GetVideoID());

	if (m_pVideo.GetAvailMedia(m_MediaFiles) > 0)
	{
		UpdateMediaTypes();


		MEDIA_TIME rtPos = 0;

		if (g_AppSettings.m_Flags & PFLAG_RESUME_VIDEOS )
		{
			//rtPos = m_pVideo.m_Detail.m_PlaybackPosMS * DURATION_MULT; 
			rtPos = 0; 
		}

		hr = SeekPosition(rtPos);
		
		if (FAILED(hr) && hr != E_PENDING)
		{

			hr = SeekPosition(0); 

			if (FAILED(hr))
			{
				m_pControlBar->Status_FormatMessage(0, "%s : %s (0x%x)", FormatVideoName(), "Cannot play media", hr); 
				OnCannotPlayContent();
				return hr; 
			}
		}
	}
	else
	{
		hr = S_FALSE;
		if (m_pVideo.m_Detail.m_TimeCompleted == 0)
			m_bBuffering = TRUE; 
		else
		{
			m_pControlBar->Status_SetMessage(0, "Error starting playback. No media files found.");
			return E_FAIL; 
		}
	}

	SetVolume(g_AppSettings.m_Volume); 
	m_pControlBar->SeekBar_Enable(true); 

	if (g_AppSettings.m_Flags & PFLAG_AUTO_SUBTITLES)
	{
		FString DefLang = g_AppSettings.m_DefSubLang; 
		if (DefLang.GetLength() == 0)
			DefLang = "RO";

		m_bHideInfoBarOnError = TRUE; 
		
		if (SUCCEEDED(SelectSubtitle(DefLang)))
			ShowInfoBar(TRUE);
		else
			ShowInfoBar(FALSE); 
	}

	SetTimer(0, UPDATE_INTERFACE_TIMEOUT); 
	SetFocus(); 
	return hr; 
}

HRESULT FMediaPlayer::ResumeLastVideo()
{
	vidtype videoID = g_AppSettings.GetLastVideoID();

	if (videoID != m_pVideo.GetVideoID())
	{
		return PlayMT(videoID, FALSE); 
	}

	return E_FAIL; 
}

HRESULT FMediaPlayer::SetVolume(long lVol)
{
	if (m_pPlayer)
	{
		return m_pPlayer->SetVolume(lVol);
	}
	return E_FAIL;
}

HRESULT FMediaPlayer::Pause()
{
	SetFocus(); 
	HRESULT hr = m_pPlayer->Pause();
	if (SUCCEEDED(hr))
	{
		m_bPaused = TRUE; 
		m_pControlBar->SetPlayState(TRUE); 
		m_pControlBar->Status_FormatMessage(0, "%s", FormatVideoName(), "Paused"); 
		UpdateLayout();
		UpdateControlBar();
	}
	return hr; 
}

HRESULT FMediaPlayer::Play()
{
	SetFocus(); 
	HRESULT hr = m_pPlayer->Play();
	m_bPaused = FALSE; 
	if (SUCCEEDED(hr))
	{
		m_pControlBar->SetPlayState(FALSE); 
		UpdateControlBar();
		if (IsStreaming())
			m_pControlBar->Status_FormatMessage(0, "Streaming: %s", FormatVideoName(), "Playing"); 
		else
			m_pControlBar->Status_FormatMessage(0, "%s", FormatVideoName(), "Playing"); 
		UpdateLayout();
	}
	return hr; 
}

HRESULT FMediaPlayer::PauseResume()
{
	if (m_pPlayer->GetVideoState() == vssPaused)
		return Play();
	return Pause(); 
}

BOOL FMediaPlayer::IsPlaying()
{
	return m_LastState == vssPlaying; 
}

void FMediaPlayer::OnMediaFinished()
{
    Pause(); 
    SeekPosition(0); 
	
	SetBgImage("data\\images\\player\\logo_finished.bmp");
	m_pPlayer->ShowVideoWnd(SW_HIDE); 
	::SendMessage(GetParent(), WM_PLUGIN_NOTIFY, PluginPlaybackFinished, 0); 

	if (g_MainFrame->IsFullScreen())
		g_MainFrame->SetFullScreen(FALSE);
	
}

HRESULT FMediaPlayer::PlayNext(BOOL bCycle)
{

	ATLASSERT(m_curClipIndex >= -1); 

	if (m_curClipIndex >= -1 && m_curClipIndex < (int)m_MediaFiles.m_Files.size() - 1)
	{
		LoadMedia(m_curClipIndex + 1, 0); 
		if (!m_bPaused)
			return Play(); 
	}
	else
	{
		if (!m_pVideo.IsDownloadFinished())
		{
			Pause();
			m_bBuffering = TRUE; 
		}
        else
        {
			if (bCycle)
			{
				LoadMedia(0, 0); 
				if (!m_bPaused)
					return Play(); 
			}
			else{
				OnMediaFinished(); 
				return S_FALSE; 
			}
        }
	}
	return E_FAIL; 
}

HRESULT FMediaPlayer::PlayPrev(BOOL bCycle)
{
	if (m_curClipIndex > 0)
	{
		LoadMedia((size_t)(m_curClipIndex - 1), 0); 
		if (!m_bPaused)
			return Play(); 
	}
	else
	{
		if (bCycle)
		{
			if (m_MediaFiles.m_Files.size() > 0)
			{
				LoadMedia(m_MediaFiles.m_Files.size() - 1, 0); 
				if (!m_bPaused)
					return Play(); 
			}
		}
	}
	return E_FAIL;
}

BOOL FMediaPlayer::PlayClip(int nClipId)
{
	if (m_curClipIndex!=-1 && 
		m_curClipIndex != nClipId && 
		nClipId >=0 && 
		(size_t)nClipId <= m_MediaFiles.m_Files.size())
	{
		HRESULT hr = LoadMedia((size_t)nClipId, 0);
		if (SUCCEEDED(hr))
		{
			if (!m_bPaused)
				Play(); 
			return TRUE; 
		}
	}
	return FALSE; 
}

extern int GetMediaType(FMediaFile* mf);


FString GetMediaTypeDShow(const tchar* pszFileName)
{
	USES_CONVERSION;
	CComPtr<IMediaDet> pMediaDet; 
	HRESULT hr = pMediaDet.CoCreateInstance(CLSID_MediaDet, NULL);
	if (SUCCEEDED(hr))
	{
		hr = pMediaDet->put_Filename(T2OLE(pszFileName));
		if (SUCCEEDED(hr))
		{
			AM_MEDIA_TYPE MediaType;
			hr = pMediaDet->get_StreamMediaType(&MediaType);
			if (SUCCEEDED(hr))
			{
				if (MediaType.majortype == MEDIATYPE_Audio)
					return "audio";
			}
		}

	}
	return "none";
}

void FMediaPlayer::UpdateMediaTypes()
{
	
	BOOL bMustSave = FALSE; 
	for (size_t k = 0; k < m_MediaFiles.m_Files.size(); k++)
	{
		FMediaFile& aFile = m_MediaFiles.m_Files[k];

		//By default, mediatype contains the content-type received from the server
		if (aFile.m_MediaType.Find("html") != -1)
			aFile.m_MediaType = "html";
		else
			aFile.m_MediaType = ""; //automatically determin


		if (aFile.m_MediaType == "" || aFile.m_MediaType == "none")
		{
			if (0 != GetMediaType(&aFile) || aFile.m_MediaType == "none")
			{
				aFile.m_MediaType = GetMediaTypeDShow(aFile.m_FileName); 
			}
			else
			{
				FString NewExt = "";
				if (aFile.m_MediaType == "flv")
					NewExt = ".flv";
				else
				if (aFile.m_MediaType == "asf")
					NewExt = ".wmv";
				else
				if (aFile.m_MediaType == "mov")
					NewExt = ".mov";
				else
				if (aFile.m_MediaType == "m4v")
					NewExt = ".m4v";
				else
				if (aFile.m_MediaType == "mp4")
					NewExt = ".mp4";

				if (NewExt.GetLength() > 0)
				{
					g_Objects._DownloadManager.RenameClipExtension(m_pVideo.m_Detail.m_VideoID, aFile, NewExt); 
					bMustSave = TRUE; 
				}
			}
			aFile.m_MediaType.MakeLower();
		}
	}


	if (bMustSave)
		g_Objects._DownloadManager.SaveDownload(m_pVideo.m_Detail.m_VideoID); 
}

HRESULT FMediaPlayer::GetNotify(IMediaPlayerNotify** pNotify)
{
	*pNotify = &m_pNotify; 
	return S_OK; 
}

void FMediaPlayer::OnCannotPlayContent()
{
	SetBgImage("data\\images\\player\\error.bmp");
	FString FileName;
	
	if (m_MediaFiles.m_Files.size() > 0)
		 FileName = m_MediaFiles.m_Files[0].m_FileName; 

	if (FileName.GetLength() && IsSafeFile(FileName) && PathFileExists(FileName))
	{
		FString Msg;
		Msg.Format(LTV_APP_NAME" cannot play this media file.\nWould you like to open it with the default application ?\n"\
			"File name: %s", FileName);
		int res = MessageBox(Msg, LTV_APP_NAME": Cannot play media", MB_YESNO | MB_ICONQUESTION); 
		if (res == IDYES)
		{

			int err = (int)ShellExecute(NULL, "open", FileName, NULL, "", SW_SHOW); 
			if (err == ERROR_NO_ASSOCIATION)
			{
				PathRemoveFileSpec(FileName.GetBuffer());
				ShellExecute(NULL, "open", FileName, NULL, "", SW_SHOW); 
			}
		}
	}

}

BOOL FMediaPlayer::IsPlaylist()
{
	//return	(m_pVideo.m_Detail.m_VideoFlags & VIDEO_FLAG_IS_PLAYLIST) ||
	return m_MediaFiles.m_Files.size() > 1; 
}

BOOL FMediaPlayer::IsStreaming()
{
	int mSize = (int)m_MediaFiles.m_Files.size();
	if (mSize > 0 && m_curClipIndex >= 0 && m_curClipIndex < mSize)
	{
		if (m_MediaFiles.m_Files[m_curClipIndex].m_FileName.Find("://") != -1)
			return TRUE; 
	}
	return FALSE;
}





