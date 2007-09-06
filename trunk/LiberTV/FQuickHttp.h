#pragma  once

#include "mz_synchro.h"
#include "mz_Thread.h"


typedef LONG FQHandle; 

class IQHttpNotify;

struct FQHttpParams{
	FString			m_Url; 
	FString			m_FileName;		//If blank, the file is downloaded to or returned from IE's cache
	IQHttpNotify*	m_pNotify;
	void*			m_pvCookie; 
	FQHttpParams()
	{
		m_pNotify = NULL; 
		m_pvCookie = 0; 
	}
};

//DownloadItem can be used in two ways:
//If m_FileName is set to a valid Path, then the file is downloaded to the specified file
//If m_FileName is blank, URLDownloadToCacheFile() is used and, on return, m_FileName points to the IE's cache file
//The file is downloaded if it doesn't exist in the cache


class IQHttpNotify
{
public:
	virtual void OnDownloadComplete(const FQHttpParams& pDownload, HRESULT hr) = 0;
};


struct FQHttpInternal
{
	FQHttpParams m_Params; 
	FQHandle	 m_hHandle;
};

class IQHttp 
{
public:
	virtual ~IQHttp(){}
	virtual FQHandle DownloadURLToFile(FQHttpParams& pParams) = 0; 
	virtual void Stop() = 0; 
};

class FQHttp : public IQHttp
{
	CAtlMap<FQHandle, FQHttpInternal*>	m_DownloadsMap; 
	FQHandle m_hHandle;
	BOOL	 m_bStopping; 
	class FQHttpThreadPool : public mz_ThreadPool{
		void Thread(void* p);
	};
	FQHttpThreadPool m_ThreadPool;
	FFastSyncSemaStack<FQHttpInternal*> m_TaskQueue;
	friend class FQHttpThreadPool;
protected:
	void	 DownloadItem(FQHttpInternal* pDownloadParams);
	void	 ReleaseItem(FQHttpInternal* pPtr);
public:
	FQHttp(int nThreads = 2);
	~FQHttp();

	void	 Stop(); 
	FQHandle DownloadURLToFile(FQHttpParams& pParams);
	FQHandle DownloadURLToCacheFile(FQHttpParams& pParams); 
	//Returns true and sets the path in szCacheFileName if file is found in IE's cache
	//Else returns false.
	//szCachedFileName must be able to hold MAX_PATH chars
	BOOL	 GetFileFromCache(const char* szURL, char* szCachedFileName);
};