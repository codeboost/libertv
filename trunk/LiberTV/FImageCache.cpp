#include "stdafx.h"
#include "FImageCache.h"
#include "crc32.h"


BOOL FImageCache::Init(const char* pszCacheDir)
{
	m_CacheDir = pszCacheDir; 


	FString FileName = _PathCombine(m_CacheDir, "index.dat"); 

	/*
	Cache index contains CRC32 for the URLs that have been downloaded	
	*/
	FILE* f = fopen(FileName, "rb");
	if (NULL != f)
	{
		while(!feof(f))
		{
			DWORD dwCRC; 
			if (1 != fread(&dwCRC, sizeof(DWORD), 1, f)) break; 
			m_CachedItems.AddTail(dwCRC);
		}
		fclose(f); 
	}
	return TRUE; 
}

BOOL FImageCache::SaveIndex()
{

	FString FileName = _PathCombine(m_CacheDir, "index.dat"); 

	FILE* f = fopen(FileName, "wb"); 

	if (NULL == f)
		return FALSE; 

	POSITION pos = m_CachedItems.GetHeadPosition();

	while (pos != 0)
	{
		DWORD& dwCRC = m_CachedItems.GetNext(pos); 
		if (1 != fwrite(&dwCRC, sizeof(DWORD), 1, f)) break; 
	}
	fclose(f); 
	return pos == 0; //Means that list end has been reached
}

BOOL FImageCache::StartDownload(const char* pszURL)
{
	DWORD dwCRC = crc32(pszURL, strlen(pszURL), 0); 
	FQHttpParams Params; 
	Params.m_pNotify = this; 
	Params.m_Url = pszURL; 
	Params.m_FileName = _PathCombine(m_CacheDir, ULongToStr(dwCRC));
	Params.m_pvCookie = (void*)dwCRC; 
	return m_pHttpDownloads.DownloadURLToFile(Params) != 0;
}

int FImageCache::GetImage(const char* _szUrl, FString& OutURL)
{
	SynchronizeThread(m_Sync);

	char szUrl[INTERNET_MAX_URL_LENGTH];
	DWORD dwLen = INTERNET_MAX_URL_LENGTH; 

	if (FAILED(UrlCanonicalize(_szUrl, szUrl,  &dwLen, 0)))
	{
		StringCbCopy(szUrl, INTERNET_MAX_URL_LENGTH, _szUrl); 
	}

	char* pszPound = strchr(szUrl, '#');
	if (pszPound)
		*pszPound = 0; 

	DWORD dwCRC = crc32(szUrl, strlen(szUrl), 0); 

	POSITION pos = m_CachedItems.Find(dwCRC); 

	FString TempPath = _PathCombine(m_CacheDir, ULongToStr(dwCRC)); 
	int nRes = IC_OK; 

	if (pos == 0 || !PathFileExists(TempPath))
	{
		char szCached[MAX_PATH + 2];
		if (m_pHttpDownloads.GetFileFromCache(szUrl, szCached))
		{
			if (CopyFile(szCached, TempPath, FALSE))
			{
				pos = m_CachedItems.AddTail(dwCRC);				
			}
		}
	}

	if (0 != pos && PathFileExists(TempPath))
	{
		DWORD dwLen = INTERNET_MAX_URL_LENGTH;
		char pszOut[INTERNET_MAX_URL_LENGTH];
		if (SUCCEEDED(UrlCreateFromPath(TempPath, pszOut, &dwLen, 0)))
		{
			OutURL = pszOut; 
		}
		else
		{
			OutURL = szUrl; 
			nRes = IC_ERROR; 
		}
	}
	else
	{
		if (pos > 0  && !PathFileExists(TempPath))
			m_CachedItems.RemoveAt(pos); 

		/*if (StartDownload(szUrl))
		{
			OutURL = szUrl; 
			nRes = IC_INPROGRESS;
		}
		else
		*/
			OutURL = szUrl; 
	}
	return nRes; 
}


void FImageCache::OnDownloadComplete(const FQHttpParams& pDownload, HRESULT hr)
{
	if (SUCCEEDED(hr))
	{
		SynchronizeThread(m_Sync); 
		m_CachedItems.AddTail((DWORD)pDownload.m_pvCookie);
		_DBGAlert("FImageCached: Download complete: %s to %s\n", pDownload.m_Url, pDownload.m_FileName);
		SaveIndex();
	}
	else
	{
		_DBGAlert("FImageCached: Download failed (0x%x): %s to %s\n", hr, pDownload.m_Url, pDownload.m_FileName);
	}
}

FImageCache::~FImageCache()
{
	m_pHttpDownloads.Stop(); 
}
//////////////////////////////////////////////////////////////////////////

void FVideoImageCache::Init( const char* pszCacheDir )
{
	m_imgCache.Init(pszCacheDir);
}

void FVideoImageCache::SetImage( DWORD dwVideoID, const char* pszURL )
{
	SynchronizeThread(m_Sync);
	m_ImageMap.SetAt(dwVideoID, FString(pszURL));
}

void FVideoImageCache::RemoveImage( DWORD dwVideoID )
{
	SynchronizeThread(m_Sync);
	m_ImageMap.RemoveKey(dwVideoID);
	CAtlMap<DWORD, FString>::CPair* p = m_ImageMap.Lookup(dwVideoID);
	if (p)
	{
		DWORD dwCRC = crc32(p->m_value, strlen(p->m_value), 0); 
		m_ImageMap.RemoveKey(dwVideoID);
		FString TempPath = _PathCombine(m_imgCache.m_CacheDir, ULongToStr(dwCRC)); 
		DeleteFile(TempPath); 
	}
}

FString FVideoImageCache::GetImageURL( DWORD videoId )
{
	SynchronizeThread(m_Sync); 
	CAtlMap<DWORD, FString>::CPair* p = m_ImageMap.Lookup(videoId);
	FString OutUrl; 
	if (p)
	{
		m_imgCache.GetImage(p->m_value, OutUrl);
	}
	return OutUrl;
}

FString FVideoImageCache::GetImageURL(const char* pszUrl)
{
	if (PathIsURL(pszUrl))
	{
		FString OutURL; 
		m_imgCache.GetImage(pszUrl, OutURL);
		return OutURL; 
	}
	return GetImageURL(strtoul(pszUrl, NULL, 10)); 
}

void FVideoImageCache::RemoveImage(const char* pszUrl)
{
	DWORD dwCRC = crc32(pszUrl, strlen(pszUrl), 0); 
	FString TempPath = _PathCombine(m_imgCache.m_CacheDir, ULongToStr(dwCRC)); 
	DeleteFile(TempPath); 
}

