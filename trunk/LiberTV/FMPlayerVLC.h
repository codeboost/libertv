#ifndef FMPlayerVLCdotH
#define FMPlayerVLCdotH

#include "FMPlayerDef.h"
#include "axvlc_h.h"
#include "vlccommon.h"

class FMediaPlayerVLC_Ex : public IFMediaPlayer
{
	CComPtr<IVLCControl2>			m_pVLC;
	CComPtr<IVLCPlaylist>			m_pPlaylist; 
	CComPtr<IVLCInput>				m_pInput; 
	CComPtr<IUnknown>				m_pPlugin; 
	IMediaPlayerNotify*				m_pNotify; 
	CComPtr<IAxWinHostWindow>		spHost;
	CAxWindow						m_wndView;		// ActiveX host window class.
	FString							m_MediaType;
	VLCStates						m_State;	
	MEDIA_TIME						m_GoToOffset;
	BOOL							m_bHasSubs;		//Found subtitles ?

public:
	HRESULT 			Init(HWND hWndParent, IMediaPlayerNotify* pNotify);
	HRESULT				Play();							//Runs the graph
	HRESULT				Stop();							//Stops and destroys the graph
	HRESULT				Pause();						//Pauses the graph
	HRESULT				LoadMedia(const tchar* pFileName, IMediaOptions* pOptions);
	VideoState			GetVideoState();			
	HRESULT				SeekPosition(MEDIA_TIME rtCurrent);
	MEDIA_TIME			GetPosition();
	MEDIA_TIME			GetDuration(); 
	MEDIA_TIME			GetAvailDuration();	

	HRESULT				SetVideoRect(const RECT& rcV);	
	HRESULT				ShowVideoWnd(int nShow); 
	long				GetVolume();			
	HRESULT				SetVolume(long lVol); 
	HRESULT				SetFullScreen(BOOL bFullScreen);
	BOOL				IsFullScreen(); 
	long				GetMediaEvent(){return EC_COMPLETE;}
	BOOL				SupportsMediaType(const tchar* pszMediaType){return TRUE; };
	HRESULT				SetMediaType(const tchar* pszMediaType){m_MediaType = pszMediaType; return S_OK; }
	HRESULT				GetMediaType(tchar* pszMediaType){strcpy(pszMediaType, m_MediaType); return S_OK; }
	HRESULT				GetBufferingProgress(long* lProgress);
	BOOL				IsMediaType(const tchar* pszMediaType){return m_MediaType == pszMediaType;}

	double				GetFrameRate(); 
	void				SetAspectRatio(int cx, int cy);
	void				Clear(); 
	void				OnAxWndTimer(UINT timerID);

	HRESULT				AddPlaylistItem(const tchar* pszPath, IMediaOptions* pOptions); 
	HRESULT				FindInPlaylist(const tchar* pszPath, long* lpIndex); 
	DWORD				GetMediaFlags();
	void				CycleSubtitles();
	HRESULT				SendVLCEvent(const char* pszKeyname, const char* pszKeyVal);
	HRESULT				SendCommand(const char* pszCmdName, void* pCmdData);

};


#endif