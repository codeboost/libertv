#include "stdafx.h"
#include "FWebWindow.h"
#include "AppSettings.h"
#include <strsafe.h>

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif



void GradientFillRect (HDC hdc, RECT* R, COLORREF StartColor, COLORREF EndColor, int Direction)
{
    TRIVERTEX verts[2]	= { 0 };
    verts[0].x = R->left; 
    verts[0].y = R->top; 
    verts[0].Red = (GetRValue(StartColor) << 8) & 0xff00; 
    verts[0].Green = (GetGValue(StartColor) << 8) & 0xff00; 
    verts[0].Blue = (GetBValue(StartColor) << 8) & 0xff00;
    verts[0].Alpha = 0; 

    verts[1].x = R->right; 
    verts[1].y = R->bottom; 
    verts[1].Red = (GetRValue(EndColor) << 8) & 0xff00  ; 
    verts[1].Green = (GetGValue(EndColor) << 8) & 0xff00 ; 
    verts[1].Blue = (GetBValue(EndColor) << 8) & 0xff00;
    verts[1].Alpha = 0; 

    GRADIENT_RECT gr; 
    gr.UpperLeft = 0;
    gr.LowerRight = 1;

    GradientFill(hdc, verts, 2, &gr, 1, Direction);
}

HRESULT FIEWindow::Navigate(const tchar* pszPath, const tchar* pTargetFrameName, DWORD dwNavFlags)
{
    m_lMax = m_lCur = 0; 
	const int c_headerLength = 32; 
	char VersionHeader[c_headerLength];
	StringCbPrintf(VersionHeader, c_headerLength, LTV_APP_NAME": %s", g_AppSettings.m_AppVersion); 
    return m_pBrowser.Navigate(pszPath, VersionHeader, dwNavFlags); 
	//SetTimer(1, 1000);
}

LRESULT FIEWindow::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
	FRect r; 
	GetClientRect(&r); 

    dword dwStyle = GetWindowLong(GWL_STYLE); 

    m_pLoadingBmp.Attach((HBITMAP)LoadImage(NULL, g_AppSettings.AppDir("data/images/img_splash.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE));
		 
	if (m_pLoadingDC)
	{
		m_pLoadingDC.SelectBitmap(NULL);
		m_pLoadingDC.DeleteDC();
	}

	m_pLoadingDC.CreateCompatibleDC(CWindowDC(m_hWnd));
	m_pLoadingDC.SelectBitmap(m_pLoadingBmp);

    dword dwChildStyle = dwStyle;
    dwChildStyle&=~WS_VISIBLE; 
	m_pBrowser.Create( m_hWnd, r, "about:blank", 
		WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN , NULL, 1 ); 

	m_pBrowser.SetWindowText("FIEBrowser"); 

    dwStyle&=~(WS_HSCROLL | WS_VSCROLL);
    SetWindowLong(GWL_STYLE, dwStyle | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
    m_pBrowser.SetNotify(this, 0); 
	
	return 0; 
}

LRESULT FIEWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_hWndNotify = NULL; 
	m_pLoadingDC.SelectBitmap(NULL); 
	m_pLoadingDC.DeleteDC();
	m_pLoadingBmp.DeleteObject(); 
	return 0; 
}

LRESULT FIEWindow::OnSize(UINT, WPARAM, LPARAM, BOOL&)
{
	if (m_pBrowser.IsWindow() && m_pBrowser.IsWindowVisible())
	{
		RECT rcWindow; GetClientRect( &rcWindow );
		m_pBrowser.MoveWindow( &rcWindow, TRUE );
	}
	else
	{
		Invalidate(); 
		UpdateWindow(); 
	}
	return 1; 
}

void FIEWindow::OnNavigateComplete(DWORD dwID, const tchar* pszURL)
{
	_DBGAlert("Navigate complete\n");
}

void FIEWindow::OnDocumentComplete(DWORD dwID, BOOL bMainFrame)
{
	_DBGAlert("Document complete\n");
	RECT rcWindow; GetClientRect( &rcWindow );
	m_pBrowser.MoveWindow( &rcWindow, TRUE );

	if (!m_pBrowser.IsWindowVisible())
	{
		m_IgnoreShowWnd = TRUE; //Make sure we don't call OnActivated() again
		m_pBrowser.ShowWindow(SW_SHOW); 
		m_IgnoreShowWnd = FALSE; 
	}

	::SendMessage(m_hWndNotify, WM_DOCUMENT_COMPLETE, (WPARAM)m_hWnd, 0); 

	if (IsViewActive())
		m_pBrowser.SetBodyFocus();
}

BOOL FIEWindow::OnNavigateError(DWORD dwID, BOOL b, const tchar* w)
{
    ::SendMessage(m_hWndNotify, WM_FRAME_LOAD_ERROR, (WPARAM)w, (LPARAM)b); 
    return false; 
}                    

void FIEWindow::OnProgressChange(DWORD dwID, long lCurrent, long lMax)
{
    if (m_bShowLoading)
    {
          m_lMax = lMax; 
          m_lCur = min(lCurrent, m_lMax); 
          Invalidate(); 
          UpdateWindow(); 
    }
}


LRESULT FIEWindow::OnTimer(UINT, WPARAM, LPARAM, BOOL&)
{
	m_bShowLoading = TRUE; 
	OnProgressChange(0, ++m_lCur, 16);
    return 0; 
}

LRESULT FIEWindow::OnPaint(UINT, WPARAM, LPARAM, BOOL&)
{
    CPaintDC PaintDC(m_hWnd); 

    FRect r; 
    GetClientRect(&r); 
    CBrush aBrush; 
	
	CDC dcCompat; 

	dcCompat.CreateCompatibleDC(PaintDC);
	CBitmap bmCompat; 

	bmCompat.CreateCompatibleBitmap(PaintDC, r.Width(), r.Height()); 
	HBITMAP oldObj = dcCompat.SelectBitmap(bmCompat);

	//e9e9dd
    aBrush.CreateSolidBrush(RGB(0xe9, 0xe9, 0xdd)); 
    dcCompat.FillRect(&r, aBrush); 

	if (m_bShowLoading)
	{
		if (m_pLoadingBmp)
		{
			SIZE size; 
			m_pLoadingBmp.GetSize(size); 

			int xP = r.Width() / 2 - size.cx / 2; 
			int yP = r.Height() / 2 - size.cy / 2; 
			dcCompat.BitBlt(xP, yP, size.cx, size.cy, m_pLoadingDC, 0, 0, SRCCOPY); 
		}
		PaintDC.BitBlt(0, 0, r.Width(), r.Height(), dcCompat, 0, 0, SRCCOPY); 
	}
	dcCompat.SelectBitmap(oldObj); 
    return 1; 
}

LRESULT FIEWindow::OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CComPtr<IHTMLElement> pEle;
	if (m_pBrowser.pDocument)
	{
		if (SUCCEEDED(m_pBrowser.pDocument->get_body(&pEle)) && pEle != NULL)
		{	
			CComQIPtr<IHTMLElement2> pBody = pEle; 
			if (pBody)
				pBody->focus();			
		}
	}
	return 0; 
}

LRESULT FIEWindow::OnKillFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0; 
}

LRESULT FIEWindow::OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//call virtual function
	///derived must save/load it's state and call the appropriate Javascript function
	if (!m_IgnoreShowWnd)
		OnActivated((BOOL)wParam);

	return 0; 
}

void FIEWindow::SetErrorPage(const char* pStrErrUrl)
{

}

BOOL FIEWindow::IsPageLoaded()
{
	READYSTATE rs; 
	if (SUCCEEDED(m_pBrowser->get_ReadyState(&rs)))
	{
		return rs == READYSTATE_COMPLETE; 
	}
	return FALSE; 
}