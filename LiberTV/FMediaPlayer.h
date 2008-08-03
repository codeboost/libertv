#pragma  once

#include <atlbase.h>
#include <atlcom.h>
#include <atlhost.h>
#include <atlctl.h>
#include "Utils.h"
#include "IControlBar.h"
#include "IFVideo.h"
#include "FVideoWindow.h"
#include "FDownload.h"
#include "CWMPEventDispatch.h"
#include "FWebWindow.h"
#include "FSubManager.h"
#include "FMPlayerWM.h"
#include "FMPlayerDef.h"

template <int ViewId>
class FStatusBarPtr
{
	IStatusBar* m_pStatusBar;
	FString		m_StrText; 
public:
	FStatusBarPtr()
	{
		m_pStatusBar = NULL; 
	}
	~FStatusBarPtr()
	{
		m_pStatusBar = NULL; 
	}
	void SetStatusIcon(int nPane, HICON hIcon)
	{
		if (m_pStatusBar)
			m_pStatusBar->SetStatusIcon(nPane, hIcon); 
	}
	void SetStatusText(int nPane, const char* pszText)
	{
		if (m_pStatusBar)
			m_pStatusBar->SetStatusText(nPane, pszText, ViewId);
		m_StrText = pszText; 
	}
	void SetStatusBar(IStatusBar* pStatusBar)
	{
		m_pStatusBar = pStatusBar; 
		if (m_pStatusBar)
			m_pStatusBar->SetStatusText(0, m_StrText, ViewId); 
	}
	FStatusBarPtr* operator ->()
	{
		return this; 
	}
};

class FMediaPlayerPool
{
public:
	FArray<IFMediaPlayer*> m_MediaPlayers; 

	~FMediaPlayerPool()
	{
		Clear(); 
	}

	void		   Clear(); 
	IFMediaPlayer* FindPlayer(const tchar* pszMediaType);
	IFMediaPlayer* CreatePlayer(const tchar* pszMediaType, BOOL bForceVLC = FALSE); 
};

#define WM_PLUGIN_NOTIFY WM_USER + 0x117
class FMediaPlayerNotify : public IMediaPlayerNotify
{
	HWND	m_hWndParent;
public:
	void Init(HWND hWnd)
	{
		m_hWndParent = hWnd; 
	}
	void NotifyPlayer(DWORD dwCode, HRESULT hr)
	{
		::PostMessage(m_hWndParent, WM_PLUGIN_NOTIFY, (WPARAM)dwCode, (LPARAM)hr);
	}
};

class FMediaPlayer : public FVideoWindow{

public:
    DECLARE_WND_CLASS_EX("A_VideoWnd", 0, COLOR_BTNTEXT);
    BEGIN_MSG_MAP(FMediaPlayer)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_PLUGIN_NOTIFY, OnPluginNotify)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy) 
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouse)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnMouse)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnMouse)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouse)
		MESSAGE_HANDLER(WM_PAINT, OnPaint) 
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_SUBTITLE_FINISHED, OnSubtitleFinished)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_DSHOW_CLIP_COMPLETE, OnClipPlaybackFinished)
		MESSAGE_HANDLER(WM_DOCUMENT_COMPLETE, OnInforbarDocComplete)
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		CHAIN_MSG_MAP(FVideoWindow)
    END_MSG_MAP();
public:
	struct InOffs{
		size_t			m_Index; 
		MEDIA_TIME	m_Offset; 
		void Reset()
		{
			m_Index = 0; 
			m_Offset = 0; 
		}
	};
	WTL::CBitmap				m_pBackgroundBmp; 
	WTL::CDC					m_BackgroundDC; 
	FMediaPlayerPool			m_pMediaPlayers; 
	IFMediaPlayer*				m_pPlayer; 
	FMediaPlayerDefault			m_pBlankMP;
	FDownload					m_pVideo; 
	int							m_curClipIndex;		//int because we use -1 by default
	IControlBar*				m_pControlBar; 
	FMediaPlaylist				m_MediaFiles; 
	BOOL						m_bPaused; 
	BOOL						m_bBuffering; 
    BOOL                        m_bInfoBarVisible; 
    FIEWindow                   m_pInfoBar; 
	FSubParser					m_SubParser; 
	BOOL						m_bShowSub; 
	BOOL						m_bHideInfoBarOnError;
	InOffs						m_PendingPlay;
	FMediaPlayerNotify			m_pNotify; 
	VideoState					m_LastState; 
	BOOL						m_Streaming; 
	int							m_PlayCount; 
	BOOL						m_bDisableUpdate; 
    BOOL						m_bForwardPluginInput;
	FStatusBarPtr<VIEW_PLAYER>	m_pStatusBar; 
	FString						m_PluginName; 

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnMouse(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){bHandled = TRUE; return 1; }
	LRESULT OnSubtitleFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){OnSubtitleFinished((HRESULT)wParam, (void*)lParam); return 1;}
	LRESULT OnPluginNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnClipPlaybackFinished(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnInforbarDocComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT	OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){
		::SetFocus(GetParent());
		bHandled = TRUE; 
		return 0; 
	}

	
	HRESULT PlayMT(vidtype videoID, BOOL bStartPaused = FALSE); 
	HRESULT	Stop(); 
	void	UpdateControlBar(); 
	void	UpdateBuffering(); 
	MEDIA_TIME GetPosition(); 
	MEDIA_TIME GetDuration();
	HRESULT	SeekPosition(MEDIA_TIME rt); 
	HRESULT	LoadMedia(size_t nIndex, MEDIA_TIME rtOffset);
	HRESULT GetNotify(IMediaPlayerNotify** pNotify);
    BOOL    ShowInfoBar(BOOL bShow); 
	vidtype GetVideoID(){return m_pVideo.GetVideoID(); }
	int		GetCurrentClipId(){return m_curClipIndex;}
	BOOL	PlayClip(int nClipId);
    void    OnMediaFinished(); 
	void	UpdateLayout(); 
	void	UpdateMediaTypes();
	

	HRESULT	SetVolume(long lVol);
	HRESULT	PauseResume();
	HRESULT	Play(); 
	HRESULT	Pause(); 
	HRESULT	SetPosition(MEDIA_TIME rPos){return SeekPosition(rPos);}
	vidtype GetCurrentVideoID()	{	return m_pVideo.GetVideoID(); 	}
	HRESULT	PlayNext(BOOL bCycle); 
	HRESULT	PlayPrev(BOOL bCycle); 

	BOOL	IsPlaying();
	HRESULT	ResumeLastVideo(); 
	BOOL	ProcessMessage(MSG* pMsg);
	void	SetStatusBar(IStatusBar* pStatusBar)
	{
		m_pStatusBar.SetStatusBar(pStatusBar); 
	}
	FString	FormatVideoName(); 

	//FMediaPlayerSub.cpp
	void	OnSubtitleFinished(HRESULT hr, void* lpVoid); 
	HRESULT	SelectSubtitle(const tchar* szLang); 
	HRESULT GetSubtitleFilename(char* pszSubFilename, DWORD* dwLen);
	void	UpdateSubtitle(ulong ulFrameNumber); 
	void	FindExistingSubtitles();
	FString	ExtractSubLanguage(const char* pszSubFilename);
	void	UpdateSubtitles(); 
	
	BOOL	IsPlaylist();
	BOOL	IsStreaming();
	void	RefreshQuickbar(); 
	void	SetBgImage(const tchar* pszImage);
	void	ReportError(HRESULT hr); 
	void	OnCannotPlayContent(); 
};

