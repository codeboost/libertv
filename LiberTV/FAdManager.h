#pragma once

#include "Utils.h"
#include "FWebWindow.h"
#include "FIniFile.h"
#include "AppSettings.h"
#include "FDownload.h"
#include "AdInterface.h"

class FAdPoints
{
public:
	vidtype			m_VideoID; 
	FArray<FAdItem> m_pAds; 
public:
	void Clear(); 
	BOOL LoadAdPoints(FIniConfig& pConf, BOOL bAddToList);
};

class FVideoAdManager : public IVideoAdManager
{
	FAdPoints	m_Ads;
	mz_Sync		m_Sync; 
public:
	BOOL LoadAds(FIniConfig& pConf, BOOL bAddToList);
	BOOL GetAds(AdTypes aType, std::vector<FAdItem> &outAds); 
	void Clear(); 
};


class FAdWindow : public IAdWindow,
				  public FBrowserEvents,
				  public FWebWindow
{
	mz_Sync					m_Sync; 
	std::vector<FAdItem>	m_Ads; 
	FAdItem					m_CurrentItem; 
	BOOL					m_bAdLoaded; 
	BOOL					m_bAdInProgress; 
	IAdWindowInit			m_InitData; 
	DWORD					m_LastState; 
	DWORD					m_TimerStarted; 
	
protected:
	BOOL	CreateWnd(); 
	BOOL	LoadAd(FAdItem& pItem); 
	void	OnAdLoaded(); 
	void	OnAdFinished(); 
	void	UpdateTimes(FAdItem& pItem, IAdPosition& pPos);
	
public:
	BEGIN_MSG_MAP(FAdWindow)
		MESSAGE_HANDLER(WM_TIMER, OnTimer); 
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
		CHAIN_MSG_MAP(FWebWindow)
	END_MSG_MAP()

	FAdWindow();
	LRESULT OnCreate(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL & bHandled);
	LRESULT OnDestroy(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL & bHandled);
	LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){bHandled = TRUE; return 1; }
	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled); 
	LRESULT OnShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled); 

	//IAdWindow
	BOOL Init(IAdWindowInit& pInitData);
	void Clear();
	long GetPreferedHeight();
	long GetPreferedWidth();
	BOOL IsAdShowing();
	HWND GetWnd(){return m_hWnd;}
	void ShowWindow(int nShow); 
	void MoveWindow(LPRECT prc);
	BOOL UpdatePosition(IAdPosition& pPos);
	BOOL SetZPosition(); 
	BOOL IsAdLoaded(); 
	//--End

	//FWebWindow
	void OnDocumentComplete(DWORD dwID, BOOL bMainFrame);
	BOOL OnNavigateError(DWORD dwID, BOOL bMainFrame, const tchar* szUrl);

};

