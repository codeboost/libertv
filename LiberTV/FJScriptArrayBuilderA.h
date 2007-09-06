#pragma once
/*
Build a string which looks like:
value, value, value; value, value, value;
which can then be split into two arrays:
ar1 = str.split(";"); 
ar1 now contains an array of comma-separated strings.
Each element can be split to produce a new array

*/
#include "IJScriptArray.h"
#include "FJSArray.h"


//Builds a javascript array of arrays
//pDispOutArray will contain elements of type array, each array will contain 
//a variable number of elements, as called by AddValue()
class FJScriptArrayBuilder : public IStatusArrayBuilder
{

	FJSArrayCRC32			m_OutArray; 
	FJSArrayCRC32			m_CurrentArray; //Current element
	DISPID					m_pDispArray; //Array object
	CComQIPtr<IDispatchEx>	m_pDispEx;	  //DispEx of the Script engine
public:
	HRESULT SetDispatch(IDispatch* pDispScript, IDispatch* pDispOutArray);

	~FJScriptArrayBuilder()
	{

	}
	void New(){}; 
	HRESULT		CreateJSArray(CComPtr<IDispatch>& pArrayOut); 
	void NewElement(const tchar* pszSeparator /* =  */);
	void AddValue(const tchar* pszName, const tchar* pFmt, ...); 
	void AddValueStr(const tchar* pszName, const tchar* pszVal);
	void AddValueDouble(const tchar* pszName, double dblVal);
	void AddValueUINT(const tchar* pszName, UINT uiVal); 
	void AddValueUINT64(const tchar* pszName, UINT64 ui64Val); 
	void AddValueVariant(VARIANT* pVar); 
	void AddArrayFromString(const tchar* pszString, const tchar* pszSepa /* =  */);
	FJSArrayCRC32& GetArray(){return m_CurrentArray;} 
	void End();

	BSTR GetAsBSTR(){return NULL;}
};

//Copies all values to an array, which is then used to build the real JS array
class FArrayBuilderHelper : public IStatusArrayBuilder
{
public:
	typedef CAtlArray <CComVariant> ElementArray; 
	CAtlArray< ElementArray* > m_vtValues;
	
	~FArrayBuilderHelper()
	{
		for (size_t k = 0; k < m_vtValues.GetCount(); k++)
			delete m_vtValues[k];

		m_vtValues.RemoveAll();
	}

	virtual void New()
	{
		for (size_t k = 0; k < m_vtValues.GetCount(); k++)
			delete m_vtValues[k];

		m_vtValues.RemoveAll();
	}

	virtual void NewElement(const tchar* pszSeparator = ";")
	{
		ElementArray* blank = new ElementArray;
		m_vtValues.Add(blank);
	}

	ElementArray& GetArray(){
		return *m_vtValues[m_vtValues.GetCount() - 1];
	}

	virtual void AddValue(const tchar* pszName, const tchar* pFmt, ...)
	{
		va_list paramList;
		va_start(paramList, pFmt);
		FString Temp; 
		Temp.FormatV(pFmt, paramList);
		va_end(paramList);
		
		GetArray().Add(CComVariant(Temp));
	}
	virtual void AddValueStr(const tchar* pszName, const tchar* pszVal)
	{
		GetArray().Add(CComVariant(pszVal));
	}
	virtual void AddValueUINT(const tchar* pszName, UINT uiVal)
	{
		GetArray().Add(CComVariant(uiVal));
	}
	virtual void AddValueDouble(const tchar* pszName, double dblVal)
	{
		GetArray().Add(CComVariant(dblVal));
	} 
	virtual void AddValueUINT64(const tchar* pszName, UINT64 ui64Val)
	{
		GetArray().Add(CComVariant((double)ui64Val));
	}
	virtual void AddArrayFromString(const tchar* pszString, const tchar* pszSepa = ",")
	{

	}
	virtual BSTR GetAsBSTR() {return NULL;}
};


class FJScriptStringBuilder : public IStatusArrayBuilder
{
public:
	FString m_StrBuffer; 
	int		m_Items;
	int		m_Element; 
	void New()
	{
		m_Items = 0; 
		m_StrBuffer = "";
		m_Element = 0; 
	}

	void NewElement(const tchar* pSeparator = ";")
	{
		if (m_Items > 0)
			m_StrBuffer.Append(pSeparator); 
		m_Element = 0; 
		m_Items++;
	}

	void AddValue(const tchar* pszName, const tchar* pFmt, ...)
	{
		va_list paramList;
		va_start(paramList, pFmt);

		FString Temp; 
		
		if	(m_Element > 0)
			m_StrBuffer.Append(","); 

		
		Temp.FormatV(pFmt, paramList); 
		EscapeSeparator(Temp, ",");
		EscapeSeparator(Temp, ";");
		m_StrBuffer.Append(Temp); 
		m_Element++; 

		va_end(paramList);
	}

	void EscapeSeparator(FString& org, const tchar* pStrSepa)
	{
		FString tmp; 
		tmp.Format("%%%d", *pStrSepa); 
		org.Replace(pStrSepa, tmp); 
	}
	BSTR GetAsBSTR()
	{
		return m_StrBuffer.AllocSysString(); 
	}

	void AddValueStr(const tchar* pszName, const tchar* pszVal)
	{
		AddValue(pszName, "%s", pszVal); 
	}
	void AddValueDouble(const tchar* pszName, double dblVal)
	{
		AddValue(pszName, "%f", dblVal);
	}
	void AddValueUINT(const tchar* pszName, UINT uiVal)
	{
		AddValue(pszName, "%d", uiVal); 
	}

	void AddValueUINT64(const tchar* pszName, UINT64 uiVal)
	{
		AddValue(pszName, "%I64d", uiVal); 
	}

	void AddArrayFromString(const tchar* pszString, const tchar* pszSepa /* =  */){

	}
};