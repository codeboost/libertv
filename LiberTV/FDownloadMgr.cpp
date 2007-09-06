#include "stdafx.h"
#include "FDownloadMgr.h"
#include "GlobalObjects.h"
#include "FVideoConv.h"
#include "mzRegMatch.h"
#include <atlcoll.h>

#define TICK_TIMEOUT 50


FDownloadMgr::FDownloadMgr()
{
	m_hWndNotify = NULL; 
	m_bStopping = FALSE; 
	m_bStorageLoaded = FALSE; 
	m_pStatusBar = NULL; 
	m_pScheduleThread = new FScheduleThread(&m_AlertNotify);
}

FDownloadMgr::~FDownloadMgr()
{
	delete m_pScheduleThread; 
}

void FScheduleThread::Thread(void* p)
{
	FScheduleThread* pThis = (FScheduleThread*)p; 

	for (;;)
	{
		int nRes = SleepEx(1000, TRUE); 
		if (nRes != WAIT_OBJECT_0)
			break; 
		pThis->ProcessSchedule();
	}
}

void FScheduleThread::ProcessSchedule()
{
	SynchronizeThread(m_Sync); 
	time_t now = time(NULL);
	
	std::list<SchedStruct>::iterator iter = m_SchedItems.begin();
	while(iter != m_SchedItems.end())
	{
		FScheduleThread::SchedStruct& pItem = *iter; 
		if (pItem.m_RetryTime < now)
		{
			FDownloadAlert pAlert; 
			pAlert.dwCode = ALERT_SCHEDULED_EVENT; 
			pAlert.m_Param1 = (void*)pItem.m_dwType; 
			pAlert.m_Param2 = pItem.m_pParam; 
			m_pNotify->Notify(pAlert);
			iter = m_SchedItems.erase(iter); 
			continue; 
		}
		iter++; 
	}
}

void FScheduleThread::ScheduleItem(void* pParam, DWORD dwType, DWORD dwSeconds)
{
	SynchronizeThread(m_Sync); 
	time_t now = time(NULL); 
	now+=dwSeconds; 
	m_SchedItems.push_back(SchedStruct(pParam, dwType, now)); 
}

BOOL FDownloadMgr::Init()
{
	m_bStopping = FALSE; 
	m_bStorageLoaded = FALSE; 
	m_ActiveDownloads = 0; 
	m_bAllPaused = TRUE; 


	if (m_AlertNotify.Init(this))
	{
		m_pScheduleThread->Create(m_pScheduleThread); 

		g_Objects._HttpDownloader->Init();
		g_Objects._ClipDownloader->SetNotifySink(&m_AlertNotify);
		g_Objects._ClipDownloader->SetHttpDownloader(g_Objects._HttpDownloader); 

		FClipDownloadConfig aConfig; 
		g_AppSettings.FillConf(aConfig); 

		if (!g_Objects._ClipDownloader->Init(aConfig))
		{
			_DBGAlert("ERROR: Cannot Init() Clip downloader\n");
			return FALSE; 
		}
	}
	else
	{
		_DBGAlert("ERROR: Cannot Init AlertNotify\n");
		return FALSE; 
	}
	return TRUE; 
}

void FDownloadMgr::Stop()
{
	m_bStopping = TRUE; 
	_DBGAlert("FDownloadMgr::StopDownloads()\n"); 
	m_pScheduleThread->StopThread();
	m_AlertNotify.Stop(); 
	StopDownloads();
	_DBGAlert("FDownloadMgr::Stop() done\n"); 
}

BOOL FDownloadMgr::IsDownload(vidtype videoID)
{
	return FindByID(videoID) != NULL; 
}

BOOL FDownloadMgr::IsDownloadFinished(vidtype videoID)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDownload = FindByID(videoID); 
	if (pDownload)
		return pDownload->IsDownloadFinished();

	return FALSE; 
}

FDownload* FDownloadMgr::FindByID(vidtype videoID)
{
	SynchronizeThread(m_Sync); 

	IdPair* pPair = m_IdMap.Lookup(videoID); 
	
	if (pPair)
		return pPair->m_value; 

	return NULL; 
}

FDownload* FDownloadMgr::FindByGUID(DWORD dwGuid)
{
	SynchronizeThread(m_Sync); 
	for (size_t k = 0; k < m_Downloads.GetCount(); k++)
		if (m_Downloads[k]->m_RSSInfo.m_Guid == dwGuid)
			return m_Downloads[k]; 

	return NULL; 
}

BOOL FDownloadMgr::StartQueuedDownload(vidtype videoID)
{
	SynchronizeThread(m_Sync);
	FDownloadEx* pEx = (FDownloadEx*)FindByID(videoID);

	if (pEx)
	{
		if (pEx->m_dwFlags & FLAG_DOWNLOAD_QUEUED)
		{
			BOOL bFound = FALSE; 
			//find and remove from queue
			std::deque<FDownloadEx*>::iterator iter = m_QueuedDownloads.begin(); 
			while (iter != m_QueuedDownloads.end())
			{
				if (*iter == pEx)
				{
					m_QueuedDownloads.erase(iter); 
					bFound = TRUE; 
					break; 
				}
				iter++; 
			}
			ATLASSERT(bFound); 
		}
		else
		{
			if (pEx->IsDownloadableStream())
			{
				pEx->m_dwClipsActive = 0; 
				pEx->m_Detail.m_SizeDownloaded = 0; 
				pEx->m_Detail.m_TimeCompleted = 0; 
				pEx->m_Clips[0]->m_TimeCompleted = 0; 
				pEx->m_Clips[0]->m_DownloadType = "http";
				pEx->m_Clips[0]->m_DataFile = "";
				pEx->m_dwFlags&=~FLAG_DOWNLOAD_FINISHED;
				pEx->m_CurClipIndex = 0; 
				pEx->m_Clips[0]->OnSerialized();
				pEx->m_Clips[0]->m_bCompleted = FALSE;
				SaveDownload(pEx);
			}
		}

		pEx->m_dwFlags|=FLAG_DOWNLOAD_FORCE;
		return QueueDownload(pEx);
	}
	return FALSE; 
}

BOOL FDownloadMgr::QueueDownload(FDownloadEx* pDown)
{

	if (pDown->m_dwFlags & FLAG_DOWNLOAD_FINISHED && !(pDown->m_dwFlags & FLAG_DOWNLOAD_PAUSED) || pDown->IsStream())
	{
		QueueNextDownloadItem(pDown); 
		return TRUE;
	}

	ATLASSERT(m_ActiveDownloads >= 0);

	if (pDown->m_dwFlags & FLAG_DOWNLOAD_PAUSED && !(pDown->m_dwFlags & FLAG_DOWNLOAD_FORCE))
	{
		pDown->m_StatusStr = "Paused";
		return TRUE; 
	}


	BOOL bEnoughSpace = CheckFreeSpace(pDown);

	if (bEnoughSpace && (m_ActiveDownloads < (int)g_AppSettings.m_MaxDownloads || pDown->m_dwFlags & FLAG_DOWNLOAD_FORCE))
	{
		m_ActiveDownloads++; 
		pDown->m_dwFlags&=~FLAG_DOWNLOAD_QUEUED;
		pDown->m_dwFlags&=~FLAG_DOWNLOAD_FORCE;
		pDown->m_dwFlags&=~FLAG_DOWNLOAD_PAUSED; 
		QueueNextDownloadItem((FDownload*)pDown);
	}
	else
	{
		pDown->m_dwFlags|=FLAG_DOWNLOAD_QUEUED;
		pDown->m_StatusStr = "Queued";
		m_QueuedDownloads.push_back(pDown);
	}

	if (pDown->m_RSSInfo.m_Guid != 0)
	{
		if (g_Objects._VideoHistory.FindGuid(pDown->m_RSSInfo.m_Guid).m_dwValue == 0)
		{
			g_Objects._VideoHistory.SetGuid(pDown->m_RSSInfo.m_Guid, GuidInfo(1));
			g_Objects._VideoHistory.Save();
		}
	}
	

	return TRUE; 
}

BOOL FDownloadMgr::AddDownload(FDownloadEx* Video)
{
	SynchronizeThread(m_Sync); 
	if (m_bStopping)
		return FALSE; 

	Video->m_dwFlags &=~ (FLAG_DOWNLOAD_STOPPED | FLAG_DOWNLOAD_ACTIVE | FLAG_DOWNLOAD_DELETED | FLAG_DOWNLOAD_ERR_MASK | FLAG_DOWNLOAD_QUEUED);

	if (Video->m_Detail.m_TimeAdded == 0)
		Video->m_Detail.m_TimeAdded = time(NULL);


    if (Video->m_Detail.m_VideoID == 0)
    {
		_DBGAlert("**AddDownload: Video ID is 0\n"); 
		return FALSE; 
    }

	if (FindByID(Video->GetVideoID()))
	{
		_DBGAlert("**AddDownload: Video ID %d already exists in collection\n", Video->GetVideoID()); 
		return FALSE; 
	}

	m_IdMap.SetAt(Video->GetVideoID(), Video); 
	m_Downloads.Add(Video); 

	
	if (Video->m_RSSInfo.m_Guid != 0)
	{
		g_Objects._RSSManager->UpdateItemVideoId(Video->m_RSSInfo.m_Guid, Video->m_Detail.m_VideoID);

		if (Video->m_Detail.m_Watched == 0 && Video->m_RSSInfo.m_RSSURL.GetLength() > 0)
		{
			//Increase number of unwatched downloads for the RSS channel's url
			DWORD channelGuid = RSSGuidFromString(Video->m_RSSInfo.m_RSSURL);
			GuidInfo pInfo = g_Objects._RSSChannelMap.FindGuid(channelGuid);
			pInfo.m_dwValue++; 
			g_Objects._RSSChannelMap.SetGuid(channelGuid, pInfo); 
		}
	}

	//Now get item image and save it

	if (!PathIsURL(Video->m_Detail.m_ImageURL))
	{
		//Try to determine
		if (!(Video->m_RSSInfo.m_dwFlags & RSS_FLAG_FROM_RSS))
		{
			//Video is from channel guide/local
			if (PathIsURL(Video->m_Detail.m_DetailURL))
				Video->m_Detail.m_ImageURL.Format("%s&display=image", Video->m_Detail.m_DetailURL);
			//SaveDownload(Video); 
		}
		else
		{
			//Try to get image from RSS (if this item exists)
			Video->m_Detail.m_ImageURL = g_Objects._RSSManager->GetItemImageURL(Video->m_RSSInfo.m_RSSGuid, Video->m_RSSInfo.m_Guid);
			if (PathIsURL(Video->m_Detail.m_ImageURL))
				SaveDownload(Video); 
		}
	}

	if (PathIsURL(Video->m_Detail.m_ImageURL) || PathIsFileSpec(Video->m_Detail.m_ImageURL))
	{
		g_Objects._ImageCache.SetImage(Video->m_Detail.m_VideoID, Video->m_Detail.m_ImageURL); 
	}

	if (!(Video->m_dwFlags & FLAG_DOWNLOAD_PAUSED))
		m_bAllPaused = FALSE; 

	return QueueDownload(Video); 
}

//0 = failed; 1 = pending; 2 = added
#define QD_FAILED 0
#define QD_PENDING 1
#define QD_ADDED 2
int FDownloadMgr::QueueDownload(FDownload* pDownload, size_t clipIndex)
{

	if (clipIndex >= pDownload->m_Clips.GetCount())
		return FALSE; 


	//This can happen if the data was moved or deleted.
	//Try to re-download the file (or abort all along?)
	//We need to re-create the directory 

	if (!EnsureDirExists(pDownload->m_Detail.m_DataPath))
		return FALSE; 

	FDownloadItem* pItem = pDownload->m_Clips[clipIndex];
	FClipDownloadInfo pInfo; 

	pInfo.m_clipIndex = pItem->m_ClipIndex; 

	pItem->m_DataFile = pItem->m_DownloadType == "torrent" ? pItem->m_TorrentFile: pItem->m_DataFile;
	pInfo.m_DataFile = pItem->m_DataFile; 
	pInfo.m_Href = pItem->m_Href; 
	pInfo.m_HrefSize = pItem->m_HrefSize; 
	pInfo.m_StorageFolder = pItem->m_DataPath;
	pInfo.m_UrlSeed1 = pItem->m_UrlSeed; 
	pInfo.m_UrlSeed2 = pItem->m_UrlSeed1;
	pInfo.m_videoID = pItem->m_videoID;
	pInfo.m_TimeAdded = pItem->m_TimeAdded; 
	pInfo.m_TimeCompleted = pItem->m_TimeCompleted;
	pInfo.m_TimePublished = pDownload->m_Detail.m_TimePublished; 
	pInfo.m_TrackerURL = pItem->m_TrackerURL; 
	

	if (pItem->m_TimeCompleted > 0)
		pInfo.m_dwFlags |= 1;	//We know that this item has been completed

	if (pDownload->m_dwFlags & FLAG_DOWNLOAD_PAUSED)
		pInfo.m_dwFlags |= 2;  //Paused

	if (pItem->m_DownloadType.CompareNoCase("torrent") == 0)
		pInfo.m_DownloadType = Torrent;
	else 
	if (pItem->m_DownloadType.CompareNoCase("http") == 0)
		pInfo.m_DownloadType = Http; 
	else
		pInfo.m_DownloadType = None; 

	
	
	int hr = QD_FAILED; 
	pItem->m_DownloadHandle = g_Objects._ClipDownloader->AddDownload(pInfo); 
	if (pItem->m_DownloadHandle != 0)
	{
		if (pInfo.m_bMetadataComplete)
			hr = QD_ADDED; 
		else
			hr = QD_PENDING;
	}
	
	return hr;
}

void FDownloadMgr::QueueDownloadItem(FDownload* pDownload, size_t clipIndex)
{
	SynchronizeThread(m_Sync);
	FDownloadItem* pItem = pDownload->m_Clips[clipIndex];
	BOOL bMustQueue = TRUE; 

	if (pItem->m_DownloadType.GetLength() == 0 || pItem->m_DownloadType.CompareNoCase("none") == 0 ||
		pItem->m_DownloadType == "stream")
	{
		bMustQueue = FALSE; 
	}

	if (pItem->m_dwFlags & CLIP_NOT_FOUND)
		bMustQueue = FALSE; 

	if (!bMustQueue)
	{
		OnItemComplete(pDownload, pItem, S_OK); 
	}
	else
	{
		//If download is already finished, process it as complete.
		//Then add it to the queue for seeding.
		if (pItem->m_TimeCompleted > 0)
			OnItemComplete(pDownload, pItem, S_OK); 

		int hr = QueueDownload(pDownload, pItem->m_ClipIndex);

		if (hr == QD_FAILED)
			OnItemComplete(pDownload, pItem, E_FAIL); 
		else
			if (hr == QD_ADDED && pItem->m_TimeCompleted > 0)
				OnItemComplete(pDownload, pItem, S_OK); 
			else
			{
				pDownload->m_dwFlags&=~FLAG_DOWNLOAD_FINISHED; 
				pDownload->m_dwFlags&=~FLAG_DOWNLOAD_QUEUED; 
				pDownload->m_dwFlags|=FLAG_DOWNLOAD_ACTIVE;
				if (pDownload->m_dwFlags & FLAG_DOWNLOAD_PAUSED)
				{
					g_Objects._ClipDownloader->PauseResume(pItem->m_DownloadHandle, FALSE); 
					pDownload->m_StatusStr = "Paused";
				}
				else
				{
					pDownload->m_StatusStr = "Downloading";
				}
			}
	}
}

void FDownloadMgr::QueueNextDownloadItem(FDownload* pDownload)
{
	if (m_bStopping)
		return;

	SynchronizeThread(m_Sync);

	BOOL bVideoFinished = pDownload->IsDownloadFinished();

	if (pDownload->m_CurClipIndex < pDownload->m_Clips.GetCount())
	{
		FDownloadItem* pItem = pDownload->m_Clips[pDownload->m_CurClipIndex];
		ATLASSERT(pItem->m_ClipIndex == pDownload->m_CurClipIndex); 
		pDownload->m_CurClipIndex++; 
		pDownload->m_dwClipsActive++; 
		QueueDownloadItem(pDownload, pDownload->m_CurClipIndex - 1);

	}
	else
	{
		_DBGAlert("**FDownloadMgr::QueueNextDownloadItem: %d: No more clips to queue\n", pDownload->GetVideoID()); 
		ATLASSERT(FALSE); 
	}
}

BOOL FDownloadMgr::RemoveAllDownloads(BOOL bRemoveFromDisk)
{
	if (!bRemoveFromDisk)
	{
		StopDownloads();
		return TRUE; 
	}
	SynchronizeThread(m_Sync); 
	for (size_t k = 0; k < m_Downloads.GetCount(); k++)
		RemoveDownload(m_Downloads[k]->m_Detail.m_VideoID); 

	return TRUE; 
}

BOOL FDownloadMgr::DeleteDownloadObject(FDownload* pDownload)
{
	for (size_t k = 0; k < m_Downloads.GetCount(); k++)
	{
		if (pDownload == m_Downloads[k])
		{
			m_IdMap.RemoveKey(pDownload->m_Detail.m_VideoID); 
			delete pDownload; 
			m_Downloads.RemoveAt(k, 1); 
			return TRUE; 
		}
	}
	return FALSE; 
}

void FDownloadMgr::EraseDownload(FDownload* pDownload)
{
	vidtype videoID = 0; 
	{
		SynchronizeThread(m_Sync); 
		videoID = pDownload->m_Detail.m_VideoID;
		m_Storage.DeleteStorage((FDownloadEx*)pDownload); 
		DeleteDownloadObject(pDownload);			
	}
	g_Objects._ImageCache.RemoveImage(videoID); 
}

void FDownloadMgr::RemoveDownload(vidtype videoID)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDownload = FindByID(videoID); 

	if (pDownload)
	{
		pDownload->m_dwFlags|=FLAG_DOWNLOAD_DELETED; 
		StopDownload(pDownload); 
	}
}

void FDownloadMgr::StopDownload(FDownload* pDownload)
{
	SynchronizeThread(m_Sync); 
	BOOL bIncomplete = (pDownload->m_dwFlags & FLAG_DOWNLOAD_FINISHED) == 0; 
	BOOL bIsPaused   = (pDownload->m_dwFlags & FLAG_DOWNLOAD_PAUSED); 
	BOOL bIsQueued	 = (pDownload->m_dwFlags & FLAG_DOWNLOAD_QUEUED);
	BOOL bIsError    = (pDownload->m_StatusStr.Find("Error") != -1); 
	BOOL bIsStream	 = pDownload->IsStream(); 
	pDownload->m_dwFlags |= FLAG_DOWNLOAD_STOPPED;
	pDownload->m_StatusStr = "Stopped";

	DWORD dwChannelGuid = 0; 
	DWORD dwGuid = pDownload->m_RSSInfo.m_Guid;
	if (dwGuid != 0 && pDownload->m_RSSInfo.m_RSSURL.GetLength() > 0)
		dwChannelGuid = RSSGuidFromString(pDownload->m_RSSInfo.m_RSSURL); 
	
	
	for (std::deque<FDownloadEx*>::iterator k = m_QueuedDownloads.begin(); k != m_QueuedDownloads.end(); ++k)
	{
		if (*k == pDownload)
		{
			m_QueuedDownloads.erase(k); 
			break; 
		}
	}

	for (size_t k = 0; k < pDownload->m_Clips.GetCount(); k++)
	{
		g_Objects._ClipDownloader->RemoveDownload(pDownload->m_Clips[k]->m_DownloadHandle);
	}

	if (!m_bStopping && bIncomplete && !bIsPaused && !bIsQueued && !bIsStream)
	{
		m_ActiveDownloads--;
		ATLASSERT(m_ActiveDownloads >= 0);
		if (!m_QueuedDownloads.empty())
		{
			FDownloadEx* pNewDownload = m_QueuedDownloads.front(); 
			m_QueuedDownloads.pop_front(); 
			QueueDownload(pNewDownload); 
		}
	}

	if (pDownload->m_dwFlags & FLAG_DOWNLOAD_DELETED)
	{
		//wait for notifications from clip downloader before erasing
		if (!bIncomplete || bIsQueued || bIsPaused || bIsError) //finished or queued
			EraseDownload(pDownload); 
	}
	else
	{
		SaveDownload(pDownload); 
	}

	if (!m_bStopping && dwGuid != 0)
	{
		g_Objects._RSSManager->UpdateItemVideoId(dwGuid, 0);
		if (dwChannelGuid != 0)
		{
			GuidInfo pInfo = g_Objects._RSSChannelMap.FindGuid(dwChannelGuid);
			if (pInfo.m_dwValue > 0) //Watched videos may not be in the map
			{
				pInfo.m_dwValue--;
				g_Objects._RSSChannelMap.SetGuid(dwChannelGuid, pInfo); 
			}
		}
	}
}

int fnSortDownloads( const void *arg1, const void *arg2 )
{
	FDownloadEx* pItem1 = NULL; 
	FDownloadEx* pItem2 = NULL; 
	pItem1 = *(FDownloadEx**)arg1; 
	pItem2 = *(FDownloadEx**)arg2; 
	return (int)(pItem2->m_Detail.m_TimeCompleted - pItem1->m_Detail.m_TimeCompleted);
}

void FDownloadMgr::LoadIndex()
{
	if (g_AppSettings.m_dwDebugFlags & START_FLAG_NO_LOAD_STORAGE)	
		return ; 

	FArray<FDownloadEx*> aDownloads; 
	m_bStorageLoaded = m_Storage.LoadStorage(aDownloads); 

	if (m_bStorageLoaded)
	{
		SynchronizeThread(m_Sync); 
		size_t count = aDownloads.GetCount(); 
		if (count > 0)
		{
			FDownloadEx** pSorted = new FDownloadEx* [count] ;

			for (size_t k = 0 ; k < count; k++)
			{
				pSorted[k] = aDownloads[k];

			}
			qsort(&pSorted[0], count, sizeof(FDownloadEx*), fnSortDownloads); 

			for (size_t k = 0 ; k < count; k++)
			{
				if (!AddDownload(pSorted[k]))
					delete pSorted[k];
			}
			delete[] pSorted; 
		}
	}
}

void FDownloadMgr::Notify(FDownloadAlert& alert)
{
	if (alert.dwCode == ALERT_LOAD_STORAGE)
	{
		LoadIndex(); 
		return; 
	}

	switch(alert.dwCode)
	{
	case ALERT_SCHEDULED_EVENT:
		OnScheduledEvent((DWORD)alert.m_Param1, (vidtype)alert.m_Param2); 
		break; 
	case ALERT_CONTENT_TYPE:	//content type received from server for HTTP downloads
		{
			char* pContentType = (char*)alert.m_Param2;
			if (pContentType)
			{
				if (strstr(pContentType, "bittorrent"))
					OnTorrentFound(alert.m_videoID, alert.m_clipIndex, alert.hr);
				else
					OnContentType(alert.m_videoID, alert.m_clipIndex, pContentType); 
				delete[] pContentType; 
			}
		}
		break; 
	case ALERT_ITEM_QUEUED:
		OnItemQueued(alert.m_videoID, alert.m_clipIndex, alert.hr); 
		break; 

	case ALERT_TORRENT_ADDED:
		OnItemAdded(alert.m_videoID, alert.m_clipIndex, alert.hr); 
		break; 

	case ALERT_ITEM_REMOVED:
		OnItemRemoved(alert.m_videoID, alert.m_clipIndex, alert.hr);
		break; 

	case ALERT_DOWNLOAD_FINISHED:
		if (alert.hr == E_ABORT)
			OnItemRemoved(alert.m_videoID, alert.m_clipIndex, alert.hr);
		else
			OnItemComplete(alert.m_videoID, alert.m_clipIndex, alert.hr); 
		break; 
	case ALERT_LISTEN_FAILED:
		{
			if (g_AppSettings.m_RandomPort > 0)
			{
				//Try another port
				srand((int)time(NULL));
				g_AppSettings.m_ListenPort = 3000 + rand();
				FClipDownloadConfig aConf; 
				g_AppSettings.FillConf(aConf);
				_DBGAlert("Listen failed. Retrying with port = %d\n", g_AppSettings.m_ListenPort); 
				if (g_Objects._ClipDownloader->UpdateConfig(aConf))
				{
					g_AppSettings.SaveSettings(); 
					return ; 
				}
			}

			if (g_AppSettings.bDeveloperMode)
			{
				FString StrMsg; 
				StrMsg.Format("LiberTV could not listen on port %d. Check settings and specify a different listen port range.", g_AppSettings.m_ListenPort);
				g_pAppManager->ShowBalloon("LiberTV: Listen Failed", StrMsg, 2000, NIIF_ERROR);
			}
		}
		break; 
	case ALERT_FILE_ERROR:
		{
			FString StrVideoName;
			m_Sync.Lock();
			FDownload* pDown = FindByID(alert.m_videoID); 
			if (NULL != pDown)
				StrVideoName = pDown->m_Detail.m_VideoName;
			m_Sync.Unlock(); 
			
			FString StrMsg; 
			if (StrVideoName.GetLength() > 0)
				StrMsg.Format("Error accessing file: %s. Check storage disk.\n", StrVideoName); 
			else
				StrMsg = "Error accessing file. Check the storage disk for errors.";
			g_pAppManager->ShowBalloon("LiberTV: File Error", StrMsg, 5000, NIIF_ERROR);
		}
	default:
		break; 
	}
}


void FDownloadMgr::OnScheduledEvent(DWORD dwType, vidtype videoID)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDown = FindByID(videoID);
	if (pDown)
	{
		//if dwType == RETRY_DOWNLOAD...
		//ResumeDownload(videoID);
		ATLASSERT(pDown->m_Clips.GetCount() == 1);
		if (!(pDown->m_dwFlags & FLAG_DOWNLOAD_PAUSED))	//user may have paused the download...
			QueueDownloadItem(pDown, 0);
	}
}

void FDownloadMgr::OnItemQueued(vidtype videoID, size_t clipIndex, HRESULT hr)
{
	SynchronizeThread(m_Sync);
	/*
		ItemQueued means that the download has been added to the queue.
		It means that the metadata has just been downloaded and the item may soon 
		be added to the torrent session (unless the torrent session is full).
		At this stage, the following must be updated:
		- HREF size - size of the meta file
		- Status = IQueued
	*/

	FDownload* pDown = FindByID(videoID);
	if (pDown != NULL && clipIndex < pDown->m_Clips.GetCount())
	{
		FDownloadItem* pItem = pDown->m_Clips[clipIndex];
		size_type hrefSize = GetFileSize(pItem->m_DataFile); 
		BOOL bSave = FALSE; 
		if (pItem->m_HrefSize != hrefSize)
		{
			bSave = TRUE; 
			pItem->m_HrefSize = hrefSize;
		}
		if (bSave)
			SaveDownload(pDown); 
		pItem->m_DownloadStatus = IQueued; 
	}
}

BOOL FDownloadMgr::CheckFreeSpace(FDownload* pDown)
{
	if (!(pDown->m_dwFlags & FLAG_DOWNLOAD_FINISHED) &&
		!(pDown->m_dwFlags & FLAG_DOWNLOAD_PAUSED) && 
		!(pDown->m_dwFlags & FLAG_DOWNLOAD_STOPPED))
	{
		size_type totalSize = pDown->m_Detail.m_TotalSize -  pDown->m_Detail.m_SizeDownloaded;
		size_type SpaceLeft = GetSpaceLeft(pDown->m_Detail.m_DataPath);
		SpaceLeft-=(size_type)g_AppSettings.m_MinSpaceOnDriveMB * MEGABYTE;
		if (totalSize >   SpaceLeft)
		{
			time_t now = time(NULL); 
			BOOL bIsAuto = (pDown->m_RSSInfo.m_dwFlags & RSS_FLAG_FROM_AUTODOWNLOAD) != 0;
			BOOL bShowWarning = TRUE; 
			if (bIsAuto)
			{
				if (now - m_LastSpaceWarning > 30)
				{
					m_LastSpaceWarning = now; 
				}
				else
					bShowWarning = FALSE; 
			}


			if (bShowWarning)
				g_pAppManager->ShowSpaceWarning(totalSize, GetSpaceLeft(pDown->m_Detail.m_DataPath), bIsAuto);
			return FALSE; 
		}
	}
	return TRUE;
}

void FDownloadMgr::OnItemAdded(vidtype videoID, size_t clipIndex, HRESULT hr)
{
	SynchronizeThread(m_Sync);
	/*
	ItemAdded: the torrent has been added to the torrent session.
	Update download status = IDownloading
	*/

	FDownload* pDown = FindByID(videoID);
	if (pDown != NULL && clipIndex < pDown->m_Clips.GetCount())
	{
		FDownloadItem* pItem = pDown->m_Clips[clipIndex];
		if (pItem->m_DownloadStatus == IQueued)
			pItem->m_DownloadStatus = IDownloading; 

		size_type totalSize = pDown->m_Detail.m_TotalSize; 
		if (totalSize == 0)
		{
			totalSize = pDown->ComputeSizeFromClips();
			pDown->m_Detail.m_TotalSize = totalSize; 
			SaveDownload(pDown); 
			CheckFreeSpace(pDown);
		}
	}
}

void FDownloadMgr::OnItemRemoved(vidtype videoID, size_t clipIndex, HRESULT hr)
{

	SynchronizeThread(m_Sync); 
	/*
		ItemRemoved: Item has been removed from the session.
		Update status = INone
	*/
	if (m_bStopping)
		return; 
	
	FDownload* pDown = FindByID(videoID);
	if (pDown != NULL && clipIndex < pDown->m_Clips.GetCount())
	{
		FDownloadItem* pItem = pDown->m_Clips[clipIndex];
		pItem->m_DownloadStatus = INone; 
		pDown->m_dwClipsActive--;
		if (pDown->m_dwClipsActive == 0)
		{
			if (pDown->m_dwFlags & FLAG_DOWNLOAD_DELETED)
				EraseDownload(pDown);
			else
			{
				DeleteDownloadObject(pDown);
				return ;
			}
		}
			
	}
}

void FDownloadMgr::OnItemComplete(vidtype videoID, size_t clipIndex, HRESULT hr)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDownload = FindByID(videoID); 

	if (NULL != pDownload && !(pDownload->m_dwFlags & FLAG_DOWNLOAD_DELETED))
	{
		if (clipIndex < pDownload->m_Clips.GetCount())
			OnItemComplete(pDownload, pDownload->m_Clips[clipIndex], hr); 
	}
}


FString StatusFromHR(HRESULT hr)
{
	FString Str;
	switch (hr)
	{
	case E_HTTP_NET_ERROR:
		Str = "Error: Network error";
		break; 
	case E_HTTP_WRITE_FILE:
		Str = "Error: Disk I/O error";
		break; 
	case E_HTTP_NOTFOUND:
		Str = "Error: File not available";
 	break;
	default:
		Str.Format("Error: Status 0x%x", hr);
		break; 
	}
	return Str; 
}

void FDownloadMgr::OnItemComplete(FDownload* pDownload, FDownloadItem* pItem, HRESULT hr)
{
	if (NULL == pDownload || NULL == pItem)
		return ; 

	SynchronizeThread(m_Sync); 
	size_t nIndex = pItem->m_ClipIndex; 

	if (SUCCEEDED(hr))
	{
		BOOL bSaveDownload = FALSE; 
		pItem->m_DownloadStatus = IFinished; 

		//Is this the first time we've downloaded it ?
		if (pItem->m_TimeCompleted == 0)
		{
			pItem->m_TimeCompleted = time(NULL); 
			bSaveDownload = TRUE; 
		}

		//HTTP file size is 0 ? Determine it
		if (pItem->m_HrefSize == 0)
		{
			FString& DataFile = pItem->m_DownloadType == "torrent" ? pItem->m_TorrentFile : pItem->m_DataFile; 
			pItem->m_HrefSize = GetFileSize(DataFile); 
			bSaveDownload = TRUE; 
		}

		SaveDownload(pDownload); 

		if (pItem->m_bCompleted)
		{
			//This happens when we schedule downloads.
			//We schedule downloads which are completed with a delay (eg. 30 seconds)
			//When added, they are marked as Finished (this function is called once for each item)
			//After thirty seconds, libtorrent will alert us that the item is complete, so we just quietly ignore
			//the alert and go on
			return; 
		}

		pItem->m_bCompleted = TRUE; 
		pDownload->m_Detail.m_SizeDownloaded+=pItem->m_FileSize;


		if (pDownload->m_CurClipIndex  < pDownload->m_Clips.GetCount())
			QueueNextDownloadItem(pDownload); 
		else
			OnDownloadComplete(pDownload); 
	}
	else
	{
		if (!(pDownload->m_dwFlags & FLAG_DOWNLOAD_STOPPED))
		{
			if (pDownload->m_Detail.m_VideoFlags & VIDEO_FLAG_IS_PLAYLIST)
				QueueNextDownloadItem(pDownload); 
			else
			{
				pDownload->m_dwFlags|=FLAG_DOWNLOAD_ERR_NETWORK; 

				if (hr == E_HTTP_NET_ERROR || hr == E_HTTP_WRITE_FILE)
				{
					//If we have multiple clips, we ignore the HTTP error for this clip and move on to the next clip
					//If it's just one clip in the video, we schedule a retry after 30 seconds.
					m_pScheduleThread->ScheduleItem((void*)pDownload->m_Detail.m_VideoID, 1, 30);
				}
				else
				if (hr == E_HTTP_NOTFOUND)
				{
					pItem->m_dwFlags |= CLIP_NOT_FOUND;
					SaveDownload(pDownload);
				}
				pDownload->m_StatusStr = StatusFromHR(hr); 
			}
		}
	}
}

void FDownloadMgr::OnContentType(vidtype videoID, size_t clipIndex, const char* pszContentType)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDownload = FindByID(videoID); 

	if (NULL != pDownload && !(pDownload->m_dwFlags & FLAG_DOWNLOAD_DELETED))
	{
		if (clipIndex < pDownload->m_Clips.GetCount())
		{
			FDownloadItem* pItem = pDownload->m_Clips[clipIndex];
			pItem->m_ContentType = pszContentType; 
		}
	}
}

void FDownloadMgr::OnTorrentFound(vidtype videoID, size_t clipIndex, HRESULT hr)
{
	SynchronizeThread(m_Sync); 
	//This will be interesting. 
	//Convert clip from HREF to TORRENT
	FDownload* pDownload = FindByID(videoID); 

	if (NULL != pDownload && !(pDownload->m_dwFlags & FLAG_DOWNLOAD_DELETED))
	{
		if (clipIndex < pDownload->m_Clips.GetCount())
		{
			ATLASSERT(clipIndex == 0);
			ATLASSERT(pDownload->m_dwFlags & FLAG_DOWNLOAD_FINISHED);
			FDownloadItem* pItem = pDownload->m_Clips[clipIndex];
			pDownload->m_CurClipIndex = clipIndex;
			pItem->ConvertToBittorrent(); 
			pItem->m_TimeCompleted = 0; 
			pDownload->m_Detail.m_TimeCompleted = 0; 
			pDownload->m_dwFlags&=~FLAG_DOWNLOAD_FINISHED;
			SaveDownload(pDownload); 
			QueueDownloadItem(pDownload, clipIndex);
			m_ActiveDownloads++; 
		}
	}
}


void FDownloadMgr::OnDownloadComplete(FDownload* pDownload)
{

	SynchronizeThread(m_Sync); 

	//Was this an 'active' download ? 
	//
	BOOL bIncomplete = (pDownload->m_dwFlags & FLAG_DOWNLOAD_FINISHED) == 0; 

	pDownload->m_dwFlags&=~FLAG_DOWNLOAD_ACTIVE;
	pDownload->m_dwFlags|=FLAG_DOWNLOAD_FINISHED; 
	pDownload->m_StatusStr = "Finished";
	if (pDownload->m_Detail.m_TimeCompleted == 0 && pDownload->m_Clips[0]->m_DownloadType != "stream")
	{
		pDownload->m_Detail.m_TimeCompleted = time(NULL); 
		FString StrMsg; 
		StrMsg.Format("%s has been downloaded.", pDownload->m_Detail.m_VideoName);
		g_pAppManager->ShowBalloon("LiberTV: Download Complete", StrMsg, 5000, NIIF_INFO); 
	}
	SaveDownload(pDownload);
	if (bIncomplete && !pDownload->IsStream())
	{
		m_ActiveDownloads--; 
		ATLASSERT(m_ActiveDownloads >= 0); 
		if (!m_QueuedDownloads.empty())
		{
			FDownloadEx* pVideo = m_QueuedDownloads.front();
			if (CheckFreeSpace(pVideo))
			{
				m_QueuedDownloads.pop_front();
				QueueDownload(pVideo);
			}
		}
	}

#if 0

	//Now. Check if the download is HTTP
	//Hash it. Convert download type to 'torrent'

	BOOL bChanged = FALSE; 
	for (size_t k = 0; k < pDownload->m_Clips.GetCount(); k++)
	{
		FDownloadItem* pItem = pDownload->m_Clips[k];

		if (pItem->m_DownloadType == "http")
		{
			FHashFile aHashFile; 
			aHashFile.m_FileName = pItem->m_DataFile; 
			aHashFile.m_TorrentFile = g_AppSettings.MetafilePath(pDownload->m_Detail.m_VideoID, k, NULL); 
			aHashFile.m_UrlSeeds.push_back(pItem->m_Href.GetBuffer()); 
			aHashFile.m_Trackers.push_back("http://tracker.libertv.ro/announce.php"); 

			if (g_Objects._ClipDownloader->HashFile(aHashFile))
			{
				pItem->m_DownloadType = "torrent";
				pItem->m_TorrentFile = aHashFile.m_TorrentFile.c_str(); 
				pItem->m_DataFile = ""; 
				pItem->m_Href = "";
				bChanged = TRUE;
			}
		}
	}
	if (bChanged)
	{
		SaveDownload(pDownload); 
		//ToDo - QueueDownload
	}
#endif 
}

void FDownloadMgr::SaveDownload(vidtype videoID)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDown = FindByID(videoID);
	if (pDown)
		SaveDownload(pDown); 
}

void FDownloadMgr::SaveDownload(FDownload* pDownload)
{
	FDownloadEx* pEx = (FDownloadEx*)pDownload; 
	pEx->SaveConf(pEx->m_Conf); 
}

void FDownloadMgr::StopDownload(vidtype videoID)
{
	SynchronizeThread(m_Sync);
	FDownload* pDownload = FindByID(videoID);
	if (pDownload)
		StopDownload(pDownload); 
}

void FDownloadMgr::StopDownloads()
{
	SynchronizeThread(m_Sync); 
	m_bStopping = TRUE; 
	m_QueuedDownloads.clear(); 
	for (size_t k = 0; k < m_Downloads.GetCount(); k++)
	{
		StopDownload(m_Downloads[k]); 
		delete m_Downloads[k];
	}
	m_Downloads.RemoveAll(); 
}

BOOL FDownloadMgr::_PauseResume(FDownload* pDownload, BOOL bResume)
{
	if (NULL != pDownload)
	{
		BOOL bIsQueued = (pDownload->m_dwFlags & FLAG_DOWNLOAD_QUEUED) != 0; 
		
		if (bIsQueued)
			return FALSE; 

		BOOL bIsPaused = (pDownload->m_dwFlags & FLAG_DOWNLOAD_PAUSED) != 0;
		BOOL bIsFinished = (pDownload->m_dwFlags & FLAG_DOWNLOAD_FINISHED) != 0; 
		BOOL bIsActive = (pDownload->m_dwFlags & FLAG_DOWNLOAD_ACTIVE) != 0;
		


		if (bIsPaused && bResume && !bIsFinished && !bIsActive)
		{
			m_bAllPaused = FALSE;
			pDownload->m_dwFlags&=~FLAG_DOWNLOAD_PAUSED;
			m_ActiveDownloads++; 
			QueueNextDownloadItem(pDownload);
			//Check if the item is in the clipdownloader's list.
			//if not, add it
			return TRUE; 
		}


		for (size_t k = 0; k < pDownload->m_Clips.GetCount(); k++)
		{
			if (bResume)
			{
				//StopDownload(pDownload);
				//pDownload->m_CurClipIndex = 0; 
				g_Objects._ClipDownloader->PauseResume(pDownload->m_Clips[k]->m_DownloadHandle, TRUE);
				m_bAllPaused = FALSE; 
			}
			else
			{
				//pause
				g_Objects._ClipDownloader->PauseResume(pDownload->m_Clips[k]->m_DownloadHandle, FALSE); 
			}
		}

		if (bResume) 
		{
			pDownload->m_dwFlags &=~FLAG_DOWNLOAD_PAUSED; 
			pDownload->m_StatusStr = pDownload->m_dwFlags & FLAG_DOWNLOAD_FINISHED ? "Finished" : "Downloading";
		}
		else 
		{
			if (!(pDownload->m_dwFlags & FLAG_DOWNLOAD_QUEUED))
			{
				pDownload->m_dwFlags |= FLAG_DOWNLOAD_PAUSED;
				pDownload->m_StatusStr = "Paused";
			}
		}

		SaveDownload(pDownload); 
		return TRUE; 
	}
	return FALSE; 
}

BOOL FDownloadMgr::PauseDownload(vidtype videoID)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDownload = FindByID(videoID); 
	
	if (NULL != pDownload)
		return _PauseResume(pDownload, FALSE); 

	return FALSE; 
}

BOOL FDownloadMgr::ResumeDownload(vidtype videoID)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDownload = FindByID(videoID); 

	if (NULL != pDownload)
		return _PauseResume(pDownload, TRUE); 

	m_bAllPaused = FALSE; 
	return FALSE; 
}

BOOL FDownloadMgr::IsDownloadPaused(vidtype videoID)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDownload = FindByID(videoID); 

	return pDownload && 
		  (pDownload->m_dwFlags & FLAG_DOWNLOAD_PAUSED) != 0;
}

void FDownloadMgr::PauseAll()
{
	SynchronizeThread(m_Sync); 
	for (size_t k = 0; k < m_Downloads.GetCount(); k++)
	{
		_PauseResume(m_Downloads[k], FALSE); 
	}
	m_bAllPaused = TRUE; 
}

void FDownloadMgr::ResumeAll()
{
	SynchronizeThread(m_Sync); 
	for (size_t k = 0; k < m_Downloads.GetCount(); k++)
	{
		_PauseResume(m_Downloads[k], TRUE); 
	}
	m_bAllPaused = FALSE; 
}

BOOL FDownloadMgr::IsAllPaused()
{
	return m_bAllPaused; 
}

//////////////////////////////////////////////////////////////////////////

void FDownloadMgr::SaveVideoDetails(vidtype videoID, FVideoDetail& rDetail)
{
	if (videoID == 0)
		return; 

	SynchronizeThread(m_Sync); 
	
	FDownload* pDown = FindByID(videoID); 
	if (NULL != pDown)
	{
		duration_type oldWatched = pDown->m_Detail.m_Watched;
		pDown->m_Detail = rDetail; 
		SaveDownload(pDown);

		if (pDown->m_Detail.m_Watched > 0 && oldWatched == 0)
		{
			DWORD dwChannelGuid = 0; 
			if (pDown->m_RSSInfo.m_Guid != 0 && pDown->m_RSSInfo.m_RSSURL.GetLength() > 0)
			{
				dwChannelGuid = RSSGuidFromString(pDown->m_RSSInfo.m_RSSURL); 
				GuidInfo pInfo = g_Objects._RSSChannelMap.FindGuid(dwChannelGuid);
				ATLASSERT(pInfo.m_dwValue > 0); 
				pInfo.m_dwValue--;
				g_Objects._RSSChannelMap.SetGuid(dwChannelGuid, pInfo); 
			}
		}
	}
}

BOOL FDownloadMgr::LoadStorageAsync()
{
	//FDownloadAlert alert; 
	//alert.dwCode = ALERT_LOAD_STORAGE; 
	//return m_AlertNotify.m_Alerts.Push(alert) ? TRUE: FALSE; 
	LoadIndex();
	return TRUE; 
}

vidtype FDownloadMgr::ConvertAndLoad(FDownloadInfo* pInfo, const tchar* pszFolder)
{
	SynchronizeThread(m_Sync); 
	BOOL bSuccess = FALSE; 
	vidtype videoID = 0; 
	FString FileName;
	const char* pUrl = pInfo->m_DownloadUrl; 
	const char* pszFileName = pInfo->m_DownloadFile; 

	FDownloadEx* pDownload = new FDownloadEx;
	CAutoPtr<FDownloadEx> paDownload;
	paDownload.Attach(pDownload);

	if (m_Storage.ConvertDownload(pszFileName, pszFolder, pDownload))
	{
		if (pUrl != NULL)
		{
			const char* pQ = strchr(pUrl, '?');
			if (NULL != pQ)
			{
				FString sidEpisde, sidShow; 
				ExtractQueryParam(pQ, "id_episode", sidEpisde); 
				ExtractQueryParam(pQ, "id_show",	sidShow); 
				pDownload->m_Detail.m_EpisodeID = strtoul(sidEpisde, NULL, 10); 
				pDownload->m_Detail.m_ShowID	  = strtoul(sidShow, NULL, 10); 

				if (pDownload->m_Detail.m_VideoID == 0)
				{
					FString sidVideo; 
					ExtractQueryParam(pQ, "id_video", sidVideo);
					pDownload->m_Detail.m_VideoID = strtoul(sidVideo, NULL, 10); 
				}
			}
		}

		if (pDownload->m_Detail.m_VideoID == 0)
			pDownload->m_Detail.m_VideoID = g_AppSettings.GetRandomVideoID(); 

		pDownload->m_Detail.m_TimeAdded = time(NULL); 

		if (pInfo->m_RSSInfo.m_dwFlags & RSS_FLAG_FROM_RSS)
			pDownload->m_RSSInfo = pInfo->m_RSSInfo;
		else
		{
			if (pDownload->m_RSSInfo.m_Guid == 0)
			{
				if (pDownload->m_RSSInfo.m_ItemURL.GetLength() > 0)
					pDownload->m_RSSInfo.m_Guid = RSSGuidFromString(pDownload->m_RSSInfo.m_ItemURL);
				else
					pDownload->m_RSSInfo.m_Guid = RSSGuidFromString(pDownload->m_Detail.m_DetailURL	); 
			}
		}

		pDownload->SaveToDisk();
		videoID = pDownload->GetVideoID(); 

		if (pDownload->m_RSSInfo.m_Guid != 0)
		{
			FDownload* pGuid = FindByGUID(pDownload->m_RSSInfo.m_Guid);
			if (pGuid)
				return pGuid->m_Detail.m_VideoID;
		}

		if (!FindByID(videoID))
			bSuccess = AddDownload(paDownload.Detach()); 
	}
	else
	{
		_DBGAlert("**FDownloadMgr::ConvertAndLoad: Error converting file...\n"); 
		//		MessageBox(g_MainFrame, "Cannot load media. Unrecognized file format.\n", "Error", MB_OK | MB_ICONERROR); 
	}

	return bSuccess ? videoID : 0;
}

DWORD FDownloadMgr::GetVideoGuid(vidtype videoID)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDown = FindByID(videoID); 
	if (pDown)
	{
		return pDown->m_RSSInfo.m_Guid;
	}
	return 0;
}

FString FDownloadMgr::GetDownloadMTTI(vidtype videoID)
{
	SynchronizeThread(m_Sync); 
	FDownloadEx* pEx = (FDownloadEx*)FindByID(videoID); 
	if (pEx)
	{
		return pEx->m_Conf.m_FileName; 
	}
	return ""; 
}


BOOL FDownloadMgr::RenameClipExtension(vidtype videoID, FMediaFile& pFile, const tchar* pszNewExtension)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDown = FindByID(videoID); 
	if (pDown)
	{
		if (pDown->RenameClipExtension(pFile, pszNewExtension))
		{
			return TRUE; 
		}
	}
	return FALSE; 
}

void FDownloadMgr::UpdateRSSItemVids(FRSSChannel* pChannel)
{
	SynchronizeThread(m_Sync); 

	for (size_t i = 0; i < pChannel->m_Items.GetCount(); i++)
	{
		FRSSItem* pItem = pChannel->m_Items[i]; 
		pItem->m_videoID = 0; 

		for (size_t k = 0; k < m_Downloads.GetCount(); k++)
		{
			FDownloadEx* pDown = m_Downloads[k];
			if (pItem->m_Guid == pDown->m_RSSInfo.m_Guid)
			{
				pItem->m_videoID = pDown->m_Detail.m_VideoID;
			}
		}
	}
}

FCollectionSums FDownloadMgr::GetCollectionCounts()
{
	SynchronizeThread(m_Sync); 
	return m_ItemSums;
}
