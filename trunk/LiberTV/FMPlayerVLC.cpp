#include "stdafx.h"
#include "FMPlayerVLC.h"
#include <math.h>
#include "AppSettings.h"


#define VLC_RT_DIVIDER 10000 

static FMediaPlayerVLC_Ex* pThisPlayer = NULL; 


//9BE31822-FDAD-461B-AD51-BE1D1C159921

DEFINE_GUID(IID_IFVLCControl, 
			0x9BE31822, 0xFDAD, 0x461B, 0xad, 0x51, 0xbe, 0x1d, 0x1c, 0x15, 0x99, 0x21);

extern HRESULT DSHelpCreateInstance( 
									BSTR          bstrLibName,		 
									REFCLSID              rclsid,	//XVID guid
									LPUNKNOWN             pUnkOuter, //NULL
									REFIID                riid,	 //IID_IBaseFilter
									LPVOID*               ppv);		//IFilter

#pragma comment(lib, "libvlc.lib")
HRESULT FMediaPlayerVLC_Ex::Init(HWND hWndParent, IMediaPlayerNotify* pNotify)
{
	USES_CONVERSION;
	m_GoToOffset = 0; 
	FRect r;
	m_bHasSubs = 0; 

	::GetWindowRect(hWndParent, &r);

	m_pNotify = pNotify; 
	m_wndView.Create(hWndParent, r, NULL, WS_DISABLED | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0);


	HRESULT hr; 

	
	hr = m_wndView.QueryHost(&spHost);

	if (SUCCEEDED(hr))
	{
		CLSID clsidVLC; 
		hr = CLSIDFromString(L"{9BE31822-FDAD-461B-AD51-BE1D1C159921}", &clsidVLC);
		hr = DSHelpCreateInstance(L"vlc\\axvlc.dll", clsidVLC, NULL, IID_IUnknown, (void**)&m_pPlugin); 
		if (FAILED(hr))
		{
			//older version had 'axvlcl.dll' 
			hr = DSHelpCreateInstance(L"vlc\\axvlcl.dll", clsidVLC, NULL, IID_IUnknown, (void**)&m_pPlugin);
			if (FAILED(hr))
				return hr; 
		}
		hr = spHost->AttachControl(m_pPlugin, m_wndView); 
		if (SUCCEEDED(hr))
		{
			hr = m_wndView.QueryControl(&m_pVLC);
			if (SUCCEEDED(hr))
			{
				hr = m_pVLC->get_playlist(&m_pPlaylist);
				if (SUCCEEDED(hr))
				{
					hr = m_pVLC->get_input(&m_pInput); 
				}
			}
		}
	}

	m_State = vlcStopped; 
	return hr; 
}

VOID CALLBACK TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (pThisPlayer != NULL)
	{
		pThisPlayer->OnAxWndTimer((UINT)idEvent); 
	}
}

FString FindSubFile(const char* pVideoFileName)
{
	const char* pszExts[] = {".srt", ".sub", ".txt", NULL};

	size_t k = 0; 
	for (;;)
	{
		char SubPath[MAX_PATH]; 
		const char* pszExt = pszExts[k++];
		if (NULL == pszExt)
			break; 

		StringCbCopy(SubPath, MAX_PATH, pVideoFileName); 
		SubPath[strlen(pVideoFileName)] = 0; 
		PathRenameExtension(SubPath, pszExt);
		if (PathFileExists(SubPath))
			return SubPath; 
	}
	return "";
}

HRESULT FMediaPlayerVLC_Ex::AddPlaylistItem(const tchar* pFileName, IMediaOptions* pOptions)
{
	
	if (!m_pPlaylist)
		return E_FAIL; 
	
	USES_CONVERSION;
	long pos = 0; 
	LPSAFEARRAY pSA = SafeArrayCreateVector(VT_VARIANT, 0, 1); 


	if (pOptions->m_Vout == "")
	{
		//If GetWindowsVersion() == WindowsVista => "direct3d" 
		//else "directx";
		pOptions->m_Vout = "direct3d";
	}

	FString StrVout; 
	StrVout.Format(":vout=%s;", pOptions->m_Vout); 

	FString Options = StrVout;
	
	m_bHasSubs = TRUE; 
	if (pOptions->m_SubPath.GetLength() > 0)
	{
		Options.Append(";:sub-file: ");
		Options.Append(pOptions->m_SubPath);
	}

	Options.Append(";:sub-autodetect-file;:sub-autodetect-fuzzy:3;");
	if (pOptions->rtOffset > 0)
	{
		FString StartTime; 
		StartTime.Format(";:start-time:%d;", pOptions->rtOffset / 1000); 
		Options.Append(StartTime);
	}

	_variant_t var(_bstr_t(T2OLE(Options))); 
	SafeArrayPutElement(pSA, &pos, (void FAR *)&var); 
	_variant_t v; 
	v.parray = pSA; 
	v.vt = VT_ARRAY;
	
	CComVariant vtName = L"LTV_Video";
	BSTR bStrName = SysAllocString(T2OLE(pFileName)); 
	long lId = 0; 
	m_pPlaylist->get_itemCount(&lId); 
	return m_pPlaylist->add(bStrName, vtName, v, &lId); 
}

HRESULT FMediaPlayerVLC_Ex::FindInPlaylist(const tchar* pszPath, long* lpIndex)
{
	return E_FAIL; 
}

HRESULT FMediaPlayerVLC_Ex::LoadMedia(const tchar* pFileName, IMediaOptions* pOptions)
{

	USES_CONVERSION; 
	HRESULT hr = E_FAIL; 
	Stop(); 
	if (!m_pVLC)
		return E_FAIL; 

	m_pVLC->put_AutoPlay(VARIANT_FALSE); 
	m_pVLC->put_AutoLoop(VARIANT_FALSE); 
	if (m_pPlaylist)
	{
		CComVariant v; 
		FString StrFileName; 
		BSTR bStrName = SysAllocString(T2OLE(pFileName)); 
		hr = m_pPlaylist->clear();
		m_State = vlcLoading;
		hr = AddPlaylistItem(pFileName, pOptions); 
		if (SUCCEEDED(hr))
		{
			//m_GoToOffset = pOptions->rtOffset;
			m_GoToOffset = 0; 
			pThisPlayer = this; 
			m_wndView.SetTimer(0, 100, TimerProc); 
		}
	}
	_DBGAlert("Media loaded: %s\n", pFileName); 
	return hr; 
}

void FMediaPlayerVLC_Ex::OnAxWndTimer(UINT timerID)
{
	HRESULT hr = E_FAIL; 
	if (timerID == 0)
	{
		if (m_pInput)
		{
			long lState = 0; 
			hr = m_pInput->get_state(&lState);
			VLCStates newState = (VLCStates)lState; 
			VLCStates prevState = m_State; 

			if (prevState != newState)
			{
				switch(newState)
				{
				case vlcStopped:
					if (prevState == vlcPlaying)
						m_pNotify->NotifyPlayer(PluginPlaybackFinished, S_OK); 
					else if (prevState == vlcLoading)
						m_pNotify->NotifyPlayer(PluginMediaReady, E_FAIL); 
					break; 
				case vlcLoading:
					break; 
				case vlcBuffering:
					m_pNotify->NotifyPlayer(PluginMediaBuffering, S_OK); 
					break; 
				case vlcPlaying:
					if (m_GoToOffset > 0)
					{
						SeekPosition(m_GoToOffset); 
						m_GoToOffset = 0; 
					}
					_DBGAlert("Media ready\n");
					m_pNotify->NotifyPlayer(PluginMediaReady, S_OK); 
					break; 
				case vlcPaused:
					break; 
				}
			}
			if (newState < 5)
				m_State = newState; 
		}
		
	}
}

HRESULT FMediaPlayerVLC_Ex::Play()
{
	HRESULT hr = E_FAIL; 
	if (m_pPlaylist)
	{
		hr = m_pPlaylist->playItem(0); 
	}
	return hr; 
}

HRESULT FMediaPlayerVLC_Ex::Pause()
{

	HRESULT hr = E_FAIL; 
	if (m_pPlaylist)
	{
		//TODO: check state; if playing - pause; if paused, do nothing
		hr = m_pPlaylist->togglePause();
	}
	return hr; 
}

HRESULT FMediaPlayerVLC_Ex::Stop()
{
	HRESULT hr = E_FAIL; 
	m_State = vlcStopped; 
	m_GoToOffset = 0; 
	if (m_pPlaylist)
	{
		hr = m_pPlaylist->stop();
		m_pPlaylist->clear(); 
	}
	m_wndView.KillTimer(0); 
	return hr; 
}

HRESULT FMediaPlayerVLC_Ex::SeekPosition(MEDIA_TIME rtCurrent)
{
	HRESULT hr = E_FAIL; 

	if (m_pInput)
	{
		double dblTime = (double)rtCurrent / VLC_RT_DIVIDER;
		hr = m_pInput->put_time(dblTime); 
	}
	return hr; 
}

MEDIA_TIME FMediaPlayerVLC_Ex::GetPosition()
{
	MEDIA_TIME rtTime = 0; 
	

	if (m_pInput)
	{
		double dblTime = 0.0; 
		HRESULT hr = m_pInput->get_time(&dblTime); 
		if (SUCCEEDED(hr))
		{
			rtTime = (MEDIA_TIME)dblTime * VLC_RT_DIVIDER; 
		}
	}
	return rtTime; 
}

MEDIA_TIME FMediaPlayerVLC_Ex::GetDuration()
{
	MEDIA_TIME rtDur = 0; 

	if (m_pInput)
	{
		double dblLen = 0; 
		HRESULT hr = m_pInput->get_length(&dblLen); 
		if (SUCCEEDED(hr))
		{
			rtDur = (MEDIA_TIME)dblLen * VLC_RT_DIVIDER; 
		}
	}

	return rtDur; 
}

MEDIA_TIME FMediaPlayerVLC_Ex::GetAvailDuration()
{
	return GetDuration(); //TODO: Fix it somehow ?
}

HRESULT FMediaPlayerVLC_Ex::SetVideoRect(const RECT &rcV)
{
	if (m_wndView.MoveWindow(&rcV, TRUE))
		return S_OK; 
	return E_FAIL; 
}

HRESULT FMediaPlayerVLC_Ex::ShowVideoWnd(int nShow)
{
	HRESULT hr = E_FAIL; 
	if (m_pVLC)
	{
		if (nShow == SW_SHOW)
		{
			hr = m_pVLC->put_Visible(VARIANT_TRUE); 
		}
		else
		{
			hr = m_pVLC->put_Visible(VARIANT_FALSE); 
		}
	}
	if (!m_wndView.ShowWindow(nShow))
		hr = E_FAIL; 
	return hr;
}

long FMediaPlayerVLC_Ex::GetVolume()
{
	CComPtr<IVLCAudio> pAudio;
	long lVolume = 0; 

	HRESULT hr = E_FAIL; 
	if (m_pVLC)
	{
		hr = m_pVLC->get_audio(&pAudio); 
		if (SUCCEEDED(hr))
			pAudio->get_volume(&lVolume); 
	}
	return lVolume; 
}

HRESULT FMediaPlayerVLC_Ex::SetVolume(long lVol)
{
	CComPtr<IVLCAudio> pAudio;
	HRESULT hr = E_FAIL; 

	if (m_pVLC)
	{
		hr = m_pVLC->get_audio(&pAudio);
		if (SUCCEEDED(hr))
			hr = pAudio->put_volume(lVol); 
	}
	return hr; 
}

HRESULT FMediaPlayerVLC_Ex::SetFullScreen(BOOL bFullScreen)
{
	HRESULT hr = E_FAIL; 
	CComPtr<IVLCVideo> pVideo;
	if (m_pVLC)
	{
		hr = m_pVLC->get_video(&pVideo);
		if (SUCCEEDED(hr))
			hr = pVideo->put_fullscreen(bFullScreen ? VARIANT_TRUE : VARIANT_FALSE); 
	}
	return hr; 
}

BOOL FMediaPlayerVLC_Ex::IsFullScreen()
{
	HRESULT hr = E_FAIL; 
	CComPtr<IVLCVideo> pVideo;
	BOOL bFullScreen = FALSE; 
	if (m_pVLC)
	{
		hr = m_pVLC->get_video(&pVideo);
		if (SUCCEEDED(hr))
		{
			VARIANT_BOOL bFull; 
			hr = pVideo->get_fullscreen(&bFull); 
			if (SUCCEEDED(hr) && bFull)
				bFullScreen = TRUE; 
		}
	}
	return bFullScreen; 
}

HRESULT FMediaPlayerVLC_Ex::GetBufferingProgress(long* lProgress)
{
	return E_NOTIMPL;
}

double FMediaPlayerVLC_Ex::GetFrameRate()
{
	HRESULT hr = E_FAIL; 
	double dblRate = 0.0; 
	if (m_pInput)
	{
		hr = m_pInput->get_fps(&dblRate); 
	}
	return dblRate; 
}

void FMediaPlayerVLC_Ex::SetAspectRatio(int cx, int cy)
{
	
}



VideoState FMediaPlayerVLC_Ex::GetVideoState()
{
	VideoState State = vssUndefined; 
	HRESULT hr = E_FAIL;
	if (m_pInput)
	{
		
		long lState = 0; 
		hr = m_pInput->get_state(&lState); 
		if (SUCCEEDED(hr))
		{
			switch(lState)
			{
			case vlcStopped:
				State = vssStopped;
				break; 
			case vlcLoading:
				State = vssWaiting; 
				break; 
			case vlcBuffering:
				State = vssBuffering;
				break; 
			case vlcPlaying:
				State = vssPlaying; 
				break; 
			case vlcPaused:
				State = vssPaused; 
				break; 
			default:
				State = vssBuffering;
				break; 
		
			}
		}
	}
	return State; 
}

void FMediaPlayerVLC_Ex::Clear()
{
	_DBGAlert("Clear()\n"); 

	m_wndView.KillTimer(0); 
	pThisPlayer = NULL; 
	Stop(); 

	if (m_pPlaylist)
		m_pPlaylist.Release();

	if (m_pInput)
		m_pInput.Release();

	if (m_pVLC)
		m_pVLC.Release();

	if (spHost)
		spHost.Release();

	if (m_pPlugin)
		m_pPlugin.Release(); 

	m_wndView.DestroyWindow();
}

DWORD FMediaPlayerVLC_Ex::GetMediaFlags()
{
	if (m_bHasSubs)
		return MEDIA_FLAG_HAS_OWN_SUBTITLES;
	return 0; 
}

void FMediaPlayerVLC_Ex::CycleSubtitles()
{
	SendVLCEvent("key-subtitle-track", "key-pressed");
}

HRESULT FMediaPlayerVLC_Ex::SendVLCEvent(const char* pszKeyName, const char* pszKeyVal)
{
	CComVariant vtVar; 
	CComQIPtr<IVLCControl> pCtrl = m_pVLC; 
	HRESULT hr = E_FAIL; 
	if (pCtrl)
	{
		CComBSTR bstrName = pszKeyName;
		CComBSTR bstrKeyPressed = pszKeyVal;

		hr = pCtrl->getVariable(bstrName, &vtVar);
		if (SUCCEEDED(hr))
		{
			hr = pCtrl->setVariable(bstrKeyPressed, vtVar); 
		}
	}
	return hr; 
}
HRESULT FMediaPlayerVLC_Ex::SendCommand(const char* pszCmdName, void* pCmdData)
{
	HRESULT hr = S_OK; 
	if (stricmp(pszCmdName, "GetPluginName") == 0)
	{
		if (pCmdData != NULL)
			StringCbCopy((char*)pCmdData, 256, "VLC");
	}
	else
	if (stricmp(pszCmdName, "cycle subtitles") == 0)
		hr = SendVLCEvent("key-subtitle-track", "key-pressed");
	else
	if (stricmp(pszCmdName, "cycle aspect ratio") == 0)
		hr = SendVLCEvent("key-aspect-ratio", "key-pressed");
	return hr; 
}