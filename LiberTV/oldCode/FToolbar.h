#ifndef __FTOOLBAR_H__
#define __FTOOLBAR_H__

#include "FWindowBase.h"
#include <atlcoll.h>

class FImageToolbar : public CWindowImpl<FImageToolbar>
{
	BEGIN_MSG_MAP(FImageToolbar)
		//CHAIN_MSG_MAP(CFlatCaptionPainter<FDownloadStatus>)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_PAINT, OnPaint); 
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd);
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouse); 
		MESSAGE_HANDLER(WM_LBUTTONUP, OnMouse); 
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnMouse);
		MESSAGE_HANDLER(WM_SIZE, OnSize);
	END_MSG_MAP();

	struct FButton
	{
		ATL::CImage	m_Image; 
		FRect		m_Placement; 
		dword		m_dwFlags; 
		int			m_idi; 
		FButton():	m_dwFlags(0), m_idi(0)
		{

		}
	};
	CBrush					m_bgBrush; 
	CAtlArray<FButton>		m_Buttons; 
	int						m_Lastbutton; 
	int						m_xOffs; 
	int						m_yOffs;
	int						m_xSpacing; 
	ATL::CImage				m_bgBitmapLoad; 
	WTL::CBitmap			m_bgBitmap; 
	ATL::CImage				m_Logo; 
    WTL::CEdit              m_EdtSearch;
    WTL::CFont              m_EdtFont; 

	FImageToolbar()
	{
		SetImageLimits(0, 0, 0); 
	}

	LRESULT OnEraseBkgnd(UINT, WPARAM, LPARAM, BOOL&){return 1;}
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnPaint(UINT, WPARAM, LPARAM, BOOL&);
	LRESULT OnMouse(UINT, WPARAM, LPARAM, BOOL&); 
	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&);
	int		GetButtonID(POINT &pt); 
	void	SetImageLimits(int xOffset, int yOffset, int xSpacing); 
	BOOL	AddButtonFromRes(int ButtonID, int nRes); 
	BOOL	AddButtonFromFile(int ButtonID, const char* szFileName); 
	void	RedrawToolbar();

	void	OnButtonPressed(int nButton); 
	void	ComputeButtonCoords(int xOffs);
	BOOL	SetBgImage(const tchar* szFileName); 
	BOOL	LoadLogo(const tchar* szFileName); 
	void	GenerateBgBitmap(int WndWidth) ;
};


#endif //__FTOOLBAR_H__