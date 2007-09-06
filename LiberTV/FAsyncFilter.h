#ifndef __FASYNCFILTER_H__
#define __FASYNCFILTER_H__

#include "asyncio.h"
#include "asyncrdr.h"

#include <atlbase.h>
#include <atlstr.h>

#define _AFX
#include <atlcoll.h>

//File is available
#define FASYNC_AVAILABLE 0x01


struct CFileEntry
{
	CAtlString	m_FileName; 		
	LONGLONG	m_FileSize;
	DWORD		m_dwFlags; 

	CFileEntry()
	{
		m_FileSize = 0; 
		m_dwFlags = 0; 
	}
};
class FMediaRenderBase 
{
public:
	virtual void OnFileUnavailable() = 0; 
};

class CMemFileStream : public CAsyncStream
{
	LONGLONG	m_llPosition; 
	LONGLONG	m_llLength;	
	CCritSec	m_csLock; 
	FMediaRenderBase* m_pRenderer; 
public:
	CAtlArray<CFileEntry> m_Files;

	CMemFileStream()
	{
		m_llPosition = 0; 
		m_llLength = 0; 
		m_pRenderer = NULL; 
	}


	void SetRenderer(FMediaRenderBase* pRend)
	{
		m_pRenderer = pRend; 
	}

	HRESULT AddFile(LPCSTR szFileName, LONGLONG lFileSize, DWORD dwFlags);
	BOOL CopyMemFromFile(LPCSTR szFileName, PVOID ppOut, LONG Offset, DWORD dwBytesToRead);
	HRESULT Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead) ;
	HRESULT SetFileFlag(LPCSTR szFileName, DWORD dwFlags); 
	void	Clear(); 


	HRESULT SetPointer(LONGLONG llPos);

	LONGLONG Size(LONGLONG *pSizeAvailable)
	{
		if (pSizeAvailable != NULL)
			*pSizeAvailable = m_llLength; 
		return m_llLength;
	}

	DWORD Alignment(){ return 1; }
	void Lock()	{ m_csLock.Lock(); }
	void Unlock()	{ m_csLock.Unlock(); }

};

class CMemReader : public CAsyncReader
{
public:

	//  We're not going to be CoCreate'd so we don't need registration
	//  stuff etc
	STDMETHODIMP Register()
	{
		return S_OK;
	}

	STDMETHODIMP Unregister()
	{
		return S_OK;
	}

	CMemReader(CMemFileStream *pStream, CMediaType *pmt, HRESULT *phr) :
	CAsyncReader(NAME("FAsyncReader\0"), NULL, pStream, phr)
	{
		m_mt = *pmt;
	}
};
#endif //__FASYNCFILTER_H__