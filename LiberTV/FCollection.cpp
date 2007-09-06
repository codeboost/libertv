#include "stdafx.h"
#include "FCollection.h"
#include "AppSettings.h"
#include "GlobalObjects.h"
#include "FDlgInputBox.h"
#include "FMainFrame.h"
#include "atlutil.h"
#include "FCollectionMenu.h"
#include "FDlgSaveVideo.h"
#include "FCollectionWindow.h"

extern FAutoWindow<FMainFrame> g_MainFrame; 


///////////////////////////////////////////////////////////////////////////
static int idMenu = 0; 

static FCollectionWindow* g_CollWindow = NULL; 


FCollectionDisplay::FCollectionDisplay()
{
	m_iView = COLLECTION_VIEW_THUMBS; 
	m_dwCollFlags = 0; 
}

void FCollectionDisplay::CycleView()
{
	if (m_iView == COLLECTION_VIEW_THUMBS)
		SetView(COLLECTION_VIEW_LIST);
	else
		SetView(COLLECTION_VIEW_THUMBS);
}
void FCollectionDisplay::SetView(int nView)
{
	m_iView = nView; 

	if (nView == COLLECTION_VIEW_LIST)
	{
		if (g_CollWindow == NULL)
		{
			g_CollWindow = new FCollectionWindow;
			g_CollWindow->Create(m_hWnd, rcDefault, "CollectionLV", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0);
			m_hWndListView = *g_CollWindow; 
		}
		::SetWindowPos(*g_CollWindow, HWND_TOP, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
		g_CollWindow->ShowWindow(SW_SHOW); 
		g_CollWindow->SetFilters(m_StrCurrentLabel, m_StrSearchString, m_dwCollFlags); 
		BOOL bHandled; 
		OnSize(WM_SIZE, 0, 0, bHandled);
		g_CollWindow->SetFocus(); 
	}
	else
	{
		if (g_CollWindow != NULL)
			g_CollWindow->ShowWindow(SW_HIDE); 
		m_pBrowser.SetBodyFocus();
	}
	
	m_pBrowser.CallJScript("onViewChanged", LongToStr((long)m_iView));
	_RegSetDword("LastCollectionView", nView); 
}

void FCollectionDisplay::OnLabelChanged(const char* pszNewLabel)
{
	m_StrCurrentLabel = pszNewLabel; 
	if (m_iView == COLLECTION_VIEW_LIST &&
		g_CollWindow && g_CollWindow->IsWindow() && g_CollWindow->IsWindowVisible())
	{
		g_CollWindow->SetFilters(m_StrCurrentLabel, m_StrSearchString, m_dwCollFlags); 
	}
}

void FCollectionDisplay::OnSearchStringChanged(const char* pszNewFilter)
{
	m_StrSearchString = pszNewFilter; 
	if (m_iView == COLLECTION_VIEW_LIST &&
		g_CollWindow && g_CollWindow->IsWindow() && g_CollWindow->IsWindowVisible())
	{
		g_CollWindow->SetFilters(m_StrCurrentLabel, m_StrSearchString, m_dwCollFlags); 
	}
}

void FCollectionDisplay::OnFlagsChanged(DWORD dwCollFlags)
{
	m_dwCollFlags = dwCollFlags; 
	if (m_iView == COLLECTION_VIEW_LIST &&
		g_CollWindow && g_CollWindow->IsWindow() && g_CollWindow->IsWindowVisible())
	{
		g_CollWindow->SetFilters(m_StrCurrentLabel, m_StrSearchString, m_dwCollFlags); 
	}
}

class FHTMLElement
{
	CComPtr<IHTMLElement> m_pEle; 
public:
	FHTMLElement(CComPtr<IHTMLElement>& pEle): m_pEle(pEle)
	{

	}

	long offsetTop(){
		long top = 0; 
		m_pEle->get_offsetTop(&top); 
		return top; 
	}
	long offsetLeft()
	{
		long left = 0; 
		m_pEle->get_offsetLeft(&left); 
		return left; 
	}
	long clientTop()
	{
		long top = 0;
		CComQIPtr<IHTMLElement2> pEle = m_pEle; 
		if (pEle)
		{
			pEle->get_clientTop(&top); 
		}
		return top; 
	}

	long clientLeft()
	{
		long left = 0;
		CComQIPtr<IHTMLElement2> pEle = m_pEle; 
		if (pEle)
		{
			pEle->get_clientTop(&left); 
		}
		return left; 
	}
	long scrollTop()
	{
		long top = 0; 
		CComQIPtr<IHTMLElement2> pEle = m_pEle; 
		if (pEle)
		{
			pEle->get_scrollTop(&top); 
		}
		return top; 
	}

	long scrollLeft()
	{
		long left = 0; 
		CComQIPtr<IHTMLElement2> pEle = m_pEle; 
		if (pEle)
		{
			pEle->get_scrollTop(&left); 
		}
		return left;
	}
	long offsetWidth()
	{
		long width = 0; 
		m_pEle->get_offsetWidth(&width); 
		return width; 
	}
	long offsetHeight()
	{
		long height = 0; 
		m_pEle->get_offsetHeight(&height); 
		return height; 
	}

	long top()
	{
		return offsetTop() + clientTop(); 
	}
	long left()
	{
		return offsetLeft() + clientLeft(); 
	}
};

void GetParentOffsets(CComPtr<IHTMLElement> obj, FRect& rc)
{
	long left, top, width, height; 
	left = top = width = height = 0; 
	FHTMLElement myEle(obj); 
	left = myEle.left(); 
	top = myEle.top(); 

	CComPtr<IHTMLElement> parent; 
	while (SUCCEEDED(obj->get_offsetParent(&parent)) && parent != NULL)
	{
		FHTMLElement myParent(parent); 
		left+=myParent.left(); 
		top+=myParent.top(); 
		obj = parent; 
		parent.Release(); 
	}

	rc.left = left; 
	rc.right = left + myEle.offsetWidth(); 
	rc.top = top; 
	rc.bottom = top + myEle.offsetHeight(); 
}

BOOL FCollectionDisplay::GetCollectionDivRect(FRect &rc)
{
	if (!IsPageLoaded())
		return FALSE; 
	
	if (!m_pBrowser.pDocument)
		return FALSE; 

	CComPtr<IHTMLElementCollection> pAll; 
	CComPtr<IHTMLDocument2> pDoc = m_pBrowser.pDocument;
	CComQIPtr<IHTMLElement> pBody = m_pBrowser;
	
	HRESULT hr = pDoc->get_all(&pAll);
	
	if (SUCCEEDED(hr) && pAll != NULL)
	{
		CComPtr<IDispatch> pDispDiv; 
		hr = pAll->item(CComVariant("collection_div"), CComVariant(0), &pDispDiv);
		if (SUCCEEDED (hr) && pDispDiv != NULL)
		{
			CComQIPtr<IHTMLElement> pDiv = pDispDiv; 
			if (pDiv)
			{
				SetRectEmpty(&rc); 
				/*
				Determine the rect in client coordinates.
				left = body.left + div.offset_left;
				top = body.top + div.offset_top;
				right = body.left + div.offset_width;
				bottom = body.top + div.offset_height; 
				*/
				GetParentOffsets(pDiv, rc); 
				return TRUE; 
			}
		}
	}
	return FALSE; 
}

LRESULT FCollectionDisplay::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	FIEWindow::OnSize(uMsg, wParam, lParam, bHandled); 

	if (m_iView == COLLECTION_VIEW_LIST && g_CollWindow != NULL && g_CollWindow->IsWindow())
	{
		FRect r; 
		FRect rWnd; 
		GetClientRect(&rWnd); 
		if (GetCollectionDivRect(r))
		{
			r.right = rWnd.right; 
			r.bottom = rWnd.bottom; 
			g_CollWindow->MoveWindow(&r, TRUE); 
		}
	}
	return 0; 
}

LRESULT FCollectionDisplay::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	OnActivated(FALSE); 
	if (g_CollWindow)
		g_CollWindow->DestroyWindow(); 

	delete g_CollWindow;
	g_CollWindow = NULL; 
	return 0; 
}

void FCollectionDisplay::OnActivated(BOOL bActive)
{
	if (bActive == FALSE)
	{
		m_pBrowser.CallJScript("onDeactivated"); 
	}
	else
	{
		m_pBrowser.CallJScript("onActivated"); 
		m_pBrowser.SetBodyFocus();
	}
}

int FCollectionDisplay::ShowContextMenu(const FArray<FString> &aVids)
{
	FCollectionMenu men; 
	return men.ShowContextMenu(m_hWnd, aVids); 
}

BOOL FCollectionDisplay::IsViewActive()
{
	return (g_MainFrame->GetCurrentView() == VIEW_STATUS);
}

FString FCollectionDisplay::ShowAddLabelDlg(const char* pszLabel)
{
	return FCollectionMenu::ShowAddLabelDlg(m_hWnd, pszLabel); 
}

int FCollectionDisplay::GetView()
{
	return m_iView;
}
//////////////////////////////////////////////////////////////////////////

LRESULT FFeedsDisplay::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	FIEWindow::OnCreate(uMsg, wParam, lParam, bHandled);
	g_Objects._RSSManager->SetNotify(this); 
	return 0; 
}

void FFeedsDisplay::OnActivated(BOOL bActive)
{
	if (bActive == FALSE)
	{
		FString SplitPos = m_pBrowser.CallScriptStr("getSplitterPosition", ""); 
		FString LastChId = m_pBrowser.CallScriptStr("getSelectedChannelId", ""); 

		_RegSetStr("FeedsSplitterPos", SplitPos); //will be returned to ExecCmd("GetSplitterPosition")
		g_AppSettings.m_dwLastChannelId = strtoul(LastChId, NULL, 10); 
		m_pBrowser.CallJScript("onDeactivated"); 
	}
	else
		m_pBrowser.CallJScript("onActivated"); 
}

LRESULT FFeedsDisplay::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	g_Objects._RSSManager->SetNotify(NULL); 
	OnActivated(FALSE); 
	return 0; 
}

LRESULT FFeedsDisplay::OnMsgFeedRefresh(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	FString StrChID;
	StrChID.Format("%u", wParam);
	FString StrState; 
	StrState.Format("%u", LOWORD(lParam));
	FString StrNewItems;
	StrNewItems.Format("%u", HIWORD(lParam));
	HRESULT hr = m_pBrowser.CallJScript("onChannelRefresh", StrChID, StrState, StrNewItems); 
	_DBGAlert("On channel refresh (%s, %s, %s): 0x%x\n", StrChID, StrState, StrNewItems, hr); 
	return 0; 
}

void FFeedsDisplay::OnChannelRefresh(DWORD dwChannelID, DWORD dwState, DWORD dwNewItems)
{
	PostMessage( WM_FEED_REFRESH, (WPARAM)dwChannelID, MAKELPARAM(dwState, dwNewItems)); 
}

BOOL FFeedsDisplay::OnBeforeNavigate(DWORD dwID, const tchar* pszURL)
{
	if (strstr(pszURL, "rssfeeds.html") == 0){
		ShellExecute(NULL, "open", pszURL, "", "", SW_SHOW); 
		return FALSE; 
	}
	return TRUE; 
}

void FFeedsDisplay::GoToChannel(DWORD dwChannelID)
{
	char strId[16];
	StringCbPrintf(strId, 16, "%d", dwChannelID); 
	m_pBrowser.CallJScript("goToChannelById", strId);
}

BOOL FFeedsDisplay::IsViewActive()
{
	return g_MainFrame->GetCurrentView() == VIEW_FEEDS;
}

BOOL FFeedsDisplay::OnSetAutoDownload()
{
	if (!PathIsDirectory(g_AppSettings.m_DownloadsFolder))
	{
		FDlgSaveVideo DlgSave;
		FDlgSaveVideo::SaveDlgOpenParams Params(&g_AppSettings); 

		int nRes = DlgSave.Open(m_hWnd, Params);
		if (nRes == IDCANCEL)
			return FALSE; 
		g_AppSettings.SaveSettings(); 
	}
	return TRUE; 
}