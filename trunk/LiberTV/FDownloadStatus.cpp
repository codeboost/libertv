#include "stdafx.h"
#include "FDownloadStatus.h"
#include "FControls.h"
#include "GlobalObjects.h"
#include "FTrayWnd.h"
#include "FPlayerWindow.h"
#include "FTorrentStatus.h"
#include <strsafe.h>
#include "FPlayerWindow.h"
#include "FStatusFrame.h"

extern FAutoWindow<FTrayWindow> g_TrayWindow; 
#define CONV_BUF_SIZE 64

static char _ConvBuffer[CONV_BUF_SIZE];

const char* mtp_StatusString(FMTProgress& p)
{
	if (p.IsFinished())
		return "Finished";

	if (p.IsPaused())
		return "Paused";
	

	if (p.IsTrackerAlive() && !p.IsError())
		return "Downloading"; 

	if (!p.IsTrackerAlive())
		return "Tracker error";

	return "Download Error";
}

#define IINDEX_DOWNLOADING	0
#define IINDEX_FINISHED		1
#define IINDEX_WARNING		2
#define IINDEX_ERROR		3


int GetStatusIconIndex(FMTProgress& p)
{
	if (p.IsFinished())
		return IINDEX_FINISHED; 
	
	if (p.IsPaused())
		return IINDEX_DOWNLOADING; 

	if (p.IsTrackerAlive() && !p.IsError())
		return IINDEX_DOWNLOADING; 

	if (!p.IsTrackerAlive())
		return IINDEX_WARNING; 

	return IINDEX_ERROR; 
}


std::string mtp_ProgressByTime(FMTProgress& p)
{
	if (p.m_TotalDuration > 0)
	{
		float dblv = ((float)p.m_DownloadedDuration / (float)p.m_TotalDuration) * 100;
		
		StringCbPrintf(_ConvBuffer, CONV_BUF_SIZE, "%0.1f %%", dblv);
		return _ConvBuffer; 

	}
	return std::string("0.0%");
}

FString mtp_ProgressBySize(FMTProgress& p)
{
	size_type aCurClipSize = 0; 
	if (p.m_CurClipStatus.state == torrent_status::downloading)
		aCurClipSize = p.m_CurClipStatus.total_done;

	return PercentString((double)(p.m_DownloadedSize + aCurClipSize), (double)p.m_RequiredSize); 
}

std::string mtp_DownloadRate(FMTProgress& p)
{
	return add_suffix(p.m_CurClipStatus.download_rate) + "/s"; 
}


LRESULT FDownloadStatus::OnCreate(UINT , WPARAM , LPARAM , BOOL& bHandled)
{
	RECT rcClient; 
	GetClientRect(&rcClient); 

	m_ListView.Create(m_hWnd, &rcClient, "StatusLV", 
		WS_VISIBLE | WS_CHILDWINDOW | LVS_REPORT | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_SHOWSELALWAYS | LVS_SINGLESEL, 
									WS_EX_CLIENTEDGE);


	m_ImageList.Create(16, 16, ILC_COLOR24 | ILC_MASK, 3, 1); 
	m_ImageList.AddIcon(AtlLoadIcon(IDI_DOWN_ARROW));
	m_ImageList.AddIcon(AtlLoadIcon(IDI_GREEN_BUTTON)); 
	m_ImageList.AddIcon(AtlLoadIcon(IDI_YELLOW_BUTTON)); 
	m_ImageList.AddIcon(AtlLoadIcon(IDI_RED_BUTTON)); 

	m_ListView.SetImageList(m_ImageList, LVSIL_SMALL); 

	m_ListView.AddColumn("Show", 0, -1);   
	m_ListView.SetColumnWidth(0, 350);
	
	m_ListView.AddColumn("Status", 1, -1, LVCF_FMT | LVCF_TEXT, LVCFMT_LEFT); 
	m_ListView.SetColumnWidth(1, 100); 
	
	m_ListView.AddColumn("Progress", 2, -1, LVCF_FMT | LVCF_TEXT, LVCFMT_RIGHT); 
	m_ListView.SetColumnWidth(2, 100); 
	
	m_ListView.AddColumn("Speed", 3, -1, LVCF_FMT | LVCF_TEXT, LVCFMT_RIGHT); 
	m_ListView.SetColumnWidth(3, 100); 

	m_ListView.SetExtendedListViewStyle(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT); 
	m_lvSub.EnableDoubleBuffering(m_ListView); 

	UpdateView(); 
	SetTimer(0, 500, NULL);  

	return 0; 
}

LRESULT FDownloadStatus::OnClose(UINT, WPARAM, LPARAM, BOOL &bHandled)
{

	g_TrayWindow->SetMTFilter(0);
	bHandled = FALSE; 
	return 0; 
}

LRESULT FDownloadStatus::OnSize(UINT, WPARAM, LPARAM, BOOL& bHandled)
{
	FRect rcClient; 
	GetClientRect(&rcClient); 
	//m_ListView.SetWindowPos(NULL, 0, 0, rcClient.Width(), rcClient.Height(),   SWP_NOZORDER);
	m_ListView.MoveWindow(&rcClient); 
	bHandled = FALSE; 
	return 1; 
}

LRESULT FDownloadStatus::OnTimer(UINT, WPARAM wTimer, LPARAM, BOOL&)
{
	if (wTimer == 0)
	{
		UpdateView();			
	}
	return 0; 
}

int FindItem(CListViewCtrl& lv, LPARAM lParam)
{
	LVFINDINFO lvfi = {0}; 
	lvfi.flags = LVFI_PARAM | LVFI_WRAP; 
	lvfi.lParam = lParam; 
	return lv.FindItem(&lvfi, 0); 
}

void FDownloadStatus::UpdateView()
{
	
	FArray<FMTProgress> aProg; 

	if (g_Objects._MTHandler.GetDownloadStatus(aProg) >= 0)
	{
		int nLine = 0; 
		dword dwAdded = 0; 
		int nLastLine = 0; 
		
		assert(aProg.GetCount() < 32); 
		
		m_ListView.SetRedraw(FALSE); 
		for (size_t k = 0; k < aProg.GetCount(); k++)
		{
			FMTProgress& p = aProg[k]; 
			
			nLine = FindItem(m_ListView, (LPARAM)p.m_mtHandle.lValue);
			if (nLine == -1)
			{
				nLine = m_ListView.InsertItem(LVIF_TEXT | LVIF_PARAM, 0,  p.m_VideoName, 0, 0, 0, (LPARAM)p.m_mtHandle.lValue);
				dwAdded <<= 1; 
			}

			if (nLine >= 0)
			{
				m_ListView.SetItem(nLine, 0, LVIF_IMAGE, "", GetStatusIconIndex(p), 0, 0, 0); 
				m_ListView.SetItemText(nLine, 1, mtp_StatusString(p));
				m_ListView.SetItemText(nLine, 2, mtp_ProgressBySize(p));
				m_ListView.SetItemText(nLine, 3, mtp_DownloadRate(p).c_str());
				//Mark line as added
				dwAdded|= (1 << nLine);
			}
			else
			{
				assert(0); 
			}
		}
		//Bits set in dwAdded represent lines which have been updated.
		//Bits 0..aProg.Size(), which are NOT set in dwAdded represent downloads which must be removed fromt he LV.

		for (dword dwBitNo = 0; dwBitNo < 32; dwBitNo++)
		{
			if (!(dwAdded & (1 << dwBitNo)))
			{
				m_ListView.DeleteItem(dwBitNo); 
			}
		}

		m_ListView.SetRedraw(TRUE); 
	}
}

LRESULT FDownloadStatus::OnNotify(UINT uMessage, WPARAM wParam, LPARAM lParam, BOOL&)
{
	NMITEMACTIVATE *lpa = (LPNMITEMACTIVATE) lParam;

	if (lpa)
	{
		switch(lpa->hdr.code)
		{
		case NM_DBLCLK:
			g_TrayWindow->PlayMediaFile((dword)m_ListView.GetItemData(lpa->iItem)); 
			break;
		case LVN_ITEMCHANGED:
			{
				MTHandle aFilter; 
				NMLISTVIEW* pLV	= (NMLISTVIEW*) lParam; 
				if (pLV->uNewState & LVIS_SELECTED)
				{
					aFilter = (dword)m_ListView.GetItemData(pLV->iItem);
				}
				else
				{
					if (m_ListView.GetSelectedIndex() == -1)
						aFilter = 0; 
				}
				g_TrayWindow->SetMTFilter(aFilter);
			}
			break; 
		case NM_RCLICK:
			{
				bool bItemSelected = lpa->iItem >=0; 
				WTL::CMenu kMenu; kMenu.LoadMenu(IDR_DOWNLOAD_STATUS);  WTL::CMenuHandle kSubMenu = kMenu.GetSubMenu(0); 
				UINT uEnable = MF_BYCOMMAND;
				uEnable|=bItemSelected ? MF_ENABLED: MF_GRAYED; 
				kSubMenu.EnableMenuItem(IDC_PLAY, uEnable); 
				kSubMenu.EnableMenuItem(IDC_PAUSE, uEnable); 
				kSubMenu.EnableMenuItem(IDC_PAUSE_EXCEPT, uEnable); 
				kSubMenu.EnableMenuItem(IDC_REMOVEDOWNLOAD, uEnable);
				kSubMenu.EnableMenuItem(IDC_REMOVEANDDISCARD, uEnable); 

				POINT& pt = lpa->ptAction;
				m_ListView.ClientToScreen(&pt); 
                int nRes = kSubMenu.TrackPopupMenu(TPM_RETURNCMD, pt.x, pt.y, m_hWnd); 

				switch(nRes)
				{
				case IDC_PLAY:
					{
						g_TrayWindow->PlayMediaFile((dword)m_ListView.GetItemData(lpa->iItem)); 
					}
					break; 
				case IDC_REMOVEDOWNLOAD:
					{
						if (MessageBox(_Lang(IDS_CONFIRM_REMOVE), _Lang(IDS_WARNING), MB_YESNO | MB_ICONWARNING) == IDYES)
							g_Objects._MTHandler.RemoveMT((dword)m_ListView.GetItemData(lpa->iItem));
					}
					break; 
				case IDC_REMOVEANDDISCARD:
					{
						if (MessageBox(_Lang(IDS_CONFIRM_REMOVE), _Lang(IDS_WARNING), MB_YESNO | MB_ICONWARNING) == IDYES)
							g_Objects._MTHandler.RemoveMT((dword)m_ListView.GetItemData(lpa->iItem), true);
					}
					break; 
				}
			}
			break; 
		default:
			break; 
		}
	}

	if (lpa && lpa->hdr.code == NM_DBLCLK)
	{
	}


	return 0; 
}
