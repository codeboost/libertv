#ifndef __FMAINFRAME_H__
#define __FMAINFRAME_H__

#include "FGlobals.h"
#include "Utils.h"
#include "FMainFrameContainer.h"
#include "FIEControlBar.h"
#include "CaptionPainter.h"
#include "FWaitDlg.h"
#include "FVideoOptionsDlg.h"
#include "FQuickBar.h"
#include "FSessionStatus.h"
#include "FCodecNotFoundDlg.h"
#include "FCommendDialog.h"

#define IDC_STATUS_BAR 100

//List of all browser controls in our app
enum eBrowsers
{
	brNone, brTopBar, brGuide, brNavBar, brCollection, brInfoBar, brMPlayer, 
	brSettings, brVideoOptions, brFeeds, brCommentDlg
};

//Status bar control object
class FStatusBarObject : public IStatusBar{
	HWND	m_hWndBar; 
	FString m_SectionStr[8];
	mz_Sync	m_Sync; 
public:
	void SetBarWnd(HWND hWndBar);
	void SetStatusIcon(int nPane, HICON hIcon);
	void SetStatusText(int nPane, const tchar* pszText, int nObject);
	void OnViewChanged(int nSection); 
};

//Main frame of the app
class FMainFrame : public IStatusBar, 
				   public CFrameWindowImpl<FMainFrame>,
				   public CMessageFilter
{
protected:
	WTL::CStatusBarCtrl			m_pStatusBar; 
	HICON						m_hConnectable; 
	HICON						m_hNotConnectable; 
public:
	DECLARE_FRAME_WND_CLASS_EX(NULL, IDR_MAINFRAME, 0, 0)
	BEGIN_MSG_MAP(FMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy) 
		MESSAGE_HANDLER(WM_SIZE, OnSize); 
		MESSAGE_HANDLER(WM_SIZING, OnSizing); 
        MESSAGE_HANDLER(WM_PLAYER_MAX, OnPlayerMaximize); 
		MESSAGE_HANDLER(WM_TIMER, OnTimer); 
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground); 
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus);
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus); 
		MESSAGE_HANDLER(WM_COMMAND, OnCommand); 
		MESSAGE_HANDLER(WM_SYSCOMMAND, OnSysCommand);
		MESSAGE_HANDLER(WM_DOCUMENT_COMPLETE, OnDocumentComplete); 
		MESSAGE_HANDLER(WM_ACTIVATE, OnActivate); 
		MESSAGE_HANDLER(WM_ENABLE, OnEnable); 
		MESSAGE_HANDLER(WM_DOWNLOAD_LOADED, OnDownloadLoaded);
		MESSAGE_HANDLER(WM_MTTI_DOWNLOAD, OnMTTDownload);
		NOTIFY_HANDLER(IDC_STATUS_BAR, NM_CLICK, OnStatusBarClick);
		CHAIN_MSG_MAP(CFrameWindowImpl<FMainFrame>)
	END_MSG_MAP()


public:
	static	FStatusBarObject	m_iStatusBar; 
	CFont						m_FntStatus; 
	FMainFrameContainer			*m_Container;
	FToolbar					*m_TopBar; 
    FWaitDlg                    m_WaitDlg; 
    DWORD                       m_Style; 
    DWORD                       m_ExStyle;
    FRect                       m_OrgRect; 
	DWORD						m_dwFlags; 
	HMENU						m_hMenu; 
	DWORD						m_dwLastAction; 
	FAutoWindow<FQuickBar>		m_QuickBar; 
	FAutoWindow<FQuickBar>		m_StatusWindow; 
	FDlgSessionStatus			m_DlgSessionStatus; 
	FAutoWindow<FVideoOptionsDlg>	m_pVideoOptions; 
	FAutoWindow<FCodecNotFoundDlg>	m_CodecNotFound; 
	FCommentDialog				m_CommentDialog; 

public:	
	//Messages
	FMainFrame(); 
	BOOL	PreTranslateMessage(MSG* pMsg);
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSizing(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);		
    LRESULT OnPlayerMaximize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDocumentComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDownloadLoaded(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMTTDownload(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnEnable(UINT, WPARAM w, LPARAM l, BOOL& bHandled);
	LRESULT OnStatusBarClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);


	BOOL	ForwardAccelerators(MSG* pMsg); 

	void	CheckProxyServer(); 

	//Video management
	bool PlayMediaFile(vidtype videoID);
	bool RemoveVideo(vidtype videoID); 
	void RestoreWindow(); 
	FMainFrameContainer& GetContainer();

	//Layout
	void	UpdateCtrlLayout(); 
	void	SetFullScreen(BOOL bFullScreen); 
	BOOL	IsFullScreen();
	void	RestoreFullScreen(); 
	void	UpdateQuickbarVisibility(); 
	
	//Functions
    void    Search(const char* szSearchString, LONG flag); 
	void	EpisodeDetails(ULONG ulVideoId); 
	vidtype GetActiveVideo();
	BOOL	OpenVideoOptions(); 
	HRESULT NavigateGuide(const tchar* pszURL, ULONG dwFlags); 
	
	//Information
	eBrowsers	GetFocusBrowser(CComQIPtr<IWebBrowser2>& rpBrowser);
	IVideoPlayer* GetPlayer();
	
	//Dialogs
	void	ShowLoading(BOOL bShow);
	long	ShowMenu(); 
	BOOL	ShowQuickBar(); 
	BOOL	ShowStatusWindow(); 
	void	ShowUnconnectableWarning(); 
	void	ShowMissingCodecDialog(const tchar* pszPath);
	DWORD	AddFeedDialog(const tchar* pszUrl, const tchar* pszName);

	//Notifications
	void	OnViewChanged(int nCurrentView); 

	//Notifications from Download Dispatcher
	void	OnVideoLoaded(DWORD dwVideoID); 

	//Status bar
	void	SetStatusIcon(int nPane, HICON hIcon);
	void	SetStatusText(int nPane, const tchar* pszText, int nObject);
	void	ShowCommentsWnd(); 
	int		GetCurrentView()
	{
		return m_Container?m_Container->m_CurrentView:VIEW_BROWSER;
	}
	void GoToRSS(vidtype videoID);
	void NavigateToDetails(const tchar* pszDetailsURL); 
	int  GetCollectionSubView()
	{
		return m_Container->m_pStatus->GetView(); 
	}

	void SetActiveView(int nView)
	{
		m_Container->SwitchActiveView(nView); 
	}

};

#endif //__FMAINFRAME_H__