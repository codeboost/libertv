#pragma once

#include "FWebWindow.h"
#include "FLabels.h"
#include "IRSSInterface.h"
#include "ILTWindows.h"

#define COLLECTION_VIEW_THUMBS	0x00
#define COLLECTION_VIEW_LIST	0x01

class FCollectionDisplay : public FIEWindow
{
	
public:
	BEGIN_MSG_MAP(FCollectionDisplay)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		CHAIN_MSG_MAP(FIEWindow)	
	END_MSG_MAP()
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 

	FCollectionDisplay(); 
	int		ShowContextMenu(const FArray<FString> &aVids); 
	BOOL	IsViewActive();
	void	OnActivated(BOOL bActive); 
	FString	ShowAddLabelDlg(const char* pszLabel); 
	void	SetView(int nView);
	int		GetView();
	void	CycleView(); 
	BOOL	GetCollectionDivRect(FRect& rc); 
	void	OnLabelChanged(const char* pszNewLabel);
	void	OnSearchStringChanged(const char* pszNewFilter);
	void	OnFlagsChanged(DWORD dwCollFlags);

	HWND			m_hWndListView; 
	int				m_iView;	
	FString			m_StrCurrentLabel;
	FString			m_StrSearchString; 
	DWORD			m_dwCollFlags; 
};


class FFeedsDisplay : public FIEWindow, public IRSSInterface
{
public:
	BEGIN_MSG_MAP(FFeedsDisplay)
		MESSAGE_HANDLER(WM_FEED_REFRESH, OnMsgFeedRefresh)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	CHAIN_MSG_MAP(FIEWindow)	
	END_MSG_MAP()

	DWORD		m_dwChannelID; 
	FFeedsDisplay(){
	}
	virtual ~FFeedsDisplay(){}
	BOOL	OnBeforeNavigate(DWORD dwID, const tchar* pszURL);
	BOOL	IsViewActive();
	LRESULT OnMsgFeedRefresh(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled); 
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	//////////////////////////////////////////////////////////////////////////
	void	OnChannelRefresh(DWORD dwChannelID, DWORD dwState, DWORD dwNewItems); 
	BOOL	OnSetAutoDownload();
	void	GoToChannel(DWORD dwChannelID); 
	void	OnActivated(BOOL bActive); 
};

