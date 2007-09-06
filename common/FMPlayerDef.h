#ifndef __FMPLAYERDEF_H__
#define __FMPLAYERDEF_H__

#include "IFVideo.h"

class FMediaPlayerDefault : public IFMediaPlayer
{
protected:
	HWND	m_hWndParent; 
	IMediaPlayerNotify* m_pNotify; 
	FString m_MediaType;
public:

	virtual ~FMediaPlayerDefault()
	{

	}

	virtual HRESULT			Init(HWND hWndParent, IMediaPlayerNotify* pNotify)
	{
		m_hWndParent = hWndParent; 
		m_pNotify = pNotify; 
		return S_OK; 
	}
	virtual void			Clear()
	{

	}
	HRESULT					Play()	{return E_NOTIMPL;}
	HRESULT					Start() {return E_NOTIMPL;}
	HRESULT					Stop()	{return E_NOTIMPL;}
	HRESULT					Pause() {return E_NOTIMPL;}
	HRESULT					SetVideoRect(const RECT& rcV){return E_NOTIMPL;}
	VideoState				GetVideoState(){return vssStopped;}
	HRESULT					ShowVideo(int nCmdShow){return E_NOTIMPL; }
	HRESULT					SeekPosition(MEDIA_TIME rtPos){return E_NOTIMPL;};
	MEDIA_TIME				GetPosition() {return 0; }		
	MEDIA_TIME				GetAvailDuration(){return 0;}
	virtual HRESULT			LoadMedia(const char* pFileName, IMediaOptions* pOptions){return E_NOTIMPL; }
	virtual HRESULT			ShowVideoWnd(int nShow)	{	return E_NOTIMPL; 	}
	virtual HRESULT			SetFullScreen(BOOL bFullScreen)	{	return E_NOTIMPL; 	}
	virtual BOOL			IsFullScreen()			{	return FALSE; 	}
	virtual double			GetFrameRate()			{	return 1.0;	}
	virtual void			SetAspectRatio(int cx, int cy)	{}
	virtual long			GetMediaEvent()			{ return 0; 	}
	virtual long			GetVolume()				{return -10000; }	
	virtual HRESULT			SetVolume(long lVol)	{return E_NOTIMPL; }
	MEDIA_TIME				GetDuration(){return 0;}
	HRESULT					GetBufferingProgress(long* lProgress){return E_NOTIMPL;}
	HRESULT					SetMediaType(const tchar* pszMediaType){m_MediaType = pszMediaType; m_MediaType.Trim(); return S_OK;}
	HRESULT					GetMediaType(tchar* pszMediaType){strcpy(pszMediaType, m_MediaType); return S_OK;}
	BOOL					IsMediaType(const tchar* pszMediaType){return m_MediaType.CompareNoCase(pszMediaType) == 0;}
	BOOL					SupportsMediaType(const tchar *pszMediaType){return TRUE; }
	DWORD					GetMediaFlags(){return 0;}
	void					CycleSubtitles(){}
	HRESULT					SendCommand(const char* pszCmdName, void* pCmdData){ return E_NOTIMPL;}

};
#endif //__FMPLAYERDEF_H__