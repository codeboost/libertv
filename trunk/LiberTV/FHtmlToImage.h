#ifndef __FHTMLTOIMAGE_H__
#define __FHTMLTOIMAGE_H__

#include "FWebWindow.h"
#include "Utils.h"

#define WM_IMAGE_DONE WM_USER + 124

class FBitmapDC
{
public:
	WTL::CDC		m_ImageDC; 
	WTL::CBitmap	m_Bitmap; 

	operator HDC()
	{
		return m_ImageDC; 
	}

	bool Create(HDC hdc, int cx, int cy)
	{
		m_ImageDC.DeleteDC();
		m_ImageDC = CreateCompatibleDC(hdc); 

		if (m_Bitmap)
			m_Bitmap.DeleteObject();
		m_Bitmap.CreateCompatibleBitmap(hdc, cx, cy); 
		m_ImageDC.SelectBitmap(m_Bitmap); 

		return true; 
	}
};

class FHtmlToImage : public FWebWindow
{
	BEGIN_MSG_MAP(FIEWindow)
		MESSAGE_HANDLER(WM_CREATE, OnCreate); 
		MESSAGE_HANDLER(WM_DOCUMENT_COMPLETE, OnDocumentComplete);
	END_MSG_MAP();

	HWND		m_hWndNotify; 
	HRESULT		CreateImage(HDC DC);
	unsigned	m_Working:1;	
	unsigned	m_msgNumber; 
public:
	LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&);

	BOOL	Init(HWND hWndNotify, LPCTSTR szUrl); 
	LRESULT OnDocumentComplete(UINT, WPARAM, LPARAM, BOOL&);

	FHtmlToImage(uint msgNumber = WM_IMAGE_DONE)
	{
		m_msgNumber = msgNumber; 
		m_Working = 0; 
	}
 
};


#endif //__FHTMLTOIMAGE_H__