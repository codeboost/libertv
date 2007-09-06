#pragma once

#include "IFVideo.h"
#include <atlwin.h>
#include "atlapp.h"
#include "atlmisc.h"

#import "libid:29866AED-1E14-417D-BA0F-1A2BE6F5A19E" no_namespace named_guids //QTOLibrary.dll
#import "libid:7b92f833-027d-402b-bff9-a67697366f4e" no_namespace named_guids //QTOControl.dll

class FMediaPlayerQT : public IFMediaPlayer
{
	IMediaPlayerNotify*				m_pNotify; 
	FString							m_MediaType; 
	CComPtr<IAxWinHostWindow>		spHost;
	CAxWindow						m_wndView; 
	CComPtr<IQTControl>				m_QTControl; 
	CComPtr<IQTMovie>				m_QTMovie; 
protected:
	void				CloseMovie(); 
public:
	HRESULT 			Init(HWND hWndParent, IMediaPlayerNotify* pNotify);
	HRESULT				Play();							//Runs the graph
	HRESULT				Stop();							//Stops and destroys the graph
	HRESULT				Pause();						//Pauses the graph
	HRESULT				LoadMedia(const tchar* pFileName, MEDIA_TIME rtOffset = 0, MEDIA_TIME rtMaxDuration = 0);
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
	long				GetMediaEvent();
	BOOL				SupportsMediaType(const tchar* pszMediaType){return TRUE; };
	HRESULT				SetMediaType(const tchar* pszMediaType){m_MediaType = pszMediaType; return S_OK; }
	HRESULT				GetMediaType(tchar* pszMediaType){strcpy(pszMediaType, m_MediaType); return S_OK; }
	HRESULT				GetBufferingProgress(long* lProgress);
	BOOL				IsMediaType(const tchar* pszMediaType){return m_MediaType == pszMediaType;}

	double				GetFrameRate(); 
	void				SetAspectRatio(int cx, int cy);
	void				Clear(); 
	void				OnAxWndTimer(UINT timerID);

	HRESULT				AddPlaylistItem(const tchar* pszPath); 
	HRESULT				FindInPlaylist(const tchar* pszPath, long* lpIndex); 

};


class FQTWnd :	public ATL::CWindowImpl<FQTWnd>, 
				public IMediaPlayerNotify
{
public:
	DECLARE_WND_CLASS_EX("FVW", 0, COLOR_WINDOW); 

	BEGIN_MSG_MAP(FQTWnd)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_RBUTTONDOWN, OnDoubleClick)
	END_MSG_MAP()
	//	BOOL ProcessWindowMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lResult, DWORD dwMsgMapID = 0);
	FMediaPlayerQT m_MediaPlayer; 

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
		
		m_MediaPlayer.Init(m_hWnd, this); 
		m_MediaPlayer.LoadMedia("F:\\Temp\\test.mov", 0, 0);
		m_MediaPlayer.Play();
		return 0; 
	}


	//IMediaPlayerNotify
	void NotifyPlayer(DWORD dwCode, HRESULT hr){}
};
