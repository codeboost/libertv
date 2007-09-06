#ifndef __FMPLAYERDSHOW_H__
#define __FMPLAYERDSHOW_H__
#include <dshow.h>
#include <tchar.h>
#include "IFVideo.h"
#include "FMPlayerDef.h"


class FMPlayerDShow	: public IFMediaPlayer
{
public:
	IMediaPlayerNotify*			m_pNotify; 
	FString						m_MediaType; 
	CAtlString					m_FileName; 
	MEDIA_TIME				m_Offset; 

	CComPtr<IFilterGraph>		m_FilterGraph;
	CComQIPtr<IMediaControl>	m_MediaControl;
	CComQIPtr<IMediaEventEx>	m_MediaEvent; 
	CComQIPtr<IVideoWindow>		m_VideoWnd;
	CComQIPtr<IBasicVideo>		m_pBV; 

	//////////////////////////////////////////////////////////////////////////
	CComPtr<IBaseFilter>        m_FileSource; 
	CComPtr<IBaseFilter>		m_AviSplitter; 
	CComQIPtr<IBaseFilter>		m_VideoRenderer;
	CComPtr<IBaseFilter>		m_VMR; 

	//Preloaded filters
	CComPtr<IBaseFilter>        m_DSoundFilter; 
	CComQIPtr<IBaseFilter>		m_xVid; 
	CComQIPtr<IBaseFilter>		m_AC3Filter; 
	CComQIPtr<IBaseFilter>		m_MSAud;


	HWND						m_hWndParent;
	HWND						m_hWndVideo; 
	WTL::CSize					m_AspectRatio; 
	BOOL						m_GraphBuilt; 
	int							m_VideoType; 
	FString						m_FourCC; 
	FString						m_StreamFormat; 
	double						m_FrameRate;
	FRect						m_VideoRect; 

	void			Clear(); 
	HRESULT			BuildGraph();
	HRESULT			SetParentWnd(HWND hWndParent);
	BOOL			GetAVIInfo(const tchar* pFileName);

public:
	//IFVideo implementation
	HRESULT			Play(); 
	HRESULT			Start(); 
	HRESULT			Stop(); 
	HRESULT			Pause(); 
	HRESULT			SetVideoRect(const RECT& rcV);  
	VideoState		GetVideoState(); 
	HRESULT			ShowVideo(int nCmdShow); 
	HRESULT			Init(HWND hWndParent, IMediaPlayerNotify* pNotify); 
	MEDIA_TIME	GetDuration(); 
	MEDIA_TIME	GetAvailDuration(){return GetDuration();}
	BOOL			IsWaiting(); 
	HRESULT			ShowVideoWnd(int nCmdShow); 
	HRESULT			SetFullScreen(BOOL bFullScreen); 
	BOOL			IsFullScreen(); 
	double			GetFrameRate(){return m_FrameRate; }
	void			SetAspectRatio(int cx, int cy){m_AspectRatio.cx = cx; m_AspectRatio.cy = cy;}
	HRESULT			SeekPosition(MEDIA_TIME rtPos);	
	MEDIA_TIME	GetPosition();		
	WTL::CSize		GetVideoSize(); 
	WTL::CRect		GetAspectRatio(long lx,long ly); 
	long			GetVolume(); 
	HRESULT			SetVolume(long lVolume); 
	long			GetMediaEvent(); 
	//IFMediaType
	HRESULT			SetMediaType(const tchar* pszMediaType){m_MediaType = pszMediaType; m_MediaType.Trim(); return S_OK;}
	HRESULT			GetMediaType(tchar* pszMediaType){strcpy(pszMediaType, m_MediaType); return S_OK;}
	BOOL			IsMediaType(const tchar* pszMediaType){return m_MediaType.CompareNoCase(pszMediaType) == 0;}
	BOOL			SupportsMediaType(const tchar *pszMediaType){return TRUE; }
	DWORD			GetMediaFlags(){return 0;}
	void			CycleSubtitles(){}
	HRESULT			SendCommand(const char* pszCmdName, void* pCmdData)
	{
		return E_NOTIMPL; 
	}

public:
	HRESULT			LoadMedia(const tchar* pFileName, IMediaOptions* pOptions);
	HRESULT			BuildTheGraph(); 
	HRESULT			InitFilters(const tchar* pFileName); 
	void			ReleaseFilters(); 
	HRESULT			DetermineVideoType(const tchar* pFileName); 
	HRESULT			GetBufferingProgress(long* lProgress){return E_NOTIMPL;}
	



	//////////////////////////////////////////////////////////////////////////
public:

	long			OnMediaEvent(); 
	void			_DbgEnumFilters(); 
	void			_DbgCheckFilters(); 

	FMPlayerDShow(); 
	virtual ~FMPlayerDShow(); 
};
#endif //__FMPLAYERDSHOW_H__