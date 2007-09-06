#ifndef __FDOWNLOADINC_H__
#define __FDOWNLOADINC_H__

#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/torrent_handle.hpp"

using namespace libtorrent; 
typedef unsigned long vidtype; 
typedef __int64 fsize_type; 

typedef dword DHANDLE; 
typedef dword FDownloadHandle;

#define DURATION_MULT 10000

//-----------------------------------------------

#define FLAG_DOWNLOAD_ACTIVE		0x0001
#define FLAG_DOWNLOAD_FINISHED		0x0002
#define FLAG_DOWNLOAD_PAUSED		0x0004
#define FLAG_DOWNLOAD_STOPPED		0x0040
#define FLAG_DOWNLOAD_DELETED		0x0080
#define	FLAG_DOWNLOAD_QUEUED		0x0100
#define FLAG_DOWNLOAD_FORCE			0x0200

#define FLAG_DOWNLOAD_ERR_NETWORK	0x1000	//.SQX
#define FLAG_DOWNLOAD_ERR_MASK		0x3000


//Clip flags
#define MEDIA_FLAG_IS_ADVERT		0x01		//Clip is an advertisement
#define MEDIA_FLAG_SHOW_ONCE		0x02		//Show only once per session
#define MEDIA_FLAG_NO_SKIP			0x03		//User cannot scroll over this clip


//Video flags
#define VIDEO_FLAG_IS_PLAYLIST		0x01		//the download contains multiple separate clips 
#define VIDEO_FLAGS_IS_SPLIT_VIDEO	0x02		//video is split into multiple clips

//-----------------------------------------------




#define FTORRENT_HANDLE 0x80000000 //download is a torrent


//HTTP errors
#define E_HTTP_REMOVED			E_FAIL + 0x1000
#define E_HTTP_NOTFOUND			E_FAIL + 0x1001
#define E_HTTP_INVALID_STATUS	E_FAIL + 0x1002
#define E_HTTP_NET_ERROR		E_FAIL + 0x1003
#define E_HTTP_WRITE_FILE		E_FAIL + 0x1004


typedef DWORD DHANDLE;


#define HTTP_FLAG_NO_RESUME	0x01	//will not resume download (just overwrite the file)
#define HTTP_FLAG_DOWNLOAD_URL	0x02	//URL pointed by m_DownloadURL must be downloaded :).


#define RSS_FLAG_FROM_AUTODOWNLOAD 0x01	//Item has been automatically downloaded
#define RSS_FLAG_FROM_RSS		   0x02 //Item comes from RSS Feed
struct FRSSInfo{
	DWORD	m_Guid;		//CRC32 of ItemURL or CRC32 of ItemGuid
	DWORD	m_dwFlags;	//RSS Flags (FROM_RSS | FROM_AUTODOWNLOAD)
	FString	m_RSSName;	//Name of the RSS Channel
	FString	m_RSSURL;	//URL of the RSS Channel
	FString	m_ItemURL;	//URL of the RSS Item
	DWORD	m_RSSGuid;	//Guid of the channel
	FRSSInfo(){
		m_Guid = 0; 
		m_dwFlags = 0; 
		m_RSSGuid = 0; 
	}
};

struct FDownloadInfo
{
	FString		m_DownloadUrl;		//HTTP url
	FString		m_DownloadFile;		//File to save to
	FString		m_MediaName;		//Video name
	FString		m_ContentType;		//Content type
	void*		m_pv;				//Internal data
	DHANDLE		m_Handle;
	DWORD		m_dwStatusCode; 
	size_type	m_ContentLength; 
	FRSSInfo	m_RSSInfo;
	DWORD		m_dwDownloadFlags; //HTTP_* Flags
	FDownloadInfo()
	{
		m_pv = NULL; 
		m_Handle = 0; 
		m_dwStatusCode = 0; 
		m_ContentLength = 0; 
		m_dwDownloadFlags = 0; 
	}
	virtual ~FDownloadInfo(){}
};

struct FDownloadProgress{
	ULONG   m_Progress; 
	ULONG   m_ProgressMax; 

	DWORD		m_LastTick; 
    size_type   m_LastSize; 
	double		m_percentComplete;  //Progress / ProgressMax
	double		m_bytesPerSec;      //Bytes per second
	double		m_upBytesPerSec;    //Upload bytes per second
	size_type	m_bytesDownloaded;	//Total downloaded
	size_type	m_bytesUploaded;	//Total uploaded (torrents only)
	size_type	m_bytesTotal;		//Total size 
	int			m_Seeds;			//Seeds
	int			m_Peers;			//Peers


	FDownloadProgress& operator += (FDownloadProgress& rp)
	{
		if (this == &rp)
			return *this; 
		m_percentComplete+=rp.m_percentComplete; 
		m_bytesPerSec+=rp.m_bytesPerSec; 
		m_upBytesPerSec+=rp.m_upBytesPerSec; 
		m_bytesDownloaded+=rp.m_bytesDownloaded; 
		m_bytesUploaded+=rp.m_bytesUploaded; 
		m_bytesTotal+=rp.m_bytesTotal;
		m_Seeds+=rp.m_Seeds; 
		m_Peers+=rp.m_Peers; 
		return *this; 
	}
	ULONG   m_StatusCode;       //Status code

    void        Recompute()
    {
        m_bytesTotal = m_ProgressMax;
        m_bytesDownloaded = m_Progress;

        m_percentComplete = m_bytesTotal > 0 ? (double)m_bytesDownloaded / (double)m_bytesTotal : 0.0;
        DWORD dwS = (GetTickCount() - m_LastTick) / 1000;
        if (dwS == 0) dwS = 1; 

        size_type sizeSinceLast = m_bytesDownloaded - m_LastSize;
        m_bytesPerSec = (double)sizeSinceLast / (double)dwS;
        m_LastTick = GetTickCount(); 
        m_LastSize = m_bytesDownloaded; 
    }

	FDownloadProgress()
	{
		memset(this, 0, sizeof(FDownloadProgress)); 
	}
};

struct FDownloadAlert
{
	dword			dwCode;		//Alert code
	HRESULT			hr;			//Result
	vidtype			m_videoID;	
	size_t			m_clipIndex; 
	FDownloadHandle m_DownloadHandle; 
	//user data cast based on alert code
	void*			m_Param1;	
	void*			m_Param2;	
	virtual ~FDownloadAlert(){}
};

class IClipDownloadNotify
{
public:
	virtual void Notify(FDownloadAlert& pAlert) = 0; 
	virtual ~IClipDownloadNotify(){}
};

class IHttpDownloader
{
public:
	virtual ~IHttpDownloader() {}; 
	virtual void	Init() = 0; 
	virtual void	Stop() = 0; 
	virtual BOOL	IsPaused(DHANDLE handle) = 0;
	virtual BOOL	Pause(DHANDLE handle) = 0; 
	virtual BOOL	Resume(DHANDLE handle) = 0; 
	virtual DHANDLE DownloadURL(FDownloadInfo* pInfo, IClipDownloadNotify* pvObj) = 0; 
	virtual BOOL	GetDownloadProgress(DHANDLE hDownload, FDownloadProgress& Progress) = 0; 
	virtual BOOL	StopDownload(DHANDLE handle) = 0; 

};

//Alert Codes - FDownloadAlert
#define ALERT_DOWNLOAD_FINISHED 1		//m_Param1 = FDownloadHandle
#define ALERT_TRACKER_REPLY		2		//m_Param1 = FDownloadHandle
#define ALERT_ITEM_REMOVED		3		//No params
#define ALERT_ITEM_QUEUED		4		//m_Param1 = FDownloadHandle
#define ALERT_HTTP_DOWNLOAD_FINISHED 5	//m_Param1 = FDownloadInfo
#define ALERT_LOAD_STORAGE		6		//No params
#define ALERT_LISTEN_FAILED		7		//No params
#define ALERT_TORRENT_ADDED		8		//m_Param1 = FDownloadHandle
#define ALERT_FILE_ERROR		9		//m_Param1 = FDownloadHandle
#define ALERT_CONTENT_TYPE		10		//m_Param1 = FDownloadHandle; m_Param2 = new char[ContentType]
#define ALERT_SCHEDULED_EVENT	11		//Scheduled event; m_Param1 = void* param
#define ALERT_NO_DISK_SPACE		12		//No disk space left to start download


struct FAlertStructBase
{
	dword dwCode;
	dword dwStatusCode; 
	void*			pDownloadItem; 
	DHANDLE			m_DHandle; 
	FAlertStructBase()
	{
		dwCode = 0; 
		dwStatusCode = 0; 
		pDownloadItem = 0; 
		m_DHandle = 0; 
	}
	virtual ~FAlertStructBase(){}
};
//Used to notify objects of download events (error, finished, started, etc)
struct FAlertStruct : FAlertStructBase
{
	vidtype			videoID;		//VideoID of the clip
	torrent_handle	hTorrent;		//The torrent handle
	dword			dwInternal;		//Internal data 
	
	FAlertStruct()
	{
		videoID = 0; 
		dwInternal = 0;
	}
};

class ITorrentNotify
{
public:	
	virtual int OnAlert(FAlertStruct& pAlert) = 0;
	virtual ~ITorrentNotify(){}
};

//////////////////////////////////////////////////////////////////////////

enum DownloadType
{
	None = 0, 
	Http, 
	Torrent
};

struct FClipDownloadConfig{
	dword		m_MaxKBDown; 
	dword		m_MaxKBUp; 
	dword		m_MaxConnections; 
	dword		m_MaxDownloads; 
	dword		m_MaxUploads; 
	dword		m_LPortMin; 
	dword		m_LPortMax; 
	dword		m_TorrentLogLevel;
	dword		m_MaxShareAge;  //Maximum number of days to seed the torrent?
	dword		m_MaxTorrents;	//Maximum number of torrents (leeching/seeding) allowed in session
	dword		m_SeedTime;		//Seed time, in seconds, for torrent rotation
	dword		m_InitialSeedAge;	//Minimum age during which the torrent is considered 'fresh'.
									//This will boost seed priority for newly published torrents - new torrents will be seeded longer during rotation 
	dword		m_InitialSeedTime;  //Time to seed 'fresh' torrents before swapping during rotation
	FString		m_Proxy; 
	FString		m_ProxyA;
	FString		m_AppVersion; 
	FString		m_IndexPath;	//Path to store .dht_State file.
	FClipDownloadConfig()
	{
		m_MaxKBDown = 0; 
		m_MaxKBDown = 0; 
		m_MaxKBUp = 0; 
		m_MaxConnections = 0; 
		m_MaxDownloads = 0; 
		m_MaxUploads = 0; 
		m_LPortMin = 0; 
		m_LPortMax = 0; 
		m_TorrentLogLevel = 0;
		m_MaxShareAge = 0; 
		m_MaxTorrents = 0; 
		m_InitialSeedAge = 3600 * 72; //3 days initial seeding
		m_InitialSeedTime = 2 * 3600; //2 hours
	}

};

struct FClipDownloadInfo
{
	vidtype				m_videoID;			//videoID
	size_t				m_clipIndex;		//clip index 
	DownloadType		m_DownloadType; 
	FString				m_Href;				//HTTP URL of the file or torrent
	size_type			m_HrefSize;			//Size of the data pointed by HREF
	FString				m_DataFile;			//Local filename where the HREF will be saved
	FString				m_StorageFolder;	//Folder where to save the data
	FString				m_UrlSeed1; 
	FString				m_UrlSeed2; 
	DWORD				m_dwFlags; 
	time_t				m_TimePublished; 
	time_t				m_TimeAdded; 
	time_t				m_TimeCompleted; 
	FString				m_TrackerURL; 

	//Out value	
	BOOL				m_bMetadataComplete;

	FClipDownloadInfo()
	{
		m_dwFlags = 0; 
		m_videoID = 0; 
		m_clipIndex = 0; 
		m_DownloadType = None; 
		m_HrefSize = 0; 
		m_bMetadataComplete = FALSE; 
		m_TimePublished = 0; 
		m_TimeAdded = 0; 
		m_TimeCompleted = 0; 
	}
};

struct FSessionInfo
{
	unsigned short m_ListenPort; 
	BOOL		   m_IsListening ;
};

class IHashProgress{
public:
	virtual BOOL OnHashProgress(std::string pFileName, int current, int max) = 0; 
	virtual void OnHashFinished(BOOL bSuccess, std::string pTorrentFile) = 0; 
	virtual ~IHashProgress(){}
};

struct FHashFile{
	std::string m_FileName; 
	std::string	m_TorrentFile; 
	std::vector<std::string> m_Trackers; 
	std::vector<std::string> m_UrlSeeds; 
	IHashProgress*			 m_pProgress; 
	FHashFile(){
		m_pProgress = NULL; 
	}
};

//IClipDownloader i-face
class IClipDownloader
{
public:
	virtual ~IClipDownloader() {}; 
	virtual BOOL Init(FClipDownloadConfig& aConf) = 0;
	virtual BOOL Stop() = 0;
	virtual BOOL UpdateConfig(FClipDownloadConfig& aConf) = 0; 

	virtual FDownloadHandle AddDownload(FClipDownloadInfo& pInfo) = 0;
	virtual BOOL			RemoveDownload(FDownloadHandle handle) = 0; 

	virtual BOOL GetProgress(FDownloadHandle handle, FDownloadProgress& pProgress) = 0;
	virtual BOOL GetTotalProgress(vidtype videoID, FDownloadProgress& pProgress) = 0; 
	virtual int GetTotalProgress(CAtlMap<FDownloadHandle, FDownloadProgress>& aMap) = 0; 
	virtual BOOL GetSessionProgress(FDownloadProgress& pProgress) = 0; 	
	virtual BOOL PauseResume(FDownloadHandle handle, BOOL bResume) = 0; 
	virtual BOOL PauseAll() = 0;
	virtual BOOL ResumeAll() = 0; 
	virtual BOOL SetNotifySink(IClipDownloadNotify *pSink) = 0; 
	virtual BOOL SetHttpDownloader(IHttpDownloader *pDownloader) = 0;
	virtual BOOL GetSessionInfo(FSessionInfo* pInfo) = 0; 
	//For statistics only. Do not use this to do stuff with torrents other than get information
	virtual libtorrent::session_status GetSessionStatus() = 0; 
	virtual libtorrent::torrent_status GetTorrentStatus(FDownloadHandle handle) = 0;
	virtual libtorrent::torrent_handle GetTorrent(FDownloadHandle handle) = 0; 
	virtual std::vector<libtorrent::torrent_handle> GetTorrents() = 0; 
	virtual BOOL HashFile(FHashFile& Params) = 0;
	
};



#endif //__FDOWNLOADINC_H__