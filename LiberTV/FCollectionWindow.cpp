#include "stdafx.h"
#include "FCollectionWindow.h"
#include "GlobalObjects.h"
#include "FCollectionMenu.h"
#include "FMenu.h"
#include "FMainFrame.h"

extern FAutoWindow<FMainFrame> g_MainFrame;

#define COL_NAME		1
#define COL_ADDED		2
#define COL_STATUS		3
#define COL_TOTAL_SIZE	4
#define COL_DOWNLOADED	5
#define COL_DOWN_SPEED	6
#define COL_UP_SPEED	7
#define COL_COMPLETED	8
#define COL_ETA			9


const int IconFinished = 0; 
const int IconInProgress = 1; 
const int IconPaused = 2;
const int IconRss = 3; 
const int IconRssAuto = 4; 
const int IconError = 5; 
const int IconQueued = 6; 

FCollListView m_LVParams; 



void InitColumns(FArray<ColDef>& Columns)
{
	Columns.Add(ColDef("Video name",300, HDF_LEFT)); 
	Columns.Add(ColDef("Added on",	120, HDF_LEFT)); 
	Columns.Add(ColDef("Status",	100, HDF_LEFT));//Downloading/Ready/Error
	Columns.Add(ColDef("Total Size",75,  HDF_LEFT));//Total size
	Columns.Add(ColDef("Downloaded",75,  HDF_LEFT));//Size downloaded
	Columns.Add(ColDef("Down Speed",75,  HDF_LEFT));//Download Speed (or 0)
	Columns.Add(ColDef("Up Speed",	75,  HDF_LEFT));//Upload speed (or 0)
	Columns.Add(ColDef("Completed",	75,  HDF_LEFT));//Percent complete or 100%
	Columns.Add(ColDef("ETA",		75,  HDF_LEFT));//Time to complete
}

HICON FLoadIcon(const char* pszName)
{
	FString StrPath = "data/images/icons/";
	StrPath.Append(pszName);
	HICON hIcon = (HICON)LoadImage(NULL, g_AppSettings.AppDir(StrPath), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
	ATLASSERT(hIcon != NULL); 
	return hIcon; 
}

LRESULT FCollectionWindow::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	DWORD dwMajor = 0;
	DWORD dwMinor = 0;
	
	m_dwFilterFlags = 0;

	if ( SUCCEEDED(AtlGetCommCtrlVersion(&dwMajor, &dwMinor)) && dwMajor >= 6 )
	{
		_bIsCC6 = true;
	}

	CListViewCtrl lvTmp; 
	lvTmp.Create(m_hWnd, rcDefault, "LVCollection", WS_CHILD | WS_VISIBLE  | LVS_REPORT | LVS_SHOWSELALWAYS | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, 0, (HMENU)IDC_LISTVIEW);	

	m_lvCollection.SubclassWindow(lvTmp.Detach());
	
	//m_lvCollection.SubclassWindow(m_lvCollectionSubClass);


	
	m_fntListView.CreateFont( 14, 0, 0, 0,
		FW_NORMAL, 0, 0, 0,DEFAULT_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
		DEFAULT_PITCH|FF_SWISS, "Arial");

	m_lvCollection.SetFont(m_fntListView); 
	m_lvCollection.SetTextColor(m_LVParams.m_clrTextColor);
	m_lvCollection.SetTextBkColor(m_LVParams.m_clrBackground);
	m_lvCollection.SetBkColor(m_LVParams.m_clrBackground); 


	m_lvImgList.Create(24, 24, ILC_COLOR24 | ILC_MASK, 6, 1); 

	m_lvImgList.AddIcon(FLoadIcon("complete.ico"));
	m_lvImgList.AddIcon(FLoadIcon("in_progress.ico"));
	m_lvImgList.AddIcon(FLoadIcon("paused.ico"));
	m_lvImgList.AddIcon(FLoadIcon("rss.ico"));
	m_lvImgList.AddIcon(FLoadIcon("rss_auto.ico"));
	m_lvImgList.AddIcon(FLoadIcon("error.ico"));
	m_lvImgList.AddIcon(FLoadIcon("queued.ico"));

	m_lvCollection.SetSortIcon(FLoadIcon("sort_ascending.ico"), TRUE);
	m_lvCollection.SetSortIcon(FLoadIcon("sort_descending.ico"), FALSE); 
	

	m_lvCollection.SetImageList(m_lvImgList, LVSIL_SMALL); 

	InitColumns(m_lvCollection.m_LVColumns); 
	m_lvCollection.LoadColumnSetup();
	RefreshListView();
	SetTimer(0, 500); 
	return 0; 
}

LRESULT FCollectionWindow::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0; 
}

void FormatLVTime(UINT uiTime, char* szOut, size_t count)
{
	if (uiTime > 0)
	{
		tm _newtime; 
		_localtime32_s(&_newtime, (__time32_t*)&uiTime);
		StringCbPrintf(szOut, count, "%02d/%02d/%d %02d:%02d:%02d", _newtime.tm_mon, _newtime.tm_mday, _newtime.tm_year, _newtime.tm_hour, _newtime.tm_min, _newtime.tm_sec);
	}
	else
	{
		strcpy(szOut, "*"); 
	}
}


FString SSpeed(double dblSpeed)
{
	FString Res; 
	const double dblMegaByte = 1024 * 1024;
	if (dblSpeed < 1024 * 1024)
		Res.Format("%.2f kB/s", dblSpeed / 1024);
	else
		Res.Format("%.2f MB/s", dblSpeed / dblMegaByte); 
	return Res; 
}


FString FormatSeconds(UINT seconds)
{
	UINT myDays, myHours, myMinutes, mySeconds;
	myDays = seconds / 86400; // 86400 seconds in a day
	seconds = seconds - (myDays * 86400); // Remove the days' worth of seconds from the seconds variable
	myHours = seconds / 3600; // 3600 seconds in an hour
	seconds = seconds - (myHours * 3600); // Remove hours' worth of seconds
	myMinutes = seconds / 60; // 60 seconds in a minute
	seconds = seconds - (myMinutes * 60); // Remove minutes' worth of seconds
	mySeconds = seconds; // Anything left over is < 60 seconds


	FString StrRes = "*"; 
	if (myDays > 0)
	{
		StrRes.Format("%02d d %02d h %02d m %02d s", myDays, myHours, myMinutes, mySeconds);
	}
	else
	if (myHours > 0)
	{
		StrRes.Format("%d h %02d m %02d s", myHours, myMinutes, mySeconds);
	}
	else
	if (myMinutes > 0)
	{
		StrRes.Format("%02d m %02d s", myMinutes, mySeconds);
	}
	else
	if (mySeconds > 0)
	{
		StrRes.Format("%02d s", mySeconds);
	}
	else
	{
		//--
	}
	return StrRes; 

}

BOOL FCollectionWindow::UpdateRow(UINT uiVideoId, FValueMap& pValue)
{
	//Try to find the item in the LV map
	int iIndex = MapIdToIndex(uiVideoId);
	if (iIndex == -1)
		return FALSE; 

	char szTime[64];
	char szTimeCompleted[64];
	FormatLVTime(pValue.GetValueUINT("videoTimeAdded"),		szTime, 64);
	FormatLVTime(pValue.GetValueUINT("videoTimeCompleted"),	szTimeCompleted, 64); 

	UINT64 totalDownloaded = pValue.GetValueUINT64("videoDownloaded");
	UINT64 totalSize = pValue.GetValueUINT64("videoTotalSize"); 
	double dblDownSpeed = pValue.GetValueDouble("videoTotalDownSpeed");
	UINT   uiFlags = pValue.GetValueUINT("videoFlags");

	FString StatusStr = pValue.GetValueStr("videoStatusStr");

	int iIcon = IconFinished; 
	FString StrETA = "*";

	if (uiFlags & FLAG_DOWNLOAD_ACTIVE)
	{
		totalDownloaded+=pValue.GetValueUINT64("curClipDownloaded"); 
		iIcon = IconInProgress;
		//RemainingSize in bytes/ DownloadSpeed in bytes/sec 
		double dblLeft = (double)totalSize - (double)totalDownloaded;
		if (dblDownSpeed > 0)
		{
			UINT secondsLeft = (UINT)(dblLeft / dblDownSpeed);
			StrETA = FormatSeconds(secondsLeft);
		}
	}
	else
	if (uiFlags & FLAG_DOWNLOAD_FINISHED)
	{
		iIcon = IconFinished; 
		dblDownSpeed = 0.0;
		UINT uiRSSFlags = pValue.GetValueUINT("RSSFlags"); 
		if (uiRSSFlags & RSS_FLAG_FROM_RSS)
			iIcon = IconRss; 
		if (uiRSSFlags & RSS_FLAG_FROM_AUTODOWNLOAD)
			iIcon = IconRssAuto; 
	}
	else
	if (uiFlags & FLAG_DOWNLOAD_QUEUED)
	{
		iIcon = IconQueued; 
	}
	else
	if (uiFlags & FLAG_DOWNLOAD_PAUSED)
	{
		iIcon = IconPaused; 
	}

	if (StatusStr.Find("Error") >= 0)
	{
		iIcon = IconError; 
	}


	SetColumn(iIndex, COL_NAME,			pValue.GetValueStr("videoName"));
	SetColumn(iIndex, COL_ADDED,		szTime);
	SetColumn(iIndex, COL_STATUS,		StatusStr);
	SetColumn(iIndex, COL_TOTAL_SIZE,	SizeString(totalSize));
	SetColumn(iIndex, COL_DOWNLOADED,	SizeString(totalDownloaded));
	SetColumn(iIndex, COL_DOWN_SPEED,	SSpeed(dblDownSpeed));
	SetColumn(iIndex, COL_UP_SPEED,		SSpeed(pValue.GetValueDouble("videoTotalUpSpeed")));
	SetColumn(iIndex, COL_COMPLETED,	szTimeCompleted);
	SetColumn(iIndex, COL_ETA,			StrETA);

	SetItemIcon(iIndex, iIcon); 
	return TRUE; 
}

LRESULT FCollectionWindow::OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	FRect r; 
	GetClientRect(&r); 
	m_lvCollection.MoveWindow(&r, TRUE); 
	return 0; 
}

LRESULT FCollectionWindow::OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	m_lvCollection.SetFocus(); 
	return 0; 
}

void FCollectionWindow::SetItemId(int iIndex, UINT uiVideoId)
{
	LVITEM Item = {0};
	Item.mask = LVIF_PARAM;
	Item.iItem = iIndex; 
	Item.lParam = uiVideoId; 
	m_lvCollection.SetItem(&Item); 
}

int FCollectionWindow::MapIdToIndex(UINT uiVideoId)
{
	LVFINDINFO  fi = {0};
	fi.flags = LVFI_PARAM; 
	fi.lParam = uiVideoId; 
	return m_lvCollection.FindItem(&fi, -1);
}

UINT FCollectionWindow::MapIndexToId(int iIndex)
{
	LVITEM Item = {0};
	Item.mask = LVIF_PARAM; 
	Item.iItem = iIndex; 
	if (m_lvCollection.GetItem(&Item))
		return (UINT)Item.lParam; 
	return 0; 
}

int FCollectionWindow::AddRow(UINT uiVideoId)
{
	int iIndex = MapIdToIndex(uiVideoId); 
	if (iIndex == -1)
	{
		iIndex = m_lvCollection.AddItem(0, 0, 0);
		if (iIndex != -1)
		{
			SetItemId(iIndex, uiVideoId); 
		}
	}
	return iIndex; 
}

BOOL FCollectionWindow::SetColumn(int iIndex, int ColIndex, const char* szValue)
{

	int iSubItem = ColIndex - 1;	/* Column index is 1 based in our definition*/

	char pszText[260];
	LVITEM tItem = {0};
	tItem.mask = LVIF_TEXT; 
	tItem.iItem = iIndex; 
	tItem.iSubItem = iSubItem; 
	tItem.pszText = pszText;
	tItem.cchTextMax = 260;
	if (m_lvCollection.GetItem(&tItem))
	{
		if (strcmp(pszText, szValue) == 0)
			return FALSE; 
	}


	LVITEM Item = {0};
	Item.mask = LVIF_TEXT;
	Item.iItem = iIndex; 
	Item.iSubItem = iSubItem; 
	Item.pszText = (LPSTR)szValue; 
	m_lvCollection.SetItem(&Item); 
	return TRUE; 
}

void FCollectionWindow::SetItemIcon(int iIndex, int iIcon)
{
	LVITEM Item = {0};
	Item.mask = LVIF_IMAGE;
	Item.iItem = iIndex; 
	Item.iImage = iIcon; 
	m_lvCollection.SetItem(&Item); 
}


void FCollectionWindow::RemoveTheRemoved()	//WOW
{
	int iRows = m_lvCollection.GetItemCount(); 
	int iRow = 0; 

	while (iRow < iRows)
	{
		UINT uiVideoId = MapIndexToId(iRow); 
		if (!m_Downloads.HasVideoId(uiVideoId))
		{
			m_lvCollection.DeleteItem(iRow); 
			iRows--;
		}
		else
			iRow++; 
	}
}

void FCollectionWindow::UpdateListView()
{
	//for each item in the downloads array
	//FValueItem &pItem = m_aDownloads.GetAt(k);
	//aRow = FindOrCreateRow(pItem.GetValueUINT("videoID"));
	//aRow.SetColumn("Video name", pItem.GetValueStr("videoName"));

	RemoveTheRemoved();
	

	size_t Items = m_Downloads.GetCount(); 

	for (size_t k = 0; k < Items; k++)
	{
		FValueMap& pItem = m_Downloads.GetAt(k); 

		UINT uiVideoId = pItem.GetVideoId();

		int iItemIndex = MapIdToIndex(uiVideoId);

		if (iItemIndex == -1)
			iItemIndex = AddRow(uiVideoId); 

		UpdateRow(uiVideoId, pItem); 

		UINT uiNewFlags = pItem.GetValueUINT("videoFlags");
		FDownloadFlagsPair* pMap = m_DownloadFlags.Lookup(uiVideoId);
		if (pMap)
		{
			if (uiNewFlags != pMap->m_value) 
			{
				m_lvCollection.RedrawItems(iItemIndex, iItemIndex); 
			}
		}
		m_DownloadFlags.SetAt(uiVideoId, pItem.GetValueUINT("videoFlags"));
	}
}

void FCollectionWindow::RefreshListView()
{
	DownloadStatusOptions options; 
	options.m_Label = m_StrLabel; 
	options.m_dwFlags = m_dwFilterFlags;
	options.m_Filter = m_StrSearchString;

	g_Objects._DownloadManager.GetDownloadStatus(options, &m_Downloads); 
	UpdateListView(); 
}

LRESULT FCollectionWindow::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	RefreshListView();
	return 0; 
}

LRESULT FCollectionWindow::OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (IsWindow())
	{
		if (wParam == FALSE)
			KillTimer(0); 
		else
		{
			RefreshListView();
			SetTimer(0, 500); 
		}
	}
	return 0; 
}

LRESULT FCollectionWindow::OnHeaderBeginDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	
	_DBGAlert("BeginDrag\n");
	bHandled = FALSE; 
	return 0; 
}

LRESULT FCollectionWindow::OnHeaderEndDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	_DBGAlert("EndDrag\n");
	return 0; 
}

LRESULT FCollectionWindow::OnListViewRightClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	FCollectionMenu Menu; 
	FArray<FString> aVids; 

	int iRows = m_lvCollection.GetItemCount(); 
	for (int iRow = 0; iRow < iRows; iRow++)
	{
		UINT uiState = m_lvCollection.GetItemState(iRow, LVIS_SELECTED); 
		if (uiState & LVIS_SELECTED)
		{
			FString S; 
			S.Format("%u", MapIndexToId(iRow));
			aVids.Add(S); 
		}
	}

	if (aVids.GetCount() > 0)
		Menu.ShowContextMenu(m_hWnd, aVids);
	return 0; 
}

LRESULT FCollectionWindow::OnListViewColumnClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{

	LPNMLISTVIEW pnlv = (LPNMLISTVIEW)pnmh;

	WTL::CHeaderCtrl ctlHeader = m_lvCollection.GetHeader();

	/* Sorting a new column always resets the order. Sorting the
	* same column only updates the order. */
	if ( pnlv->iSubItem != (int)_wSortCol )
	{
		HDITEM hdi = {(_bIsCC6 ? 0 : HDI_BITMAP)|HDI_FORMAT, 0};

		/* Clear the arrow for the previous column */
		ctlHeader.GetItem(_wSortCol, &hdi);
		ATLASSERT(!hdi.hbm || !_bIsCC6);

		hdi.hbm  = NULL;
		hdi.fmt &= ~(HDF_BITMAP|HDF_BITMAP_ON_RIGHT|HDF_SORTDOWN|HDF_SORTUP);
		ctlHeader.SetItem(_wSortCol, &hdi);

		_wSortCol = (WORD)pnlv->iSubItem;
		_bSortASC = 1;
	}
	else
	{      
		_bSortASC = !_bSortASC;
	}

	m_lvCollection._wSortCol = _wSortCol;
	m_lvCollection._bSortASC = _bSortASC; 
	m_lvCollection.SortItems(_LvItemCompare, (LPARAM)this);
	return 0; 
}

int SpeedTest(double speed1, double speed2)
{
	int nTestRes = 0; 
	if (speed2 > speed1)
		nTestRes = 1;
	else
	if (speed2 < speed1)
		nTestRes = -1; 

	return nTestRes; 
}

int SizeTest(INT64 Size1, INT64 Size2)
{
	INT64 res = Size2 - Size1; 
	if (res == 0)
		return 0; 
	return res > 0 ? 1 : -1;
}

struct StatusStruct
{
public:
	UINT m_uiVal; 
	StatusStruct(UINT uiVal)
	{
		m_uiVal = uiVal; 
	}
	BOOL IsQueued() const
	{
		return m_uiVal & FLAG_DOWNLOAD_QUEUED; 
	}
	BOOL IsActive() const
	{
		return (m_uiVal & FLAG_DOWNLOAD_ACTIVE) && ((m_uiVal & FLAG_DOWNLOAD_ERR_MASK) == 0);
	}
	BOOL IsError() const 
	{
		return (m_uiVal & FLAG_DOWNLOAD_ERR_MASK) != 0; 
	}
	BOOL IsPaused() const
	{
		return m_uiVal & FLAG_DOWNLOAD_PAUSED;
	}
	BOOL IsFinished() const
	{
		return m_uiVal & FLAG_DOWNLOAD_FINISHED; 
	}

	int operator -(const StatusStruct& rhs)
	{
		/*
		Sort order:
		Downloading
		Error
		Paused
		Finished
		Queued
		*/
		int retVal = 0; 
		if (IsActive())
			retVal = rhs.IsActive() ? 0 : -1;
		else
		if (IsPaused())
		{
			if (rhs.IsPaused())
				retVal = 0;
			else
			if (rhs.IsActive())
				retVal = 1;  //below active
			else
				retVal = -1; //above everything else
		}
		else
		if (IsError())
		{
			if (rhs.IsError())
				retVal = 0;		
			else
			if (rhs.IsActive() || rhs.IsPaused())
				retVal = 1; //below active and paused
			else
				retVal = -1; //above finished and queued
		}
		else
		if (IsFinished())
		{
			if (rhs.IsFinished())
				retVal = 0; 
			if (rhs.IsActive() || rhs.IsPaused() || rhs.IsError())
				retVal = 1; //below active, paused and error
			else
				retVal = -1; //above queued
		}
		if (IsQueued())
		{
			if (rhs.IsQueued())
				retVal = 0; 
			else
				retVal = -1 ;
		}
		return retVal; 
	}
};


int CALLBACK FCollectionWindow::_LvItemCompare( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	FCollectionWindow* _this = (FCollectionWindow*)lParamSort; 
	int nTestRes = 0; 
	//Test result between lParam1 and lParam2: -1 lParam1 > lParam2; 1 lParam1 < lParam2
	FValueMap* p1 = _this->m_Downloads.Lookup((UINT)lParam1); 
	FValueMap* p2 = _this->m_Downloads.Lookup((UINT)lParam2); 
	
	int nSortCol = _this->_wSortCol + 1;
	ATLASSERT(p1 && p2); 
	
	switch(nSortCol)
	{
	case COL_NAME:
		{
			const CComVariant* v1 = p1->GetValueVariant("videoName"); 
			const CComVariant* v2 = p2->GetValueVariant("videoName"); 
			if (v1 && v2 && v1->vt == VT_BSTR && v2->vt == VT_BSTR)
				nTestRes = wcsicmp(v1->bstrVal, v2->bstrVal);
		}
		break; 
	case COL_ADDED:
		{
			const CComVariant* v1 = p1->GetValueVariant("videoTimeAdded"); 
			const CComVariant* v2 = p2->GetValueVariant("videoTimeAdded"); 
			if (v1 && v2)
				nTestRes = v2->uintVal - v1->uintVal; 
		}
		break; 
	case COL_STATUS:
		{
			StatusStruct Flags1 = p1->GetValueUINT("videoFlags");
			StatusStruct Flags2 = p2->GetValueUINT("videoFlags");
			nTestRes = Flags2 - Flags1;
		}
		break; 
	case COL_TOTAL_SIZE:
		{
			nTestRes = SizeTest((INT64)p1->GetValueUINT64("videoTotalSize"), (INT64)p2->GetValueUINT64("videoTotalSize"));
		}
		break; 
	case COL_DOWNLOADED:
		{
			nTestRes = SizeTest((INT64)p1->GetValueUINT64("videoDownloaded"), (INT64)p2->GetValueUINT64("videoDownloaded"));
		}
		break; 
	case COL_DOWN_SPEED:
		{
			nTestRes = SpeedTest(p1->GetValueDouble("videoTotalUpSpeed"), p2->GetValueDouble("videoTotalUpSpeed"));
		}
		break; 
	case COL_UP_SPEED:
		{
			nTestRes = SpeedTest(p1->GetValueDouble("videoTotalUpSpeed"), p2->GetValueDouble("videoTotalUpSpeed"));
		}
		break; 
		
	}
	if (!_this->_bSortASC)
		nTestRes = -nTestRes;
		
	return nTestRes; 
}


void  EnableHighlighting(HWND hWnd, int row, bool bHighlight)
{
	ListView_SetItemState(hWnd, row, bHighlight? 0xff: 0, LVIS_SELECTED);
}


LRESULT FCollectionWindow::OnListViewCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	bHandled = TRUE;
	static bool bIsHighlighted = false;

	NMLVCUSTOMDRAW* plvCustomDraw = reinterpret_cast<NMLVCUSTOMDRAW*>(pnmh);

	int iItemIndex = (int)plvCustomDraw->nmcd.dwItemSpec;

	switch (plvCustomDraw->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		return CDRF_NOTIFYITEMDRAW;

	case CDDS_ITEMPREPAINT:
		{
			plvCustomDraw->nmcd.uItemState &= ~CDIS_SELECTED;//avoid system to draw selection background by itself
			bIsHighlighted = m_lvCollection.GetItemState(iItemIndex, LVIS_SELECTED) != 0;

			COLORREF bgColor = m_LVParams.m_clrBackground;
			COLORREF clrText = m_LVParams.m_clrTextColor;
			if (iItemIndex % 2)
				bgColor = m_LVParams.m_clrEvenItem;

			if (bIsHighlighted)
			{
				bgColor = m_LVParams.m_clrSelection;
			}
			else
			{
				//Is in progress ?
				UINT videoID = MapIndexToId(iItemIndex);
				FDownloadFlagsPair* pMap = m_DownloadFlags.Lookup(videoID);
				if (pMap)
				{
					DWORD flags = pMap->m_value; 

					if (flags & FLAG_DOWNLOAD_ACTIVE)
						bgColor = m_LVParams.m_clrDownloading ? m_LVParams.m_clrDownloading : bgColor;

					if (flags & FLAG_DOWNLOAD_PAUSED && !(flags & FLAG_DOWNLOAD_FINISHED))
						bgColor = m_LVParams.m_clrPaused ? m_LVParams.m_clrPaused : bgColor; 

					if (flags & FLAG_DOWNLOAD_QUEUED)
					{
						clrText = m_LVParams.m_clrQueued ? m_LVParams.m_clrQueued : m_LVParams.m_clrTextColor;
					}
				}
			}

			plvCustomDraw->clrTextBk = bgColor; 
			plvCustomDraw->clrText = clrText; 
			return CDRF_NEWFONT;
		}
		break; 
	case CDDS_ITEMPOSTPAINT:
		{
		}
		break; 
	}

	return CDRF_DODEFAULT;
}

LRESULT FCollectionWindow::OnHeaderCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	return 0; 
}

LRESULT FCollectionWindow::OnListViewDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	return 0; 
}

LRESULT FCollectionWindow::OnListViewKeyDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{
	NMLVKEYDOWN * pKey = (NMLVKEYDOWN *)pnmh; 
	
	
	switch(pKey->wVKey)
	{
	case VK_RETURN:
		{
			int iIndex = m_lvCollection.GetNextItem(-1, LVNI_SELECTED | LVNI_ALL);
			if (iIndex > 0)
			{
				UINT videoID = MapIndexToId(iIndex); 
				if (videoID > 0)
					g_MainFrame->PlayMediaFile(videoID); 
			}
		}
		break; 
	}
	return 0; 
}

LRESULT FCollectionWindow::OnListViewDblClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled)
{

	NMITEMACTIVATE* pItem = (NMITEMACTIVATE*)pnmh; 
	if (pItem->iItem >=0 )
	{
		UINT videoId = MapIndexToId(pItem->iItem); 

		FValueMap* pMap = m_Downloads.Lookup(videoId);
		if (pMap)
		{
			if (pMap->GetValueUINT("videoCanPlay") == 1)
				g_MainFrame->PlayMediaFile(videoId);
		}
			
	}
	
	return 0; 
}

void FCollectionWindow::RemoveAllItems()
{
	m_DownloadFlags.RemoveAll();
	m_lvCollection.DeleteAllItems();
}

void FCollectionWindow::SetFilters( const char* pszLabel, const char* pszSearchString, DWORD dwFlags )
{
	BOOL bNeedsRefresh = FALSE; 
	if (m_StrLabel != pszLabel)
	{
		m_StrLabel = pszLabel;
		bNeedsRefresh = TRUE; 
	}
	if (m_StrSearchString != pszSearchString)
	{
		m_StrSearchString = pszSearchString; 
		bNeedsRefresh = TRUE; 
	}
	if (m_dwFilterFlags != dwFlags)
	{
		m_dwFilterFlags = dwFlags; 
		bNeedsRefresh = TRUE; 
	}
	if (bNeedsRefresh)
	{
		RemoveAllItems();
		RefreshListView(); 
	}
}