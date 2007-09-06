#ifndef __CAPTIONPAINTER_H__
#define __CAPTIONPAINTER_H__

#pragma once

/////////////////////////////////////////////////////////////////////////////
// CCaptionPainter - Implementation of custom drawn captions
//
// See Q99046 for more information.
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2001 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//

#ifndef __cplusplus
  #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef __ATLAPP_H__
  #error CaptionPainter.h requires atlapp.h to be included first
#endif


/////////////////////////////////////////////////////////////////////////////
// Generic Caption Painter class

template< class T >
class CCaptionPainter
{
public:
   BEGIN_MSG_MAP(CCaptionPainter)
      MESSAGE_HANDLER(WM_NCPAINT, OnNcPaint)
      MESSAGE_HANDLER(WM_NCACTIVATE, OnNcActivate)
      MESSAGE_HANDLER(WM_SETTEXT, OnSetText)
      MESSAGE_HANDLER(WM_SYSCOLORCHANGE, OnColorChange)
      MESSAGE_HANDLER(WM_SETTINGCHANGE, OnColorChange)
   END_MSG_MAP()

   bool m_bDefaultMinimzed;
   BOOL m_bActive;

   CCaptionPainter()
   {
      m_bDefaultMinimzed = true;
   }

   LRESULT OnNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      T* pT = static_cast<T*>(this);
      BOOL bActive = (BOOL)wParam;
      
	  //if( bActive == m_bActive ) return 1;

      // A well-described hack by the Windows guru Paul DiLascia.
      // Must call DefWindowProc() - but it repaints the original caption - 
      // so we turn off the visible flag for a brief period.
      DWORD dwStyle = pT->GetStyle();
      if( dwStyle & WS_VISIBLE ) pT->SetWindowLong(GWL_STYLE, (dwStyle & ~WS_VISIBLE));
      pT->DefWindowProc(uMsg, wParam, lParam);
      if( dwStyle & WS_VISIBLE ) pT->SetWindowLong(GWL_STYLE, dwStyle);

      m_bActive = bActive;
      pT->SendMessage(WM_NCPAINT);

      return 1;
   }

   LRESULT OnSetText(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      T* pT = static_cast<T*>(this);
      DWORD dwStyle = pT->GetStyle();
      if( dwStyle & WS_VISIBLE ) pT->SetWindowLong(GWL_STYLE, (dwStyle & ~WS_VISIBLE));
      LRESULT lRes = pT->DefWindowProc(uMsg, wParam, lParam);
      if( dwStyle & WS_VISIBLE ) pT->SetWindowLong(GWL_STYLE, dwStyle);
      pT->SendMessage(WM_NCPAINT);
      pT->Invalidate();
      return lRes;
   }

   LRESULT OnColorChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
   {
      T* pT = static_cast<T*>(this);
      pT->Invalidate();
      pT->SendMessage(WM_NCPAINT);
      return 0;
   }

   LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
   {
      T* pT = static_cast<T*>(this);
      
      if( m_bDefaultMinimzed && pT->IsIconic() ) {
         // Let Windows paint minimized taskbar stuff
         bHandled = FALSE;
         return 0;
      }

      HRGN hRgn = (HRGN) wParam;
      RECT rcCap, rcCapScreen;
      GetCaptionRect(&rcCap, &rcCapScreen);

      // Don't bother painting if the caption doesn't lie within the region.
      if( (WORD) hRgn > 1 && !::RectInRegion(hRgn, &rcCapScreen) ) {
         bHandled = FALSE;
         return 0;
      }

      // Exclude caption from update region
      HRGN hRgnCaption = ::CreateRectRgnIndirect(&rcCapScreen);
      HRGN hRgnNew     = ::CreateRectRgnIndirect(&rcCapScreen);
      if( (WORD) hRgn > 1 ) {
        // wParam is a valid region: subtract caption from it
        ::CombineRgn(hRgnNew, hRgn, hRgnCaption, RGN_DIFF);
      } 
      else {
        // wParam is not a valid region: create one that's the whole
        // window minus the caption bar
        RECT rcWin;
        pT->GetWindowRect(&rcWin);
        HRGN hRgnAll = ::CreateRectRgnIndirect(&rcWin);
        ::CombineRgn(hRgnNew, hRgnAll, hRgnCaption, RGN_DIFF);
        ::DeleteObject(hRgnAll);
      }

      // Call Windows to do WM_NCPAINT with altered update region
      pT->DefWindowProc(uMsg, (WPARAM)hRgnNew, lParam);
      ::DeleteObject(hRgnCaption);
      ::DeleteObject(hRgnNew);

      // Now paint my special caption
      pT->DoPaintCaption(hRgn, rcCap); 
      return 0;
   }

protected:
   void GetCaptionRect(RECT *rcClient=NULL, RECT *prcScreen=NULL)
   {
      T* pT = static_cast<T*>(this);
      DWORD dwStyle = pT->GetStyle();
      SIZE szFrame;
      szFrame.cx = ::GetSystemMetrics( dwStyle & WS_THICKFRAME ? SM_CXSIZEFRAME : SM_CXFIXEDFRAME );
      szFrame.cy = ::GetSystemMetrics( dwStyle & WS_THICKFRAME ? SM_CYSIZEFRAME : SM_CYFIXEDFRAME );
      RECT rc;
      pT->GetWindowRect(&rc);
      rc.left += szFrame.cx;
      rc.right -= szFrame.cx;
      rc.top += szFrame.cy;
      rc.bottom = rc.top + ::GetSystemMetrics(SM_CYCAPTION) - ::GetSystemMetrics(SM_CYBORDER); 
      if( prcScreen!=NULL ) *prcScreen = rc;
      ::OffsetRect(&rc, -rc.left+szFrame.cx, -rc.top+szFrame.cy);
      if( rcClient != NULL ) *rcClient = rc;
   }
};


/////////////////////////////////////////////////////////////////////////////
// CFlatCaptionPainter

template<class T>
class CFlatCaptionPainter : public CCaptionPainter<T>
{
public:
   BEGIN_MSG_MAP(CFlatCaptionPainter)
      MESSAGE_HANDLER(WM_CREATE, OnCreate)
      MESSAGE_HANDLER(WM_SETTINGCHANGE, OnSettingsChange)
      MESSAGE_HANDLER(WM_NCHITTEST, OnNcHitTest)
      MESSAGE_HANDLER(WM_NCLBUTTONDOWN, OnNcLButtonDown)
      if( m_bHasCapture ) {
         MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
         MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
         MESSAGE_HANDLER(WM_CAPTURECHANGED, OnCaptureChanged)
      }
      CHAIN_MSG_MAP(CCaptionPainter<T>)
   END_MSG_MAP()

   CFont m_fontCaption;
   DWORD m_dwStyle;
   int m_iButtonPress;
   bool m_bHasCapture;

   CFlatCaptionPainter()
      : m_iButtonPress(0), m_bHasCapture(false)
   {
      m_bDefaultMinimzed = false;
      BOOL bDummy;
      OnSettingsChange(0,0,0,bDummy);
   }

   // Message handlers

   LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
   {
      T* pT = static_cast<T*>(this);
      m_dwStyle = pT->GetStyle();
      bHandled = FALSE; // Let other handlers get a shot too
      return 0;
   }

   LRESULT OnSettingsChange(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
   {
      NONCLIENTMETRICS ncm = { 0 };
      ncm.cbSize = sizeof(ncm);
      ::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0);
      if( !m_fontCaption.IsNull() ) m_fontCaption.DeleteObject();
      m_fontCaption.CreateFontIndirect(&ncm.lfCaptionFont);
      bHandled = FALSE; // Let other handlers get a shot too
      return 0;
   }

   LRESULT OnNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
   {
      T* pT = static_cast<T*>(this);
      LRESULT lRes = pT->DefWindowProc(uMsg, wParam, lParam);
      switch( lRes ) {
      case HTCAPTION:
      case HTCLOSE:
      case HTMAXBUTTON:
      case HTMINBUTTON:
      case HTSYSMENU:
         break;
      default:
         return lRes;
      }
      
      POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
      RECT rcCaption;
      GetCaptionRect(NULL, &rcCaption);
      RECT rc;
      if( m_dwStyle & WS_SYSMENU ) {
         GetButtonRect(rcCaption, WS_SYSMENU, rc);
         if( ::PtInRect(&rc, pt) ) return HTCLOSE; // Return HTSYSMENU to have system menu instead
      }
      if( m_dwStyle & WS_MAXIMIZEBOX ) {
         GetButtonRect(rcCaption, WS_MAXIMIZEBOX, rc);
         if( ::PtInRect(&rc, pt) ) return HTMAXBUTTON;
      }
      if( m_dwStyle & WS_MINIMIZEBOX ) {
         GetButtonRect(rcCaption, WS_MINIMIZEBOX, rc);
         if( ::PtInRect(&rc, pt) ) return HTMINBUTTON;
      }
      return HTCAPTION;
   }

   LRESULT OnNcLButtonDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
   {     
      T* pT = static_cast<T*>(this);
      switch( wParam ) {
      case HTCLOSE:
      case HTMINBUTTON:
      case HTMAXBUTTON:
         m_iButtonPress = (int)wParam;
         pT->SetCapture();
         m_bHasCapture = true;
         pT->SendMessage(WM_NCPAINT);
         return 0;
      case HTCAPTION:
         if( pT->GetStyle() & WS_MINIMIZE ) return 0; // BUG: Prevents move of iconic window, but fixes Windows freeze
         break;
      }
      bHandled = FALSE; // Let other handlers get a shot too
      return 0;
   }

   LRESULT OnNcLButtonUp(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
   {
      T* pT = static_cast<T*>(this);
      pT->SendMessage(WM_NCPAINT);
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnLButtonUp(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
   {
      T* pT = static_cast<T*>(this);
      // Turn off capture now
      int iButtonPress = m_iButtonPress;
      // The ReleaseCapture() will result in a OnCaptureChanged() call
      ::ReleaseCapture();
      // Check if button was hit
      POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
      pT->ClientToScreen(&pt);
      int iHit = (int)pT->SendMessage(WM_NCHITTEST, 0, MAKELPARAM(pt.x,pt.y));
      if( iHit==iButtonPress ) {
         switch( iHit ) {
         case HTCLOSE:
            pT->PostMessage(WM_CLOSE);
            break;
         case HTSYSMENU:
            // Is handled by Windows
            break;
         case HTMAXBUTTON:
            pT->ShowWindow( (pT->GetStyle() & WS_MAXIMIZE ? SW_RESTORE : SW_MAXIMIZE ));
            break;
         case HTMINBUTTON:
            pT->ShowWindow( (pT->GetStyle() & WS_MINIMIZE ? SW_RESTORE : SW_MINIMIZE ));
            break;
         }
      }
      return 0;
   }

   LRESULT OnCaptureChanged(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
   {
      T* pT = static_cast<T*>(this);
      m_bHasCapture = false;
      m_iButtonPress = 0;
      pT->SendMessage(WM_NCPAINT);
      bHandled = FALSE;
      return 0;
   }

   LRESULT OnMouseMove(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& /*bHandled*/)
   {
      T* pT = static_cast<T*>(this);
      POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
      pT->ClientToScreen(&pt);
      int iHit = (int)pT->SendMessage(WM_NCHITTEST, 0, MAKELPARAM(pt.x,pt.y));
      switch( m_iButtonPress ) {
      case HTCLOSE:
      case HTSYSMENU:
      case HTMAXBUTTON:
      case HTMINBUTTON:
         // Did mouse move outside control
         if( iHit != m_iButtonPress ) {
            m_iButtonPress = 0;
            pT->SendMessage(WM_NCPAINT);
         }
         break;
      }
      return 0;
   }

   // Helper functions

   void DoPaintCaption(HRGN /*hRgn*/, RECT &rect)
   {
      T* pT = static_cast<T*>(this);
      
      CWindowDC dc(pT->m_hWnd);
      RECT rc;

      dc.FillRect(&rect, ::GetSysColorBrush(COLOR_3DFACE));
      if( m_dwStyle & WS_SYSMENU ) {
         GetButtonRect(rect, WS_SYSMENU, rc);
         int iFlags = DFCS_FLAT|DFCS_CAPTIONCLOSE;
         if( m_iButtonPress==HTCLOSE ) iFlags |= DFCS_PUSHED;
         dc.DrawFrameControl(&rc, DFC_CAPTION, iFlags);
      }
      if( m_dwStyle & WS_MINIMIZEBOX ) {
         GetButtonRect(rect, WS_MINIMIZEBOX, rc);
         int iFlags = DFCS_FLAT;
         if( m_iButtonPress==HTMINBUTTON ) iFlags |= DFCS_PUSHED;
         iFlags |= ( pT->GetStyle() & WS_MINIMIZE ? DFCS_CAPTIONRESTORE : DFCS_CAPTIONMIN );
         dc.DrawFrameControl(&rc, DFC_CAPTION, iFlags);
      }
      if( m_dwStyle & WS_MAXIMIZEBOX ) {
         GetButtonRect(rect, WS_MAXIMIZEBOX, rc);
         int iFlags = DFCS_FLAT;
         if( m_iButtonPress==HTMAXBUTTON ) iFlags |= DFCS_PUSHED;
         iFlags |= ( pT->GetStyle() & WS_MAXIMIZE ? DFCS_CAPTIONRESTORE : DFCS_CAPTIONMAX );
         dc.DrawFrameControl(&rc, DFC_CAPTION, iFlags);
      }
      int nHeight = rect.bottom-rect.top;
      rect.left += 4;
      if( m_dwStyle & WS_SYSMENU ) rect.left += nHeight;
      if( m_dwStyle & WS_MINIMIZEBOX ) rect.right -= nHeight;
      if( m_dwStyle & WS_MAXIMIZEBOX ) rect.right -= nHeight;
      TCHAR szCaption[80]; // Q99046 tells us max length is 78 chars
      pT->GetWindowText(szCaption, sizeof(szCaption)/sizeof(TCHAR));
      dc.SetBkMode(TRANSPARENT);
      HFONT fontOld = dc.SelectFont(m_fontCaption);
      dc.DrawText(szCaption, ::lstrlen(szCaption), &rect, DT_CENTER | DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
      dc.SelectFont(fontOld);
   }

   void GetButtonRect(RECT &rcCaption, int iButton, RECT &rc)
   {
      int nHeight = rcCaption.bottom-rcCaption.top;
      switch( iButton ) {
		case WS_MINIMIZEBOX:      
         ::SetRect(&rc, rcCaption.left, rcCaption.top, rcCaption.left+nHeight, rcCaption.bottom);
         break;
	  case WS_MAXIMIZEBOX:
         ::SetRect(&rc, rcCaption.right-(nHeight*2), rcCaption.top, rcCaption.right-(nHeight*1), rcCaption.bottom);
         break;
	  case WS_SYSMENU:
	 	 ::SetRect(&rc, rcCaption.right-(nHeight*1), rcCaption.top, rcCaption.right-(nHeight*0), rcCaption.bottom);
         break;
      }
      ::InflateRect(&rc, -1,-1);
   }
};


#endif //__CAPTIONPAINTER_H__
