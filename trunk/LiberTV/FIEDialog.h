#ifndef __FIEDIALOG_H__
#define __FIEDIALOG_H__

#include "FWebWindow.h"
#include "FGlobals.h"


/*
Derive class from FIEDialod
Don't forget to CHAIN_MSG_MAP() messages to FIEDialog in order to receive OnLoadComplete() notifications
*/
class FIEDialog :	public CWindowImpl<FIEDialog>, 
					public CMessageFilter
{
public:
	DECLARE_WND_CLASS_EX("ACx", 0, COLOR_WINDOW); 
	BEGIN_MSG_MAP(FIEDialog)
		MESSAGE_HANDLER(WM_CREATE, OnCreate); 
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd); 
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy); 		
		MESSAGE_HANDLER(WM_SIZE, OnSize); 
		MESSAGE_HANDLER(WM_DOCUMENT_COMPLETE, OnLoadComplete); 
		MESSAGE_HANDLER(WM_SIZING, OnSizing); 
	END_MSG_MAP();

	FIEWindow		m_pW;
	WTL::CSize		m_MinSize; 
	WTL::CSize		m_MaxSize; 

protected:
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&){return 1;}
	LRESULT OnDestroy(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&); 
	LRESULT OnLoadComplete(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnSizing(UINT, WPARAM, LPARAM, BOOL&); 
	//Override
	virtual void OnLoadComplete() = 0; 
	virtual void OnCreated() = 0; 
public:
	BOOL PreTranslateMessage(MSG* pMsg)
	{
		if((pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST) &&
			(pMsg->message < WM_MOUSEFIRST || pMsg->message > WM_MOUSELAST))
			return FALSE;

		// give HTML page a chance to translate this message
		return (BOOL)::SendMessage(m_pW.m_pBrowser, WM_FORWARDMSG, 0, (LPARAM)pMsg);
	}
	void	SetMinSize(int cx, int cy); 
	void	SetMaxSize(int cx, int cy); 

	BOOL	Open(HWND hWndParent, FRect& rcCreate); 
	FWebWindow& GetBrowser(){return m_pW.m_pBrowser;}
};



#endif //__FIEDIALOG_H__