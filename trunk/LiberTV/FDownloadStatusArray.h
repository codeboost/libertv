#pragma once

#include "Utils.h"
#include "atlcoll.h"
#include "IJScriptArray.h"
#include "crc32.h"


/*
	FDownloadStatusArray is an object used to get the download status from the download object. 
	It stores all values a value maps between property name and VARIANT value of the property.

*/

typedef CAtlMap<FString, CComVariant> FValueMapT; 

class FValueMap : public FValueMapT
{
	typedef FValueMap::CPair FValueMapPair; 
public:

	const CComVariant* GetValueVariant(const char* pszKey)
	{
		FValueMapPair* pPair = Lookup(pszKey); 
		if (NULL == pPair)
			return NULL; 
		CComVariant& v = pPair->m_value; 
		return &v; 
	}

	FString GetValueStr(const char* pszKey)
	{	
		FValueMapPair* pPair = Lookup(pszKey); 
		if (NULL == pPair)
			return ""; 

		CComVariant& v = pPair->m_value; 

		if (v.vt == VT_BSTR)
		{
			USES_CONVERSION; 
			return OLE2T(v.bstrVal); 
		}
		else
			if (v.vt == VT_UI4)
			{
				char szBuf[sizeof("4294967295")];
				ultoa((ulong)v.uintVal, szBuf, 10); 
				return szBuf; 
			}
			else
				if (v.vt == VT_R8)
				{
					FString S; 
					S.Format("%f", v.dblVal); 
					return S; 
				}
				return ""; 
	}
	UINT GetValueUINT(const char* pszKey)
	{
		FValueMapPair* pPair = Lookup(pszKey); 
		if (NULL == pPair)
			return 0; 

		CComVariant& v = pPair->m_value; 

		if (v.vt == VT_UI4)
			return v.uintVal; 
		else
			if (v.vt == VT_BSTR)
			{
				USES_CONVERSION; 
				return (UINT)strtoul(OLE2T(v.bstrVal), NULL, 10); 
			}
			else
				if (v.vt == VT_R8)
					return (UINT)v.dblVal; 
		return 0; 
	}

	UINT64 GetValueUINT64(const char* pszKey)
	{
		FValueMapPair* pPair = Lookup(pszKey); 
		if (NULL == pPair)
			return 0; 

		CComVariant& v = pPair->m_value; 
		return (UINT64)v.dblVal; 
	}

	double GetValueDouble(const char* pszKey)
	{
		FValueMapPair* pPair = Lookup(pszKey); 
		if (NULL == pPair)
			return 0.0; 
		CComVariant& v = pPair->m_value; 
		return v.dblVal; 
	}

	UINT m_VideoId; 
	FValueMap()
	{
		m_VideoId = 0; 
	}
	inline void SetVideoId(UINT uiVideoId)
	{
		m_VideoId = uiVideoId; 
	}
	inline UINT GetVideoId()
	{
		return m_VideoId; 
	}
};

class FDownloadStatusArray : public IStatusArrayBuilder
{
	CAtlArray<FValueMap> m_aValues; 
public:
	void New() 
	{
		m_aValues.RemoveAll();
	}
	void NewElement(const tchar* pszSeparator = ";")
	{
		m_aValues.Add(); 
	}
	void AddValue(const tchar* pszName, const tchar* pFmt, ...)
	{
		ATLASSERT(!m_aValues.IsEmpty()); //Must call NewElement()
		if (m_aValues.IsEmpty())
			return; 

		FValueMap& v = m_aValues.GetAt(m_aValues.GetCount() - 1); 

		va_list paramList;
		va_start(paramList, pFmt);

		char Temp[2048];
		StringCbVPrintf(Temp, 2048, pFmt, paramList);
		v.SetAt(pszName, CComVariant(Temp));

		if (strcmp(pszName, "videoID") == 0)
		{
			UINT uiVal = (UINT)strtoul(Temp, NULL, 10); 
			v.SetVideoId(uiVal); 
		}
	}
	void AddValueStr(const tchar* pszName, const tchar* pszVal)
	{
		ATLASSERT(!m_aValues.IsEmpty()); //Must call NewElement()
		if (!m_aValues.IsEmpty())
		{
			FValueMap& v = m_aValues.GetAt(m_aValues.GetCount() - 1);
			v.SetAt(pszName, CComVariant(pszVal)); 
		}
	}
	void AddValueUINT(const tchar* pszName, UINT uiVal)
	{
		ATLASSERT(!m_aValues.IsEmpty()); //Must call NewElement()
		if (!m_aValues.IsEmpty())
		{
			FValueMap& v = m_aValues.GetAt(m_aValues.GetCount() - 1);
			v.SetAt(pszName, CComVariant(uiVal)); 
		}
	}
	void AddValueDouble(const tchar* pszName, double dblVal)
	{
		ATLASSERT(!m_aValues.IsEmpty()); //Must call NewElement()
		if (!m_aValues.IsEmpty())
		{
			FValueMap& v = m_aValues.GetAt(m_aValues.GetCount() - 1);
			v.SetAt(pszName, CComVariant(dblVal)); 
		}
	}
	void AddValueUINT64(const tchar* pszName, UINT64 ui64Val)
	{
		ATLASSERT(!m_aValues.IsEmpty()); //Must call NewElement()
		if (!m_aValues.IsEmpty())
		{
			FValueMap& v = m_aValues.GetAt(m_aValues.GetCount() - 1);
			v.SetAt(pszName, CComVariant((double)ui64Val)); 
		}
	}
	void AddArrayFromString(const tchar* pszString, const tchar* pszSepa = ",")
	{
	}
	BSTR GetAsBSTR() {return NULL;}

	FValueMap& GetAt(size_t k)
	{
		ATLASSERT(k < m_aValues.GetCount()); 
		return m_aValues[k];
	}
	size_t GetCount()
	{
		return m_aValues.GetCount(); 
	}

	BOOL HasVideoId(UINT uiVideoId)
	{
		size_t Count = m_aValues.GetCount(); 		
		for (size_t k = 0; k < Count; k++)
			if (m_aValues[k].GetVideoId() == uiVideoId)
				return TRUE; 
		return FALSE; 
	}

	FValueMap* Lookup(UINT uiVideoId)
	{
		//		CAtlMap<UINT, FValueMap*>::CPair* p = m_VideoMap.Lookup(uiVideoId); 
		//		if (p)
		//			return p->m_value; 

		size_t Count = m_aValues.GetCount(); 		
		for (size_t k = 0; k < Count; k++)
			if (m_aValues[k].GetVideoId() == uiVideoId)
				return &m_aValues[k]; 
		return NULL; 
	}

	void AddProperty(const char* pszName, CComVariant& pValue)
	{

	}

};