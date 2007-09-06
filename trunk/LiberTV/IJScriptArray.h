#pragma once

class IStatusArrayBuilder{

public:
	virtual ~IStatusArrayBuilder(){}
	virtual void New() = 0; 
	virtual void NewElement(const char* pszSeparator = ";") = 0; 
	virtual void AddValue(const char* pszName, const char* pFmt, ...) = 0; 
	virtual void AddValueStr(const char* pszName, const char* pszVal) = 0; 
	virtual void AddValueUINT(const char* pszName, UINT uiVal) = 0; 
	virtual void AddValueDouble(const char* pszName, double dblVal) = 0; 
	virtual void AddValueUINT64(const char* pszName, UINT64 ui64Val) = 0; 
	virtual void AddArrayFromString(const char* pszString, const char* pszSepa = ",") = 0;
	virtual BSTR GetAsBSTR() = 0; 
};
