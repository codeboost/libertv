#include "stdafx.h"
#include "FToolbar.h"


#define FLAG_PRESSED 0x01
#define FLAG_MOUSEOVER 0x02


void FImageToolbar::SetImageLimits(int xOffset, int yOffset, int xSpacing)
{
	m_xOffs = xOffset; 
	m_yOffs = yOffset; 
	m_xSpacing = xSpacing; 
}

BOOL FImageToolbar::AddButtonFromFile(int nButtonID, const char* szFileName)
{
	m_Buttons.Add();
	FButton& rButton = m_Buttons[m_Buttons.GetCount() - 1];
	rButton.m_idi = nButtonID; 
	rButton.m_Image.Load(szFileName); 
	return !rButton.m_Image.IsNull();
}

BOOL FImageToolbar::AddButtonFromRes(int nButtonID, int nRes)
{
	m_Buttons.Add();
	FButton& rButton = m_Buttons[m_Buttons.GetCount() - 1];
	rButton.m_idi = nButtonID; 
	rButton.m_Image.LoadFromResource(g_Instance, nRes); 
	return rButton.m_Image.IsNull();
}

void FImageToolbar::RedrawToolbar()
{
	FRect rcClient; 
	GetClientRect(&rcClient); 
	int xOffs = m_xOffs; 

	int xTotal = 0; 
	for (size_t k = 0; k < m_Buttons.GetCount(); k++)
	{
		if (!m_Buttons[k].m_Image.IsNull())
		{
			xTotal+=m_Buttons[k].m_Image.GetWidth();
			xTotal+=m_xSpacing;
		}
	}

	//xOffs = rcClient.Width() / 2 - xTotal / 2; 
	xOffs = m_xOffs; 

	for (size_t k = 0; k < m_Buttons.GetCount(); k++)
	{
		if (!m_Buttons[k].m_Image.IsNull())
		{
			int ImageWidth = m_Buttons[k].m_Image.GetWidth();
			int ImageHeight = m_Buttons[k].m_Image.GetHeight();

			m_Buttons[k].m_Placement.left = xOffs; 
			m_Buttons[k].m_Placement.right = xOffs + ImageWidth; 
			m_Buttons[k].m_Placement.top = m_yOffs; 
			m_Buttons[k].m_Placement.bottom = m_yOffs + ImageHeight;

			xOffs+=ImageWidth;
			xOffs+=m_xSpacing;
		}
	}
}

BOOL FImageToolbar::SetBgImage(const tchar* szFileName)
{
	if (SUCCEEDED(m_bgBitmapLoad.Load(szFileName)))
	{
		//GenerateBgBitmap(3200);		
		return TRUE; 
	}
	return FALSE; 
}

void FImageToolbar::GenerateBgBitmap(int wndWidth)
{
	if (!m_bgBitmap.IsNull())
		m_bgBitmap.DeleteObject();

	if (m_bgBitmapLoad.IsNull())
		return ; 

	int count = wndWidth / m_bgBitmapLoad.GetWidth() + 1; 
	int srcWidth = m_bgBitmapLoad.GetWidth(); 

	CClientDC dcCompat(m_hWnd); 

	m_bgBitmap.CreateCompatibleBitmap(dcCompat, wndWidth, m_bgBitmapLoad.GetHeight()); 
	dcCompat.SelectBitmap(m_bgBitmap); 

	//Copy the source bitmap into the compatible bitmap count times
	for (int k = 0; k < count; k++)
	{
		m_bgBitmapLoad.BitBlt(dcCompat, srcWidth * k, 0, SRCCOPY); 
	}
}

#define IDI_SEARCH 9999

LRESULT FImageToolbar::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
	m_Lastbutton = -1; 
	SetImageLimits(0, 0, 5); 
    m_bgBrush.CreateSolidBrush(RGB(255, 255, 255));

    //m_EdtSearch.Create(m_hWnd, rcDefault, "EdtSearch", WS_CHILD | WS_VISIBLE, 0, IDI_SEARCH, NULL); 

    //m_EdtFont.CreatePointFont(80, "Arial"); 
    //m_EdtSearch.SetFont(m_EdtFont, TRUE); 
	return 0; 
}

LRESULT FImageToolbar::OnSize(UINT, WPARAM, LPARAM, BOOL&)
{
    RedrawToolbar(); 

    int SearchWidth = 100; 
    int SearchHeight = 0; 
    FRect r; 
    GetClientRect(&r); 

    r.left = r.right - SearchWidth - 2; 
    r.right-=2; 
    r.top = r.Height() / 2 - SearchHeight / 2; 
    r.bottom = r.top + SearchHeight; 

   // m_EdtSearch.MoveWindow(r, TRUE); 
    
    return 0;
}

LRESULT FImageToolbar::OnPaint(UINT, WPARAM, LPARAM, BOOL&)
{
	CPaintDC PaintDC(m_hWnd); 
	CDC dcCompat; 
	WTL::CRect rcClient; 
	GetClientRect(&rcClient); 
		 
	dcCompat.CreateCompatibleDC(PaintDC);
	CBitmap bmCompat; 
	bmCompat.CreateCompatibleBitmap(PaintDC, rcClient.Width(), rcClient.Height()); 
	dcCompat.SelectBitmap(bmCompat);
	dcCompat.FillRect(&rcClient, m_bgBrush);

	if (!m_bgBitmapLoad.IsNull())
	{
		//Copy the source bitmap into the compatible bitmap count times
		for (int k = 0; k < rcClient.Width() / m_bgBitmapLoad.GetWidth() + 1; k++)
		{
			m_bgBitmapLoad.BitBlt(dcCompat, m_bgBitmapLoad.GetWidth() * k, 0, SRCCOPY); 
		}
	}


	for (size_t k = 0; k < m_Buttons.GetCount(); k++)
	{
		if (!m_Buttons[k].m_Image.IsNull())
		{
			POINT ptImage = {m_Buttons[k].m_Placement.left, m_Buttons[k].m_Placement.top};

			m_Buttons[k].m_Image.Draw(dcCompat, ptImage); 
			//m_Buttons[k].m_Image.TransparentBlt(dcCompat, ptImage)

			if (m_Buttons[k].m_dwFlags & FLAG_PRESSED)
			{
				DrawEdge(dcCompat, &m_Buttons[k].m_Placement, EDGE_SUNKEN, BF_RECT); 
			}
		}
	}

	DrawEdge(dcCompat, &rcClient, BDR_RAISEDINNER, BF_BOTTOM);

	PaintDC.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), dcCompat, 0, 0, SRCCOPY); 

	return 1; 
}

int FImageToolbar::GetButtonID(POINT &pt)
{
	for (size_t k = 0; k < m_Buttons.GetCount(); k++)
	{
		if (PtInRect(&m_Buttons[k].m_Placement, pt))
			return (int)k;
	}
	return -1; 
}

LRESULT FImageToolbar::OnMouse(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL&)
{

	POINT ptMouse = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};

	int nButton = GetButtonID(ptMouse); 

	switch(uMsg)
	{
	case WM_MOUSEMOVE:
		{
			if (nButton >= 0)
			{
				if (m_Lastbutton != nButton)
				{
					SetCapture(); 
					SetCursor(LoadCursor(NULL, IDC_HAND));
					m_Lastbutton = nButton; 
				}
			}
			else
			{
				if (m_Lastbutton != -1)
				{
					m_Buttons[m_Lastbutton].m_dwFlags&=~(FLAG_MOUSEOVER | FLAG_PRESSED);
					InvalidateRect(&m_Buttons[m_Lastbutton].m_Placement);
					m_Lastbutton = -1; 
					ReleaseCapture();
				}
			}
		}
		break; 
	case WM_LBUTTONDOWN:
		{
			if (nButton >=0)
			{
				m_Buttons[nButton].m_dwFlags&=~FLAG_MOUSEOVER;
				m_Buttons[nButton].m_dwFlags|=FLAG_PRESSED; 
				m_Lastbutton = nButton;
				InvalidateRect(&m_Buttons[nButton].m_Placement);
			}
		}
		break; 
	case WM_LBUTTONUP:
		{
			if (nButton >=0)
			{
				m_Buttons[nButton].m_dwFlags|=FLAG_MOUSEOVER;			
				m_Buttons[nButton].m_dwFlags&=~FLAG_PRESSED; 
				InvalidateRect(&m_Buttons[nButton].m_Placement);
				OnButtonPressed(nButton); 
			}

		}
		break; 
	}
	return 0; 
}

void FImageToolbar::OnButtonPressed(int nButton)
{
	SendMessage(GetParent(), WM_COMMAND, MAKEWPARAM(m_Buttons[nButton].m_idi, nButton), (LPARAM)m_hWnd);
}