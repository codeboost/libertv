#pragma once

#include "FQuickHttp.h"

//return values for GetImage()
#define IC_ERROR 0
#define IC_OK 1
#define IC_INPROGRESS 2

/*
	Simple image cache
	
*/
class FImageCache : public IQHttpNotify
{
protected:
	FQHttp			m_pHttpDownloads; 
	
	mz_Sync			m_Sync; 
	CAtlList<DWORD> m_CachedItems; 
	BOOL   StartDownload(const char* pszURL); 
public:
	FString			m_CacheDir; 
	~FImageCache();
	BOOL	Init(const char* pszCacheDir);
	
	/*
		Retrieves local path to the image, based on input URL, or input URL if image is not yet in the cache
		return values: IC_ERROR - some internal error; OutURL = OrgURL
		IC_OK - OutURL is a local file
		IC_INPROGRESS - image is being downloaded; OutURL = OrgURL
	*/
	int		GetImage(const char* szUrl, FString& OutURL); 
	BOOL	SaveIndex(); 

	//IHttpNotify
	void OnDownloadComplete(const FQHttpParams& pDownload, HRESULT hr);

};

class FVideoImageCache
{
public:
	FImageCache				m_imgCache; 
	CAtlMap<DWORD, FString> m_ImageMap;
	mz_Sync					m_Sync; 

	void Init(const char* pszCacheDir);
	void SetImage(DWORD dwVideoID, const char* pszURL);
	void RemoveImage(DWORD dwVideoID);
	FString GetImageURL(DWORD videoId);
	//Returns either orgURL or a local file:// url
	FString GetImageURL(const char* pszOrgURL);
	void	RemoveImage(const char* pszUrl); 
};