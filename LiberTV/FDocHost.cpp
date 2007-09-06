#include "stdafx.h"
#include "FDocHost.h"
#include "resource.h"
#include "AppSettings.h"


#define IDM_COPY 0x4324


HRESULT STDMETHODCALLTYPE CDocHostUIHandler::GetExternal(IDispatch **ppDispatch)
{
	CComQIPtr<IDispatch> pDisp = _Module.m_LibraryObject; 

	if (pDisp)
	{
		*ppDispatch = pDisp.Detach(); 
		return S_OK;
	}
	return E_NOTIMPL; 
}


HRESULT STDMETHODCALLTYPE CDocHostUIHandler::ShowContextMenu(/* [in] */ DWORD dwID,
															 /* [in] */ DWORD x,
															 /* [in] */ DWORD y,
															 /* [in] */ IUnknown *pcmdtReserved,
															 /* [in] */ IDispatch *pdispReserved,
															 /* [retval][out] */ HRESULT *dwRetVal)
{

	if (g_AppSettings.m_LogEnabled)
		return E_NOTIMPL; 

	*dwRetVal = S_OK; 
	return S_OK;
}


HRESULT STDMETHODCALLTYPE CDocHostUIHandler::GetHostInfo( 
	/* [out][in] */ DWORD *pdwFlags,
	/* [out][in] */ DWORD *pdwDoubleClick)
{
	*pdwFlags = DOCHOSTUIFLAG_NO3DBORDER ;
	*pdwDoubleClick = DOCHOSTUIDBLCLK_DEFAULT ; 

	return S_OK; 
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::GetDropTarget(IUnknown *pDropTarget, IUnknown **ppDropTarget)
{
	*ppDropTarget = NULL; 
	return S_OK; 
}

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::TranslateUrl(DWORD dwTranslate, BSTR bstrURLIn, BSTR *pbstrURLOut)
{
	return E_NOTIMPL; 
}

//////////////////////////////////////////////////////////////////////////

HRESULT STDMETHODCALLTYPE CDocHostUIHandler::TranslateAccelerator(DWORD_PTR hWnd, DWORD nMessage, DWORD_PTR wParam, DWORD_PTR lParam, BSTR bstrGuidCmdGroup, DWORD nCmdID, HRESULT *dwRetVal)
{
	//disable F5
	if(nMessage == WM_KEYDOWN && 
		wParam == VK_F5 )
	{
		*dwRetVal = S_FALSE;
		return S_OK;
	}

	if(GetKeyState(VK_CONTROL) & 0x8000)
	{
		//disable ctrl + O
		if(nMessage == WM_KEYDOWN && GetAsyncKeyState(0x4F) < 0)
		{
			*dwRetVal = S_OK;
			return S_OK; 
		}
		//disable ctrl + p
		if(nMessage == WM_KEYDOWN && GetAsyncKeyState(0x50) < 0)
		{
			*dwRetVal = S_OK;
			return S_OK;
		}
		//disable ctrl + N
		if(nMessage == WM_KEYDOWN && GetAsyncKeyState(0x4E) < 0)
		{
			*dwRetVal = S_OK;
			return S_OK;
		}
	}
	*dwRetVal = S_FALSE; 
	return S_OK;
}

