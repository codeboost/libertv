#pragma once
#include "stdafx.h"
#include "FControls.h"

//////////////////////////////////////////////////////////////////////////

FProgress::~FProgress()
{
	//DeleteObject(m_Cursors[0]);
	//DeleteObject(m_Cursors[1]);
}

LRESULT FProgress::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
	RECT rcThis; 
	GetClientRect(&rcThis); 
	m_Cursors[0] = LoadCursor(NULL, IDC_ARROW);
	m_Cursors[1] = LoadCursor(NULL, IDC_HAND); 
	m_BgBrush.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
	return 0; 
}

void FProgress::SetColors(COLORREF cRange, COLORREF cAvail, COLORREF cCurrent)
{
	m_Pens[0].CreatePen(PS_SOLID, gPenWidth, cRange); //Range
	m_Pens[1].CreatePen(PS_SOLID, gPenWidth, cAvail); //Available
	m_Pens[2].CreatePen(PS_SOLID, gPenWidth, cCurrent);	  //Current
}

void FProgress::SetBgColor(COLORREF cBg)
{
	m_BgBrush.DeleteObject();
	m_BgBrush.CreateSolidBrush(cBg); 
}

LRESULT FProgress::OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	bHandled = TRUE; 
	return 0; 
}

LRESULT FProgress::OnPaint(UINT, WPARAM, LPARAM, BOOL&)
{
	CPaintDC PaintDC(m_hWnd); 
	FRect rcPaint, rcClient; 
	GetClientRect(&rcClient); 
	rcPaint = rcClient; 
	CDC dcCompat;	 dcCompat.CreateCompatibleDC(PaintDC);
	CBitmap bmCompat;bmCompat.CreateCompatibleBitmap(PaintDC, rcPaint.Width(), rcPaint.Height()); 
	dcCompat.SelectBitmap(bmCompat);

	dcCompat.FillRect(&rcPaint, m_BgBrush);

	dword dwStyle = GetWindowLong(GWL_STYLE); 
	if (dwStyle & PROG_DRAW_EDGE)
		dcCompat.DrawEdge(&rcPaint, EDGE_ETCHED, BF_RECT | BF_ADJUST);

	DrawMeter(dcCompat, rcPaint); 
	PaintDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), dcCompat, 0, 0, SRCCOPY); 
	return 0; 
}

void FProgress::DrawMeter(CDC& dc, FRect& rcPaint)
{

	//fill the unavailable 
	int nLines = rcPaint.Width() / (gLineDistance + gPenWidth) + 1; 

	int nAvailLines = (int)GetAvail((double)nLines) ;
	int nCurrLines  = (int)GetCurrent((double)nLines);
	
	assert(nAvailLines >= nCurrLines);

	int x = rcPaint.left; 
	int y = rcPaint.top; 

	dc.MoveTo(x, y); 

	int lh = rcPaint.Height() - 0; 
	int mh = lh; 

	int nPenIndex = 2; 
	dc.SelectPen(m_Pens[nPenIndex]); 
	dword dwStyle = GetWindowLong(GWL_STYLE); 

	for (int k = 0; k < nLines; k++)
	{
		if (k == nCurrLines)
		{
			if (k < nAvailLines)
				dc.SelectPen(m_Pens[1]);	
			else
				dc.SelectPen(m_Pens[0]);
			if (dwStyle & PROG_LARGER_CURRENT)
			{
				lh = lh / 2; 
				y += lh / 2; 
				dc.MoveTo(x, y);  
			}
		}
		else if (k == nAvailLines)
		{
			dc.SelectPen(m_Pens[0]); 
		}

		if (dwStyle & PROG_GROWING)
		{
			double dblNow = (double)k / (double)nLines; 
			lh = (int)(mh * dblNow); 
			y = mh - lh; 
			dc.MoveTo(x, y); 
		}

		dc.LineTo(x, y + lh);
		x += gLineDistance; 
		x += gPenWidth;
		dc.MoveTo(x, y); 
	}
}

LRESULT FProgress::OnSetCursor(UINT, WPARAM, LPARAM lHit, BOOL& bHandled)
{
	bHandled = TRUE; 

	if (GetCursor() != m_Cursors[m_CurCursorIndex])
	{
		SetCursor(m_Cursors[m_CurCursorIndex]); 
	}
	return 1;  
}

LRESULT FProgress::OnMouseMove(UINT, WPARAM, LPARAM lParam, BOOL&)
{
	POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
	FRect rClient; 

	GetClientRect(&rClient);
	int AvailPx = (int)GetAvail(rClient.Width());

	if (pt.x <= AvailPx)
		m_CurCursorIndex = 1;	//Hand
	else
		m_CurCursorIndex = 0;	//Arrow

	if (m_dwFlags & FPROGRESS_MOUSEDOWN)
	{
		if (pt.x > rClient.Width())
			pt.x = rClient.Width(); 

		SetCurrentFromMax((double)pt.x, (double)rClient.Width());
		InvalidateRect(NULL, TRUE); 
		SendMessage(GetParent(), WM_HSCROLL, MAKEWPARAM(GetMenu(), TB_THUMBPOSITION), (LPARAM)m_hWnd);
	}

	return 0; 
}

LRESULT FProgress::OnMouseButton(UINT message, WPARAM, LPARAM lParam, BOOL&)
{
	if (message == WM_LBUTTONDOWN)
	{
		POINT pt = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
		FRect rClient; 
		GetClientRect(&rClient);
		int AvailPx = (int)GetAvail(rClient.Width());
		if (pt.x < AvailPx)
		{
			if (m_hWnd != GetCapture())
			{
				SetCapture();
				m_dwFlags|=FPROGRESS_MOUSEDOWN; 

				SetCurrentFromMax((double)pt.x, (double)rClient.Width()); 
				
				InvalidateRect(NULL, TRUE); 
				SendMessage(GetParent(), WM_HSCROLL, MAKEWPARAM(GetMenu(), TB_THUMBPOSITION), (LPARAM)m_hWnd);
				m_LastMousePt = pt; 
			}
		}
	}
	else
		if (message == WM_LBUTTONUP)
		{
			m_LastMousePt.x = m_LastMousePt.y = 0; 
			ReleaseCapture(); 
			m_dwFlags&=~FPROGRESS_MOUSEDOWN; 
		}
		return 0; 
}

void FProgress::SetValue(double dblAvail, double dblCurrent)
{
	m_dblVals[1] = dblAvail < m_dblVals[0] ? dblAvail : m_dblVals[0]; 
	m_dblVals[2] = dblCurrent < m_dblVals[0] ? dblCurrent : m_dblVals[0]; 
	InvalidateRect(NULL, TRUE); 
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
