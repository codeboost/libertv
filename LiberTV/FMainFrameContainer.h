#ifndef __FMAINFRAMECONTAINER_H__
#define __FMAINFRAMECONTAINER_H__


#include "Utils.h"
#include "FAutoWindow.h"
#include <atlsplit.h>
#include "FWindowBase.h"
#include "FChannelGuide.h"
#include "FMediaContainer.h"
#include "FCollection.h"
#include "FGlobals.h"

#include "ILTWindows.h"

#define TIMER_QUICKBAR_UPDATE 1

class FSplitterWindow : public CSplitterWindowImpl<FSplitterWindow>
{
	BEGIN_MSG_MAP(FSplitterWindow)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnDblClick);
        MESSAGE_HANDLER(WM_LBUTTONDOWN, OnButtonDown); 

        CHAIN_MSG_MAP(CSplitterWindowImpl<FSplitterWindow>);

        MESSAGE_HANDLER(WM_CREATE, OnCreate); 
        MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove);
        
	END_MSG_MAP()

	int	m_LastPos; 
    LRESULT OnCreate(UINT , WPARAM , LPARAM , BOOL& bHandled);
	LRESULT OnDblClick(UINT, WPARAM, LPARAM, BOOL&);
    LRESULT OnMouseMove(UINT, WPARAM, LPARAM, BOOL&); 
    LRESULT OnButtonDown(UINT, WPARAM, LPARAM, BOOL&);
	
public:
	FSplitterWindow()
	{
		m_LastPos = 0; 
	}

    void StartMouseTrack();
    void OnOff(); 
	bool IsOn(); 

};

class FMainFrame;

#define F_QUICKBAR_LOADED 0x01
#define F_STATUS_LOADED 0x02

#define VIEW_COLLECTION_TEST 4


class FMainFrameContainer : public CFrameWindowImpl<FMainFrameContainer>
{	
public:
	friend class FMainFrame;
	DECLARE_WND_CLASS_EX("FMainFrameContainer", 0, COLOR_WINDOW);
protected:
	BEGIN_MSG_MAP(FMainFrame)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DOCUMENT_COMPLETE, OnDocumentComplete)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground); 
		MESSAGE_HANDLER(WM_SIZE, OnSize) 
		MESSAGE_HANDLER(WM_SETFOCUS, OnFocus)
		MESSAGE_HANDLER(WM_KILLFOCUS, OnKillFocus)
		MESSAGE_HANDLER(WM_PLUGIN_NOTIFY, OnPluginNotify)
		CHAIN_MSG_MAP(CFrameWindowImpl<FMainFrameContainer>)
	END_MSG_MAP()

		FMainFrameContainer()
		{
			m_dwFlags = 0; 
		} 
		
		FIEWindow						m_pToolbar; 
		static int						m_CurrentView; 
		HWND							m_ActiveWindow; 

		FAutoWindow<FCollectionDisplay>	m_pStatus;
		FAutoWindow<FChannelGuide>		m_pBrowser; 
		FAutoWindow<FMediaContainer>	m_pMediaPlayer; 
		FAutoWindow<FFeedsDisplay>		m_pFeeds; 
		dword							m_dwFlags;
		FString							m_QuickSearchString; 
		

protected:
		LRESULT	OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) {return 1;}
		LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 	
		LRESULT	OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
		LRESULT OnDocumentComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
		LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
		LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
		LRESULT OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
		LRESULT OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
		LRESULT OnPluginNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 

		BOOL	GetCollectionDivRect(FRect &rc);
public:
		void	SwitchActiveView(int nView, BOOL bFromExternal = FALSE);
		void	HideActiveView(); 
		void	OnMinMax(int nType);
	 
};
#endif //__FMAINFRAMECONTAINER_H__