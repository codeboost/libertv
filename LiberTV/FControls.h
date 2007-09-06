#pragma once

#include <atlgdi.h>
#include <atlmisc.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atldlgs.h>


const int gPenWidth = 1;
const int gLineDistance = 0;

#define FPROGRESS_MOUSEDOWN 0x01


#define PROG_DRAW_EDGE 0x01			//Edge around control
#define PROG_LARGER_CURRENT 0x02	//Current lines will be larger
#define PROG_GROWING 0x04			//Growing lines

class FProgress :  public CWindowImpl<FProgress, CWindow, 
    CWinTraits<WS_CHILD | WS_POPUPWINDOW, 0> > 
{
    BEGIN_MSG_MAP(FProgress)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd);
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove);
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor);
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouseButton); 
		MESSAGE_HANDLER(WM_LBUTTONUP, OnMouseButton)
    END_MSG_MAP()

protected:
	//indices:
	//0 = range; 1 = available; 2 = current
	WTL::CPen	m_Pens[3];	 
	WTL::CBrush m_BgBrush; 
	double		m_dblVals[3]; 
	HCURSOR		m_Cursors[2];
	HCURSOR		m_CurrentCursor; 
	int			m_CurCursorIndex; 
	dword		m_dwFlags; 
	POINT		m_LastMousePt; 
protected:
	inline void SetCurrentFromMax(double dblValue, double dblMax) 
	{
		m_dblVals[2] = (dblValue / dblMax) * m_dblVals[0];
		if (m_dblVals[2] < 0.0)
			m_dblVals[2] = 0.0; 
	}

	inline void SetCurrentValue(double dblCurrent)
	{
		m_dblVals[2] = dblCurrent; 
	}

	inline void SetAvailValue(double dblAvail)
	{
		m_dblVals[1] = dblAvail; 
	}

	inline void SetMax(double dblMax)
	{
		m_dblVals[0] = dblMax;
	}

	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL& bHandled);
	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnSetCursor(UINT, WPARAM, LPARAM lHit, BOOL& bHandled);
	LRESULT OnMouseMove(UINT, WPARAM, LPARAM lParam, BOOL&);
	LRESULT OnMouseButton(UINT message, WPARAM, LPARAM lParam, BOOL&);
	void	DrawMeter(CDC& dc, FRect& rcPaint);

public:
	FProgress()
	{
		m_dblVals[0] = 1.0; 
		m_dblVals[1] = 0; 
		m_dblVals[2] = 0; 
		m_CurCursorIndex = 0; 
		m_dwFlags =0; 
		m_LastMousePt.x = m_LastMousePt.y = 0; 
	}
	~FProgress();
	inline double GetAvail(double dblValue)  {return (m_dblVals[1] / m_dblVals[0]) * dblValue;}
	inline double GetCurrent(double dblValue){return (m_dblVals[2] / m_dblVals[0]) * dblValue;}
	inline double GetRange(){return m_dblVals[0];}
	void	SetRange(double dblMax){m_dblVals[0] = dblMax <= 0 ? 1.0 : dblMax;}
	void	SetValue(double dblAvail, double dblCurrent);
	void	SetColors(COLORREF cRange, COLORREF cAvail, COLORREF cCurrent);
	void	SetBgColor(COLORREF cBg);
};

//---------------------------------------

struct FInfoLine
{
	FString m_Text; 
};
class FInfoPanel :  public CWindowImpl<FInfoPanel, CWindow, 
    CWinTraits<WS_CHILD | WS_POPUPWINDOW, 0> > 
{
   CFont   m_fntText; 
   std::vector<FInfoLine> m_Lines; 
   int	   m_LineSpacing; 

    BEGIN_MSG_MAP(FInfoPanel)
        MESSAGE_HANDLER(WM_CREATE, OnCreate);
        MESSAGE_HANDLER(WM_PAINT, OnPaint);
        MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd);
    END_MSG_MAP()
    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {

		m_LineSpacing = 1; 
		CreateLines(1); 
        m_fntText.CreateFont( 14, 0, 0, 0,
            FW_NORMAL, 0, 0, 0,DEFAULT_CHARSET,
            OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
            DEFAULT_PITCH|FF_DONTCARE, "Arial");


        return 0; 
    }

public:

	void CreateLines(int nLines)
	{
		m_Lines.resize(nLines);
	}
	void SetLineSpacing(int nLineSpacing)
	{
		m_LineSpacing = nLineSpacing ;
	}

	void SetText(int nLine, const char* pStrText)
	{
		if (!IsWindow())
			return; 

		if (nLine < (int)m_Lines.size())
		{
			m_Lines[nLine].m_Text = pStrText;
		}
		else
		{
			assert(nLine < (int)m_Lines.size());
		}
		InvalidateRect(NULL, TRUE); 
	}
	
	void FormatText(int nLine, const char* pFormat, ...)
	{
		if (!IsWindow())
			return; 

		va_list paramList;
		va_start(paramList, pFormat);
		char Temp[4096];
		vsprintf(Temp, pFormat, paramList); 
		SetText(nLine, Temp); 
		va_end(paramList);
	}

    LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
		int LineSpacing = m_LineSpacing; 

        CPaintDC PaintDC(m_hWnd); 
        RECT rcPaint; 
        GetClientRect(&rcPaint); 
        SelectObject(PaintDC, m_fntText); 
        PaintDC.FillRect(&rcPaint, COLOR_BTNTEXT);
        PaintDC.SetTextColor(RGB(255, 255, 255));
        PaintDC.SetBkMode(TRANSPARENT);

		int nLineH = rcPaint.bottom - rcPaint.top; 
		if (m_Lines.size() > 0)
		{
			nLineH = (rcPaint.bottom - rcPaint.top) / (int)m_Lines.size() - (int)m_Lines.size() * LineSpacing; 	
		}
		

		int top = 0; 
		for (int k = 0; k < (int)m_Lines.size(); k++)
		{
			rcPaint.top = top; 
			rcPaint.bottom = top + nLineH;
			PaintDC.DrawText(m_Lines[k].m_Text, (int)m_Lines[k].m_Text.GetLength(), &rcPaint, DT_LEFT | DT_VCENTER | DT_SINGLELINE );
			top+=nLineH + LineSpacing; 
		}

        bHandled = TRUE; 
        return 0; 
    }

    LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL& bHandled)
    {
        bHandled = TRUE; 
        return 0; 
    }
};
