#include "stdafx.h"
#include "FJSArray.h"
#include "strutils.h"


UINT VariantToUINT(VARIANT &vtVar)
{
	USES_CONVERSION; 

	if (vtVar.vt == VT_UINT)
		return vtVar.uintVal; 
	else
		if (vtVar.vt == VT_BSTR)
			return strtoul(OLE2T(vtVar.bstrVal), NULL, 10); 
		else
			if (vtVar.vt == VT_I4)
				return vtVar.lVal; 
	if (vtVar.vt == VT_R8)
		return (UINT)vtVar.dblVal;


	return 0; 
}

FJSArray::~FJSArray()
{
}

HRESULT FJSArray::SetArray( IDispatch* pArray )
{
	OLECHAR FAR* szMember = L"push";
	m_pArray = pArray; 
	m_DispObject.SetDispatch(pArray); 
	return m_pArray->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_SYSTEM_DEFAULT, &m_pJSPush);
}

HRESULT FJSArray::AddProperty( const tchar* pszName, VARIANT* vtValue )
{
	return m_DispObject.AddProperty(pszName, vtValue);
}

HRESULT FJSArray::AddElement( VARIANT *pVar )
{
	DISPPARAMS dispparams;
	HRESULT hr = E_FAIL; 
	SETDISPPARAMS(dispparams, 1, pVar, 0, NULL);
	hr = m_pArray->Invoke(m_pJSPush, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispparams, NULL, NULL, NULL);
	return hr;
}

HRESULT FJSArray::AddElementStr( const tchar* pszStr )
{
	USES_CONVERSION; 
	CComVariant vt = pszStr; 
	return AddElement(&vt);
}

HRESULT FJSArray::AddElementDispatch( IDispatch* pDisp )
{
	CComVariant vt = pDisp; 
	return AddElement(&vt);
}

HRESULT FJSArray::AddElementUINT( UINT uiVal )
{
	CComVariant vt = uiVal; 
	return AddElement(&vt);
}

HRESULT FJSArray::AddElementDouble( double dblVal )
{
	CComVariant vt = dblVal; 
	return AddElement(&vt);
}

HRESULT FJSArray::AddElementUINT64( UINT64 uiVal )
{
	CComVariant vt = (double)uiVal; 
	return AddElement(&vt);
}

HRESULT FJSArray::AddElementsFromString(const tchar* pszString, const tchar* pszSepa)
{
	FArray<FString> aStr;
	int nCount = SplitStringToArray(pszString, aStr, pszSepa);
	for (size_t k = 0; k < aStr.GetCount(); k++)
		AddElementStr(aStr[k]);
	return S_OK; 
}

HRESULT FDispObject::CreateObject(IDispatch* pDispScript)
{
	CComQIPtr<IDispatchEx> pExScript = pDispScript; 
	if (!pExScript)
		return E_NOINTERFACE; 

	CComBSTR bstrObject = L"Object";
	DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
	DISPID dispid; 

	//Construct a new Object
	//Get the dispid of the object
	HRESULT hr = pExScript->GetDispID(bstrObject, 0, &dispid);
	if (SUCCEEDED(hr))
	{
		//Construct it
		CComVariant var;
		hr = pExScript->InvokeEx(dispid, LOCALE_USER_DEFAULT, 
			DISPATCH_CONSTRUCT, &dispparamsNoArgs, 
			&var, NULL, NULL);
		if (SUCCEEDED(hr) && var.vt == VT_DISPATCH)
		{
			//Set as dispatch
			SetDispatch(var.pdispVal);
		}
	}
	return hr; 
}

void FDispObject::SetDispatch(IDispatch* pDisp)
{
	m_pDisp = pDisp;
}

HRESULT FDispObject::AddProperty( const tchar* pszName, VARIANT* vtValue )
{
	USES_CONVERSION;
	DISPID dispid; 
	CComQIPtr<IDispatchEx> pDispEx = m_pDisp ;
	if (NULL == pDispEx)
		return E_NOINTERFACE; 

	BSTR   bstrName = SysAllocString(T2OLE(pszName));
	HRESULT hr = pDispEx->GetDispID(bstrName, fdexNameEnsure, &dispid);
	SysFreeString(bstrName);
	if (SUCCEEDED(hr))
	{
		DISPID putid = DISPID_PROPERTYPUT;
		DISPPARAMS dispparams;
		dispparams.rgvarg = vtValue;
		dispparams.rgdispidNamedArgs = &putid;
		dispparams.cArgs = 1;
		dispparams.cNamedArgs = 1;
		hr = pDispEx->InvokeEx(dispid, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUTREF, &dispparams,	NULL, NULL, NULL);
	}
	return hr;
}

HRESULT FDispObject::GetProperty( const tchar* pszName, VARIANT* vtValue )
{
	USES_CONVERSION;
	DISPID dispid; 
	CComBSTR bstrName = T2OLE(pszName);
	HRESULT hr = m_pDisp->GetIDsOfNames(IID_NULL, &bstrName, 1, LOCALE_SYSTEM_DEFAULT, &dispid);

	if (SUCCEEDED(hr))
	{
		DISPPARAMS dispparams;
		dispparams.rgvarg = NULL;
		dispparams.rgdispidNamedArgs = NULL;
		dispparams.cArgs = 0;
		dispparams.cNamedArgs = 0;
		hr = m_pDisp->Invoke(dispid, IID_NULL,  LOCALE_SYSTEM_DEFAULT, DISPATCH_PROPERTYGET, &dispparams, vtValue, NULL, NULL);
	}
	return hr;
}

FString FDispObject::GetPropertyStr(const tchar* pszName)
{
	USES_CONVERSION; 
	CComVariant vt; 
	if (SUCCEEDED(GetProperty(pszName, &vt)) && vt.vt == VT_BSTR)
	{
		return OLE2T(vt.bstrVal);
	}
	return "";
}

UINT FDispObject::GetPropertyUINT(const tchar* pszName)
{
	USES_CONVERSION;
	CComVariant vt; 
	if (SUCCEEDED(GetProperty(pszName, &vt)))
	{
		return VariantToUINT(vt);
	}
	return 0;
}

HRESULT FDispObject::AddPropertyStr(const tchar* pszName, const tchar* pszValue)
{
	CComVariant vt = pszValue; 

	return AddProperty(pszName, &vt); 
}

HRESULT FDispObject::AddPropertyUINT(const tchar* pszName, UINT uiVal)
{
	CComVariant vt = uiVal; 
	return AddProperty(pszName, &vt); 
}