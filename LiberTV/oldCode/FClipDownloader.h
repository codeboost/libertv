#ifndef __FCLIPDOWNLOADER_H__
#define __FCLIPDOWNLOADER_H__
#pragma once

#include "FDownload.h"
#include "FDownloadInc.h"


interface IDownloader
{

public:	
	DHANDLE		DownloadFile(const tchar* pszFileName, const tchar* pszMethod); 
	BOOL		QueryProgress(DHANDLE hHandle, FDownloadProgress* pProgress); 

};

class FDownload;


struct FQueueItem{
	FDownloadItem* pItem; 
	time_t		   time_to_add;
	FQueueItem(FDownloadItem* Item, time_t tta){
		pItem = Item; 
		time_to_add = tta; 
	}
};




class FClipDownloader : FAsyncDownloadNotify
{
	friend class CFHtmlEventDispatcher;
	CAtlList<FDownloadItem*>	m_Clips; 
	CAtlList<FQueueItem>		m_ClipQueue;
	mz_Sync					m_Sync; 

	class FTickThread : public mz_ThreadPool{
		void Thread(void* p);
	};
	class FTickThread2 : public mz_Thread{
		void Thread(void* p); 
	};

	friend class FTickThread; 
	friend class FTickThread2; 

	FTickThread								m_TickThread; 
	FTickThread2							m_TickThread2; 
	FFastSyncSemaStack<FAlertStructBase*>	m_sCompleted; 
	libtorrent::session*					m_TorrentSession;
	DWORD									m_LastCheck;
	mz_Semaphore							m_ScheduleSem; 

protected:
	libtorrent::torrent_handle		LoadTorrent(FDownloadItem* pItem); 
	POSITION	_SyncFindClip(FDownloadItem* pItem); 
	FDownloadItem* FindClip(DHANDLE DownloadHandle); 
	BOOL		RemoveClip(FDownloadItem* ppClip); 
	void		OnAlert(FAlertStructBase* as); 
	void		SendNotifyAlert(FDownloadItem* pItem, DWORD dwCode, DWORD dwStatusCode); 
	void		PollTorrentStatus(); 
	FDownloadItem*	FindByTorrentHandle(torrent_handle& handle); 
	BOOL		WriteResume(FDownloadItem* pItem); 
	BOOL		WriteResume(libtorrent::torrent_handle& handle, const tchar* pszResumeFile); 
	void		WriteResumes(); 
public:
	

	//Notification stack. When downloads are finished, parent is notified through this sema-stack
	FFastSyncSemaStack<FAlertStructBase*>	*m_pNotify;

	FClipDownloader(FFastSyncSemaStack<FAlertStructBase*>* pNotify): m_pNotify(pNotify)
	{
	}; 
	void		LoadConfig(); 
	BOOL		AddTorrent(FDownloadItem* pItem); 
	BOOL		IsValidClip(FDownloadItem* pItem); 
	BOOL		Init(); 
	BOOL		QueueDownload(FDownloadItem* pItem);	
	BOOL		StopDownload(FDownloadItem* pItem); 
	BOOL		IsDownloadComplete(const tchar* pszFileName, const fsize_type FileSize);
	void		OnMetadataComplete(FDownloadItem* pItem, HRESULT hr); 
	void		OnHttpDownloadComplete(HRESULT hr, FUrlInfo* pInfo);
	BOOL		GetProgress(FDownloadItem* pItem, FDownloadProgress& pProgress); 
	void		Stop(); 
	BOOL		PauseResume(FDownloadItem* pItem, BOOL bResume); 
	BOOL		PauseResumeAll(BOOL bResume); 
	BOOL		ScheduleDownload(FDownloadItem* pItem, DWORD dwSecondsToWait = 30); 
	BOOL		ProcessScheduled(); 
	BOOL		RemoveScheduled(FDownloadItem* pItem); 

	FDownloadItem*	RemoveCompletedTorrent(BOOL bForceRemove); 
};

#endif //__FCLIPDOWNLOADER_H__