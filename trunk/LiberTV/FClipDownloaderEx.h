#pragma  once
#include "FDownloadInc.h"

//Clip flags
#define CFLAG_COMPLETE 0x01
#define CFLAG_PAUSED 0x02

//Compatible with libtorrent torrent_status
enum EDownloadStatus{
	ENone = 0, 
	EMetaData,				//downloading metadata
	EQueued, 
	EDownloading,           //download is in progress
	EFinished,              //download completed successfully
	EPaused
};

struct FDownloadPair
{
	torrent_handle				m_torrentHandle; 
	DHANDLE						m_httpHandle; 
	time_t						m_ResumeWritten;
	EDownloadStatus				m_DownloadStatus;
	FClipDownloadInfo			m_pInfo; 
	FDownloadHandle				m_Handle; 
	time_t						m_SeedStarted; 
	dword						m_SecondsSeeded;
	DWORD						m_dwHttpCode;
	BOOL						m_bRemoved; 
	FString						m_ContentType; 
	FDownloadPair(FDownloadHandle handle, FClipDownloadInfo& pClip)
	{
		m_Handle = handle; 
		m_pInfo = pClip; 
		m_httpHandle = 0; 
		m_SeedStarted = 0; 
		m_SecondsSeeded = 0; 
		m_dwHttpCode = 0; 
		m_bRemoved = FALSE; 
	}
};

typedef CAtlMap<FDownloadHandle, FDownloadPair*>::CPair FPair; 

class FClipDownloaderEx : public IClipDownloader
{
	class FHttpNotify : public IClipDownloadNotify
	{
	public:	
		FFastSyncSemaStack<FDownloadAlert>	m_HttpAlerts;		  //alerts which are
		void Notify(FDownloadAlert& pAlert){m_HttpAlerts.Push(pAlert);}
	};
	class FQueueThread : public mz_Thread{
		void Thread(void*); 
	};
	class FAlertThread : public mz_ThreadPool{
		void Thread(void*); 
	};

	friend class FAlertThread; 	
	libtorrent::session*						m_TorrentSession; //libtorrent session
	CAtlMap<FDownloadHandle, FDownloadPair*>	m_Downloads;	  //list of active downloads
	std::deque<FDownloadHandle>					m_CQueue;		  //Queued downloads - Complete
	std::deque<FDownloadHandle>					m_IQueue;		  //Queued downloads - Incomplete

	mz_Sync										m_Sync;			  //critical section
	FDownloadHandle								m_CurHandle;	  //current download handle counter
	IClipDownloadNotify*						m_pNotify;
	IHttpDownloader*							m_pHttpDownloader; 
	FClipDownloadConfig							m_Config;  
	FHttpNotify									m_HttpNotify; 
	FQueueThread								m_QueueThread; 
	FAlertThread								m_AlertThread; 


	BOOL StartDHT(); 
	BOOL LoadConfig(); 
	BOOL QueueDownload(FDownloadPair* pPair); 
	void OnMetadataComplete(FDownloadPair* pPair, HRESULT hr, BOOL bSendAlert = TRUE); 
	void SendAlert(FDownloadPair* pPair, DWORD dwCode, HRESULT hr); 
	void SendAlert(libtorrent::torrent_handle h, DWORD dwCode, HRESULT hr); 
	void RemoveCompletedTorrent(std::vector<torrent_handle>& torrent_array); 
	FDownloadHandle FindByTorrent(libtorrent::torrent_handle h); 
	FDownloadPair* FindPairByTorrent(libtorrent::torrent_handle h); 
	BOOL AddTorrent(FDownloadHandle handle); 
	BOOL AddTorrent(FDownloadPair* pPair); 
	BOOL CheckIncompleteQueue(); 
	BOOL CheckCompleteQueue(); 
	BOOL WriteResume(const tchar* pszResumeFile, torrent_handle h);
	BOOL WriteResume(FDownloadPair* pPair); 
	BOOL WriteResume(libtorrent::torrent_handle handle); 
	void ProcessAlerts(); 
	void OnHttpDownloadComplete(HRESULT hr, FDownloadInfo* pUrlInfo);
	BOOL StopTorrent(libtorrent::torrent_handle handle); 
	BOOL RemoveQueueHandle(std::deque<FDownloadHandle>& queue, FDownloadHandle handle);
	BOOL PauseResumeDownload(FDownloadPair* pPair, BOOL bResume); 
	BOOL PauseResumeAll(BOOL bResume); 
	BOOL GetProgress(FDownloadPair* pPair, FDownloadProgress& pProgress); 
	void OnTorrentFinished(libtorrent::torrent_handle& theHandle);
public:
	FClipDownloaderEx();
	~FClipDownloaderEx(); 
	BOOL Init(FClipDownloadConfig& aConf); 
	BOOL Stop(); 
	FDownloadHandle AddDownload(FClipDownloadInfo& pInfo);
	BOOL SetNotifySink(IClipDownloadNotify *pSink);
	BOOL SetHttpDownloader(IHttpDownloader* pDownloader); 
	BOOL GetProgress(FDownloadHandle handle, FDownloadProgress& pProgress);
	BOOL GetTotalProgress(vidtype videoID, FDownloadProgress& pProgress); 
	BOOL GetSessionProgress(FDownloadProgress& pProgress); 
	BOOL RemoveDownload(FDownloadHandle handle);
	BOOL PauseResume(FDownloadHandle handle, BOOL bResume);
	BOOL PauseAll(); 
	BOOL ResumeAll(); 
	BOOL UpdateConfig(FClipDownloadConfig& aConf); 
	BOOL GetSessionInfo(FSessionInfo* pInfo);


	libtorrent::session_status GetSessionStatus();
	libtorrent::torrent_status GetTorrentStatus(FDownloadHandle handle);
	std::vector<libtorrent::torrent_handle> GetTorrents(); 
	libtorrent::torrent_handle GetTorrent(FDownloadHandle handle); 

	int GetTotalProgress(CAtlMap<FDownloadHandle, FDownloadProgress>& aMap); 
	BOOL HashFile(FHashFile& Params); 
	BOOL HashFileEx(FHashFile& Params); 
};