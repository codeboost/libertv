#include "stdafx.h"
#include "FChannelGuide.h"
#include "GlobalObjects.h"
#include "FMainFrame.h"
#include "atlutil.h"
#include "IAppManager.h"
#include "FMenu.h"

#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#define max(a, b)  (((a) > (b)) ? (a) : (b))

const int ToolbarHeight = 0;
static BOOL g_firstLoad = TRUE; 
static BOOL g_FirstDoc =  TRUE; 


extern FAutoWindow<FMainFrame> g_MainFrame; 

BOOL FChannelGuide::OnNavigateError(DWORD dwID, BOOL bMainFrame, const tchar* pszURL)
{
    FIEWindow::OnNavigateError(dwID, bMainFrame, pszURL);
    if (strstr(pszURL, NOTFOUND_HTML) == NULL)
    {
        Navigate(g_AppSettings.AppDir(NOTFOUND_HTML)); 
        return true; 
    }
	if (strcmp(pszURL, "about:blank") != 0)
		ShowLoading(FALSE); 

	m_bNavigating = FALSE; 
	g_MainFrame->m_iStatusBar.SetStatusText(0, "Done", VIEW_BROWSER); 
    return false; 
}

void FChannelGuide::OnNavigateComplete(DWORD dwID, const tchar* pszURL)
{
	SetWBFocus();
	m_bNavigating = FALSE; 
	g_MainFrame->m_iStatusBar.SetStatusText(0, "Done", VIEW_BROWSER); 
}

BOOL FChannelGuide::OnBeforeNavigate(DWORD dwID, const tchar* pszURL)
{
	BOOL bNavigate = TRUE; 
	CUrl url; 

	if (url.CrackUrl(pszURL))
	{
		FString FStrPath = url.GetUrlPath();
		FString StrExt = PathFindExtension(FStrPath); 
		StrExt.MakeLower();
		if (StrExt.Find(".torrent") != -1)
		{
			FDownloadInfo pInfo; 
			pInfo.m_DownloadUrl = pszURL; 
			pInfo.m_RSSInfo.m_Guid = RSSGuidFromString(pszURL); 
			g_Objects._DownloadDispatcher.OpenMTTFromHTTP(pInfo); 
			bNavigate = FALSE; 
		}
		else
		if (StrExt.Find(".xml") != -1)
		{
			g_MainFrame->AddFeedDialog(pszURL, "");
			bNavigate = FALSE; 
		}

		//Democracy
		//?url1=http%3A//feeds.feedburner.com/comedycentral/motherload
		FString Query = url.GetExtraInfo();
		FString FeedURL; 
		if (Query.Find("?url1=http%3A//") == 0)	//Democracy
		{
			FeedURL = Query; 
			FeedURL.Replace("?url1=http%3A//", "http://");
		}
		else
		if (Query.Find("?skin=rss") == 0)		//Blip.tv
		{
			FeedURL = pszURL; 
		}

		if (FeedURL.GetLength() > 0)
		{
			bNavigate = FALSE; 	
			DWORD dwChannelID = g_MainFrame->AddFeedDialog(FeedURL, ""); 
			if (dwChannelID > 0)
			{
				g_MainFrame->m_Container->SwitchActiveView(VIEW_FEEDS, TRUE); 
				g_MainFrame->m_Container->m_pFeeds->GoToChannel(dwChannelID);
			}
		}
	}
	
	if (!bNavigate)
		OnDocumentComplete(dwID, TRUE); 
	
	if (strnicmp(pszURL, "javascript:", 11) != 0)
	{
		g_MainFrame->ShowLoading(TRUE); 
	}
	
	m_bNavigating = bNavigate; 

	return bNavigate; 
}

void FChannelGuide::OnDownloadBegin(DWORD dwID)
{
}

void FChannelGuide::OnProgressChange(DWORD dwID, long lCurrent, long lMax)
{
	FString StrProgress; 
	if (lMax > 0 || lCurrent > 0 && lMax >= lCurrent)
	{
		dword lPercent = min((dword)(100.0f * lCurrent / lMax), 100);
		if (lPercent > 0)
		{
			StrProgress.Format("Opening... (%d%%)", lPercent); 
			g_MainFrame->m_iStatusBar.SetStatusText(0, StrProgress, VIEW_BROWSER); 
		}
	}
	else
	{
		if (lCurrent == -1)
		{
			g_MainFrame->m_iStatusBar.SetStatusText(0, "Done", VIEW_BROWSER); 
		}
	}
}

void FChannelGuide::SetStation(const tchar* pszStation)
{
	m_StationURL = pszStation; 
	Navigate(m_StationURL); 
}

LRESULT FChannelGuide::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	FIEWindow::OnCreate(uMsg, wParam, lParam, bHandled); 

	DWORD dwNavFlag = 0; 
    FString StrURL = _RegStr("LastNav"); 

    if (StrURL.GetLength() == 0)
        StrURL = g_AppSettings.ChannelGuideUrl(NULL);

	if (g_firstLoad)
	{
        StrURL = g_AppSettings.ChannelGuideUrl(NULL); 
		dwNavFlag = navNoHistory | navNoReadFromCache;
	}
    Navigate(StrURL);
	
	return 0; 
}

HRESULT FChannelGuide::Navigate(const tchar* pszURL, DWORD dwNavFlags)
{
	ShowLoading(TRUE); 
	return FIEWindow::Navigate(pszURL, "", dwNavFlags); 
}

LRESULT FChannelGuide::OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
{
    USES_CONVERSION;
    CComBSTR bstrLoc; 
    if (SUCCEEDED(m_pBrowser->get_LocationURL(&bstrLoc)))
    {
        FString StrCurUrl = OLE2T(bstrLoc.m_str); 
        _RegSetStr("LastNav", StrCurUrl);	  
		g_FirstDoc = TRUE; 
    }

	return 0;
}

void FChannelGuide::SetWBFocus()
{
	CComQIPtr<IWebBrowser2> pWB2 = m_pBrowser; 
	if (pWB2)
	{
		CComPtr<IDispatch> pDisp; 

		if (SUCCEEDED(pWB2->get_Document(&pDisp)) && pDisp != NULL)
		{
			CComQIPtr<IHTMLDocument2> pDoc = pDisp; 
			if (pDoc)
			{
				CComQIPtr<IHTMLElement> pEle; 

				if (SUCCEEDED(pDoc->get_body(&pEle) && pEle != NULL))
				{
					CComQIPtr<IHTMLElement2> pBody2 = pEle; 
					if (pBody2)
					{
						pBody2->focus(); 
					}
				}
			}
		}
	}
}

LRESULT FChannelGuide::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SetWBFocus(); 
	return 0; 
}

void FChannelGuide::OnDocumentComplete(DWORD dwID, BOOL bMainFrame)
{
	FIEWindow::OnDocumentComplete(dwID, bMainFrame); 
	m_bNotFound = FALSE; 
	ShowLoading(FALSE); 
	g_FirstDoc = FALSE; 

	if (!g_pAppManager->IsConnectable())
		ShowConnectibleWarning(TRUE);

	SetWBFocus();
	
	m_bNavigating = FALSE; 
	g_MainFrame->m_iStatusBar.SetStatusText(0, "Done", VIEW_BROWSER); 
}

void FChannelGuide::OnToolbarCommand(UINT nCmd)
{
	switch(nCmd)
	{
	case TOOLBAR_IDC_BACK:
		m_pBrowser.m_spWB->GoBack(); 
		break; 
	case TOOLBAR_IDC_FORWARD:
		m_pBrowser.m_spWB->GoForward(); 
		break; 
	case TOOLBAR_IDC_REFRESH:
		Navigate(g_AppSettings.ChannelGuideUrl(""), navNoHistory | navNoReadFromCache);
		break; 
	}
}

void FChannelGuide::Refresh(DWORD dwFlag)
{
	CComVariant vt = dwFlag; 
	if (m_pBrowser.IsWindow())
	{
		m_pBrowser->Refresh2(&vt); 
		//ShowLoading(TRUE); 
	}
}

void FChannelGuide::ShowLoading(BOOL bShow)
{
	m_bShowLoading = bShow;
	m_pBrowser.ShowWindow(m_bShowLoading ? SW_HIDE : SW_SHOW);
	g_MainFrame->ShowLoading(bShow); 
	
}

void FChannelGuide::ShowEpisodeDetails(ULONG ulEpisodeID)
{
	FString UrlRel; 
	UrlRel.Format("?id_episode=%u",ulEpisodeID); 
	Navigate(g_AppSettings.ChannelGuideUrl(UrlRel)); 
}

void FChannelGuide::ShowConnectibleWarning(BOOL bShow)
{
	m_pBrowser.CallJScript("showConnectibleWarning", bShow ? "1" : "0"); 
}

enum CGMenuItems{
	miCut = 1,
	miCopy, 
	miPaste,
	miSelectAll,
	miClear
};

int FChannelGuide::ShowContextMenu()
{

	if (!m_pBrowser.pDocument)
		return 0; 


	HMENU hmenu = CreatePopupMenu();
	int i = 0; 

	_InsertMenuItem(hmenu, 0, miCut,   "Cut");
	_InsertMenuItem(hmenu, 1, miCopy,  "Copy");
	_InsertMenuItem(hmenu, 2, miPaste, "Paste");

	CComPtr<IHTMLElement> pElement; 
	m_pBrowser.pDocument->get_activeElement(&pElement);

	BOOL bDisablePaste = TRUE; 
	if (pElement)
	{
		CComQIPtr<IHTMLInputElement> pInput = pElement; 
		VARIANT_BOOL bReadOnly = VARIANT_TRUE; 

		if (pInput)
		{
			if (SUCCEEDED(pInput->get_readOnly(&bReadOnly)))
				bDisablePaste = bReadOnly == VARIANT_TRUE ? TRUE : FALSE; 
		} else
		{
			CComQIPtr<IHTMLTextAreaElement> pTextArea = pElement; 
			if (pTextArea)
			{
				if (SUCCEEDED(pTextArea->get_readOnly(&bReadOnly)))
					bDisablePaste = bReadOnly == VARIANT_TRUE ? TRUE : FALSE; 
			}
			else
				bDisablePaste = TRUE; 
		}
	}

	if (bDisablePaste)
	{
		EnableMenuItem(hmenu, miCut, MF_BYCOMMAND | MF_GRAYED); 
		EnableMenuItem(hmenu, miPaste, MF_BYCOMMAND | MF_GRAYED); 
	}

	POINT pt;
	GetCursorPos(&pt);

	int nCmd = TrackPopupMenu(hmenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL);
	CComVariant vtVal = 0; 
	VARIANT_BOOL vtRet; 

	switch(nCmd)
	{
	case miCut:
		m_pBrowser.pDocument->execCommand(L"Cut", VARIANT_FALSE, vtVal, &vtRet);
		break;
	case miCopy:
		m_pBrowser.pDocument->execCommand(L"Copy", VARIANT_FALSE, vtVal, &vtRet);
		break; 
	case miPaste:
		m_pBrowser.pDocument->execCommand(L"Paste", VARIANT_FALSE, vtVal, &vtRet); 
		break; 
	}

	return nCmd; 

}

void FChannelGuide::OnActivated(BOOL bActivated)
{
	if (bActivated)
		m_pBrowser.CallJScript("onActivated"); 
	else
		m_pBrowser.CallJScript("onDeactivated"); 
}

///////////////////////////////////////////////////////////////////////////

































