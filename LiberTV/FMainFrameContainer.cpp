#include "stdafx.h"
#include "FMainFrameContainer.h"
#include "GlobalObjects.h"
#include "FMainFrame.h"
#include "FMediaContainer.h"


extern FAutoWindow<FMainFrame> g_MainFrame; 
int FMainFrameContainer::m_CurrentView = 0; 

int	OpenSplitterWidth = 220;
int MinOpenSplitterWidth = 20; 

LRESULT FSplitterWindow::OnCreate(UINT , WPARAM , LPARAM , BOOL& bHandled)
{
    m_cxyMin = 0; 
	m_cxySplitBar = 2; 
	m_cxyMaxPane = 300; 
	
    return 0; 
}

void FSplitterWindow::OnOff()
{
    int nSplitterPos = GetSplitterPos();

    if (nSplitterPos == m_cxyMin)
	{
		nSplitterPos = m_LastPos; 
		if (nSplitterPos < MinOpenSplitterWidth)
			nSplitterPos = OpenSplitterWidth; 
	}
    else
    {
        m_LastPos = nSplitterPos;
        nSplitterPos = 0; 
    }
	SetSplitterPos( nSplitterPos); 
	
}

bool FSplitterWindow::IsOn()
{
	return !(GetSplitterPos() <= m_cxyMin);
}

LRESULT FSplitterWindow::OnDblClick(UINT, WPARAM, LPARAM, BOOL&)
{
	SetSinglePaneMode(SPLIT_PANE_RIGHT); 
	g_AppSettings.m_Flags&=~PFLAG_QUICKBAR_ON; 
    return 0; 
}

LRESULT FSplitterWindow::OnMouseMove(UINT , WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
    return 0; 
}

LRESULT FSplitterWindow::OnButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    if (GetSplitterPos() <= MinOpenSplitterWidth)
        OnOff();
    else
    {
        CSplitterWindowImpl<FSplitterWindow>::OnLButtonDown(uMsg, wParam, lParam, bHandled);
    }
    return 0;
}

void FSplitterWindow::StartMouseTrack()
{
    TRACKMOUSEEVENT tme = { 0 };
    tme.cbSize = sizeof(tme);
    tme.dwFlags = TME_LEAVE;
    tme.hwndTrack = m_hWnd;
    _TrackMouseEvent(&tme);
}

dword dwClientEdge = 0; 

LRESULT FMainFrameContainer::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
	SwitchActiveView(m_CurrentView); 
	return 0; 
}

void FMainFrameContainer::HideActiveView()
{
	switch(m_CurrentView)
	{
	case VIEW_BROWSER:
		if (m_pBrowser.IsObjectAndWindow())
		{
			m_pBrowser.ShowWindow(SW_HIDE); 
		}
		break; 
	case VIEW_STATUS:
		if (m_pStatus.IsObjectAndWindow())
		{
			m_pStatus.ShowWindow(SW_HIDE); 
		}
		
		break; 
	case VIEW_PLAYER:
		if (m_pMediaPlayer.IsObjectAndWindow())
		{
			m_pMediaPlayer.ShowWindow(SW_HIDE); 
		}
		break; 
	case VIEW_FEEDS:
		if (m_pFeeds.IsObjectAndWindow())
		{
			m_pFeeds.ShowWindow(SW_HIDE); 
		}
		break; 
	}
}

LRESULT FMainFrameContainer::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	::SetFocus(m_ActiveWindow); 
	return 0; 
}

LRESULT FMainFrameContainer::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0; 
}



void FMainFrameContainer::SwitchActiveView(int nView, BOOL bFromExternal)
{
	dword dwChildStyle =  WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	if (m_CurrentView != nView)
		HideActiveView();
	m_CurrentView = nView; 
	
	switch(m_CurrentView)
	{
	case VIEW_BROWSER:
		if (!m_pBrowser.IsObjectAndWindow())
		{
			m_pBrowser.Create(m_hWnd, &rcDefault, "about:blank", dwChildStyle, dwClientEdge);
			m_pBrowser->SetWindowText("FMainBrowser"); 
		}

		m_pBrowser.ShowWindow(SW_SHOW);
		m_ActiveWindow = m_pBrowser; 
		
		break; 
	case VIEW_PLAYER:
		{
			if (!m_pMediaPlayer.IsObjectAndWindow())
			{
				//m_pMediaPlayer.Free();
				//m_pMediaPlayer.Attach(new FMediaContainer);
				m_pMediaPlayer.Create(m_hWnd, &rcDefault, "FMediaContainer", dwChildStyle, dwClientEdge); 
				m_pMediaPlayer->SetStatusBar(&FMainFrame::m_iStatusBar);
			}
			m_pMediaPlayer.ShowWindow(SW_SHOW); 
			m_ActiveWindow = m_pMediaPlayer; 
		}
		break;
	case VIEW_FEEDS:
		{
			if (!m_pFeeds.IsObjectAndWindow())
			{
				m_pFeeds.Create(m_hWnd, &rcDefault, "about:blank", dwChildStyle | WS_VSCROLL, dwClientEdge); 
				m_pFeeds->Navigate(g_AppSettings.AppDir("data/rssfeeds.html"), ""); 
				m_pFeeds->SetWindowText("FFeeds"); 
			}
			m_pFeeds->m_hWndNotify = m_hWnd; 
			if (!m_pFeeds->IsWindowVisible())
				m_pFeeds.ShowWindow(SW_SHOW); 
			else
				m_pFeeds->OnActivated(TRUE); 

			m_ActiveWindow = m_pFeeds; 
		}
		break; 
	case VIEW_STATUS:
		if (!m_pStatus.IsObjectAndWindow())
		{
			m_pStatus.Create(m_hWnd, &rcDefault, "about:blank", dwChildStyle | WS_VSCROLL, dwClientEdge); 
			m_pStatus->Navigate(g_AppSettings.AppDir("data/collection.html"), ""); 
		}
		m_pStatus->m_hWndNotify = m_hWnd; 
		m_pStatus.ShowWindow(SW_SHOW); 
		m_ActiveWindow = m_pStatus->m_hWnd; 

		break; 
	}

	FString StrActive;StrActive.Format("%d", nView);
	g_MainFrame->m_TopBar->m_pBrowser.CallJScript("set_active_section", StrActive); 
	g_MainFrame->UpdateCtrlLayout();
	g_MainFrame->OnViewChanged(m_CurrentView); 

	BOOL bHandled; 
	OnSize(WM_SIZE, 0, 0, bHandled); 
	::SetFocus(m_ActiveWindow); 
}

LRESULT FMainFrameContainer::OnDocumentComplete(UINT, WPARAM wWnd, LPARAM, BOOL&)
{
	if ((HWND)wWnd == m_pStatus)
	{
		m_dwFlags|=F_STATUS_LOADED; 
		m_pStatus->m_pBrowser.CallJScript("setDeveloperMode", g_AppSettings.bDeveloperMode?"1":"0");
	}
	return 0; 
}


LRESULT FMainFrameContainer::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	FRect rc;
	GetClientRect(&rc); 
	CWindow(m_ActiveWindow).MoveWindow(&rc, TRUE); 
	return 0; 
}


LRESULT FMainFrameContainer::OnPluginNotify(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (wParam == PluginPlaybackFinished)
	{
		if (g_MainFrame->IsFullScreen())
			g_MainFrame->SetFullScreen(TRUE);
	}
	return 0; 
}

void FMainFrameContainer::OnMinMax(int nType)
{
	FString StrJScript; 

	switch(nType)
	{
	case SC_MINIMIZE:
		::ShowWindow(m_ActiveWindow, SW_HIDE); 
		break; 
	case SC_RESTORE:
		::ShowWindow(m_ActiveWindow, SW_SHOW); 
	default:
		break; 
	}
}










