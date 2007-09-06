#pragma once

#include "dispex.h"
#include "comdef.h"
#include <activscp.h>
#include "crc32.h"

//Macros for setting DISPPARAMS structures
#define SETDISPPARAMS(dp, numArgs, pvArgs, numNamed, pNamed) \
{\
	(dp).cArgs=numArgs;\
	(dp).rgvarg=pvArgs;\
	(dp).cNamedArgs=numNamed;\
	(dp).rgdispidNamedArgs=pNamed;\
}

UINT VariantToUINT(VARIANT &vtVar);

class FDispObject
{
public:
	CComPtr<IDispatch> m_pDisp; 
	void	SetDispatch(IDispatch* pDisp);
	HRESULT AddProperty( const tchar* pszName, VARIANT* vtValue );
	HRESULT GetProperty(const tchar* pszName, VARIANT* vtValue);
	HRESULT CreateObject(IDispatch* pDispScript); 
	FString	GetPropertyStr(const tchar* pszName); 
	UINT	GetPropertyUINT(const tchar* pszName); 
	HRESULT AddPropertyStr(const tchar* pszName, const tchar* pszValue); 
	HRESULT AddPropertyUINT(const tchar* pszName, UINT uiVal); 
};
class FJSArray
{
public:
	DISPID				m_pJSPush;		//dispid of the push() javascript function
	CComPtr<IDispatch>	m_pArray; 
	FDispObject			m_DispObject; 
	~FJSArray(); 
	HRESULT SetArray(IDispatch* pArray);
	HRESULT AddProperty(const tchar* pszName, VARIANT* vtValue);
	HRESULT AddElement(VARIANT *pVar);
	HRESULT AddElementStr(const tchar* pszStr);
	HRESULT AddElementDispatch(IDispatch* pDisp);
	HRESULT AddElementUINT(UINT uiVal);
	HRESULT AddElementDouble(double dblVal);
	HRESULT AddElementUINT64(UINT64 uiVal);
	HRESULT AddElementsFromString(const tchar* pszString, const tchar* pszSepa); 
};

//Computes CRC for each added item
class FJSArrayCRC32
{
	FJSArray m_JSBuilder;
	uint32_t		m_CRC32; 
public:

	FJSArrayCRC32()
	{
		m_CRC32 = 0; 
	}
	BOOL	HasArray(){
		return m_JSBuilder.m_pArray != NULL; 
	}
	void	ReleaseArray()
	{
		if (m_JSBuilder.m_pArray != NULL)
			m_JSBuilder.m_pArray.Release();
	}
	HRESULT SetArray(IDispatch* pArray)
	{
		m_CRC32 = 0; //reset the CRC for current array, BLEA (took me two days to debug this)
		return m_JSBuilder.SetArray(pArray);
	}
	HRESULT AddProperty(const tchar* pszName, VARIANT* vtValue)
	{
		return m_JSBuilder.AddProperty(pszName, vtValue); 
	}
	HRESULT AddElement(VARIANT *pVar)
	{
		switch(pVar->vt)
		{
		case VT_UI4:
			m_CRC32 = crc32(&pVar->uintVal, sizeof(UINT), m_CRC32);
			break; 
		case VT_R8:
			m_CRC32 = crc32(&pVar->dblVal, sizeof(double), m_CRC32); 
			break; 
		}
		return m_JSBuilder.AddElement(pVar); 
	}
	HRESULT AddElementStr(const tchar* pszStr)
	{
		m_CRC32 = crc32(pszStr, strlen(pszStr), m_CRC32); 
		return m_JSBuilder.AddElementStr(pszStr); 
	}
	HRESULT AddElementDispatch(IDispatch* pDisp)
	{
		return m_JSBuilder.AddElementDispatch(pDisp);
	}
	HRESULT AddElementUINT(UINT uiVal)
	{
		m_CRC32 = crc32(&uiVal, sizeof(UINT), m_CRC32); 
		return m_JSBuilder.AddElementUINT(uiVal); 
	}
	HRESULT AddElementDouble(double dblVal)
	{
		INT64 intVal = (INT64)(dblVal * 10000);
		m_CRC32 = crc32(&intVal, sizeof(INT64), m_CRC32); 
		return m_JSBuilder.AddElementDouble(dblVal); 
	}
	HRESULT AddElementUINT64(UINT64 uiVal)
	{
		m_CRC32 = crc32(&uiVal, sizeof(UINT64), m_CRC32); 
		return m_JSBuilder.AddElementUINT64(uiVal); 
	}
	HRESULT AddElementsFromString(const tchar* pszString, const tchar* pszSepa)
	{
		m_CRC32 = crc32(pszString, strlen(pszString), m_CRC32); 
		return m_JSBuilder.AddElementsFromString(pszString, pszSepa); 
	}
	uint32_t GetCRC32()
	{
		return m_CRC32; 
	}

};

