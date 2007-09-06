#ifndef __FASYNCHTTPDOWNLOAD_H__
#define __FASYNCHTTPDOWNLOAD_H__

#include "mz_Inc.h"
#include "FDownloadInc.h"

/*
To use this:
Derive your class from FAsyncDownloadNotify

Call g_HttpDownload->DownloadURL(url, fileOut, this, yourVoid);
Implement FAsyncDownloadNotify::OnHttpDownloadComplete()
SUCCEEDED(hr) means download is ok. 
pv contains your data. pFileName is fileOut. pUrl is the org url.
*/


struct FHttpDownloadProgress : public FDownloadProgress, public IBindStatusCallback
{
public:
	BOOL	m_Abort; 
	BOOL	m_Paused; 

	STDMETHODIMP   QueryInterface(REFIID riid ,void ** ppv){return E_NOINTERFACE;}
	STDMETHODIMP_(ULONG)    AddRef(){return 0;}
	STDMETHODIMP_(ULONG)    Release(){return 0;}
	STDMETHODIMP    OnStartBinding(DWORD grfBSCOption, IBinding* pbinding){return 0;}
	STDMETHODIMP    GetPriority(LONG* pnPriority){return 0;}
	STDMETHODIMP    OnLowResource(DWORD dwReserved){return 0;}
	STDMETHODIMP    OnStopBinding(HRESULT hrResult, LPCWSTR szError){return 0;}
	STDMETHODIMP    GetBindInfo(DWORD* pgrfBINDF, BINDINFO* pbindinfo){return 0;}
	STDMETHODIMP    OnDataAvailable(DWORD grfBSCF, DWORD dwSize, FORMATETC *pfmtetc,STGMEDIUM* pstgmed){return 0; }
	STDMETHODIMP    OnObjectAvailable(REFIID riid, IUnknown* punk){return 0;}
	STDMETHODIMP	OnProgress(ULONG ulProgress,ULONG ulProgressMax,ULONG ulStatusCode,LPCWSTR szStatusText)
	{
        m_Progress = ulProgress;
        m_ProgressMax = ulProgressMax;
        m_StatusCode = ulStatusCode; 

        BINDSTATUS bs = (BINDSTATUS)ulStatusCode; 
        if (bs == BINDSTATUS_BEGINDOWNLOADDATA)
        {
           m_LastTick = GetTickCount(); 
           m_LastSize = ulProgress;
        }
		return m_Abort ? E_ABORT : 0;
	}

	FHttpDownloadProgress()
	{
		m_Abort = FALSE; 
		m_Paused = FALSE; 
	}
};

class FAsyncDownload : public IHttpDownloader
{
	struct FAsyncDownData 
	{
		IClipDownloadNotify*	m_pNotify;
		FHttpDownloadProgress	pBindStatusCallback;
		FDownloadInfo*				m_pUrlInfo; 
		HRESULT					hr;
		FAsyncDownData()
		{
			m_pNotify = NULL; 
			m_pUrlInfo = NULL; 
			hr = E_FAIL; 
		}
		void	SendAlert(dword dwCode); 
	};

    class FHttpDownloadTP : public mz_ThreadPool
    {
    public:
        FAsyncDownload* m_pThis; 
        void Thread(void*); 
        HRESULT ProcessDownload(FAsyncDownData *pData); 
    };
	
    friend class FHttpDownloadTP; 
	friend class FHttpAdd;
	mz_Sync				m_Sync; 
	 
    FHttpDownloadTP     m_DownloadPool;
	BOOL				m_Stopping; 
    DWORD               m_HandleID; 
	
	CAtlArray<FAsyncDownData*> m_aDownloads; 
	CAtlArray<FAsyncDownData*> m_aPaused; 
	void		OnDownloadComplete(FAsyncDownData* pData, HRESULT hr);
	BOOL		AddDownload(FAsyncDownData* pData);
	void		AbortDownloads(); 
	
public:							
	FAsyncDownload()
	{
	}
	FFastSyncSemaStack<FAsyncDownData*> m_Urls;

	
	void		Stop(); 
	DHANDLE		DownloadURL(FDownloadInfo* pInfo, IClipDownloadNotify* pvObj); 
	BOOL		StopDownload(DHANDLE hDownload); 
	void		Init(); 
    BOOL        GetDownloadProgress(DHANDLE hDownload, FDownloadProgress& Progress); 
	BOOL		Pause(DHANDLE hDownload);
	BOOL		Resume(DHANDLE hDownload);
	BOOL		IsPaused(DHANDLE handle);
};


class FDownloadNotify : public IClipDownloadNotify
{
	class FAlertThread : public mz_Thread{
		void Thread(void *p); 
	};
public:
	FFastSyncSemaStack<FDownloadAlert> m_Alerts; 
	FAlertThread					   m_AlertThread; 
	IClipDownloadNotify*			   m_Object; 

	void Notify(FDownloadAlert &alert);
	BOOL Init(IClipDownloadNotify* pMgr); 
	void Stop(); 
};



#endif //__FASYNCHTTPDOWNLOAD_H__