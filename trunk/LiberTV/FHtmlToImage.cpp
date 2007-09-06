#include "stdafx.h"
#include "FHtmlToImage.h"

LRESULT FHtmlToImage::OnCreate(UINT, WPARAM, LPARAM, BOOL&)
{
	m_Working = 0; 
	return 0; 
}

LRESULT FHtmlToImage::OnDocumentComplete(UINT, WPARAM, LPARAM, BOOL&)
{
	if (1 == m_Working)
	{
		::PostMessage(m_hWndNotify, m_msgNumber, 0, (LPARAM)0); 
	}
	return 0; 
}

BOOL FHtmlToImage::Init(HWND hWndNotify, LPCTSTR szUrl)
{
	if (0 == m_Working)
	{
		m_hWndNotify = hWndNotify; 
		m_Working = 1; 
		if (Navigate(szUrl, ""))
			return TRUE; //will notify through WM_IMAGE_DONE
	}
	m_Working = 0; 
	return FALSE; 
}

HRESULT FHtmlToImage::CreateImage(HDC dc)
{
	USES_CONVERSION;
	ATLASSERT(pDocument);
	
	if (NULL == dc)
		return E_FAIL; 

	//	Get our interfaces before we create anything else
	IHTMLElement	   *pElement = (IHTMLElement *) NULL;
	IHTMLElementRender *pRender = (IHTMLElementRender *) NULL;

	HRESULT hr = pDocument->get_body(&pElement);

	//	Let's be paranoid...
	if (FAILED(hr))
		return hr;

	hr = pElement->QueryInterface(IID_IHTMLElementRender, (void **) &pRender);

	if (FAILED(hr))
		return hr; 

	
	return pRender->DrawToDC(dc);
}
