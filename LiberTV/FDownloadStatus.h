#ifndef FDownloadStatus_dot_H
#define FDownloadStatus_dot_H
#pragma once
#include <atlbase.h>
#include <atlapp.h>
extern CMyServerAppModule _Module;
#include <atlwin.h>
#include <atlframe.h>
#include <atlctrls.h>
#include <atlctrlw.h>
#include <atldlgs.h>
#include "mz_Inc.h"
#include "resource.h"
#include "CaptionPainter.h"
#include "FListView.h"




class FDownloadStatus : public CWindowImpl<FDownloadStatus>//, 
						//public CFlatCaptionPainter<FDownloadStatus> 
{
public:
	BEGIN_MSG_MAP(FDownloadStatus)
		//CHAIN_MSG_MAP(CFlatCaptionPainter<FDownloadStatus>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_SIZE, OnSize); 
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd);
		MESSAGE_HANDLER(WM_TIMER, OnTimer); 
		MESSAGE_HANDLER(WM_NOTIFY, OnNotify); 
		MESSAGE_HANDLER(WM_DESTROY, OnClose); 
	END_MSG_MAP()

	LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL &a){return 1;}
protected:
	CListViewCtrl		m_ListView;
	FListView			m_lvSub; 
	WTL::CImageList		m_ImageList; 

	LRESULT OnCreate(UINT , WPARAM , LPARAM , BOOL& bHandled);
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnTimer(UINT, WPARAM wTimer, LPARAM, BOOL&); 
	LRESULT OnClose(UINT, WPARAM, LPARAM, BOOL&); 
	LRESULT OnWindowPosChanging(UINT, WPARAM, LPARAM, BOOL&); 
	LRESULT OnNotify(UINT, WPARAM wParam, LPARAM lParam, BOOL&); 
	void	UpdateView(); 
};

#endif