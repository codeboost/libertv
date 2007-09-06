#include "stdafx.h"
#include "FCollectionListView.h"
#include "FMenu.h"

void FGradientFillRect (HDC hdc, RECT* R, COLORREF StartColor, COLORREF EndColor, int Direction)
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

void GradientFillHeader(HDC hdc, RECT* r, COLORREF StartColor, COLORREF EndColor)
{
	FRect r1 = *r; 
	/*r1.bottom /= 2;
	FGradientFillRect(hdc, &r1, StartColor, EndColor, GRADIENT_FILL_RECT_V);
	r1.top = r1.bottom; 
	r1.bottom = r->bottom; 
	*/
	FGradientFillRect(hdc, &r1, EndColor, StartColor, GRADIENT_FILL_RECT_V);
}


void FHeaderCtrl::Init()
{
	m_PaintDC.CreateCompatibleDC(CWindowDC(m_hWnd));
	//m_PaintDC.SelectBitmap(m_pLoadingBmp);

	m_GradientStart = RGB(100, 100, 100);
	m_GradientEnd =  RGB(130, 130, 130);
	m_GradientSelStart = RGB(171, 171, 171);
	m_GradientSelEnd = RGB(100, 100, 100); 
}

LRESULT FHeaderCtrl::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{

	//We need to paint the remaining header column on the right
	//We compute the rect of all items and what's left is the column that must be painted.
	FRect rcItems;

	rcItems.SetRect(0, 0, 0, 0); 

	int iItemCount = GetItemCount(); 

	for (int k = 0; k < iItemCount; k++)
	{
		FRect rcCol; 
		GetItemRect(k, &rcCol); 
		if (rcItems.right < rcCol.right)
			rcItems.right = rcCol.right;
	}

	FRect rcHeader; 
	GetClientRect(&rcHeader); 
	if (rcItems.right < rcHeader.right)
	{
		//Must clear the HDS_BUTTONS style before painting, because the header
		//draws a raised frame if HDS_BUTTON is set
		SetWindowLongPtr(GWL_STYLE, GetStyle() & ~HDS_BUTTONS);
		DefWindowProc(uMsg, wParam, lParam); 
		SetWindowLongPtr(GWL_STYLE, GetStyle() | HDS_BUTTONS);
		CPaintDC paintDC(m_hWnd); 
		rcHeader.left = rcItems.right; 

		int ScrollBarWidth = GetSystemMetrics(SM_CXVSCROLL);
		rcHeader.right -= ScrollBarWidth; 
		GradientFillHeader(paintDC, &rcHeader, m_GradientStart, m_GradientEnd);
		paintDC.FillSolidRect(rcHeader.right, rcHeader.top, ScrollBarWidth, rcHeader.Height(), RGB(0x3e, 0x38, 0x38)); 
	}
	else
	{
		DefWindowProc(uMsg, wParam, lParam); 
	}
	bHandled = TRUE; 
	return TRUE; 
}

LRESULT FHeaderCtrl::OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return DefWindowProc(uMsg, wParam, lParam);
}

void FCollectionListView::Init()
{
	m_uiColFlag = 0; 
	m_hSortIconUp = NULL; 
	m_hSortIconDown = NULL; 

	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	if ( SUCCEEDED(AtlGetCommCtrlVersion(&dwMajor, &dwMinor)) && dwMajor >= 6 )
	{
		_bIsCC6 = true;
	}


	m_fntHeader.CreateFont( 14, 0, 0, 0,
		FW_BOLD, 0, 0, 0,DEFAULT_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH|FF_SWISS, "Arial");

	WTL::CHeaderCtrl ctlHeader = GetHeader();


	ATLASSERT(::IsWindow(ctlHeader));
	ctlHeader.SetWindowLongPtr(GWLP_ID, IDC_HDRFRAGMENTS);
	ctlHeader.SetWindowLongPtr(GWL_STYLE, HDS_FULLDRAG | HDS_FLAT | (ctlHeader.GetStyle()));
	m_hHeader.SubclassWindow(ctlHeader.Detach()); 
	SetExtendedListViewStyle((_bIsCC6 ? LVS_EX_DOUBLEBUFFER : 0)|LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_INFOTIP | LVS_EX_LABELTIP | LVS_EX_UNDERLINECOLD | LVS_EX_UNDERLINEHOT, 0);
	m_hHeader.SetBitmapMargin(0); 


}

void FCollectionListView::SetSortIcon(HICON hIcon, BOOL bUp)
{
	if (bUp)
		m_hSortIconUp = hIcon; 
	else
		m_hSortIconDown = hIcon; 
}

LRESULT FCollectionListView::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	SaveColumnSetup(); 
	return 0; 
}

BOOL FCollectionListView::ShowHeaderContextMenu( )
{
	HMENU hmenu = CreatePopupMenu();

	UINT uiFlag = _RegDword("Columns Flag", COL_REG_KEY, 0); 
	for (int iCol = 0; iCol < GetHeader().GetItemCount(); iCol++)
	{
		ColDef& cd = m_LVColumns[iCol];
		int iColWidth = GetColumnWidth(iCol); 
		_InsertCheckedMenuItem(hmenu, iCol, iCol + 1 , iColWidth > 0, cd.m_Text); 
	}

	POINT pt;
	GetCursorPos(&pt);

	int nCmd = TrackPopupMenu(hmenu, TPM_RETURNCMD, pt.x, pt.y, 0, m_hWnd, NULL);
	if (nCmd > 0)
	{
		int iCol = nCmd - 1; 
		int iCurWidth = GetColumnWidth(iCol); 
		if (iCurWidth == 0)	//column hidden
			iCurWidth = m_LVColumns[iCol].m_iWidth;	//restore default width
		else
			iCurWidth = 0; 

		SetColumnWidth(iCol, iCurWidth); 

		if (iCurWidth > 0)
			m_uiColFlag|= (1 << iCol);
		else
			m_uiColFlag&=~(1 << iCol);

		return TRUE; 
	}
	return FALSE; 
}

void FCollectionListView::AddColumn(const char* pszColumn, int iWidth, int iCol)
{
	if (iCol == -1)
		iCol = GetItemCount(); 

	if (CListViewCtrl::AddColumn(pszColumn, iCol) != -1)
	{
		SetColumnWidth(iCol, iWidth); 
	}
}

BOOL FCollectionListView::LoadColumnSetup()
{
	int iHeaderCount = (int)m_LVColumns.GetCount(); 
	int* aColOrder = new int[iHeaderCount];

	ATLASSERT(iHeaderCount > 0); 

	for (int iCol = 0; iCol < iHeaderCount; iCol++)
	{
		FString StrCol; 
		StrCol.Format("col_%d", iCol); 

		FString StrColValue = _RegStr(StrCol, COL_REG_KEY); 
		int iColWidth = 0; 
		int iColOrder = 0;  
		int iScanned = sscanf(StrColValue, "%d,%d", &iColWidth, &iColOrder); 
		if (iScanned < 2)
		{
			iColWidth = m_LVColumns[iCol].m_iWidth;
			iColOrder = iCol; 
		}

		AddColumn(m_LVColumns[iCol].m_Text, iColWidth, iCol); 
		LVCOLUMN lvColumn = {0};
		lvColumn.mask = LVCF_FMT;
		lvColumn.fmt = m_LVColumns[iCol].m_dwTextFormat;
		SetColumn(iCol, &lvColumn); 

		if (iColWidth > 0)
			m_uiColFlag|= (1 << iCol);

		aColOrder[iCol] = iColOrder; 
	}

	SetColumnOrderArray(iHeaderCount, aColOrder); 
	delete[] aColOrder;
	return TRUE; 
}

BOOL FCollectionListView::SaveColumnSetup()
{
	int iHeaderCount = GetHeader().GetItemCount();
	ATLASSERT(iHeaderCount > 0); 
	int* aColOrder = new int[iHeaderCount];
	GetColumnOrderArray(iHeaderCount, aColOrder); 
	for (int iCol = 0; iCol < iHeaderCount; iCol++)
	{
		FString StrCol; 
		StrCol.Format("col_%d", iCol); 
		FString StrColValue;
		StrColValue.Format("%d,%d", GetColumnWidth(iCol), aColOrder[iCol]);
		_RegSetStr(StrCol, StrColValue, COL_REG_KEY); 
	}
	delete[] aColOrder;
	return TRUE; 
}

LRESULT FCollectionListView::OnBeginTrack(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	//Prevent hidden columns from being resized
	HD_NOTIFY   *pHDN = (HD_NOTIFY*)pnmh;
	int iIndex = pHDN->iItem; 
	if ((m_uiColFlag & (1 << iIndex)) == 0)
	{
		//Column hidden
		bHandled = TRUE; 
		return TRUE; 
	}
	return 0; 
}

LRESULT FCollectionListView::OnHeaderRightClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	ShowHeaderContextMenu();
	return 0; 
}



LRESULT FCollectionListView::OnHeaderCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMCUSTOMDRAW * pCD = (NMCUSTOMDRAW*)pnmh; 

	switch(pCD->dwDrawStage)
	{
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW | CDRF_NOTIFYSUBITEMDRAW;
	case CDDS_ITEMPREPAINT:
		{
			int iIndex = pCD->dwItemSpec; 
			HDITEM hdItem = {0};
			hdItem.mask = HDI_FORMAT; 
			GetHeader().GetItem(iIndex, &hdItem);



			::SetBkMode(pCD->hdc, TRANSPARENT); 
			::SelectObject(pCD->hdc, m_fntHeader); 
			
			FRect rText = pCD->rc; 
			
			if (_wSortCol == iIndex)
			{
				::SetTextColor(pCD->hdc, RGB(255, 255, 255));
				FGradientFillRect(pCD->hdc, &pCD->rc, m_hHeader.m_GradientSelStart, m_hHeader.m_GradientSelEnd, GRADIENT_FILL_RECT_V);
				HICON hDrawIcon = _bSortASC ? m_hSortIconDown : m_hSortIconUp; 
				if (hDrawIcon)
				{
					DrawIconEx(pCD->hdc, pCD->rc.right - 16, pCD->rc.top, hDrawIcon, 16, 16, 0, NULL, DI_NORMAL); 
					rText.right-=16;
					if (rText.right < rText.left) rText.right = rText.left; 
				}
			}
			else
			{
				::SetTextColor(pCD->hdc, RGB(0, 0, 0)); 
				GradientFillHeader(pCD->hdc, &pCD->rc, m_hHeader.m_GradientStart, m_hHeader.m_GradientEnd); 
			}

			FString& Text = m_LVColumns[pCD->dwItemSpec].m_Text; 
			
			DWORD dwTextFlags = 0; 

			
			InflateRect(&rText, -6, 0); 

			switch(m_LVColumns[pCD->dwItemSpec].m_dwTextFormat)
			{
				case HDF_CENTER:
					dwTextFlags=DT_CENTER; 
				break; 
				case HDF_LEFT:
					dwTextFlags = DT_LEFT;
					break; 
				case HDF_RIGHT:
					dwTextFlags = DT_RIGHT; 
					break; 
				default:
					dwTextFlags = DT_CENTER; 
			}

			DrawEdge(pCD->hdc, &pCD->rc, EDGE_RAISED, BF_RIGHT );
			DrawText(pCD->hdc, Text, Text.GetLength(), &rText, dwTextFlags | DT_VCENTER | DT_SINGLELINE);
			return CDRF_SKIPDEFAULT; 
		}
		break; 
	case CDDS_SUBITEM | CDDS_ITEMPREPAINT:
		{
			return CDRF_SKIPDEFAULT; 
		}
		break; 
	case CDDS_ITEMPOSTPAINT:
		{
			return CDRF_SKIPDEFAULT; 
		}
		break; 
	}
	return CDRF_SKIPDEFAULT; 
}

LRESULT FCollectionListView::OnHeaderSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	POINT pt; 
	GetCursorPos(&pt); 
	::ScreenToClient(m_hHeader, &pt); 
	HDHITTESTINFO lphtti = {0};
	lphtti.pt = pt; 
	m_hHeader.SendMessage(HDM_HITTEST, 0, (LPARAM)&lphtti);
	if ((lphtti.flags & HHT_ONDIVOPEN) || lphtti.flags & HHT_ONDIVIDER)
	{
		//Item hidden ? Do not change cursor
		if (!(m_uiColFlag & (1 << lphtti.iItem)))
		{
			bHandled = TRUE; 
			return 1; 
		}
	}
	return 0; 
}

LRESULT FCollectionListView::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0; 
}

LRESULT FHeaderCtrl::OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (::SendMessage(GetParent(), WM_HSETCURSOR, wParam, lParam) == 0)
		bHandled = FALSE; 
	else
	{
		bHandled = TRUE ;
		return TRUE; 
	}
	return TRUE; 
}

LRESULT FHeaderCtrl::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = TRUE; 
/*
	FRect r; 
	GetWindowRect(&r); 
	ScreenToClient(&r);
	GradientFillHeader((HDC)wParam, &r, RGB(100, 100, 100), RGB(130, 130, 130));
*/
	return FALSE; 
}

FString FCollectionListView::GetItemText(int iItem, int iSubItem)
{
	FString ItemText;
	char* pszBuf = ItemText.GetBufferSetLength(255);
	CListViewCtrl::GetItemText(iItem, iSubItem, pszBuf, 255); 
	return ItemText; 
}