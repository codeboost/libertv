#ifndef __FMPLAYERHTML_H__
#define __FMPLAYERHTML_H__


#include <dshow.h>
#include <tchar.h>
#include "FMPlayerDef.h"
#include "FWebWindow.h"

/*
How plugins work:
Media player calls 
	hr = IMediaPlayer->Init()
	if (hr == E_PENDING)
		wait for pluginInitComplete notification
	else //SUCCEEDED(hr)
	hr = IMediaPlayer->LoadMedia(...)

	if (hr == E_PENDING)
		wait for mediaReady notification
	else //SUCCEEDED(hr)
		Play()
*/


class FMediaPlayerHTML : public FIEWindow, public IFMediaPlayer 
{
	BEGIN_MSG_MAP(FMediaPlayerFLV)
		CHAIN_MSG_MAP(FIEWindow)
	END_MSG_MAP()
public:

	HWND				m_hWndParent; 
	FString				m_MediaType; 
	IMediaPlayerNotify* m_pPlayerNotify; 

	FMediaPlayerHTML()
	{
		m_pPlayerNotify = NULL; 
	}
	virtual ~FMediaPlayerHTML()
	{
		Clear(); 
	}

	void					OnDocumentComplete(DWORD dwID, BOOL bMain);
	BOOL					OnNavigateError(DWORD dwID, BOOL, const tchar*);
	HRESULT 				Init(HWND hWndParent, IMediaPlayerNotify* pNotify); 
	HRESULT 				LoadMedia(const tchar* pFileName, IMediaOptions* pOptions);
	HRESULT 				SetVideoRect(const RECT& rcV);
	HRESULT					ShowVideoWnd(int nShow);
	HRESULT					Stop(); 
	HRESULT					Play(); 
	HRESULT					Pause(); 
	void					Clear(); 
	long					GetMediaEvent();
	VideoState				GetVideoState();

	HRESULT					ShowVideo(int nCmdShow){return E_NOTIMPL; }
	HRESULT					SeekPosition(REFERENCE_TIME rtPos);
	REFERENCE_TIME			GetPosition();		
	WTL::CSize				GetVideoSize() {return WTL::CSize(0, 0);}
	WTL::CRect				GetAspectRatio(long lx,long ly){ return WTL::CRect(0, 0, 0, 0);}

	virtual HRESULT			SetFullScreen(BOOL bFullScreen)	{	return E_NOTIMPL; 	}
	virtual BOOL			IsFullScreen()			{	return FALSE; 	}
	virtual double			GetFrameRate()			{	return 1.0;	}
	virtual void			SetAspectRatio(int cx, int cy)	{}
	virtual long			GetVolume();
	virtual HRESULT			SetVolume(long lVol);
	REFERENCE_TIME			GetDuration();
	REFERENCE_TIME			GetAvailDuration(); 

	HRESULT					SetMediaType(const tchar* pszMediaType){m_MediaType = pszMediaType; m_MediaType.Trim(); return S_OK;}
	HRESULT					GetMediaType(tchar* pszMediaType){strcpy(pszMediaType, m_MediaType); return S_OK;}
	BOOL					IsMediaType(const tchar* pszMediaType){return m_MediaType.CompareNoCase(pszMediaType) == 0;}
	BOOL					SupportsMediaType(const tchar *pszMediaType){return TRUE; }
	HRESULT					GetBufferingProgress(long* lProgress);
	DWORD					GetMediaFlags(){return 0;}
	void					CycleSubtitles(){}
	HRESULT					SendCommand(const char* pszCmdName, void* pCmdData)
	{
		return E_NOTIMPL; 
	}
};

class FMediaPlayerFLV : public FMediaPlayerHTML
{
public:
    HRESULT 				Init(HWND hWndParent, IMediaPlayerNotify* pNotify);
};

class FMediaPlayerWMV : public FMediaPlayerHTML
{
public:
    HRESULT 				Init(HWND hWndParent, IMediaPlayerNotify* pNotify);
};

class FMediaPlayerMOV :  public FMediaPlayerHTML
{
public:
	HRESULT 				Init(HWND hWndParent, IMediaPlayerNotify* pNotify);
	HRESULT 				LoadMedia(const tchar* pFileName, IMediaOptions* pOptions);
};

class FMediaPlayerVLC : public FMediaPlayerHTML
{
	HRESULT					Init(HWND hWndParent, IMediaPlayerNotify* pNotify); 
};

#endif //__FMPLAYERHTML_H__