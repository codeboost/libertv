#include "stdafx.h"
#include "FMPlayerWM.h"
#include "math.h"
#include "AppSettings.h"

#define RT_DIVIDER 10000000 

BOOL SucceededLog(HRESULT hr, const tchar* Context, int ErrCode = 0)
{
	return SUCCEEDED(hr); 

	if (FAILED(hr))
	{
		_DBGAlert("Call failed: 0x%x; Context=%s; ErrorCode = %d\n", hr, Context, ErrCode);
		return FALSE; 
	}
	else
	{
		_DBGAlert("Call succeeded: %s\n", Context);
	}
	return TRUE; 
}

HRESULT FMPlayerWM::Init(HWND hWndParent, IMediaPlayerNotify* pNotify)
{
	FRect r; 
	::GetClientRect(hWndParent, &r);
	CComPtr<IConnectionPointContainer>  spConnectionContainer;
	CComPtr<IWMPEvents>                 spEventListener;

	m_pNotify = pNotify; 
	m_wndView.Create(hWndParent, r, NULL, WS_CHILD  | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0);

	if (!m_wndView.IsWindow())
	{
		SucceededLog(E_FAIL, "CreateWindow", GetLastError()); 
	}

	HRESULT hr; 
	hr = m_wndView.QueryHost(&spHost);

	SucceededLog(hr, "QueryHost"); 

	hr = spHost->CreateControl(CComBSTR(_T("{6BF52A52-394A-11d3-B153-00C04F79FAA6}")), m_wndView, 0);
	SucceededLog(hr, "CreateControl"); 
	hr = m_wndView.QueryControl(&m_spWMPPlayer);

	SucceededLog(hr, "QueryControl"); 
	// start listening to events

	hr = FWMPEventHandler::CreateInstance(&m_pEventProxy);

	SucceededLog(hr, "EventHandler::CreateInstance"); 

	spEventListener = m_pEventProxy;
	m_pEventProxy->m_pParentObj = this; 

	hr = m_spWMPPlayer->QueryInterface(&spConnectionContainer);

	SucceededLog(hr, "QueryInterface - spConnectionContainer"); 
	
	// See if OCX supports the IWMPEvents interface
	hr = spConnectionContainer->FindConnectionPoint(__uuidof(IWMPEvents), &m_spConnectionPoint);
	if (!SucceededLog(hr, "spConnectionContainer->FindConnectionPoint"))
	{
		// If not, try the _WMPOCXEvents interface, which will use IDispatch
		hr = spConnectionContainer->FindConnectionPoint(__uuidof(_WMPOCXEvents), &m_spConnectionPoint);
		SucceededLog(hr, "spConnectionContainer->FindConnectionPoint2");
	}

	hr = m_spConnectionPoint->Advise(spEventListener, &m_dwAdviseCookie);

	SucceededLog(hr, "m_spConnectionPoint->Advise()");

	m_spWMPControls = m_spWMPPlayer; 
	m_spWMPControls3 = m_spWMPPlayer;
	m_spSettings	 = m_spWMPPlayer; 

	SucceededLog(m_spWMPPlayer->put_uiMode(L"none"), "put_uiMode");
	SucceededLog(m_spWMPPlayer->put_enableContextMenu(VARIANT_FALSE), "put_enableContextMenu");
	SucceededLog(m_spWMPPlayer->put_enabled(VARIANT_FALSE), "put_enabled"); 
	m_CurrentState = wmppsStopped;
	return hr; 
}

void FMPlayerWM::Clear()
{
	if (m_spWMPControls)
		m_spWMPControls->stop();

	ShowVideoWnd(SW_HIDE); 

	m_spSettings.Release();
	m_spWMPControls3.Release();
	m_spWMPControls.Release();
	m_spWMPPlayer.Release(); 
	m_CurrentState = wmppsUndefined;
	m_pEventProxy->m_pParentObj = NULL; 
	spHost.Release();
	m_wndView.DestroyWindow();
}

HRESULT FMPlayerWM::Play()
{
	if (!m_spWMPControls)
		return E_FAIL; 
	return m_spWMPControls->play(); 
}

HRESULT FMPlayerWM::Stop()
{
	if (m_spWMPControls)
		return m_spWMPControls->stop(); 
	return E_FAIL;
}

HRESULT FMPlayerWM::Pause()
{
	if (!m_spWMPControls)
		return E_FAIL; 
	return m_spWMPControls->pause();
}

MEDIA_TIME FMPlayerWM::GetPosition()
{
	double dblPos; 
	if (m_spWMPControls && SUCCEEDED(m_spWMPControls->get_currentPosition(&dblPos)))
	{
		return (MEDIA_TIME)(dblPos * RT_DIVIDER);
	}
	return 0; 
}

MEDIA_TIME FMPlayerWM::GetDuration()
{
	if (!m_spWMPPlayer)
		return 0; 

	CComPtr<IWMPMedia> CurrentMedia; 
	HRESULT hr = m_spWMPPlayer->get_currentMedia(&CurrentMedia);
	if (SUCCEEDED(hr) && CurrentMedia != NULL)
	{
		double dblDuration;
		hr = CurrentMedia->get_duration(&dblDuration); 
		if (SUCCEEDED(hr))
		{
			return (MEDIA_TIME)(dblDuration * RT_DIVIDER); 
		}
	}
	return 0; 
}

HRESULT FMPlayerWM::SeekPosition(MEDIA_TIME rtCurrent)
{
	if (!m_spWMPControls)
		return E_FAIL; 

	double dblPos = (double)(rtCurrent * 1.0) / RT_DIVIDER; 
	return m_spWMPControls->put_currentPosition(dblPos); 
}

VideoState FMPlayerWM::GetVideoState()
{
	WMPPlayState pState; 
	if (m_spWMPPlayer && SUCCEEDED(m_spWMPPlayer->get_playState(&pState)))
	{
		return (VideoState)pState;
	}

	return vssStopped; 
}

MEDIA_TIME FMPlayerWM::GetAvailDuration()
{
	CComQIPtr<IWMPNetwork> pNet = m_spWMPPlayer;
	MEDIA_TIME Duration = 0;
	if (pNet)
	{
		Duration = GetDuration(); 
		long lDownloadProgress = 0; 
		if (SUCCEEDED(pNet->get_downloadProgress(&lDownloadProgress)))
		{
			if (lDownloadProgress == 0)
			{
				if (GetVideoState() == vssBuffering)
					Duration = 0;
			}
			else
			{
				double fPercent = (double)lDownloadProgress / 100; 
				Duration = (MEDIA_TIME)(Duration * fPercent); 
			}
		}
		
	}
	return Duration;
}

HRESULT FMPlayerWM::GetBufferingProgress(long* lProgress)
{
	CComQIPtr<IWMPNetwork> pNet = m_spWMPPlayer;
	if (pNet)
	{
		return pNet->get_bufferingProgress(lProgress);
	}
	return E_FAIL; 
}

HRESULT FMPlayerWM::SetVideoRect(const RECT& rcV)
{
	_DBGAlert("SetVideoRect: (%d, %d) - (%d, %d)\n", rcV.left, rcV.top, rcV.right, rcV.bottom);
	m_CurrentRect = rcV; 
	if (m_wndView.MoveWindow(&rcV, TRUE))
	{
		m_wndView.BringWindowToTop();
		return S_OK; 
	}
	_DBGAlert("SetVideoRect failed!\n");
	return E_FAIL; 
}

HRESULT FMPlayerWM::ShowVideoWnd(int nShow)
{
	if (m_wndView.ShowWindow(nShow))
	{
		return SetVideoRect(m_CurrentRect); 
	}
	return E_FAIL; 
}

HRESULT FMPlayerWM::SetVolume(long lVol)
{
	if (!m_spSettings)
		return E_FAIL; 
	return m_spSettings->put_volume(lVol); 
}

long FMPlayerWM::GetVolume()
{
	LONG lVolPercent = 0; 
	if (m_spSettings && SUCCEEDED(m_spSettings->get_volume(&lVolPercent)))
	{
		return lVolPercent; 
	}
	return 0; 
}

HRESULT FMPlayerWM::SetFullScreen(BOOL bFullScreen)
{
	if (!m_spWMPPlayer)
		return E_FAIL; 

	return m_spWMPPlayer->put_fullScreen(bFullScreen ? VARIANT_TRUE : VARIANT_FALSE);
}

BOOL FMPlayerWM::IsFullScreen()
{
	if (!m_spWMPPlayer)
		return FALSE; 

	VARIANT_BOOL vbFull = VARIANT_FALSE; 
	m_spWMPPlayer->get_fullScreen(&vbFull); 
	return vbFull == VARIANT_TRUE ? TRUE : FALSE; 
}

double FMPlayerWM::GetFrameRate()
{
	return 25.0;
}

void FMPlayerWM::SetAspectRatio(int cx, int cy)
{

}

HRESULT FMPlayerWM::LoadMedia(const tchar* pFileName, IMediaOptions* pOptions)
{
	USES_CONVERSION;
	if (!m_spWMPPlayer)
		return E_FAIL; 

	HRESULT hr = m_spWMPPlayer->put_URL(T2OLE(pFileName)); 
	if (SUCCEEDED(hr))
	{
		CComQIPtr<IWMPPlayer2> pPlayer2 = m_spWMPPlayer;
		if (pPlayer2)
		{
			pPlayer2->put_stretchToFit(VARIANT_TRUE); 
			pPlayer2->put_windowlessVideo(VARIANT_TRUE);
		}
		if (pOptions->rtOffset > 0)
			return SeekPosition(pOptions->rtOffset);
		else
			return S_OK; 
	}
	return hr; 
}

void STDMETHODCALLTYPE FMPlayerWM::PlayStateChange(long NewState)
{
	
}

void STDMETHODCALLTYPE FMPlayerWM::StatusChange()
{
	WMPPlayState pState; 
	if (m_spWMPPlayer && SUCCEEDED(m_spWMPPlayer->get_playState(&pState)))
	{
		_DBGAlert("StatusChange: Old = %d; New = %d\n", m_CurrentState, pState); 
		switch(pState)
		{
		case wmppsReady:
			{
				if (m_CurrentState != PluginMediaReady)
					m_pNotify->NotifyPlayer(PluginMediaReady, S_OK); 
			}
			break; 
		case wmppsMediaEnded:
			{
				m_pNotify->NotifyPlayer(PluginPlaybackFinished, S_OK); 
			}
			break; 
		case wmppsPlaying:
			{
				m_pNotify->NotifyPlayer(PluginMediaPlaying, S_OK);
			}
			break; 
		case wmppsBuffering:
			{
				if (m_CurrentState != wmppsBuffering)
					m_pNotify->NotifyPlayer(PluginMediaBuffering, S_OK); 
			}
		}
		m_CurrentState = pState; 
	}
}


