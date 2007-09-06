#pragma once

#include "FAutoWindow.h"
#include "FCollectionListView.h"
#include "FDownloadStatusArray.h"

#define IDC_LISTVIEW 100
class FCollectionWindow : public CWindowImpl<FCollectionWindow>
{
public:
	DECLARE_WND_CLASS_EX("FCW", 0, 0); 
	BEGIN_MSG_MAP(FCollectionWindow)
		MESSAGE_HANDLER(WM_ERASEBKGND,	OnEraseBackground)
		MESSAGE_HANDLER(WM_CREATE,		OnCreate)
		MESSAGE_HANDLER(WM_SIZE,		OnSize)
		MESSAGE_HANDLER(WM_TIMER,		OnTimer)
		MESSAGE_HANDLER(WM_SHOWWINDOW,	OnShowWindow)
		MESSAGE_HANDLER(WM_DESTROY,		OnDestroy)
		MESSAGE_HANDLER(WM_DRAWITEM,	OnListViewDrawItem)
		MESSAGE_HANDLER(WM_SETFOCUS,	OnFocus)

		NOTIFY_HANDLER(IDC_HDRFRAGMENTS, HDN_BEGINDRAG,	OnHeaderBeginDrag)
		NOTIFY_HANDLER(IDC_HDRFRAGMENTS, HDN_ENDDRAG,	OnHeaderEndDrag)
		NOTIFY_HANDLER(IDC_LISTVIEW,	 NM_RCLICK,		OnListViewRightClick)
		NOTIFY_HANDLER(IDC_LISTVIEW,	 NM_DBLCLK,		OnListViewDblClick)
		NOTIFY_HANDLER(IDC_LISTVIEW,	 LVN_COLUMNCLICK,OnListViewColumnClick)
		NOTIFY_HANDLER(IDC_LISTVIEW,	 NM_CUSTOMDRAW,	 OnListViewCustomDraw)
		NOTIFY_HANDLER(IDC_LISTVIEW,	 LVN_KEYDOWN,	 OnListViewKeyDown)
	END_MSG_MAP()

	FCollectionListView			m_lvCollection; 
	WTL::CImageList				m_lvImgList;  
	CAtlMap<UINT, DWORD>		m_DownloadFlags; 
	typedef CAtlMap<UINT, DWORD>::CPair FDownloadFlagsPair; 
		
	FDownloadStatusArray		m_Downloads; 
	CFont						m_fntListView; 
	FString						m_StrLabel; 
	DWORD						m_dwFilterFlags; 
	FString						m_StrSearchString; 

	LRESULT			OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){return 1; }
	LRESULT			OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT			OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT			OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT			OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT			OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT			OnListViewDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT			OnFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	//Notify handlers
	LRESULT			OnHeaderBeginDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT			OnHeaderEndDrag(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT			OnListViewRightClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT			OnListViewColumnClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT			OnHeaderCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT			OnListViewCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT			OnListViewKeyDown(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT			OnListViewDblClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	

	void			UpdateListView(); 
	void			RefreshListView(); //Gets status and calls UpdateListView();
	void			RemoveAllItems();
	void			SetFilters(const char* pszLabel, const char* pszSearchString, DWORD dwFlags);


	BOOL	UpdateRow(UINT uiVideoId, FValueMap& pValue);
	int		MapIdToIndex(UINT uiVideoId);
	void	SetItemId(int iIndex, UINT uiVideoId);
	UINT	MapIndexToId(int iIndex);
	int		AddRow(UINT uiVideoId);
	BOOL	SetColumn(int iIndex, int iColIndex, const char* szValue);
	void	SetItemIcon(int iIndex, int iIcon); 
	void	RemoveTheRemoved();	//WOW
	static int CALLBACK _LvItemCompare( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort );
	
	#pragma warning( disable : 4201 )
	struct
	{
		ULONG _wSortCol : 16;
		ULONG _bSortASC : 1;
		ULONG _bIsCC6   : 1;
	};
	#pragma warning( default : 4201 )
};