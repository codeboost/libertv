#include "stdafx.h"
#include "FQuickHttp.h"


FQHttp::FQHttp(int nThreads /* = 2 */)
{
	m_hHandle = 0; 
	m_bStopping = FALSE; 
	m_TaskQueue.SetSize(512);
	m_ThreadPool.Create(this, false, nThreads);
	
}

FQHttp::~FQHttp()
{
	Stop(); 
}

void FQHttp::Stop()
{
	m_bStopping = TRUE; 
	m_TaskQueue.Clear(); 
	m_ThreadPool.StopAndWait(5000); 
}

void FQHttp::FQHttpThreadPool::Thread( void* p )
{
	FQHttp* _this = (FQHttp*)p;

	for (;;)
	{
		if (_this->m_bStopping)
			break; 
		bool bPopped = false; 

		FQHttpInternal* v; 
		int nRes = _this->m_TaskQueue.PopWait(v, bPopped); 
		if (nRes == WAIT_OBJECT_0)
		{
			_this->DownloadItem(v); 
			_this->ReleaseItem(v); 
		}
		else
			break; 
	}
}

FQHandle FQHttp::DownloadURLToFile( FQHttpParams& pParams )
{
	//Some parameters checks
	if (!PathIsURL(pParams.m_Url))
		return 0; 

	if (pParams.m_pNotify == NULL)
		return 0; 

	FQHttpInternal* pObj = new FQHttpInternal;

	pObj->m_Params = pParams; 
	FQHandle hHandle = InterlockedIncrement(&m_hHandle);
	pObj->m_hHandle = hHandle; 
	m_TaskQueue.Push(pObj); 
	return hHandle;
}

void FQHttp::DownloadItem( FQHttpInternal* pDownloadParams )
{
	HRESULT hr = E_FAIL; 
	if (pDownloadParams->m_Params.m_FileName.GetLength() > 0)
	{
		 hr = URLDownloadToFile(NULL, pDownloadParams->m_Params.m_Url, pDownloadParams->m_Params.m_FileName, 0, NULL);
	}
	else
	{
		char szPath[MAX_PATH + 2] = {0};
		hr = URLDownloadToCacheFile(NULL, pDownloadParams->m_Params.m_Url, szPath, MAX_PATH, 0, NULL); 
		if (SUCCEEDED(hr) && !IsBadReadPtr(pDownloadParams->m_Params.m_pNotify, sizeof(IQHttpNotify*)))
		{
			pDownloadParams->m_Params.m_FileName = szPath; 
		}

	}
	if (!m_bStopping)
	{
		if (pDownloadParams->m_Params.m_pNotify && 
			!IsBadReadPtr(pDownloadParams->m_Params.m_pNotify, sizeof(IQHttpNotify*)))
		{
			pDownloadParams->m_Params.m_pNotify->OnDownloadComplete(pDownloadParams->m_Params, hr);
		}
	}
}

void FQHttp::ReleaseItem( FQHttpInternal* pPtr )
{
	delete pPtr;
}

BOOL FQHttp::GetFileFromCache( const char* szURL, char* szCachedFileName )
{
	DWORD dwBufferSize = 4096;
	char* pBuf = new char[dwBufferSize];
	LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) pBuf;
	lpCacheEntry->dwStructSize = dwBufferSize;
	//retrieve the cache entry
	BOOL bRes = GetUrlCacheEntryInfo(szURL,lpCacheEntry, &dwBufferSize);

	if (bRes)
	{
		StringCbCopy(szCachedFileName, MAX_PATH, lpCacheEntry->lpszLocalFileName);
	}

	delete[] pBuf; 
	return bRes;
}