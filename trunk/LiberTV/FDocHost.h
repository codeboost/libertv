#ifndef __FDOCHOST_H__
#define __FDOCHOST_H__

#include <list>

class CDocHostUIHandler: public IDocHostUIHandlerDispatch
{
	virtual HRESULT STDMETHODCALLTYPE ShowContextMenu( 
		/* [in] */ DWORD dwID,
		/* [in] */ DWORD x,
		/* [in] */ DWORD y,
		/* [in] */ IUnknown *pcmdtReserved,
		/* [in] */ IDispatch *pdispReserved,
		/* [retval][out] */ HRESULT *dwRetVal);

	virtual HRESULT STDMETHODCALLTYPE GetHostInfo( 
		/* [out][in] */ DWORD *pdwFlags,
		/* [out][in] */ DWORD *pdwDoubleClick);

		virtual HRESULT STDMETHODCALLTYPE ShowUI( 
		/* [in] */ DWORD dwID,
		/* [in] */ IUnknown *pActiveObject,
		/* [in] */ IUnknown *pCommandTarget,
		/* [in] */ IUnknown *pFrame,
		/* [in] */ IUnknown *pDoc,
		/* [retval][out] */ HRESULT *dwRetVal){return E_NOTIMPL;}

		virtual HRESULT STDMETHODCALLTYPE HideUI( void){return E_NOTIMPL;}

		virtual HRESULT STDMETHODCALLTYPE UpdateUI( void){return E_NOTIMPL;}

		virtual HRESULT STDMETHODCALLTYPE EnableModeless( 
			/* [in] */ VARIANT_BOOL fEnable){return E_NOTIMPL;}

			virtual HRESULT STDMETHODCALLTYPE OnDocWindowActivate( 
			/* [in] */ VARIANT_BOOL fActivate){return E_NOTIMPL;}

			virtual HRESULT STDMETHODCALLTYPE OnFrameWindowActivate( 
			/* [in] */ VARIANT_BOOL fActivate){return E_NOTIMPL;}

			virtual HRESULT STDMETHODCALLTYPE ResizeBorder( 
			/* [in] */ long left,
			/* [in] */ long top,
			/* [in] */ long right,
			/* [in] */ long bottom,
			/* [in] */ IUnknown *pUIWindow,
			/* [in] */ VARIANT_BOOL fFrameWindow){return E_NOTIMPL;}

			virtual HRESULT STDMETHODCALLTYPE TranslateAccelerator( 
			/* [in] */ DWORD_PTR hWnd,
			/* [in] */ DWORD nMessage,
			/* [in] */ DWORD_PTR wParam,
			/* [in] */ DWORD_PTR lParam,
			/* [in] */ BSTR bstrGuidCmdGroup,
			/* [in] */ DWORD nCmdID,
			/* [retval][out] */ HRESULT *dwRetVal);

			virtual HRESULT STDMETHODCALLTYPE GetOptionKeyPath( 
			/* [out] */ BSTR *pbstrKey,
			/* [in] */ DWORD dw){return E_NOTIMPL;}

			virtual HRESULT STDMETHODCALLTYPE GetDropTarget( 
			/* [in] */ IUnknown *pDropTarget,
			/* [out] */ IUnknown **ppDropTarget);

		virtual HRESULT STDMETHODCALLTYPE GetExternal( 
			/* [out] */ IDispatch **ppDispatch);

			virtual HRESULT STDMETHODCALLTYPE TranslateUrl( 
			/* [in] */ DWORD dwTranslate,
			/* [in] */ BSTR bstrURLIn,
			/* [out] */ BSTR *pbstrURLOut);

			virtual HRESULT STDMETHODCALLTYPE FilterDataObject( 
			/* [in] */ IUnknown *pDO,
			/* [out] */ IUnknown **ppDORet){return E_NOTIMPL;}


			//////////////////////////////////////////////////////////////////////////
};


#endif //__FDOCHOST_H__