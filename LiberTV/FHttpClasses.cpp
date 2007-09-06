#include "stdafx.h"
#include "FHttpClasses.h"


BOOL FHttpClass::IsNull()
{
	return (m_Headers.GetCount() == 0) && (m_Data.GetCount() == 0); 
}

void FHttpClass::AddHeader(const char* pHeader)
{
	if (pHeader && strlen(pHeader) > 0)
	{
		DataItem* pItem = new DataItem(pHeader, (DWORD)strlen(pHeader));
		pItem->m_pData[pItem->m_Size] = 0; 
		m_Headers.AddTail(pItem); 
	}
}

void FHttpClass::AddData  (const char* pData, DWORD dwSize)
{
	if (pData && dwSize > 0)
	{
		m_Data.AddTail(new DataItem(pData, dwSize));
	}
}

POSITION FHttpClass::FindHeader(const char* pHeaderName)	//very primitive and error-prone
{
	POSITION nextpos = m_Headers.GetHeadPosition();

	while (nextpos)
	{
		POSITION curpos = nextpos; 
		DataItem* pItem = m_Headers.GetNext(nextpos); 
		if (strstr(pItem->m_pData, pHeaderName) != 0)
		{
			return curpos;
		}
	}
	return 0; 
}

void FHttpClass::RemoveHeader(const char* pHeaderName)
{
	POSITION pos = FindHeader(pHeaderName); 
	if (pos)
	{
		delete m_Headers.GetAt(pos); 
		m_Headers.RemoveAt(pos); 
	}
}

void FHttpClass::AdjustContentLength()
{
	POSITION pos = m_Data.GetHeadPosition();

	DWORD dwTotal = 0; 
	while (pos)
	{
		DataItem* pItem = m_Data.GetNext(pos); 
		dwTotal+=pItem->m_Size;
	}

	RemoveHeader("Content-Length"); 
	SetContentLength(dwTotal); 
}

void FHttpClass::SetContentLength(DWORD dwLen)
{
	CAtlString aFormat; 
	aFormat.Format("Content-Length: %d", dwLen); 
	AddHeader(aFormat); 
}

void FHttpClass::Clear()
{

	POSITION pos = m_Headers.GetHeadPosition();

	while (pos)
	{
		delete m_Headers.GetNext(pos); 
	}

	pos = m_Data.GetHeadPosition();

	while (pos)
	{
		delete m_Data.GetNext(pos); 
	}

	m_Headers.RemoveAll(); 
	m_Data.RemoveAll(); 
}

BOOL FHttpClass::BuildHeaders(CAtlString& aHeaderOut)
{
	POSITION pos = m_Headers.GetHeadPosition();

	DWORD dwHeaderSize = 0; 
	while (pos)
	{
		DataItem* pItem = m_Headers.GetNext(pos); 
		dwHeaderSize+=pItem->m_Size; 
	}

	if (dwHeaderSize > 0)
	{
		dwHeaderSize+= 2* (DWORD)m_Headers.GetCount(); //newlines for each header
		dwHeaderSize+=2; //final \r\n

		aHeaderOut.GetBufferSetLength(dwHeaderSize); 
		pos = m_Headers.GetHeadPosition();

		char* pOut = aHeaderOut.GetBuffer();
		*pOut = 0; 

		while (pos)
		{
			DataItem* pItem = m_Headers.GetNext(pos); 

			if (pItem)
			{
				if (pItem->m_pData && pItem->m_Size > 0)
				{
					strncat(pOut, pItem->m_pData, pItem->m_Size);
					strcat(pOut, "\r\n"); 
				}
			}
		}			   
		strcat(pOut, "\r\n"); 
	}

	return dwHeaderSize > 0; 
}

//////////////////////////////////////////////////////////////////////////

BOOL FHttpResponse::BuildResponse(DataItem& aBuffer)
{
	FString Headers;
	AdjustContentLength();
	if (BuildHeaders(Headers))
	{
		POSITION pos = m_Data.GetHeadPosition();

		DWORD dwTotal = 0; 
		while (pos)
		{
			DataItem* vi = m_Headers.GetNext(pos); 
			dwTotal+=vi->m_Size;
		}
		
		aBuffer.m_Size = dwTotal + Headers.GetLength();
		aBuffer.m_pData = new char[aBuffer.m_Size];

		char* pBuf = aBuffer.m_pData;
		memcpy(pBuf, (const char*)Headers, Headers.GetLength()); 

		pBuf+=Headers.GetLength(); 

		pos = m_Data.GetHeadPosition(); 
		while (pos)
		{
			DataItem* vi = m_Headers.GetNext(pos); 
			memcpy(pBuf, vi->m_pData, vi->m_Size);
			pBuf+=vi->m_Size; 
		}
		return TRUE; 
	}
	return FALSE; 
}









