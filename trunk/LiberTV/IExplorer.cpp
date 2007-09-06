#include "stdafx.h"
#include "IExplorer.h"
#include "AppSettings.h"

long FDocument::CallScriptLng(const tchar* wFunc, const tchar* aParam)
{
	USES_CONVERSION;
	CComVariant vtRes; 
	long lRes = 0; 
	if (SUCCEEDED(CallJScript(wFunc, aParam, &vtRes)))
	{
		if (vtRes.vt == VT_BSTR)
			lRes = strtol(OLE2T(vtRes.bstrVal), NULL, 10); 
		if (vtRes.vt == VT_I4)
			lRes = vtRes.intVal;
		if (vtRes.vt == VT_UINT)
			lRes = vtRes.uintVal;
	}
	return lRes;

}

FString FDocument::CallScriptStr(const tchar* wFunc, const tchar* aParam)
{
	USES_CONVERSION;
	CComVariant vtRes; 
	FString StrRes; 
	if (SUCCEEDED(CallJScript(wFunc, aParam, &vtRes)))
    {
        if (vtRes.vt == VT_BSTR)
		    StrRes = OLE2T(vtRes.bstrVal);
		if (vtRes.vt == VT_I4)
			StrRes.Format("%d", vtRes.intVal);
		if (vtRes.vt == VT_R8)
			StrRes.Format("%u", (DWORD)vtRes.dblVal);
    }
	return StrRes;
}

HRESULT FDocument::CallJScript(const tchar* wFunc, CComVariant* pvRes)
{
	CAtlArray<CAtlString> aTemp; 
	return CallJScript(wFunc, aTemp, pvRes); 
}

HRESULT FDocument::CallJScript(const tchar* wFunc, const tchar* aParam1, CComVariant* pRes)
{
	CAtlArray<CAtlString> aTemp; 
	aTemp.Add(aParam1); 
	return CallJScript(wFunc, aTemp, pRes); 
}

HRESULT FDocument::CallJScript(const tchar* wFunc, const tchar* aParam1, const tchar* aParam2, CComVariant* pRes)
{
	CAtlArray<CAtlString> aTemp; 
	aTemp.Add(aParam1); 
	aTemp.Add(aParam2); 
	return CallJScript(wFunc, aTemp, pRes); 
}

HRESULT FDocument::CallJScript(const tchar* wFunc, const tchar* aParam1, const tchar* aParam2, const tchar* aParam3, CComVariant* pRes)
{
	CAtlArray<CAtlString> aTemp; 
	aTemp.Add(aParam1); 
	aTemp.Add(aParam2); 
	aTemp.Add(aParam3); 
	return CallJScript(wFunc, aTemp, pRes); 
}


HRESULT FDocument::CallJScript(const tchar* szFunctionName, ATL::CAtlArray<ATL::CAtlString>& aParams, CComVariant* pvResult)
{

    if (!pDocument)
		return E_FAIL; 

	CComPtr<IDispatch> spScript;
	HRESULT hr = pDocument->get_Script(&spScript);

	if(FAILED(hr))
		return hr; 

	CComBSTR bstrURL; 
	pDocument->get_URL(&bstrURL);
	
	CComBSTR bstrMember(szFunctionName);
	DISPID dispid = NULL;
	hr = spScript->GetIDsOfNames(IID_NULL,&bstrMember,1, LOCALE_SYSTEM_DEFAULT,&dispid);

	if(FAILED(hr))
		return hr; 

	size_t arraySize = aParams.GetCount(); 

	DISPPARAMS dispparams;
	memset(&dispparams, 0, sizeof dispparams);
	dispparams.cArgs = (UINT)arraySize;
	dispparams.rgvarg = new VARIANT[dispparams.cArgs];

	for( size_t i = 0; i < arraySize; i++)
	{
		CComBSTR bstr = aParams.GetAt(arraySize - 1 - i); // back reading
		bstr.CopyTo(&dispparams.rgvarg[i].bstrVal);
		dispparams.rgvarg[i].vt = VT_BSTR;
	}
	dispparams.cNamedArgs = 0;

	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);
	CComVariant vaResult;
	UINT nArgErr = (UINT)-1;  // initialize to invalid arg

	hr = spScript->Invoke(dispid,IID_NULL,0,
		DISPATCH_METHOD,&dispparams,&vaResult,&excepInfo,&nArgErr);

	for (size_t i = 0; i < arraySize; i++)
		SysFreeString(dispparams.rgvarg[i].bstrVal);

	delete [] dispparams.rgvarg;

	if(FAILED(hr))
		return hr; 

	if(pvResult)
	{
		*pvResult = vaResult;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////

HRESULT FDocument::QueryHtmlElement( LPCSTR ElementName, LPDISPATCH * ppDispatch )
{
	CComPtr<IDispatch>				pDispatch;
	CComPtr<IHTMLElementCollection> pAll;
	_bstr_t FieldToSearch( ElementName );
	HRESULT hr = E_FAIL; 

	if (pDocument != NULL)
	{
		hr = pDocument->get_all( &pAll );
		if (SUCCEEDED(hr))
			hr = pAll->item( _variant_t( FieldToSearch ), _variant_t( (LONG)0 ), (IDispatch**) &pDispatch);
		*ppDispatch = pDispatch ;
	}
	return hr;
}

HRESULT FDocument::QueryHtmlElementFormElement( LPCTSTR FormName, LPCTSTR ElementName, LPDISPATCH * ppDispatch )
{
	CComPtr<IDispatch>				pDispatch;
	CComPtr<IHTMLElementCollection> pAll;
	CComQIPtr<IHTMLFormElement>		pForm;

	_bstr_t FormToSearch( FormName );
	_bstr_t FieldToSearch( ElementName );


	HRESULT hr;

	// Get document.all from html page

	hr = pDocument->get_all( &pAll );

	if ( !FAILED( hr ) )
		// query form element
		hr = pAll->item( _variant_t( FormToSearch ), CComVariant( 0 ), (IDispatch**) &pDispatch);

	if ( !FAILED( hr ) ) 
	{
		// QueryInterface...
		pForm = pDispatch;
		// We want to reuse it
		pDispatch.Release();
	}
	// query the field into form items collection 
	if ( !FAILED(hr) && pForm != NULL )
		hr = pForm->item( _variant_t( FieldToSearch ), CComVariant( 0 ), (IDispatch**) &pDispatch);
	*ppDispatch = pDispatch ;
	return  hr;
}

HRESULT FDocument::QueryHtmlElementValueListBox( ATL::CString & retVal, LPCTSTR Form, LPCTSTR Field, BOOL bPutValue)
{
	LPDISPATCH pDispatch;
	CComQIPtr<IHTMLSelectElement> pElement;

	HRESULT hr;
	hr = QueryHtmlElementFormElement( Form, Field, & pDispatch );
	pElement = pDispatch;
	if ( pElement )
	{

		if ( ! bPutValue )
		{
			LONG p;
			pElement->get_selectedIndex( &p );
			if ( p != -1 ) // we have some option selected
			{	
				CComPtr<IDispatch> pValue;
				CComQIPtr<IHTMLOptionElement> pOption;
				pElement->item( _variant_t( p ),  CComVariant( 0 ), &pValue );
				pOption = pValue;
				BSTR value;
				pOption->get_value( &value );
				retVal = ATL::CString( value );
			}
			else
			{
				retVal = "";
			}
			// GetValue

		}
		else
		{
		}
	}
	return hr;
}


HRESULT FDocument::GetSetText( ATL::CString & retVal, LPCTSTR Form, LPCTSTR Field, BOOL bPutValue )
{
	LPDISPATCH pDispatch;
	CComQIPtr<IHTMLInputElement> pElement;

	BSTR bstrValue;
	HRESULT hr;
	QueryHtmlElementFormElement( Form, Field, & pDispatch );
	pElement = pDispatch;
	if ( pElement )
	{
		if ( ! bPutValue )
		{
			// GetValue
			hr |= pElement->get_value( &bstrValue );
			// set retval
			retVal = bstrValue;
		}
		else
			hr |= pElement->put_value( _bstr_t( retVal ) );
	}
	return hr;
}

HRESULT FDocument::GetSetCheck( BOOL & retVal, LPCTSTR Form, LPCTSTR Field, BOOL bPutValue)
{
	LPDISPATCH pDispatch;
	CComQIPtr<IHTMLInputElement> pElement;

	HRESULT hr;
	QueryHtmlElementFormElement( Form, Field, & pDispatch );
	pElement = pDispatch;
	if ( pElement )
	{

		if ( ! bPutValue )
		{
			// GetValue
			VARIANT_BOOL bChecked;
			hr |= pElement->get_checked( &bChecked);
			// set retval
			retVal = bChecked == VARIANT_TRUE ? true : false;
		}
		else
		{
			VARIANT_BOOL bChecked;
			bChecked = retVal == TRUE ? VARIANT_TRUE : VARIANT_FALSE;
			hr |= pElement->put_checked( bChecked );
		}
	}
	return hr;
}

HRESULT FDocument::SetCheck(LPCSTR Form, LPCSTR Field, BOOL bValue)
{
	BOOL bVal = bValue; 
	return GetSetCheck(bVal, Form, Field, TRUE);
}

BOOL FDocument::GetCheck(LPCSTR Form, LPCSTR Field)
{
	BOOL bVal = FALSE; 
	GetSetCheck(bVal, Form, Field, FALSE); 
	return bVal; 
}

FString FDocument::GetText(LPCTSTR Form, LPCTSTR Field)
{
	FString aRes; 
	GetSetText(aRes, Form, Field, FALSE);
	return aRes; 
}

DWORD FDocument::GetDword(LPCTSTR Form, LPCTSTR Field)
{
	FString aRes = GetText(Form, Field);
	return strtoul(aRes, NULL, 10);
}

DWORD FDocument::GetDword(LPCSTR Form, LPCTSTR Field, DWORD& dwValOut)
{
	dwValOut = GetDword(Form, Field); 
	return dwValOut; 
}

HRESULT FDocument::SetDword(LPCSTR Form, LPCTSTR Field, DWORD dwValue)
{
	FString aText; 
	aText.Format("%u", dwValue); 
	return GetSetText(aText, Form, Field, TRUE); 
}

HRESULT FDocument::SetText(LPCSTR Form, LPCSTR Field, ATL::CString& Value)
{
	return GetSetText(Value, Form, Field, TRUE); 
}

HRESULT FDocument::SetElementInnerText(LPCSTR ElementName, LPCSTR InnerText)
{
	CComPtr<IDispatch>	pDispatch;
	HRESULT hr = QueryHtmlElement(ElementName, &pDispatch); 
	if (SUCCEEDED(hr))
	{
		CComQIPtr<IHTMLElement> pEle = pDispatch; 
		if (pEle)
			hr = pEle->put_innerText(_bstr_t(InnerText));
	}
	return hr; 
}

HRESULT FDocument::SetElementInnerHTML(BSTR ElementName, BSTR InnerHTML)
{
	CComPtr<IDispatch>				pDispatch;
	CComPtr<IHTMLElementCollection> pAll;
	HRESULT hr;
	hr = pDocument->get_all( &pAll );
	if ( !FAILED( hr ) )
		// query form element
		hr = pAll->item( _variant_t( ElementName ), _variant_t( (LONG)0 ), (IDispatch**) &pDispatch);

	if (SUCCEEDED(hr))
	{
		CComQIPtr<IHTMLElement> pEle = pDispatch; 
		if (pEle)
			hr = pEle->put_innerHTML(_bstr_t(InnerHTML));
		else
			hr = E_FAIL; 
	}

	return hr; 
}

HRESULT CWTLIExplorer::SetObjectHandler( INT iType, LPCTSTR objectName,  DWORD pMember, DWORD pThis)
{
	LPDISPATCH				ppHandler;
	CComPtr<IDispatch>		pDispatch;
	CComQIPtr<IHTMLElement> pElement;

	IFHtmlEventHandler * pEventHandler;

	// Create a new COM-Dual Interface-IDispatch-Object
	CComPtr<IFHtmlEventHandler> pp;
	HRESULT hr ;

	hr = ::CoCreateInstance( _uuidof( FHtmlEventHandler  ), NULL, CLSCTX_ALL, __uuidof(IFHtmlEventHandler), (void**) &pEventHandler );

	if (S_OK == hr && NULL != pEventHandler)
	{
		// Set the pointer to the member that will handle this event...
		pEventHandler->SetHandler( pThis, pMember );

		// Query object for IDispatch implementation
		hr = pEventHandler->QueryInterface( IID_IDispatch, (void**)& ppHandler );

		// Query the object to set it handle (part 1)
		if ( hr == S_OK && SUCCEEDED( QueryHtmlElement( objectName, &pDispatch ) ) )
		{
			// Query the IHTMLElement Interface (part 2)
			pElement = pDispatch;
			if (pElement)
			{
				_variant_t vHandler(ppHandler);

				// Put the new object handler, we should use a VARIANT type
				hr = pElement->put_onclick( vHandler ) ;
			}
		}
		pEventHandler->Release();

		return S_OK;
	}

	return E_NOTIMPL;
}

EVENTFN CWTLIExplorer::event_OnNavigateError(IDispatch* pDisp, VARIANT *URL, VARIANT *TargetFrameName, VARIANT *StatusCode, VARIANT_BOOL *Cancel)
{
	USES_CONVERSION; 
	CComQIPtr<IWebBrowser2> pWebBrowser = pDisp; 
    *Cancel = VARIANT_TRUE; 
	if (pWebBrowser)
	{
		BOOL bMainFrame = pWebBrowser.IsEqualObject(m_spWB);
		//notify parent that the frame failed to load.
		CAtlString pStrName = OLE2T(URL->bstrVal);
        if (m_pNotify)
            if (m_pNotify->OnNavigateError(m_dwID, bMainFrame, pStrName))
                *Cancel = VARIANT_FALSE; 
	}
	
}

EVENTFN CWTLIExplorer::event_OnDocumentComplete( IDispatch* pDisp,  VARIANT* URL )
{
	HRESULT hr = E_FAIL; 

	//We only process the Top-level frame
	CComQIPtr<IWebBrowser2> pWebBrowser = pDisp; 

    if (!pWebBrowser)
    {
        ATLASSERT(FALSE);
        return;
    }

	if (pWebBrowser.IsEqualObject(m_spWB))
	{
		m_spWB->put_RegisterAsDropTarget(VARIANT_FALSE); 
		CComPtr<IDispatch> pDispDoc; 
		hr = pWebBrowser->get_Document(&pDispDoc);

		if (SUCCEEDED(hr) && pDispDoc)
		{
			pDocument = pDispDoc; 
            if (_bstr_t("about:blank") != _bstr_t(URL->bstrVal) )
            {
                if (m_pNotify)
                    m_pNotify->OnDocumentComplete(m_dwID, TRUE);
            }
		}
	}
    else
    {
		if (m_pNotify)
              m_pNotify->OnDocumentComplete(m_dwID, FALSE); 
    }
	CAxWindow::SetExternalDispatch( (IDispatch*) _Module.m_LibraryObject );
	CComQIPtr<IDocHostUIHandlerDispatch> pExtUI = _Module.m_UIHandler;
	if (pExtUI)
		SetExternalUIHandler(pExtUI);
}

EVENTFN CWTLIExplorer::event_OnProgressChange(long lCur, long lMax)
{
    if (m_pNotify)
        m_pNotify->OnProgressChange(m_dwID, lCur, lMax); 
}

EVENTFN CWTLIExplorer::event_OnBeforeNavigate2(IDispatch *pDisp, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel)
{
	*Cancel = VARIANT_FALSE; 
	USES_CONVERSION;
	if (m_pNotify)
	{
		if (m_pNotify->OnBeforeNavigate(m_dwID, OLE2T(URL->bstrVal)) == FALSE)
			*Cancel = TRUE; 
	}
}

EVENTFN CWTLIExplorer::event_OnNavigateComplete(IDispatch* pDisp, VARIANT* URL )
{
	USES_CONVERSION;
	if (m_pNotify)
	{
		m_pNotify->OnNavigateComplete(m_dwID, OLE2T(URL->bstrVal));
	}
}

EVENTFN CWTLIExplorer::event_OnDownloadBegin()
{
	if (m_pNotify)
	{
		m_pNotify->OnDownloadBegin(m_dwID); 
	}

}

EVENTFN CWTLIExplorer::event_OnNewWindow2(IDispatch **ppDisp, VARIANT_BOOL *Cancel)
{
	*Cancel = VARIANT_TRUE; 
}

EVENTFN CWTLIExplorer::event_OnNewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags, BSTR bstrUrlContext, BSTR bstrUrl)
{
	USES_CONVERSION; 
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
		*Cancel = VARIANT_TRUE; 
	else
		ShellExecute(NULL, "open", OLE2T(bstrUrl), "", "", SW_SHOW); 
}

HRESULT CWTLIExplorer::Navigate(const CHAR* pszPath, const tchar* pAdditionalHeaders, DWORD dwNavFlags)
{
	USES_CONVERSION; 
	ATLASSERT(IsWindow()); //Browser must be created before this
	if (pszPath == NULL)
		return E_FAIL; 

	if (m_spWB == NULL)
		return E_POINTER; 

	FString FullPath; 
	CComVariant vtHeaders;

	if (!PathIsURL(pszPath))
	{
		//CHAR lpszDir[MAX_PATH]="";
		//GetCurrentDirectory( MAX_PATH, lpszDir);
		FullPath.Format("file://%s", pszPath);
        
	}
	else
	{
		FullPath = pszPath; 
		if (pAdditionalHeaders && strlen(pAdditionalHeaders) > 1 && strchr(pAdditionalHeaders, ':')!=NULL)
			vtHeaders = pAdditionalHeaders; 
	}
	CComVariant vtFlags = dwNavFlags; 

	return  m_spWB->Navigate(_bstr_t( FullPath ), &vtFlags, NULL, NULL, &vtHeaders);
}



HRESULT CWTLIExplorer::SetBodyFocus()
{
	HRESULT hr = E_FAIL;
	if (pDocument)
	{
		CComPtr<IHTMLElement> pBodyEle; 
		hr = pDocument->get_body(&pBodyEle);
		if (SUCCEEDED(hr))
		{
			CComQIPtr<IHTMLElement2> pBody = pBodyEle;
			if (pBody)
				hr = pBody->focus(); 
		}
	}
	return hr; 
}