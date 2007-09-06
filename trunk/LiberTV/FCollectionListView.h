#pragma once


#define IDC_LISTVIEW 100
#define IDC_HDRFRAGMENTS 101

#define COL_REG_KEY LTV_REG_KEY"\\Player\\LVCollection"

#define WM_HSETCURSOR WM_APP + 100

void FGradientFillRect (HDC hdc, RECT* R, COLORREF StartColor, COLORREF EndColor, int Direction);

struct ColDef
{
	FString m_Text; 
	int		m_iWidth;
	DWORD	m_dwTextFormat; 
	ColDef(const char* szText, int iWidth, DWORD dwTextFmt = DT_LEFT)
	{
		m_Text = szText; 
		m_iWidth = iWidth; 
		m_dwTextFormat = dwTextFmt; 
		
	}
	ColDef(const ColDef& r)
	{
		m_Text = r.m_Text; 
		m_iWidth = r.m_iWidth; 
		m_dwTextFormat = r.m_dwTextFormat; 
	}
};

struct FCollListView
{
	COLORREF m_clrBackground; 
	COLORREF m_clrEvenItem; 
	COLORREF m_clrTextColor; 
	COLORREF m_clrSelection; 
	COLORREF m_clrDownloading; 
	COLORREF m_clrPaused; 
	COLORREF m_clrQueued; 
	COLORREF m_clrPlaying; 
	FCollListView()
	{
		m_clrBackground =  RGB(62, 56, 56);
		m_clrEvenItem = RGB(74, 68, 68);
		m_clrTextColor = RGB(255, 255, 255); 
		m_clrSelection = RGB(101, 117, 70); 
		m_clrDownloading = 0; 
		m_clrPaused = RGB(82, 76, 76);
		m_clrQueued = 0;
	}
};


#define IDC_HDRFRAGMENTS 101

template< class T>
class FHeaderCtrlT : public CWindowImpl<T, CHeaderCtrl>
{
public:
	BEGIN_MSG_MAP(FHeaderCtrlT)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()
	
	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(m_hWnd == NULL);
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = CWindowImpl< T, CHeaderCtrl>::SubclassWindow(hWnd);
		if (bRet)
			Init(); 
		return bRet;
	}
	virtual void Init() = 0; 
	virtual ~FHeaderCtrlT(){}
};

class FHeaderCtrl : public FHeaderCtrlT<FHeaderCtrl>
{
public:
	DECLARE_WND_CLASS("FHeaderCtrl")
	BEGIN_MSG_MAP(FHeaderCtrl)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_NCPAINT, OnNcPaint);
	CHAIN_MSG_MAP(FHeaderCtrlT<FHeaderCtrl>)
		

	END_MSG_MAP()

	CDC				m_PaintDC; 
	WTL::CBitmap	m_PaintBitmap; 
	COLORREF		m_GradientStart; 
	COLORREF		m_GradientEnd; 
	COLORREF		m_GradientSelStart; 
	COLORREF		m_GradientSelEnd; 

	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	void	Init(); 
};

template< class T, class TBase = CListViewCtrl, class TWinTraits = CControlWinTraits >
class  FCollectionListViewT : public CWindowImpl< T, TBase, TWinTraits >
{
public:
	BEGIN_MSG_MAP(FCollectionListViewT)
		DEFAULT_REFLECTION_HANDLER()
	END_MSG_MAP()

	virtual ~FCollectionListViewT(){}
	BOOL SubclassWindow(HWND hWnd)
	{
		ATLASSERT(m_hWnd == NULL);
		ATLASSERT(::IsWindow(hWnd));
		BOOL bRet = CWindowImpl< T, TBase, TWinTraits >::SubclassWindow(hWnd);
		if (bRet)
			Init(); 
		return bRet;
	}

	virtual void Init() = 0; 
};

class FCollectionListView : public FCollectionListViewT<FCollectionListView>
{
public:
	DECLARE_WND_CLASS(_T("FListView32"))

	BEGIN_MSG_MAP(FCollectionListView)
		//WM_CREATE doesn't arrive, because we are subclassing the window. Use Init()
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_HSETCURSOR, OnHeaderSetCursor)
		NOTIFY_HANDLER(IDC_HDRFRAGMENTS, HDN_BEGINTRACKA, OnBeginTrack)
		NOTIFY_HANDLER(IDC_HDRFRAGMENTS, HDN_BEGINTRACKW, OnBeginTrack)
		NOTIFY_HANDLER(IDC_HDRFRAGMENTS, NM_RCLICK,		OnHeaderRightClick)
		NOTIFY_HANDLER(IDC_HDRFRAGMENTS, NM_CUSTOMDRAW, OnHeaderCustomDraw)
		CHAIN_MSG_MAP(FCollectionListViewT<FCollectionListView>)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
	END_MSG_MAP()

	FArray<ColDef>				m_LVColumns;	//List of default column names and sizes.
	UINT						m_uiColFlag;	//Bitfield that shows which columns are visible. If bit n is set, column n is visible
	CFont						m_fntHeader; 
	HICON						m_hSortIconUp; 
	HICON						m_hSortIconDown; 
	FHeaderCtrl					m_hHeader; 
#pragma warning( disable : 4201 )
	struct
	{
		ULONG _wSortCol : 16;
		ULONG _bSortASC : 1;
		ULONG _bIsCC6   : 1;
	};
#pragma warning( default : 4201 )
	
	void	Init(); 
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnHeaderSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 

	LRESULT		OnBeginTrack(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT		OnHeaderRightClick(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	LRESULT		OnHeaderCustomDraw(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
	


	int	 GetLastVisibleColumn(); 
	BOOL SaveColumnSetup(); 
	BOOL LoadColumnSetup();
	BOOL ShowHeaderContextMenu();
	void UpdateColumnWidths();
	void AddColumn(const char* pszColumn, int iWidth, int iCol = -1);

	void SetSortIcon(HICON hIcon, BOOL bUp); 
	FString GetItemText(int iItem, int iSubItem); 
};
