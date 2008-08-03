#ifndef __FDOWNLOAD_H__
#define __FDOWNLOAD_H__

#include "Utils.h"
#include "FIniFile.h"
#include "FLabels.h"
#include "FDownloadInc.h"


//VideoItem structure. Details about a video
/*
Each video item is composed of a one or more clips.
Each clip has a an array of Params.
*/

//Video Duration is in milliseconds. 
//Multiply by 10000 to get REFERENCE_TIME

typedef unsigned __int64 duration_type; 


struct FDownload;

enum ItemDownloadStatus
{
	INone = 0, 
	IQueued, 
	IDownloading, 
	IFinished
};

#define CLIP_NOT_FOUND 0x01

struct FMediaFile
{
	FString                 m_FileName;             //Media filename 
	size_type               m_FileSize;             //File size 
	duration_type			m_DurationMS;             //Media duration
	FString					m_MediaType;			//Type of media. Can be "avi", "mpeg", "html", "swf", etc
	double					m_FPS; 
	size_t					m_Index; 
	FString					m_Description; 

	FMediaFile(){
		m_FileSize = 0;
		m_DurationMS = 0; 
		m_Index = 0; 
		m_FPS = 0.0;
	}
};


/*
A FMediaClip file encapsulates information about a metafile and it's collection of media files.

A .torrent file is metafile, which can contain one or more media files.

*/
struct FDownloadItem{

	FDownload*				m_Video;				//Pointer to parent video
	CComPtr<IXMLDOMNode>	m_pXMLNode;				//Our own XML Node !!!
	size_t					m_ClipIndex;			//Index of the clip in the parent video
	FString					m_DownloadType;			//Type of download. Can be "torrent" or "http"
	FString					m_Href;					//URL of the downloadable clip /torrent file.
	fsize_type				m_HrefSize;				//Size of the metadata (eg. torrent file)
	FString					m_UrlSeed; 
	FString					m_UrlSeed1; 
	FString					m_TrackerURL;			//Tracker which is used to override the default tracker in the torrent file.
	FString					m_DataFile;				//For "http", "file" or "none" downloads. This is the file with the actual content.
	fsize_type				m_FileSize;				//Total size of the download
	duration_type			m_DurationMS;			//Total Duration (must be equal to sum of media file's duration).
	FString					m_Description; 
	time_t					m_TimeAdded;			//Time when download was started
	time_t					m_TimeCompleted;		//Time when download was completed
	FString					m_StreamType;			//Only if DownloadType is "stream". Can be "mov", "wmv", "flv"
	FString					m_ContentType;			//Content-type received from server
	dword					m_dwFlags;				//Is Ad, Show Once, Can Skip, etc.

	//Not serialize-able
	vidtype					m_videoID; 
	FString					m_DataPath; 
	FString					m_TorrentFile;			//Name of the meta file for this clip (eg. torrent file). Equals to DataFile
	/*
		m_DataFile differs from m_TorrentFile in that the DataFile can be located outside of the storage folder.
	*/

	//Download helpers

	FDownloadHandle			m_DownloadHandle; 
	ItemDownloadStatus		m_DownloadStatus; 
	BOOL					m_bCompleted;			//Marks item as 'already processed'
	BOOL					m_bInProgress; 
	size_t					GetMediaFiles(std::vector<FMediaFile>& MediaFiles); 

	FDownloadItem(FDownload* pVideo = NULL): m_Video(pVideo)
	{
		m_TimeAdded = 0; 
		m_TimeCompleted = 0; 
		m_ClipIndex = 0; 
		m_DownloadHandle = 0; 
		m_HrefSize = 0;
        m_DownloadType = "http";
		m_DurationMS = 0; 
		m_FileSize = 0; 
		m_dwFlags = 0; 
		m_videoID = 0; 
		m_bCompleted = FALSE; 
		m_DownloadHandle = 0; 
		m_bInProgress = FALSE; 
		m_DownloadStatus = INone; 
		m_pXMLNode = NULL; 
	}
	BOOL			Serialize(FIniConfig& pConf, const tchar* pszSection, BOOL bPut); 
	FString			DataPath(const tchar* pszRelative);
	BOOL			IsFinished(){
		return (m_TimeCompleted > 0);
	}
	BOOL			IsInProgress()
	{
		return m_DownloadStatus == IDownloading || 
			   m_DownloadStatus == IQueued;
	}
	void			OnSerialized(); 
	void			ConvertToBittorrent();	//Convert from HTTP to bittorrent.
};


//Used to create downloads from URL and such
struct FDownloadCreateInfo{
	FString		m_VideoName; 
	vidtype		videoID; 
	FString		m_URL; 
	FString		m_SavePath; 
	FString		m_SaveFileName; 
	FString		m_Description; 
	size_type	m_Size; 
	time_t		m_TimePublished; 
	FString		m_DownloadType; 
	DWORD		m_GUID; 

	FDownloadCreateInfo(){
		m_Size = 0; 
		m_TimePublished = 0; 
	}
};


struct FMediaPlaylist
{
	std::vector<FMediaFile> m_Files; 
	duration_type			m_TotalDurationMS; 
	FString					m_VideoName; 
	FMediaPlaylist()
	{
		m_TotalDurationMS = 0; 
	}

	void Clear()
	{
		m_TotalDurationMS = 0; 
		m_Files.clear(); 
	}

	BOOL GetTimeIndex(duration_type rtIndex, size_t& nIndex, duration_type& rtOffset); 
	BOOL SavePlaylist(const char* pszFileName);
};


struct FVideoDetail{
	FString			m_VideoName;			//Name of the video
	FString			m_DataPath;				//Path were we save the video data to
	FString			m_SubURL;				//GetSubtitle script URL
	vidtype			m_VideoID;				
	dword			m_ShowID; 
	dword			m_EpisodeID; 
	dword			m_VideoFlags; 
	fsize_type		m_TotalSize; 
	time_t          m_TimeAdded; 
	time_t          m_TimeCompleted; 
	time_t			m_TimePublished;		//Server date when the download was published.
	fsize_type		m_SizeDownloaded;
	duration_type	m_TotalDurationMS;
	duration_type	m_PlaybackPosMS; 
	duration_type	m_Watched; 
	FLabels			m_Labels; 
	FString			m_DetailURL; 
	FString			m_ImageURL;				//Video icon
	FString			m_Description; 

	FVideoDetail()
	{
		Reset(); 
	}

	void Reset();
};

struct FDownload 
{
	FDownload(){

		Clear(); 
	}
	virtual ~FDownload()
	{
		Clear(); 
	}

	FDownload(const FDownload& rVid); 
	FDownload& operator = (const FDownload& avid); 
	void FDownload::Clear()
	{
		RemoveClips();
		m_Detail.Reset(); 
		m_CurClipIndex = 0; 
		m_dwFlags = 0; 
		m_dwClipsActive = 0; 
	}

	BOOL            LoadConf(FIniConfig	&Conf); 
	BOOL            SaveConf(FIniConfig	&Conf);
	BOOL			Serialize(FIniConfig& Conf, BOOL bPut); 
	void			CopyFrom(const FDownload& rVid); 
	void			RemoveClips(); 
	duration_type	GetAvailDuration();
	BOOL			GetRTPosition(duration_type rtOffset, size_t &iIndex, duration_type& rtVideoOffset); 
	duration_type	GetClipRTOffset(int nIndex); 
	BOOL			IsValid(){	return (m_Detail.m_VideoID != 0); 	}
	inline vidtype	GetVideoID(){return m_Detail.m_VideoID;}
	size_t			GetAvailMedia(FMediaPlaylist &aFiles); 
	size_t          GetAvailMediaCount(); 
	void			RefreshInfo(); 
	BOOL            IsDownloadFinished(){return m_dwFlags & FLAG_DOWNLOAD_FINISHED;}
    BOOL            IsPaused(){return m_dwFlags & FLAG_DOWNLOAD_PAUSED;}
	BOOL			IsDownloadableStream();
	BOOL			IsStream(); 
	size_type		ComputeSizeFromClips(); 
	BOOL			RenameClipExtension(FMediaFile& pFile, const tchar* pszNewExtension); 
	FDownloadItem*	AddNewClip(); 
	//Storage path for 'index' files:
	//basically .torrent  and .resume files.
	//Returns g_IndexPath\videoID.vf
	FString			IndexPath();	

	//Retun
	FString			ClipPath(size_t clipIndex);

	FArray<FDownloadItem*>	m_Clips;
	FVideoDetail		m_Detail; 
	FRSSInfo			m_RSSInfo; 
	size_t				m_CurClipIndex; 
	DWORD				m_dwFlags; 
	FString				m_StatusStr; 
	dword				m_dwClipsActive; //active clips in the download manager
	CComPtr<IXMLDOMNode>	m_pXMLNode;				//Our own XML Node !!! Yeah !

	HRESULT				LoadFromXML(IXMLDOMNode* pEntryTag); 
	HRESULT				LoadEntries(IXMLDOMNodeList* pIXMLDOMNodeList);
	HRESULT				LoadEntry(IXMLDOMNode* pIXMLNode); 
	HRESULT				SaveToXML(IXMLDOMNode* pEntryTag, BOOL bOutputAllNodes); 
	void AdjustDataPath(FString& DataPath);
};


struct FDownloadEx : public FDownload
{
	FIniConfig			m_Conf; 
	FDownloadProgress	m_LastProgress;
	int					m_ProgressCount; 
	FDownloadEx();
	~FDownloadEx()
	{
	}
	BOOL		SaveToDisk(); 
	BOOL		LoadFromDisk(const tchar* pszFileName, const tchar* pszDataDir); 
	BOOL		CreateFromFileName(const tchar* pszFileName); 
	BOOL		CreateDownload(FDownloadCreateInfo& pInfo); 

};

#endif //__FDOWNLOADH__