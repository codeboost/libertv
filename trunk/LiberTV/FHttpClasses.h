#ifndef __FHTTPCLASSES_H__
#define __FHTTPCLASSES_H__


#include "Utils.h"
#include "mz_Inc.h"


struct DataItem
{
	char*			m_pData; 
	DWORD			m_Size; 
	DataItem()
	{
		m_pData = NULL; 
		m_Size = 0; 
	}
	DataItem(const char* pData, DWORD pvSize)
	{
		m_pData = new char[pvSize + 1];
		m_Size = pvSize; 
		memcpy(m_pData, pData, pvSize);
	}
	void Clear()
	{
		delete m_pData; 
		m_pData = NULL; 
		m_Size = 0; 
	}
	~DataItem()
	{
		Clear(); 
	}

};

struct FHttpClass
{
	CAtlList<DataItem*> m_Headers; 
	CAtlList<DataItem*> m_Data; 

	FHttpClass()
	{

	}
	~FHttpClass()
	{
		Clear(); 
	}

	void	AddHeader(const char* pHeader)	;
	void	RemoveHeader(const char* pHeaderName);
	BOOL	BuildHeaders(CAtlString& aHeaderOut);
	POSITION FindHeader(const char* pHeaderName);
	
	void	AdjustContentLength();
	void	SetContentLength(DWORD dwLen);
	void	Clear();
	
	void	AddData  (const char* pData, DWORD dwSize);
	DWORD	GetDataLength(); 
	BOOL	IsNull(); 
};


struct FHttpResponse : public FHttpClass
{

public:
	BOOL BuildResponse(DataItem& aBuffer); 	
};


#endif //__FHTTPCLASSES_H__