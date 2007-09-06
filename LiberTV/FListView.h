#ifndef __FLISTVIEW_H__
#define __FLISTVIEW_H__
/*
	Flicker-free list view implementation
*/
//Double Buffer Window
class FDoubleBufferWindow : public CWindowImpl<FDoubleBufferWindow>
{
	//TODO: Optimize this class

public:
	BEGIN_MSG_MAP(FDoubleBufferWindow);
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
	END_MSG_MAP(); 
	LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL){return 1;}
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		FRect rcWnd; GetClientRect(&rcWnd); 
		CPaintDC PaintDC(m_hWnd); CBitmap bmCompat; bmCompat.CreateCompatibleBitmap(PaintDC, rcWnd.Width(), rcWnd.Height()); 
		m_dcCompat.SelectBitmap(bmCompat);

		CallWindowProc(m_pfnSuperWindowProc, m_hWnd, WM_ERASEBKGND, (WPARAM)m_dcCompat.m_hDC, 0); 
		CallWindowProc(m_pfnSuperWindowProc, m_hWnd, WM_PAINT, (WPARAM)m_dcCompat.m_hDC, 0);
		
		PaintDC.BitBlt(0, 0, rcWnd.Width(), rcWnd.Height(), m_dcCompat, 0, 0, SRCCOPY); 
		bHandled = TRUE; 
		return 1; 
	}

	bool EnableDoubleBuffering(HWND hWindow)
	{
		SubclassWindow(hWindow); 
		CPaintDC PaintDC(m_hWnd); 
		m_dcCompat.CreateCompatibleDC(PaintDC);
		return true; 
	}
protected:
	CDC		m_dcCompat;
	
};

class FListView
{
	FDoubleBufferWindow  m_LVArea; 
	FDoubleBufferWindow  m_HeaderArea; 
public:
	bool EnableDoubleBuffering(HWND hWndList)
	{
		m_LVArea.EnableDoubleBuffering(hWndList); 
		m_HeaderArea.EnableDoubleBuffering(ListView_GetHeader(hWndList)); 
		return true; 
	}
};



#endif //__FLISTVIEW_H__