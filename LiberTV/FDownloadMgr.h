#ifndef __FDOWNLOADMGR_H__
#define __FDOWNLOADMGR_H__

#include "Utils.h"
#include "FDownloadInc.h"
#include "FDownload.h"
#include "FAsyncHttpDownload.h"
#include "IJScriptArray.h"
#include "FDownload_Storage.h"
#include "LTV_RSS.h"
#include "IControlBar.h"
#include "FDownloadIndex.h"

class FDownloadMgr;

struct FCollectionSums{
	DWORD m_dwAll; 
	DWORD m_dwQueued; 
	DWORD m_dwUnwatched; 
	DWORD m_dwInProgress; 
	DWORD m_dwSubscribed; 
	DWORD m_dwStreams;

	FCollectionSums(){
		m_dwAll = 0; 
		m_dwQueued = 0; 
		m_dwUnwatched = 0; 
		m_dwInProgress =0 ; 
		m_dwSubscribed = 0; 
		m_dwStreams = 0; 
	}
	void Clear(){
		m_dwAll = 0; 
		m_dwQueued = 0; 
		m_dwUnwatched = 0; 
		m_dwInProgress =0 ; 
		m_dwSubscribed = 0; 
		m_dwStreams = 0; 
	}
};

#define STATUS_FLAG_HIDE_QUEUED			0x01
#define STATUS_FLAG_HIDE_INPROGRESS		0x02

#define STATUS_FLAG_FIELD_DESCRIPTION	0x100	//Will add the 'description' field to the array
#define STATUS_FLAG_COLLECTION			0x1000	//Means that the call is coming from collection (will update statusbar)

//Thread used to schedule actions. 
//Scheduled tasks are added by calling AddScheduleItem()
//When scheduled time has arrived, an alert (ALERT_SCHEDULED_EVENT) is posted to the notification object
//Event::m_Param1 contains Schedule type; Event::m_Param2 contains void* parameter
class FScheduleThread : public mz_Thread{
public:

	void ScheduleItem(void* pParam, DWORD dwType, DWORD dwSeconds);
protected:
	struct SchedStruct{
		time_t			m_RetryTime; //time when callback must be called
		void*			m_pParam;	 //void parameter set by 
		DWORD			m_dwType; 
		SchedStruct(void* pParam, DWORD dwType, time_t When): 
		m_pParam(pParam), 
		m_RetryTime(When), 
		m_dwType(dwType){}
	};
	mz_Sync						m_Sync; 
	std::list<SchedStruct>		m_SchedItems;	//List of downloads to retry
	FDownloadNotify*			m_pNotify;
	void Thread(void* p);
	void ProcessSchedule(); 
public:
	FScheduleThread(FDownloadNotify* pNotify): m_pNotify(pNotify){}
};

struct DownloadStatusOptions
{
	FString m_Filter;		//return videos with names containing this substring
	FString m_Label;		//return videos with this label only
	FString m_Sort;			//Sort by (name, status, added)
	FString m_SortDir;		//Sort Direction
	DWORD	m_dwFlags;		//STATUS_FLAG_HIDE_QUEUED, STATUS_FLAG_HIDE_INPROGRESS, STATUS_FLAG_COLLECTION
	int		m_StartIndex;	//0 .. item count
	int		m_Count;		//-1 = all

	//Out value
	size_t  m_TotalCount;	//Total number of items matching criteria 
							//(different from number of items actually returned, because of StartIndex)
	DownloadStatusOptions()
	{
		m_StartIndex = 0; 
		m_Count = -1; 
		m_dwFlags = 0; 
		m_TotalCount = 0; 
	}
};

//Download manager main class. 
class FDownloadMgr : public IClipDownloadNotify
{

	friend class CFHtmlEventDispatcher;
	mz_Sync							m_Sync;				//Critical section; locks m_Downloads array
	IStatusArrayBuilder*			m_JSArray;			//Object to build Javascript status arrays
	FArray<FDownloadEx*>			m_Downloads;		//Active downloads (including queued downloads)
	BOOL							m_bStorageLoaded;	//Storage has been processed and loaded
	BOOL							m_bStopping;		//Download Manager stopping
	FDownload_Storage				m_Storage;			//Storage helper functions
	FDownloadIndex					m_Index; 
	FDownloadNotify					m_AlertNotify;		//Notification object; receives events from FClipDownloaderEx
	std::deque<FDownloadEx*>		m_QueuedDownloads;	//Queue of 'queued' downloads 
	int								m_ActiveDownloads;	//Number of downloads in progress
	IStatusBar*						m_pStatusBar;		//Pointer to the status bar object
	CAtlMap<vidtype, FDownloadEx*>	m_IdMap;			//Fast videoID->FDownloadEx* lookup map
	BOOL							m_bAllPaused;		//All downloads are paused
	FScheduleThread					*m_pScheduleThread; 
	time_t							m_LastSpaceWarning;	//Last time we showed a 'warning: not enough space' warning
	typedef CAtlMap<vidtype, FDownloadEx*>::CPair IdPair; 

protected:


	void		OnScheduledEvent(DWORD dwType, vidtype videoID);
	FDownload*	FindByID(vidtype videoID); 
	FDownload*	FindByGUID(DWORD dwGuid); 

	void		EraseDownload(FDownload* pDownload); 
	void		StopDownload(FDownload* pDownload); 
	void		SaveDownload(FDownload* pDownload); 
	
	void		LoadIndex(); 
	void		MarkAsComplete(FDownloadItem* pItem); 
	void		OnItemComplete(vidtype videoID, size_t clipIndex, HRESULT hr); 
	void		OnItemComplete(FDownload* pDownload, FDownloadItem* pItem, HRESULT hr); 
	void		OnDownloadComplete(FDownload* pDownload); 
	void		OnItemQueued(vidtype videoID, size_t clipIndex, HRESULT hr); 
	void		OnItemAdded(vidtype videoID, size_t clipIndex, HRESULT hr); 
	void		OnItemRemoved(vidtype videoID, size_t clipIndex, HRESULT hr); 
	void		OnTorrentFound(vidtype videoID, size_t clipIndex, HRESULT hr); 
	void		OnContentType(vidtype videoID, size_t clipIndex, const char* pszContentType); 
	BOOL		QueueDownload(FDownloadEx* pDown); 
	int			QueueDownload(FDownload* pDown, size_t clipIndex);
	void		QueueNextDownloadItem(FDownload* pDownload); 
	void		QueueDownloadItem(FDownload* pDownload, size_t clipIndex); 
	BOOL		DeleteDownloadObject(FDownload* pDownload); //removes from array; deletes object
	//returns TRUE if there is enough free space or FALSE if the disk is full
	BOOL		CheckFreeSpace(FDownload* pDownload); 
	
public:

	FDownloadMgr();
	~FDownloadMgr(); 
	//FAsyncDownloadNotify
	void		Notify(FDownloadAlert& alert); 

	//Downloads

	BOOL		Init(); 
	void		SaveDownload(vidtype videoID); 
	BOOL		IsStorageLoaded(){return m_bStorageLoaded;}
	BOOL		AddDownload(FDownloadEx* pDownload); 
    BOOL		GetDownloadStatusString(vidtype videoID, IStatusArrayBuilder* pBuilder); 
	BOOL		_GetItemDownloadStatus(FDownload* pDownload, IStatusArrayBuilder* pJSArray, FDownloadProgress& ItemProgress, DWORD dwGetFlags); 
    BOOL		_GetDownloadStatus(DownloadStatusOptions& pOptions, IStatusArrayBuilder* pJS);
	BSTR		GetDownloadStatusStr(const tchar* pszNameFilter, const tchar* pszLabelFilter, const tchar* pszSort, const tchar* pszSortDir); 
	BOOL		GetDownloadStatus(DownloadStatusOptions& pOptions, IStatusArrayBuilder* pOutArray);
	void		OnAlert(FDownloadAlert& pAlert); 
	void		StopDownloads(); 
	void		Stop(); 
    BOOL		ReannounceVideo(vidtype videoID); 
	BOOL		IsDownload(vidtype videoID); 
	BOOL		IsDownloadFinished(vidtype videoID); 
	void		RemoveDownload(vidtype videoID); 
	void		StopDownload(vidtype videoID); 
	FDownload	GetDownloadInfo(vidtype videoID); 
	FIniConfig  GetDownloadConf(vidtype videoID); 
	void		SaveVideoDetails(vidtype videoID, FVideoDetail& aDetail);
	BOOL		LoadStorageAsync(); 
	BOOL		_PauseResume(FDownload* pDownload, BOOL bResume); 
	BOOL		PauseDownload(vidtype videoID);
	BOOL		IsAllPaused();
	void		ResumeAll();
	void		PauseAll();
	BOOL		IsDownloadPaused(vidtype videoID);
	BOOL		ResumeDownload(vidtype videoID);	
	BOOL		RemoveAllDownloads(BOOL bRemoveFromDisk);
	BOOL		GetDownloadProgress(vidtype videoID, FDownloadProgress& aProgress); 
    vidtype 	ConvertAndLoad(FDownloadInfo* pInfo, const tchar* pszFolder); 
	int			LoadDownload(const tchar* pszFileName, const tchar* pszOrgUrl = NULL); 
	BOOL		RenameClipExtension(vidtype videoID, FMediaFile& pFile, const tchar* pszNewExtension);
	DWORD		GetVideoGuid(vidtype videoID); 

	//Labels - TODO: Move to separate class
	FString 	GetVideoLabels(); 
	FString		GetLabel(vidtype videoID); 
	void		AddLabel(vidtype videoID, const tchar* pszLabel); 
	void		SetLabel(vidtype videoID, const tchar* pszLabel); 
	void		RemoveLabel(vidtype videoID, const tchar* pszLabel); 
	void		AddLabel(const FArray<FString>& aItems, const tchar* pszLabel); 
	void		SetLabel(const FArray<FString>& aItems, const tchar* pszLabel); 
	void		RemoveLabel(const FArray<FString>& aItems, const tchar* pszLabel); 
	void		RemoveLabel(const tchar* pszLabel); 

	//Misc ?
	void		SetUpdateStatus(BOOL bDoUpdate);
	void		UpdateRSSItemVids(FRSSChannel* pChannel); 
	FString		GetDownloadMTTI(vidtype videoID);
	BOOL		StartQueuedDownload(vidtype videoID); 

	HWND m_hWndNotify; 

	void		SetStatus(IStatusBar* pStatus)
	{
		m_pStatusBar = pStatus; 
	}

	CAtlMap<FDownloadHandle, FDownloadProgress> m_Progress; 


	FCollectionSums	m_ItemSums;
	FCollectionSums GetCollectionCounts();
};

#endif //__FDOWNLOADMGR_H__