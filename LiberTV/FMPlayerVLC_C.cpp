#include "stdafx.h"
#include "FMPlayerVLC_C.h"
#include <math.h>

#include "AppSettings.h"




static FMediaPlayerVLC_C* pThisPlayer = NULL; 


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
static libvlc_instance_t*		g_p_libvlc = NULL; 


VOID CALLBACK C_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	if (pThisPlayer != NULL)
	{
		pThisPlayer->OnAxWndTimer((UINT)idEvent); 
	}
}

FMediaPlayerVLC_C::FMediaPlayerVLC_C()
{
	_p_libvlc = NULL; 
}

HRESULT FMediaPlayerVLC_C::Init(HWND hWndParent, IMediaPlayerNotify* pNotify)
{
	

	if (_p_libvlc != NULL)
	{
		libvlc_destroy(_p_libvlc); 
		_p_libvlc = NULL; 
		g_p_libvlc = NULL; 
	}

	if (g_p_libvlc == NULL)
	{
		int   ppsz_argc = 1;
		char *ppsz_argv[32] = { "vlc" };

		FString Pathname = g_AppSettings.AppDir("libertv.exe"); //
		FString PlugPath = g_AppSettings.AppDir("vlc\\plugins");//

		ppsz_argv[0] = Pathname.GetBuffer();

		ppsz_argv[ppsz_argc++] = "--plugin-path"; //
		ppsz_argv[ppsz_argc++] = PlugPath.GetBuffer(); //

		ppsz_argv[ppsz_argc++] = "--no-one-instance";//

		//common settings 
		ppsz_argv[ppsz_argc++] = "-vv";
		ppsz_argv[ppsz_argc++] = "--no-stats";
		//ppsz_argv[ppsz_argc++] = "--no-media-library";
		ppsz_argv[ppsz_argc++] = "--intf";
		ppsz_argv[ppsz_argc++] = "dummy";

		ppsz_argv[ppsz_argc++] = "--fast-mutex";
		ppsz_argv[ppsz_argc++] = "--win9x-cv-method=1";

		g_p_libvlc = libvlc_new(ppsz_argc, ppsz_argv, NULL);
	}

	if (NULL == g_p_libvlc)
		return E_FAIL; 
	
	_p_libvlc = g_p_libvlc; 

	USES_CONVERSION;
	m_GoToOffset = 0; 
	FRect r;
	m_bHasSubs = 0; 

	::GetWindowRect(hWndParent, &r);

	m_pNotify = pNotify; 
	if (!m_wndView.IsWindow())
		m_wndView.Create(hWndParent, r, NULL, WS_DISABLED | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0);

	libvlc_video_set_parent(_p_libvlc,
		reinterpret_cast<libvlc_drawable_t>(m_wndView.m_hWnd), NULL);

	m_State = vlcStopped; 
	
	return S_OK; 
}

HRESULT FMediaPlayerVLC_C::AddPlaylistItem(const tchar* pFileName, IMediaOptions* pOptions)
{

	return E_NOTIMPL;
}

HRESULT FMediaPlayerVLC_C::FindInPlaylist(const tchar* pszPath, long* lpIndex)
{
	return E_FAIL; 
}

HRESULT FMediaPlayerVLC_C::LoadMedia(const tchar* pFileName, IMediaOptions* pOptions)
{

	USES_CONVERSION; 
	HRESULT hr = E_FAIL; 
	Stop(); 

	libvlc_exception_t ex; 

	libvlc_playlist_delete_item(_p_libvlc, 0, NULL);
	libvlc_playlist_clear(_p_libvlc, NULL); 


	const char *options[32] = {0};
	int i_options = 0;

	if (pOptions->m_Vout == "")
		pOptions->m_Vout = "direct3d";

	FString StrVout; 
	StrVout.Format(":vout=%s", pOptions->m_Vout); 
	options[i_options++] = StrVout.GetBuffer();

	FString SubOpt; 
	FString StartTime; 
	if (pOptions->m_SubPath.GetLength() > 0)
	{
		
		SubOpt.Format(":sub-file=%s", pOptions->m_SubPath);
		options[i_options++] = SubOpt.GetBuffer();
	}

	options[i_options++] = ":sub-autodetect-file";
	options[i_options++] = ":sub-autodetect-fuzzy=3";
	if (pOptions->rtOffset > 0)
	{
		StartTime.Format(":start-time=%d", pOptions->rtOffset / VLC_RT_DIVIDER / 1000);
		options[i_options++] = StartTime.GetBuffer();
	}
	

	libvlc_exception_init(&ex);
	int nRes = libvlc_playlist_add_extended(_p_libvlc, pFileName, NULL, i_options, options, &ex);

	if (libvlc_exception_raised(&ex))
	{
		OutputDebugString("EXCEPTION !\n"); 
	}
	pThisPlayer = this; 
	::SetTimer(m_wndView, 0, 100, C_TimerProc); 
	return S_OK; 
}



void FMediaPlayerVLC_C::OnAxWndTimer(UINT timerID)
{
	HRESULT hr = E_FAIL; 
	if (timerID == 0)
	{
		
		//hr = m_pInput->get_state(&lState);

		libvlc_input_t* p_input = libvlc_playlist_get_input(_p_libvlc, NULL); 

		long lState = vlcStopped; 
		if (p_input)
		{
			lState = (long)libvlc_input_get_state(p_input, NULL);
			libvlc_input_free(p_input); 
		}

		

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

HRESULT FMediaPlayerVLC_C::Play()
{
	HRESULT hr = S_OK; 
	libvlc_playlist_play(_p_libvlc, -1, 0, NULL, NULL);
	return hr; 
}

HRESULT FMediaPlayerVLC_C::Pause()
{
	libvlc_playlist_pause(_p_libvlc, NULL); 
	return S_OK; 
}

HRESULT FMediaPlayerVLC_C::Stop()
{
	for (size_t k = 0; k < 5; k++)
		libvlc_playlist_stop(_p_libvlc, NULL);
	
	int n = 0; 
	while (n++ < 10)
	{
		if (0 == libvlc_playlist_isplaying(_p_libvlc, NULL))
			break; 

		SleepEx(50, TRUE); 
	}

	n = 0; 
	while (n++ < 10)
	{
		libvlc_playlist_delete_item(_p_libvlc, 0, NULL);
		libvlc_playlist_clear(_p_libvlc, NULL); 	
		int nCount = libvlc_playlist_items_count(_p_libvlc, NULL); 
		if (nCount == 0)
			break; 
		SleepEx(50, TRUE);
	}

	//int nCount = libvlc_playlist_items_count(_p_libvlc, NULL);
	::KillTimer(m_wndView, 0); 


	return S_OK; 
}

HRESULT FMediaPlayerVLC_C::SeekPosition(MEDIA_TIME rtCurrent)
{
	HRESULT hr = E_FAIL; 

	libvlc_input_t* p_input = libvlc_playlist_get_input(_p_libvlc, NULL); 
	if (p_input)
	{
		vlc_int64_t vlcCur = rtCurrent / VLC_RT_DIVIDER; 
		libvlc_exception_t ex;
		libvlc_exception_init(&ex);
		libvlc_input_set_time(p_input, vlcCur, &ex); 
		libvlc_input_free(p_input); 
		if (!libvlc_exception_raised(&ex))
			hr = S_OK; 
	}
	return hr; 
}

MEDIA_TIME FMediaPlayerVLC_C::GetPosition()
{
	MEDIA_TIME rtTime = 0; 

	libvlc_input_t* p_input = libvlc_playlist_get_input(_p_libvlc, NULL); 
	if (p_input)
	{
		
		libvlc_exception_t ex;
		libvlc_exception_init(&ex);
		vlc_int64_t vlcCur = libvlc_input_get_time(p_input, &ex);
		float ftPos = libvlc_input_get_position(p_input, &ex); 
		libvlc_input_free(p_input); 
		if (!libvlc_exception_raised(&ex))
			rtTime = vlcCur * VLC_RT_DIVIDER; 
	}

	return rtTime; 
}

MEDIA_TIME FMediaPlayerVLC_C::GetDuration()
{
	MEDIA_TIME rtDur = 0; 

	libvlc_input_t* p_input = libvlc_playlist_get_input(_p_libvlc, NULL); 
	if (p_input)
	{

		libvlc_exception_t ex;
		libvlc_exception_init(&ex);
		vlc_int64_t vlcLen = libvlc_input_get_length(p_input, &ex);
		libvlc_input_free(p_input); 
		if (!libvlc_exception_raised(&ex))
			rtDur = vlcLen * VLC_RT_DIVIDER; 
	}

	return rtDur; 
}

MEDIA_TIME FMediaPlayerVLC_C::GetAvailDuration()
{
	return GetDuration(); //TODO: Fix it somehow ?
}

HRESULT FMediaPlayerVLC_C::SetVideoRect(const RECT &rcV)
{
	if (m_wndView.MoveWindow(&rcV, TRUE))
		return S_OK; 

	libvlc_video_set_size(_p_libvlc,
		rcV.right-rcV.left,
		rcV.bottom-rcV.top,
		NULL );

	return E_FAIL; 
}

HRESULT FMediaPlayerVLC_C::ShowVideoWnd(int nShow)
{
	HRESULT hr = E_FAIL; 
	if (!m_wndView.ShowWindow(nShow))
		hr = E_FAIL; 
	return hr;
}

long FMediaPlayerVLC_C::GetVolume()
{
	long lVolume = libvlc_audio_get_volume(_p_libvlc, NULL); 
	return lVolume; 
}

HRESULT FMediaPlayerVLC_C::SetVolume(long lVol)
{
	HRESULT hr = S_OK; 
	libvlc_audio_set_volume(_p_libvlc, lVol, NULL); 
	return hr; 
}

HRESULT FMediaPlayerVLC_C::SetFullScreen(BOOL bFullScreen)
{
	HRESULT hr = E_FAIL; 
	return hr; 
}

BOOL FMediaPlayerVLC_C::IsFullScreen()
{
	HRESULT hr = E_FAIL; 
	return FALSE; 
}

HRESULT FMediaPlayerVLC_C::GetBufferingProgress(long* lProgress)
{
	return E_NOTIMPL;
}

double FMediaPlayerVLC_C::GetFrameRate()
{
	HRESULT hr = E_FAIL; 
	double dblRate = 0.0; 
	return dblRate; 
}

void FMediaPlayerVLC_C::SetAspectRatio(int cx, int cy)
{

}

long FMediaPlayerVLC_C::vlcGetState()
{
	long state = 0; 
	libvlc_exception_t ex;
	libvlc_exception_init(&ex);

	libvlc_input_t *p_input = libvlc_playlist_get_input(_p_libvlc, &ex);
	if( ! libvlc_exception_raised(&ex) )
	{
		state = libvlc_input_get_state(p_input, &ex);
		libvlc_input_free(p_input);
		if( libvlc_exception_raised(&ex) )
		{
			state = 0; 
		}
	}
	libvlc_exception_clear(&ex);
	return state; 
}


VideoState FMediaPlayerVLC_C::GetVideoState()
{
	VideoState State = vssUndefined; 
	HRESULT hr = E_FAIL;

	// don't fail, just return the idle state
	long lState = vlcGetState(); 
	//hr = m_pInput->get_state(&lState); 
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
	return State; 
}

void FMediaPlayerVLC_C::Clear()
{
	Stop(); 
	m_wndView.DestroyWindow();

	if (_p_libvlc != NULL)
	{
		libvlc_destroy(_p_libvlc); 
		_p_libvlc = NULL; 
		g_p_libvlc = NULL; 
	}
}

DWORD FMediaPlayerVLC_C::GetMediaFlags()
{
	if (m_bHasSubs)
		return MEDIA_FLAG_HAS_OWN_SUBTITLES;
	return 0; 
}

void FMediaPlayerVLC_C::CycleSubtitles()
{
	SendVLCEvent("key-subtitle-track", "key-pressed");
}

HRESULT FMediaPlayerVLC_C::SendVLCEvent(const char* pszKeyName, const char* pszKeyVal)
{
	HRESULT hr = S_OK; 
	vlc_value_t evt; 
	
	int id = libvlc_get_vlc_id(_p_libvlc); 

	VLC_VariableGet(id, pszKeyName, &evt); 

	VLC_VariableSet(id, pszKeyVal, evt); 
	return hr; 
}
HRESULT FMediaPlayerVLC_C::SendCommand(const char* pszCmdName, void* pCmdData)
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