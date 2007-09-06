#include "stdafx.h"
#include "FAdManager.h"



//////////////////////////////////////////////////////////////////////////
void FAdPoints::Clear()
{
	m_pAds.RemoveAll();
}

BOOL FAdPoints::LoadAdPoints(FIniConfig& pConf, BOOL bAddToList)
{
	if (!bAddToList)
		Clear(); 

	for (size_t k = 0; k < 16; k++)
	{
		FString StrSection; 
		StrSection.Format("Point %d", k); 
		//if (!pConf.SectionExists(StrSection))
		//	break; 

		FString StrURL = pConf.GetValue(StrSection, "URL"); 
		StrURL.Trim(); 
		if (StrURL.GetLength() > 0)
		{
			FAdItem pItem;
			pItem.m_URL = StrURL; 
			pItem.m_TimeMS = pConf.GetValueUINT64(StrSection, "VideoPositionMS");
			pItem.m_TimeEndMS = pConf.GetValueUINT64(StrSection, "VideoPositionEndMS"); 
			pItem.m_Type = (AdTypes)pConf.GetValueDWORD(StrSection, "DisplayType"); 
			pItem.m_TimeoutMS = pConf.GetValueDWORD(StrSection, "TimeoutMS"); 
			pItem.m_dwFlags = pConf.GetValueDWORD(StrSection, "Flags", 16); 
			pItem.m_Height = pConf.GetValueDWORD(StrSection, "Height"); 
			pItem.m_Width = pConf.GetValueDWORD(StrSection, "Width"); 
			pItem.m_MSPlayed = pConf.GetValueDWORD(StrSection, "PlayedMS"); 
			pItem.m_TimePercent = (float)atof(pConf.GetValue(StrSection, "VideoPositionPercent")) / 100; 
			pItem.m_TimeEndPercent = (float)atof(pConf.GetValue(StrSection, "VideoPositionEndPercent")) / 100; 
			pItem.m_dwAdID = k + 1; 
			m_pAds.Add(pItem); 
		}
	}
	return TRUE; 
}

void FVideoAdManager::Clear()
{
	SynchronizeThread(m_Sync); 
	m_Ads.Clear(); 
}

BOOL FVideoAdManager::LoadAds(FIniConfig& pConf, BOOL bAddToList)
{
	SynchronizeThread(m_Sync); 
	if (!bAddToList)
		m_Ads.Clear();
	return m_Ads.LoadAdPoints(pConf, bAddToList); 
}

BOOL FVideoAdManager::GetAds(AdTypes aType, std::vector<FAdItem> &outAds)
{
	SynchronizeThread(m_Sync); 
	size_t count = 0; 
	for (size_t k = 0; k < m_Ads.m_pAds.GetCount(); k++)
	{
		FAdItem& pItem = m_Ads.m_pAds[k];

		if (pItem.m_Type  == aType)		//TODO: What filters should we have here ?
		{
			outAds.push_back(pItem); 
			count++; 
		}
	}

	return count > 0; 
}

//////////////////////////////////////////////////////////////////////////

FAdWindow::FAdWindow()
{
	m_bAdLoaded = FALSE; 
	m_bAdInProgress = FALSE; 
	m_LastState = 0; 
}

BOOL FAdWindow::CreateWnd()
{
	FWebWindow::SetNotify(this, m_InitData.m_dwWinID); 

	Create(m_InitData.m_hWndParent, rcDefault, "about:blank", WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

	DWORD dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE); 
	dwStyle &= ~(WS_HSCROLL | WS_VSCROLL);
	SetWindowLong(GWL_STYLE, dwStyle);

	return m_hWnd != NULL; 
}

BOOL FAdWindow::Init(IAdWindowInit& pInitData)
{
	m_InitData = pInitData; 
	m_Ads.clear(); 
	return m_InitData.m_pAdManager->GetAds(m_InitData.m_AdType, m_Ads);
}

void FAdWindow::Clear()
{
	m_Ads.clear(); 
	m_bAdInProgress = FALSE; 
	m_bAdLoaded = FALSE; 
	m_CurrentItem.m_URL = "";
	m_CurrentItem.m_dwAdID = 0; 
}

void FAdWindow::UpdateTimes(FAdItem& pItem, IAdPosition& pPos)
{
	if (pItem.m_TimePercent > 0.0f && pPos.m_TotalDurationMS)
	{
		pItem.m_TimeMS = (duration_type)(pItem.m_TimePercent * pPos.m_TotalDurationMS);
	}

	if (pItem.m_TimeEndPercent > 0.0f && pPos.m_TotalDurationMS)
	{
		pItem.m_TimeEndMS = (duration_type)(pItem.m_TimeEndPercent * pPos.m_TotalDurationMS);
	}
}

BOOL FAdWindow::UpdatePosition(IAdPosition& pPos)
{
	//TODO: Find ads matching this position.
	//Hide ad if m_EndPos > pPos.curPos

	DWORD dwPlayState = pPos.m_dwFlags & FLAG_PLAYBACK_MASK; 

	if (m_bAdLoaded)
	{
		if (m_CurrentItem.m_TimeEndMS < pPos.m_PlaybackPosMS)
		{
			//Hide ad if no timeout exists.
			if (m_CurrentItem.m_TimeoutMS == 0 && !(m_CurrentItem.m_dwFlags & AD_FLAG_PERSISTENT))
				OnAdFinished();
		}
	}
	//Still loaded ?
	if (m_bAdLoaded)
	{
		if (dwPlayState == FLAG_PLAYBACK_PAUSED)
		{
			if (m_LastState != FLAG_PLAYBACK_PAUSED)
			{
				//User paused the playback. Compute remaining timeout. 
				KillTimer(1); 
				m_CurrentItem.m_TimeoutMS -= (GetTickCount() - m_TimerStarted);
			}
		}
		else if (dwPlayState == FLAG_PLAYBACK_RUNNING)
		{
				if (m_LastState == FLAG_PLAYBACK_PAUSED)
				{
					if (m_CurrentItem.m_TimeoutMS > 0)
						SetTimer(1, (UINT)m_CurrentItem.m_TimeoutMS); 
				}
		}
	}
	
	m_LastState = dwPlayState;

	//Total duration is 0, this is invalid and we shouldn't 
	//try to locate ads for it; until it becomes > 0
	if (pPos.m_TotalDurationMS == 0)
		return FALSE; 

	for (size_t k = 0; k < m_Ads.size(); k++)
	{
		FAdItem& pItem = m_Ads[k];
		UpdateTimes(pItem, pPos);	//recompute position (if specified in percent)

		//START / END ads can only be shown ONCE
		if (pItem.m_dwFlags & AD_FLAG_AT_START)
		{	
			if ((dwPlayState == FLAG_PLAYBACK_START || 
			   (dwPlayState == FLAG_PLAYBACK_RUNNING && pPos.m_MSPlayed < 5000)) && 
				pItem.m_LastDisplay == 0)
			{
				if (LoadAd(pItem))
					return TRUE; 
			}
			continue; 
		}

		if (pItem.m_dwFlags & AD_FLAG_AT_END)
		{
			if (dwPlayState == FLAG_PLAYBACK_ENDED && pItem.m_LastDisplay == 0)
				if (LoadAd(pItem))
					return TRUE; 

			continue; 
		}

		if (pItem.m_MSPlayed > 0 && pPos.m_MSPlayed > pItem.m_MSPlayed && pItem.m_LastDisplay == 0)
		{
			if (LoadAd(pItem))
				return TRUE; 
		}
		else if (pItem.m_MSPlayed == 0)
		{
			duration_type TimeEndMS = pItem.m_TimeEndMS;

			if (TimeEndMS == 0)
				TimeEndMS = pItem.m_TimeMS + pItem.m_TimeoutMS;

			if (pItem.m_TimeMS <= pPos.m_PlaybackPosMS)
			{
				if (TimeEndMS >= pPos.m_PlaybackPosMS)
				{
					//Good candidate
					if (LoadAd(pItem))
						return TRUE; 
				}
			}
		}
	}
	return FALSE; 
}

BOOL FAdWindow::LoadAd(FAdItem& pItem)
{
	//If this is the currently displayed ad, see if 
	//it has been hidden long enough

	if (m_CurrentItem.m_dwAdID == pItem.m_dwAdID && m_bAdInProgress)
		return FALSE; 

	if (m_CurrentItem.m_dwAdID == pItem.m_dwAdID && !m_bAdInProgress)
	{
		DWORD dwTimeout = MIN_AD_REDISPLAY_TIMEOUT; 
		if (IsAdShowing() || GetTickCount() - m_CurrentItem.m_LastDisplay < dwTimeout)
			return FALSE; 
	}


	m_CurrentItem = pItem; 
	m_bAdInProgress = TRUE; 
	m_bAdLoaded = FALSE; 
	
	if (m_hWnd == NULL && !CreateWnd())
		return FALSE; 
	
	return SUCCEEDED(Navigate(m_CurrentItem.m_URL));
}

LRESULT FAdWindow::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
{
	if (wParam == 1)
	{
		KillTimer(1); 
		
		if (m_CurrentItem.m_TimeEndMS == 0)
			OnAdFinished();	//No ending time, timeout expired.
		else
		{
			//If the ad has a ending time, then we don't need to hide it - updatePosition will do this for us.
			//We must, however, reset the timeout value so that it's closed when the position reaches 
			//the desired millisecond
			m_CurrentItem.m_TimeoutMS = 0; 
		}
	}
	return 0; 
}

void FAdWindow::OnAdFinished()
{
	m_bAdLoaded = FALSE; 
	m_bAdInProgress = FALSE; 
	m_InitData.m_pNotify->OnAdFinished(GetId()); 
}

void FAdWindow::OnAdLoaded()
{
	m_bAdLoaded = TRUE; 
	m_bAdInProgress = FALSE; 

	//Update timeout values, etc.

	long dwTimeout = StrToLong(CallScriptStr("ltv_getAdDuration", ""));
	
	if (dwTimeout <= 0)
		dwTimeout = (long)m_CurrentItem.m_TimeoutMS;

	if (dwTimeout > 0)
		m_CurrentItem.m_dwFlags &= ~AD_FLAG_PERSISTENT; 
	else //timeout is 0
	{
		if (m_CurrentItem.m_TimeEndMS == 0)	//has end position ?
			dwTimeout = DEFAULT_AD_DURATION; 
	}

	if (m_CurrentItem.m_dwFlags & AD_FLAG_PERSISTENT)
	{
		//Persistent, no timer required.
	}
	else
	{
		m_CurrentItem.m_TimeoutMS = dwTimeout; 
	}
}

void FAdWindow::OnDocumentComplete(DWORD dwID, BOOL bMainFrame)
{
	m_bAdLoaded = TRUE; 

	if (!m_bAdInProgress)	
		return ; //Ad canceled.

	long lHeight = StrToLong(CallScriptStr("ltv_getAdHeight", ""));
	long lWidth = StrToLong(CallScriptStr("ltv_getAdWidth", ""));

	FString Str;
	Str.Format("%u", GetId());
	CallJScript("ltv_setWindowID", Str);
	Str.Format("%u", m_InitData.m_AdType);
	CallJScript("ltv_setAdType", Str); 

	if (lHeight > 0)
		m_CurrentItem.m_Height = lHeight; 

	if (lWidth > 0)
		m_CurrentItem.m_Width = lWidth; 

	if (m_CurrentItem.m_Height == 0 && (m_CurrentItem.m_Type == AdHBottom || m_CurrentItem.m_Type == AdHTop))
	{
		m_CurrentItem.m_Height = 90; 
	}

	if (m_CurrentItem.m_Width == 0 && (m_CurrentItem.m_Type == AdVLeft || m_CurrentItem.m_Type == AdVRight))
	{
		m_CurrentItem.m_Width = 90;
	}

	OnAdLoaded();	
	m_InitData.m_pNotify->OnDocumentComplete(dwID, bMainFrame); 
	m_CurrentItem.m_LastDisplay = GetTickCount(); 
	
}

BOOL FAdWindow::OnNavigateError(DWORD dwID, BOOL bMainFrame, const tchar* szUrl)
{
	m_bAdLoaded = FALSE; 
	m_bAdInProgress = FALSE; 
	m_InitData.m_pNotify->OnNavigateError(dwID, bMainFrame, szUrl); 
	return FALSE; 
}

long FAdWindow::GetPreferedHeight()
{
	return m_CurrentItem.m_Height; 
}

long FAdWindow::GetPreferedWidth()
{
	return m_CurrentItem.m_Width; 
}

BOOL FAdWindow::IsAdShowing()
{
	return m_bAdLoaded && IsWindowVisible(); 
}

BOOL FAdWindow::IsAdLoaded()
{
	return m_bAdLoaded; 
}

void FAdWindow::ShowWindow(int nShow)
{
	if (IsWindow())
		::ShowWindow(m_hWnd, nShow); 

	if (nShow == SW_HIDE)
	{
		m_bAdInProgress = FALSE; 
		m_CurrentItem.m_LastDisplay = GetTickCount(); 
		if (IsWindow())
			KillTimer(1); 
		m_CurrentItem.m_TimeoutMS -= (GetTickCount() - m_TimerStarted);
	}
	else
	if (nShow == SW_SHOW)
	{
		if (m_bAdLoaded)
		{
			if (m_CurrentItem.m_TimeoutMS > 0)
			{
				m_TimerStarted = GetTickCount(); 
				SetTimer(1, (UINT)m_CurrentItem.m_TimeoutMS);
			}
		}
	}
}

void FAdWindow::MoveWindow(LPRECT prc)
{
	FWebWindow::MoveWindow(prc, TRUE); 
}

BOOL FAdWindow::SetZPosition()
{
	if (m_CurrentItem.m_dwFlags & AD_FLAG_NORESIZE)
	{
		SetWindowPos(HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		return TRUE; 
	}
	return FALSE; 
}
