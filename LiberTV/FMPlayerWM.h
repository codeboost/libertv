#ifndef __FMPLAYERWM_H__
#define __FMPLAYERWM_H__


#pragma once

#include <wmp.h>
#include <atlbase.h>
#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>
#include "IControlBar.h"
#include "CWMPEventDispatch.h"
#include "IFVideo.h"

class IFWMPlayer 
{
public:
	virtual void STDMETHODCALLTYPE PlayStateChange(long NewState) = 0;
	virtual void STDMETHODCALLTYPE StatusChange() = 0;
};


class FWMPEventDispatch : public CWMPEventDispatch
{
public:
	FWMPEventDispatch()
	{
		m_pParentObj = NULL; 
	}

	void STDMETHODCALLTYPE PlayStateChange(long NewState)
	{
		if (m_pParentObj)
		m_pParentObj->PlayStateChange(NewState); 
	}

	void STDMETHODCALLTYPE StatusChange()
	{
		if (m_pParentObj)
		m_pParentObj->StatusChange();
	}

	


	IFWMPlayer* m_pParentObj; 

};

typedef CComObject<FWMPEventDispatch> FWMPEventHandler;

class FMPlayerWM : public IFMediaPlayer, public IFWMPlayer
{
protected:
	CComPtr<IAxWinHostWindow>	spHost;
	CComPtr<IConnectionPoint>	m_spConnectionPoint; 
	CAxWindow					m_wndView;		// ActiveX host window class.
	CComPtr<IWMPPlayer>			m_spWMPPlayer;  // Smart pointer to IWMPPlayer interface.
	CComQIPtr<IWMPControls>		m_spWMPControls; 
	CComQIPtr<IWMPControls3>	m_spWMPControls3; 
	CComPtr<IWMPPlaylist>       m_spPlaylist; 
	CComQIPtr<IWMPSettings>		m_spSettings; 
	DWORD						m_dwAdviseCookie; 
	FWMPEventHandler*			m_pEventProxy; //
	IMediaPlayerNotify*			m_pNotify; 
	WMPPlayState				m_CurrentState; 
	FString						m_MediaType; 
	FRect						m_CurrentRect; 

public:
	HRESULT 			Init(HWND hWndParent, IMediaPlayerNotify* pNotify);
	HRESULT				Play();							//Runs the graph
	HRESULT				Stop();							//Stops and destroys the graph
	HRESULT				Pause();						//Pauses the graph
	HRESULT				LoadMedia(const tchar* pFileName, IMediaOptions* pOptions);
	VideoState			GetVideoState();				//----
	HRESULT				SeekPosition(MEDIA_TIME rtCurrent);
	MEDIA_TIME			GetPosition();
	MEDIA_TIME			GetDuration(); 
	MEDIA_TIME			GetAvailDuration();	//For now

	HRESULT				SetVideoRect(const RECT& rcV);	//---
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
	DWORD				GetMediaFlags(){return 0;}
	void				CycleSubtitles(){}
	HRESULT				SendCommand(const char* pszCmdName, void* pCmdData)
	{
		return E_NOTIMPL; 
	}

	//WMP Notifications
	void STDMETHODCALLTYPE PlayStateChange(long NewState);
	void STDMETHODCALLTYPE StatusChange(); 

};


#endif //__FMPLAYERWM_H__