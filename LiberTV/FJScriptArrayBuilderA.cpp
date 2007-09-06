#include "stdafx.h"
#include "FJScriptArrayBuilderA.h"
#include "StrSafe.h"

HRESULT FJScriptArrayBuilder::SetDispatch(IDispatch* pDispScript, IDispatch* pDispOutArray)
{

	HRESULT hr = E_FAIL;
	hr = m_OutArray.SetArray(pDispOutArray);

	if (FAILED(hr))
		return hr; 

	m_pDispEx = pDispScript;
	if (m_pDispEx)
	{
		//Get ID of Array object
		DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};

		BSTR bstrName = SysAllocString(OLESTR("Array"));
		hr = m_pDispEx->GetDispID(bstrName, 0, &m_pDispArray);
		SysFreeString(bstrName);
	}
	return hr; 
}

void FJScriptArrayBuilder::End()
{
	if (m_CurrentArray.HasArray())
	{
		CComVariant vtval = (double)m_CurrentArray.GetCRC32(); 
		m_CurrentArray.AddProperty("crc32", &vtval);
		m_CurrentArray.ReleaseArray();
	}
}

HRESULT FJScriptArrayBuilder::CreateJSArray(CComPtr<IDispatch>& pArrayOut)
{
	DISPPARAMS dispparamsNoArgs = {NULL, NULL, 0, 0};
	CComVariant var; 
	//Create a new javascript Array object
	//var = new Array;
	HRESULT hr = m_pDispEx->InvokeEx(m_pDispArray, LOCALE_USER_DEFAULT, DISPATCH_CONSTRUCT, &dispparamsNoArgs, &var, NULL, NULL);
	if (SUCCEEDED(hr))
		pArrayOut = var.pdispVal; 
	return hr; 
}

void FJScriptArrayBuilder::NewElement(const tchar* pszSeparator )
{
	CComPtr<IDispatch> pNewArray; 

	HRESULT hr = CreateJSArray(pNewArray);

	if (SUCCEEDED(hr))
	{
		if (m_CurrentArray.HasArray())
		{
			CComVariant vtval = (double)m_CurrentArray.GetCRC32(); 
			m_CurrentArray.AddProperty("crc32", &vtval);
			m_CurrentArray.ReleaseArray();
		}

		if (SUCCEEDED(m_CurrentArray.SetArray(pNewArray)))
		{
			//Add the new array to the output array.
			hr = m_OutArray.AddElementDispatch(pNewArray);
		}
	}

	if (FAILED(hr))
		ATLASSERT(FALSE);
}

void FJScriptArrayBuilder::AddArrayFromString(const tchar* pszString, const tchar* pszSepa )
{
	if (!m_CurrentArray.HasArray())
		return ; 

	CComPtr<IDispatch> pDispLabelsArray;
	//Create the Labels array
	HRESULT hr = CreateJSArray(pDispLabelsArray); 
	if (SUCCEEDED(hr))
	{
		CComVariant vtval = pDispLabelsArray; 
		//Create a labels property in current array
		//m_CurrentArray.labels = labels array
		hr = m_CurrentArray.AddProperty("labels", &vtval);
		if (SUCCEEDED(hr))
		{
			FJSArrayCRC32 LabelsArray; 
			hr = LabelsArray.SetArray(pDispLabelsArray); 
			if (SUCCEEDED(hr))
			{
				LabelsArray.AddElementsFromString(pszString, pszSepa);
				//Save the computed CRC32
				CComVariant vt = (double)LabelsArray.GetCRC32();
				LabelsArray.AddProperty("crc32", &vt);
				LabelsArray.ReleaseArray();
			}
		}
	}
}

void FJScriptArrayBuilder::AddValue(const tchar* pszName, const tchar* pFmt, ...)
{
	va_list paramList;
	va_start(paramList, pFmt);

	char Temp[2048];
	StringCbVPrintf(Temp, 2048, pFmt, paramList);
	if (m_CurrentArray.HasArray())
	{
		m_CurrentArray.AddElementStr(Temp); 
	}
}

void FJScriptArrayBuilder::AddValueUINT(const tchar* pszName, UINT uiVal)
{
	if (m_CurrentArray.HasArray())
		m_CurrentArray.AddElementUINT(uiVal); 
}

void FJScriptArrayBuilder::AddValueStr(const tchar* pszName, const tchar* pszVal)
{
	if (m_CurrentArray.HasArray())
		m_CurrentArray.AddElementStr(pszVal); 
}

void FJScriptArrayBuilder::AddValueVariant(VARIANT* pVar)
{
	if (m_CurrentArray.HasArray())
		m_CurrentArray.AddElement(pVar);
}

void FJScriptArrayBuilder::AddValueDouble(const tchar* pszName, double dblVal)
{
	if (m_CurrentArray.HasArray())
		m_CurrentArray.AddElementDouble(dblVal); 
}

void FJScriptArrayBuilder::AddValueUINT64(const tchar* pszName, UINT64 ui64Val)
{
	if (m_CurrentArray.HasArray())
		m_CurrentArray.AddElementUINT64(ui64Val);
}