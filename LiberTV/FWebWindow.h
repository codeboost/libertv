#ifndef __FWEBWINDOW_H__
#define __FWEBWINDOW_H__

#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include "resource.h"
#include "IExplorer.h"
#include "ILTWindows.h"

#define IMPLEMENT_HTML_MESSAGE_MAP() void OnHTMLSetHandlers(){_DefineHtmlMsgMap();}

class FWebWindow : public CWTLIExplorer
{
public:
	virtual ~FWebWindow(){
		if (m_hWnd != NULL)
			DestroyWindow();
	}
};

//Default Notification implementation
class FBrowserEvents : public virtual IHttpNotify
{
public:
	 BOOL OnBeforeNavigate(DWORD dwID, const tchar* pszURL){return TRUE;} //return FALSE to cancel navigation
	 void OnDownloadBegin(DWORD dwID){}
	 void OnProgressChange(DWORD dwID, long lCurrent, long lMax){};
	 void OnNavigateComplete(DWORD dwID, const tchar* pszURL){}
	 void OnDocumentComplete(DWORD dwID, BOOL bMainFrame){}
	 BOOL OnNavigateError(DWORD dwID, BOOL bMainFrame, const tchar* szUrl){return false;};
};

class FIEWindow :   public IIEWindowI,
					public CWindowImpl<FIEWindow>, 
                    public FBrowserEvents
{

protected:
	WTL::CBitmap	m_pLoadingBmp;
	WTL::CDC		m_pLoadingDC; 
public:
    DECLARE_WND_CLASS_EX("FIEWindow",  0 , COLOR_WINDOW);
    HWND            m_hWndNotify; 
    FWebWindow      m_pBrowser; 
	BOOL			m_bShowLoading;
    long            m_lMax;
    long            m_lCur; 
	BOOL			m_IgnoreShowWnd; 
	
		BEGIN_MSG_MAP(FIEWindow)
			MESSAGE_HANDLER(WM_CREATE, OnCreate)
			MESSAGE_HANDLER(WM_PAINT, OnPaint)
            MESSAGE_HANDLER(WM_SIZE, OnSize)
            MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
			MESSAGE_HANDLER(WM_TIMER, OnTimer)
			MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
			MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
			MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
			MESSAGE_HANDLER(WM_SHOWWINDOW, OnShowWindow)
		END_MSG_MAP()

        FIEWindow()
        {
            m_hWndNotify = NULL; 
			m_bShowLoading = FALSE; 
            m_lMax = m_lCur = 0; 
			m_IgnoreShowWnd = FALSE; 
        }
		virtual ~FIEWindow()
		{

		}

        LRESULT	OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){	bHandled = TRUE; return 1; }
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);	
		LRESULT	OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
        LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
		LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
		LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 

		//Notifications
        void	OnDocumentComplete(DWORD dwID, BOOL bMainFrame);
        void	OnProgressChange(DWORD dwID, long lCurrent, long lMax); 
        BOOL	OnNavigateError(DWORD dwID, BOOL, const tchar*);
		void	OnNavigateComplete(DWORD dwID, const tchar* pszURL);

		//IIEWindow
        HRESULT Navigate(const tchar* pszPath, const tchar* pTargetFrameName="", DWORD dwNavFlags = navNoHistory);
		BOOL	IsViewActive(){return FALSE;}		//asks if this window is currently the active view
		void	OnActivated(BOOL bActivated){}		//called when view is activated or deactivated
		void	SetNotifyWnd(HWND hWnd){m_hWndNotify = hWnd;}
		void	SetErrorPage(const char* pStrErrUrl); 
		BOOL	IsPageLoaded(); 
        
};

#endif //__FWEBWINDOW_H__