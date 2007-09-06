#ifndef FMediaContainerDotH
#define FMediaContainerDotH

#include "FVideoWindow.h"
#include "FMediaPlayer.h"
#include "AdInterface.h"
#include "FAutoWindow.h"
#include "FIEControlBar.h"

/*
Purpose: Servers as a container for the media player window.
FMediaContainer may contain skins, ads, etc, which don't interfere
with MediaPlayer's implementation.
*/

#define TIMER_ADS 1
#define TIMER_MOUSE 2

class FAdNotify :   public IAdNotify
{
	BOOL OnBeforeNavigate(DWORD dwID, const tchar* pszURL){return TRUE;} //return FALSE to cancel navigation
	void OnDownloadBegin(DWORD dwID){}
	void OnProgressChange(DWORD dwID, long lCurrent, long lMax){};
	void OnNavigateComplete(DWORD dwID, const tchar* pszURL){}
	void OnDocumentComplete(DWORD dwID, BOOL bMainFrame){}
	BOOL OnNavigateError(DWORD dwID, BOOL bMainFrame, const tchar* szUrl){return false;};
};

#define MAX_AD_WINDOWS 5 //number of IAdWindow objects in the m_pAdWindows array

class FMediaContainer :  public FVideoWindow, public FAdNotify
{
	DECLARE_WND_CLASS_EX("FMediaContainer", 0, COLOR_BTNTEXT);

	BEGIN_MSG_MAP(FMediaContainer)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown) 
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus) 
		MESSAGE_HANDLER(WM_ADWINDOW_CLOSE, OnAdClose) 
		MESSAGE_HANDLER(WM_PLUGIN_NOTIFY, OnPlayerNotify)
		MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		MESSAGE_HANDLER(WM_DOCUMENT_COMPLETE, OnDocumentComplete)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)

		CHAIN_MSG_MAP(FVideoWindow)
	END_MSG_MAP()

	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){return 1; }
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnAdClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnPlayerNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnDocumentComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 


	FMediaPlayer					m_pMediaPlayer; 
	CAutoPtr<IVideoAdManager>		m_AdManager; 
	CAutoPtr<IAdWindow>				m_pAdWindows[MAX_AD_WINDOWS];
	BOOL							m_bFullScreenAd; 
	BOOL							m_bPlaying; 
	vidtype							m_CurVideoID; 
	DWORD							m_MSPlayed; 
	DWORD							m_MSLast; 
	IStatusBar*						m_pStatusBar; 
	FIEControlBar					m_ControlBar; 
	int								m_CBarHeight;
protected:
	void UpdateLayout(); 
public:

	FMediaContainer(); 
	~FMediaContainer(); 
	

	//IAdNotify
	void OnDocumentComplete(DWORD dwID, BOOL bMainFrame);
	BOOL OnNavigateError(DWORD dwID, BOOL bMainFrame, const tchar* pszUrl);
	void OnAdFinished(DWORD dwID); 
	HRESULT GetFocusBrowser(CComQIPtr<IWebBrowser2>& rpBrowser);

	//IVideoPlayer 
	HRESULT	SetVolume(long lVol);
	HRESULT	SetPosition(MEDIA_TIME lPos);
	HRESULT	Play();
	HRESULT	Pause();
	HRESULT	Stop();
	BOOL 	IsPlaying();
	HRESULT	PlayNext(BOOL bCycle);
	HRESULT	PlayPrev(BOOL bCycle);
	ulong	GetVideoID();
	int		GetCurrentClipId();
	BOOL	PlayClip(int nClipId);
	HRESULT	SelectSubtitle(const tchar* pszLang);
	HRESULT PlayMT(ulong videoID, BOOL bPaused /* = FALSE */);
	HRESULT GetSubtitleFilename(char* pszSubFilename, DWORD* dwLen);
	HRESULT GetNotify(IMediaPlayerNotify** pNotify);
	void	ShowNavbar(BOOL bShow)
	{
		m_ControlBar.ShowWindow(bShow ? SW_SHOW : SW_HIDE); 
	}
	int		GetNavBarHeight()
	{
		return m_CBarHeight;
	}

	BOOL	ProcessMessage(MSG* pMsg);

	class FAdLoader : public mz_Thread
	{
		void Thread(void*); 
	};

	FAdLoader					m_AdThread; 

	FFastSyncSemaStack<vidtype> m_AdRequests; 
	BOOL DownloadAdpoints(vidtype videoID); 
	void SetStatusBar(IStatusBar* pStatusBar);
	
};



#endif