#include "stdafx.h"
#include "FClipDownloader.h"
#include "FDownload.h"
#include "GlobalObjects.h"
#include <libtorrent/alert_types.hpp>

using namespace libtorrent; 
static BOOL g_LogTrackerMessages = FALSE; 

#define SEED_TIME 3600 //1 minute seed

BOOL FClipDownloader::Init()
{
#ifdef _DEBUG
	g_LogTrackerMessages = TRUE; 
#endif

	boost::filesystem::path::default_name_check(boost::filesystem::native); 
	m_TorrentSession = new libtorrent::session;

	m_ScheduleSem.Create();
	m_TickThread.Create(this, false, 1);
	m_TickThread2.Create(this); 
	m_sCompleted.SetSize(256);

	LoadConfig(); 

	m_TorrentSession->listen_on(std::make_pair(g_AppSettings.m_LPortMin, g_AppSettings.m_LPortMax)); 
	m_TorrentSession->set_severity_level(alert::info);


	return TRUE; 
}

typedef std::vector<torrent_handle> torrent_t; 

void FClipDownloader::LoadConfig()
{

	int iKBDown = (int)g_AppSettings.m_MaxKBDown * 1024; if (iKBDown == 0)	iKBDown = -1; 
	int iKBUp   = (int)g_AppSettings.m_MaxKBUp * 1024; if (iKBUp == 0) iKBUp = -1; 

	m_TorrentSession->set_download_rate_limit(iKBDown); 
	m_TorrentSession->set_upload_rate_limit(iKBUp); 
	m_TorrentSession->set_max_connections(g_AppSettings.m_MaxConnections); 
	m_TorrentSession->set_max_uploads(g_AppSettings.m_MaxUploads); 

	//HTTP Settings
	FString UserAgent; 
	UserAgent.Format("ltv_%s", g_AppSettings.m_AppVersion); 

#if LT_VERSION <= 0x09
	http_settings hts;
	hts.stop_tracker_timeout = 2;	//when stopping, just notify, don't wait for response
	hts.tracker_timeout = 15;		//wait for response from tracker
	hts.user_agent = UserAgent;		

	//Proxy stuff
	std::string porttmp; 
	if (StrSplit(g_AppSettings.m_Proxy, hts.proxy_ip, porttmp))
	{
		hts.proxy_port = StrToInt(porttmp.c_str()); 
		StrSplit(g_AppSettings.m_ProxyA, hts.proxy_login, hts.proxy_password);
	}
	m_TorrentSession->set_http_settings(hts);
#else
	session_settings sset;
	sset.stop_tracker_timeout = 2; 
	sset.tracker_completion_timeout  = 15; 
	sset.user_agent = UserAgent; 
	//Proxy stuff
	std::string porttmp; 
	if (StrSplit(g_AppSettings.m_Proxy, sset.proxy_ip, porttmp))
	{
		sset.proxy_port = StrToInt(porttmp.c_str()); 
		StrSplit(g_AppSettings.m_ProxyA, sset.proxy_login, sset.proxy_password);
	}

	sset.max_out_request_queue = 2048;
	sset.max_allowed_in_request_queue = 2048; 
	m_TorrentSession->set_settings(sset); 

#endif
}

void FClipDownloader::Stop()
{
	SynchronizeThread(m_Sync); 
	
	
	m_sCompleted.Clear(); 
	m_TickThread.StopAndWait(INFINITE); 

	m_ClipQueue.RemoveAll();
	m_TickThread2.StopThread(); 

	torrent_t vTorrents = m_TorrentSession->get_torrents(); 

	torrent_t::iterator i = vTorrents.begin();

	
	for (torrent_t::iterator i = vTorrents.begin(), end(vTorrents.end()); i != end; ++i)
	{
		torrent_handle h = *i; 
		if (h.is_valid())
		{

			h.pause(); 
			FDownloadItem* pItem = FindByTorrentHandle(h); 
			if (pItem)
				WriteResume(pItem);
			m_TorrentSession->remove_torrent(h); 
		}
	}

	delete m_TorrentSession; 
}

BOOL FClipDownloader::IsDownloadComplete(const tchar* pszFileName, const fsize_type FileSize)
{
	return (PathFileExists(pszFileName) && 
		GetFileSize(pszFileName) == FileSize);
}


/*
	How downloads are processed:
	We start by downloading the file pointed by href (the metafile).
	For this, we generate the filename = StorageDir\videoID_index.vf
	The filename is stored as pItem->m_MetaFile in the mtti file. The metafile size is stored as pItem->m_MetaSize.
	These two members are used to check weather or not to start the actual download 
	(eg. if the file exists on disk and is of the right size)

	When the metafile is downloaded, we look at pItem->DownloadType.

	1. If pItem->DownloadType == "http" or "", download is finished. Notify DownloadMgr
	2. If pItem->DownloadType == "torrent", we load the torrent from the MetaFile and start downloading
	the torrent. 
*/

BOOL FClipDownloader::QueueDownload(FDownloadItem* pItem)
{
	SynchronizeThread(m_Sync); 

	_DBGAlert("QueueDownload: %d, clipID = %d\n", pItem->m_Video->m_Detail.m_VideoID, pItem->m_ClipIndex);
	m_Clips.AddHead(pItem); 

	FDownload* pVideo = pItem->m_Video;
	vidtype videoID = pVideo->GetVideoID(); 

	if (pItem->m_TimeAdded == 0)
		pItem->m_TimeAdded = time(NULL); 

	pItem->m_DownloadStatus = FMetaData; 

	if (pItem->m_Href.GetLength() == 0 || pItem->m_DownloadType == "none")
	{
		OnMetadataComplete(pItem, S_OK); 
		return TRUE; 
	}

	if (pItem->m_DownloadType.GetLength() == 0)
		pItem->m_DownloadType = "http";

	FString Href = pItem->m_Href; 
	FString DataFile = "";
	fsize_type hrefSize = pItem->m_HrefSize; 
	
	if (pItem->m_DownloadType.CompareNoCase("http") == 0)
	{
		DataFile = pItem->m_DataFile; 
	}
	else
	if (pItem->m_DownloadType.CompareNoCase("torrent") == 0)
	{
		DataFile = pItem->m_TorrentFile; 
	}
	else
	{
		if (pItem->m_DataFile.GetLength() > 0)
			DataFile = pItem->m_DataFile; 
		else
		{
			if (PathIsFileSpec(pItem->m_Href) == FALSE)
				DataFile = pItem->m_Href;
		}
	}


	if (PathIsURL(pItem->m_Href))
	{
		if (DataFile == "")
		{
			ATLASSERT(FALSE); 
			return FALSE; 
		}

		if (IsDownloadComplete(DataFile, hrefSize))
		{
			OnMetadataComplete(pItem, S_OK); 
			return TRUE; 
		}
		FUrlInfo* pInfo = new FUrlInfo; 
		pInfo->m_DownloadUrl = Href; 
		pInfo->m_DownloadFile = DataFile; 
		pInfo->m_pv = (void*) pItem; 
		pItem->m_DownloadHandle = g_Objects._HttpDownloader.DownloadURL(pInfo, this);

		if (pItem->m_DownloadHandle != 0)
		{
			_DBGAlert("QueueDownload: %d: Queued HTTP download: %s\n", videoID, pItem->m_Href); 
			return TRUE; 
		}
		else
		{
			pItem->m_DownloadStatus = FDownloadError;
			g_Objects._HttpDownloader.ReleaseUrlInfo(pInfo); 
		}
	}
	else
	{
		if (PathIsFileSpec(pItem->m_Href) == FALSE)
		{
			if (PathFileExists(pItem->m_Href))
			{
				pItem->m_DataFile = DataFile; 
				pItem->m_HrefSize = GetFileSize(DataFile);
				OnMetadataComplete(pItem, S_OK); 
				return TRUE; 
			}
		}
		_DBGAlert("**QueueDownload: [%d]: href is not a URL\n", videoID); 
		return FALSE; 
	}

	return FALSE; 
}


void FClipDownloader::OnMetadataComplete(FDownloadItem* pItem, HRESULT hr)
{
    if (hr == E_ABORT)
    {
        SendNotifyAlert(pItem, ALERT_ITEM_REMOVED, 200);	
        return; 
    }

    if (pItem->m_DownloadType == "torrent")
    {
        ATLASSERT(pItem->m_DownloadStatus == FMetaData); 
		pItem->m_HrefSize = GetFileSize(pItem->m_TorrentFile); 
		if (pItem->m_FileSize == 0)
			pItem->m_FileSize = pItem->m_HrefSize; 

        //200: .torrent downloaded and added to the torrent session. Download is in progress
        //500: .torrent download failed. Download cannot continue.
        //501: .torrent downloaded, but couldn't be added to the torrent session.

        if (SUCCEEDED(hr))
        {
            if (!AddTorrent(pItem))
                hr = E_FAIL; 
        }
        SendNotifyAlert(pItem, ALERT_ITEM_ADDED, hr); 
        return; 
    }
    else
    {
		pItem->m_HrefSize = GetFileSize(pItem->m_DataFile); 
		if (pItem->m_FileSize == 0)
			pItem->m_FileSize = pItem->m_HrefSize; 
    }

    SendNotifyAlert(pItem, ALERT_HTTP_DOWNLOAD_FINISHED, hr); 
}

void FClipDownloader::OnHttpDownloadComplete(HRESULT hr, FUrlInfo* pInfo)
{
    SynchronizeThread(m_Sync); 

    FDownloadItem* pItem = (FDownloadItem*)pInfo->m_pv;

    _DBGAlert("OnHttpDownloadComplete: %d: 0x%x - %s\n", pItem->m_ClipIndex, hr, pInfo->m_DownloadUrl); 
    if (IsValidClip(pItem))
    {
        OnMetadataComplete(pItem, hr); 
    }
    else
    {
        //ATLASSERT(FALSE); 
		DeleteFile(pInfo->m_DownloadFile);
    }

	g_Objects._HttpDownloader.ReleaseUrlInfo(pInfo);
}

FDownloadItem* FClipDownloader::FindClip(DHANDLE DownloadHandle)
{
	SynchronizeThread(m_Sync); 

	POSITION myPos = m_Clips.GetHeadPosition(); 

	while(myPos != NULL)
	{
		FDownloadItem* pItem = m_Clips.GetNext(myPos); 
		if (pItem->m_DownloadHandle == DownloadHandle)
			return pItem; 

	}
	return NULL; 
}

POSITION FClipDownloader::_SyncFindClip(FDownloadItem* ppClip)
{
	POSITION myPos = m_Clips.GetHeadPosition(); 

	POSITION lastPos = myPos; 
	while(myPos != NULL)
	{

		FDownloadItem* pItem = m_Clips.GetNext(myPos); 
		if (pItem == ppClip)
			return lastPos; 
		lastPos = myPos; 
	}
	return NULL; 
}

BOOL FClipDownloader::IsValidClip(FDownloadItem* ppClip)
{
	SynchronizeThread(m_Sync); 
	return (_SyncFindClip(ppClip) != NULL);
}


BOOL FClipDownloader::RemoveClip(FDownloadItem* ppClip)
{
	SynchronizeThread(m_Sync); 
	POSITION myPos = _SyncFindClip(ppClip); 
	ATLASSERT(myPos != NULL);
	m_Clips.RemoveAt(myPos); 
	m_ScheduleSem.Release(); 
	return myPos != NULL; 
}

BOOL FClipDownloader::WriteResume(libtorrent::torrent_handle& h, const tchar* pszResumeFile)
{
	try{
		entry data = h.write_resume_data();
		boost::filesystem::ofstream out(pszResumeFile, std::ios_base::binary);
		out.unsetf(std::ios_base::skipws);
		bencode(std::ostream_iterator<char>(out), data);
		out.close();
		return TRUE; 
	}
	catch(std::exception& e)
	{
		_DBGAlert("**FClipDownloader: WriteResume(): %s\n", e.what());
	}
	return FALSE; 
}

BOOL FClipDownloader::WriteResume(FDownloadItem* pItem)
{
	
	if (pItem->m_TorrentHandle.is_valid())
	{
			path file = pItem->m_TorrentFile; 
			path resume_file = boost::filesystem::change_extension(file, ".resume");

			torrent_handle& h = pItem->m_TorrentHandle;
			if (WriteResume(h, resume_file.string().c_str()))
			{
				pItem->m_ResumeWritten = time(NULL); 
				return TRUE; 
			}
	}
	return FALSE; 

}

void FClipDownloader::WriteResumes()
{
	SynchronizeThread(m_Sync); 

	time_t nowTime = time(NULL); 

	POSITION myPos = m_Clips.GetHeadPosition(); 
	while(myPos != NULL)
	{
		FDownloadItem* pItem = m_Clips.GetNext(myPos); 
		if (pItem)
		{
			if (pItem->m_TorrentHandle.is_valid() && !pItem->IsFinished())
			{
				if (nowTime - pItem->m_ResumeWritten > 60 * 1)
				{
					WriteResume(pItem); 
				}
			}
		}
	}
}

BOOL FClipDownloader::StopDownload(FDownloadItem* pItem)
{
	SynchronizeThread(m_Sync); 

	if (RemoveScheduled(pItem))
		return TRUE; 

	if (!_SyncFindClip(pItem))
	{
		return FALSE;	
	}
	//remove clip from list
	RemoveClip(pItem); 

	if (pItem->m_DownloadType == "torrent")
	{
		if (pItem->m_DownloadStatus == FMetaData)
		{
				if (!g_Objects._HttpDownloader.StopDownload(pItem->m_DownloadHandle))
				{
					SendNotifyAlert(pItem, ALERT_ITEM_REMOVED, 200);	
					return TRUE; 
				}
		}
		else
		{
			if (pItem->m_TorrentHandle.is_valid())
			{
				try{

					pItem->m_TorrentHandle.pause();

					WriteResume(pItem);

					m_TorrentSession->remove_torrent(pItem->m_TorrentHandle); 

					while (pItem->m_TorrentHandle.is_valid())
					{
						if (WAIT_IO_COMPLETION == SleepEx(100, TRUE))
							break; 
					}

				} catch(std::exception& e){	
					_DBGAlert("Remove Torrent exception: %s\n", e.what());
					SendNotifyAlert(pItem, ALERT_ITEM_REMOVED, 500); 
				}
			}
			else
				SendNotifyAlert(pItem, ALERT_ITEM_REMOVED, 404); 
		}
	}
	else
	if (pItem->m_DownloadType == "http")
	{
		if (!g_Objects._HttpDownloader.StopDownload(pItem->m_DownloadHandle))
			SendNotifyAlert(pItem, ALERT_ITEM_REMOVED, 200);	
	}

	return TRUE; 
}

void FClipDownloader::SendNotifyAlert(FDownloadItem* pItem, DWORD dwCode, DWORD dwStatusCode)
{
	FAlertStructBase* pNewAlert = new FAlertStructBase;
	pNewAlert->dwCode = dwCode; 
	pNewAlert->pDownloadItem = (void*)pItem; 
	pNewAlert->dwStatusCode = dwStatusCode; 
	pNewAlert->m_DHandle = pItem->m_DownloadHandle;
	m_pNotify->Push(pNewAlert); 
}

//Scheduler
BOOL FClipDownloader::ScheduleDownload(FDownloadItem* pItem, DWORD dwSecondsToWait)
{
	SynchronizeThread(m_Sync); 
	m_ClipQueue.AddTail(FQueueItem(pItem, time(NULL) + dwSecondsToWait)); 

	if (m_TorrentSession->get_torrents().size() < g_AppSettings.m_MaxTorrents)
		m_ScheduleSem.Release();
	return TRUE; 
}


BOOL FClipDownloader::RemoveScheduled(FDownloadItem* pDownload)
{
	SynchronizeThread(m_Sync);
	POSITION myPos = m_ClipQueue.GetHeadPosition(); 
	POSITION lastPos = myPos; 
	while(myPos != NULL)
	{
		lastPos = myPos; 
		FQueueItem& pItem = m_ClipQueue.GetNext(myPos); 
		if (pItem.pItem == pDownload)
		{
			m_ClipQueue.RemoveAt(lastPos); 
			return TRUE; 
		}
	}
	return FALSE; 
}

BOOL FClipDownloader::ProcessScheduled()
{
	SynchronizeThread(m_Sync); 
	time_t now = time(NULL); 
	POSITION myPos = m_ClipQueue.GetHeadPosition(); 
	POSITION lastPos = myPos; 
	while(myPos != NULL)
	{
		lastPos = myPos; 
		FQueueItem& pItem = m_ClipQueue.GetNext(myPos); 
		if (pItem.time_to_add <= now)
		{
			FDownloadItem* pDItem = NULL; 
			if (m_TorrentSession->get_torrents().size() >= g_AppSettings.m_MaxTorrents)
			{
				pDItem = RemoveCompletedTorrent(FALSE); 
			}


			m_ClipQueue.RemoveAt(lastPos); 
			QueueDownload(pItem.pItem);
			if (NULL != pDItem)
			{
				m_ClipQueue.AddTail(FQueueItem(pDItem, time(NULL) + SEED_TIME));
			}
			return TRUE; 

		}
	}
	return FALSE; 
}

void FClipDownloader::FTickThread2::Thread(void* p)
{
	FClipDownloader* pThis = (FClipDownloader*)p;
	int SleepTime = 1000; 
	for (;;)
	{
		int nRes = pThis->m_ScheduleSem.Wait(10 * 1000); 
		
		if (nRes != WAIT_OBJECT_0 && nRes != WAIT_TIMEOUT)
			break; 

		if (pThis->ProcessScheduled())
		{
			nRes = SleepEx(1000, TRUE); 
			if (nRes != 0)
				break; 
		}
	}
}

void FClipDownloader::FTickThread::Thread(void* p)
{
	FClipDownloader* pThis = (FClipDownloader*)p;

	pThis->m_LastCheck = GetTickCount(); 
	for (;;)
	{

		int nRes = SleepEx(500, TRUE); 
		if (nRes != 0)
			break; 
		pThis->PollTorrentStatus(); 
	}
}



//////////////////////////////////////////////////////////////////////////

BOOL FClipDownloader::AddTorrent(FDownloadItem* pItem)
{
	SynchronizeThread(m_Sync); 

	ATLASSERT(IsValidClip(pItem)); 
	pItem->m_TorrentHandle = LoadTorrent(pItem); 
	if (pItem->m_TorrentHandle.is_valid())
	{
		_DBGAlert("LoadTorrent: %d, clipID = %d\n", pItem->m_Video->m_Detail.m_VideoID, pItem->m_ClipIndex);
		pItem->m_TorrentHandle.set_download_limit(-1); 
		pItem->m_DownloadStatus = FDownloading;
		pItem->m_ResumeWritten = time(NULL); 
	}
	else
	{
		_DBGAlert("**LoadTorrent Failed: %d, clipID = %d\n", pItem->m_Video->m_Detail.m_VideoID, pItem->m_ClipIndex);
		pItem->m_DownloadStatus = FDownloadError;
	}

	return pItem->m_TorrentHandle.is_valid(); 
}

FDownloadItem* FClipDownloader::FindByTorrentHandle(torrent_handle& handle)
{
	SynchronizeThread(m_Sync); 
	POSITION myPos = m_Clips.GetHeadPosition(); 

	while(myPos != 0)
	{
		FDownloadItem* pItem = m_Clips.GetNext(myPos); 
		if (pItem->m_TorrentHandle == handle)
			return pItem; 
	}			 
	return NULL; 
}


FDownloadItem* FClipDownloader::RemoveCompletedTorrent(BOOL bForceRemove)
{
	SynchronizeThread(m_Sync);
	std::vector<torrent_handle> torrents = m_TorrentSession->get_torrents(); 

	_DBGAlert("RemoveCompletedTorrent()\n");
	FDownloadItem* pCanditate = NULL; 
	FDownloadItem* pAnyItem = NULL; 
	int max_seeds = 0;
	dword minTime = 0; 

	time_t now = time(NULL); 

	for (size_t k = 0; k < torrents.size(); k++)
	{
		torrent_handle h = torrents[k];
		if (h.is_seed())
		{
			FDownloadItem* pItem = FindByTorrentHandle(h); 
			
			if (NULL != pItem && pItem->m_SeedStarted > 0)
			{
				pAnyItem = pItem; 
				int dif = now - pItem->m_SeedStarted;
				
				if (dif > SEED_TIME)
				{
					torrent_status t = h.status(); 
					if (max_seeds <= t.num_seeds)
					{
						max_seeds = t.num_seeds; 
						pCanditate = pItem; 
					}
				}
			}

		}
	}

	if (NULL == pCanditate && bForceRemove)
		pCanditate = pAnyItem; 

	if (NULL != pCanditate)
	{
		try{
			m_TorrentSession->remove_torrent(pCanditate->m_TorrentHandle); 	
			_DBGAlert("RemoveCompletedTorrent: Removed item: %d - %d: Seeds = %d\n", pCanditate->m_videoID, pCanditate->m_ClipIndex, max_seeds);
			return pCanditate; 
		} catch(std::exception& e){
			_DBGAlert("RemoveCompletedTorrent exception: %s\n", e.what());
		}
	}

	return NULL; 
}


libtorrent::torrent_handle FClipDownloader::LoadTorrent(FDownloadItem* pItem)
{
	torrent_handle tHandle;
	path file = pItem->m_TorrentFile; 

	path resume_file = boost::filesystem::change_extension(file, ".resume");

	_DBGAlert("Resume File: %s\n", resume_file.string().c_str()); 
	

	std::ifstream in_t(file.string().c_str(), std::ios_base::binary);
	if (!in_t.is_open())
	{
		_DBGAlert("**FClipDownloader::LoadTorrent: Cannot open file %s\n", file.string().c_str());
		return tHandle;
	}
	in_t.unsetf(std::ios_base::skipws);
	entry e = bdecode(std::istream_iterator<char>(in_t), std::istream_iterator<char>()); 
	in_t.close();

	torrent_info t(e);
	if (pItem->m_UrlSeed.GetLength() > 0)
		t.add_url_seed(std::string(pItem->m_UrlSeed)); 

	if (pItem->m_UrlSeed1.GetLength() > 0)
		t.add_url_seed(std::string(pItem->m_UrlSeed1)); 

	if (t.num_files() > 0)
	{
		//Load resume data
		entry resume_data;
		std::ifstream in(resume_file.string().c_str(), std::ios_base::binary); 
		if (in.is_open())
		{
			in.unsetf(std::ios_base::skipws);
			try{
				resume_data = bdecode(std::istream_iterator<char>(in), std::istream_iterator<char>());
			} catch(std::exception& e){
				_DBGAlert("**FClipDownloader::LoadTorrent exception: Cannot load resume file: %s\n", e.what()); 
			}
			in.close();
		}
		else
		{
			_DBGAlert("FClipDownloader::LoadTorrent: Cannot load resume file %s\n", resume_file.string().c_str());
		}
		

		try
		{

			if (m_TorrentSession->get_torrents().size() > g_AppSettings.m_MaxTorrents)
			{
				RemoveCompletedTorrent(TRUE); 
			}

			int blockSize = pItem->m_UrlSeed.GetLength() > 0 ? 512 * 1024 : 32 * 1024; 
			boost::filesystem::path StorageDir = pItem->m_DataPath; 
			tHandle = m_TorrentSession->add_torrent(t, StorageDir, resume_data, false, blockSize); 
		} 
		catch (std::exception& e)
		{
			_DBGAlert("**FClipDownloader::LoadTorrent: Exception: %s\n", e.what()); 
		}
	}
	return tHandle; 
}

BOOL FClipDownloader::GetProgress(FDownloadItem* pItem, FDownloadProgress& pProgress)
{
	if (pItem->m_DownloadType == "http")
	{
		if (g_Objects._HttpDownloader.GetDownloadProgress(pItem->m_DownloadHandle, pProgress))
		{
			if (pItem->m_FileSize == 0)
				pItem->m_FileSize = pProgress.m_bytesTotal; 
		}
	}
	else
	{
		try{
			if (pItem->m_TorrentHandle.is_valid())
			{
				torrent_status aStatus = pItem->m_TorrentHandle.status();
				pProgress.m_bytesPerSec = aStatus.download_rate;
				pProgress.m_percentComplete = aStatus.progress;
				pProgress.m_upBytesPerSec = aStatus.upload_rate;
				pProgress.m_bytesDownloaded = aStatus.total_done; 
				pProgress.m_bytesUploaded = aStatus.total_upload; 
				pProgress.m_bytesTotal = aStatus.total_wanted; 
				pProgress.m_Seeds = aStatus.num_seeds;
				pProgress.m_Peers = aStatus.num_peers; 
			}
		}
		catch(std::exception &){}
	}
	return TRUE; 
}

void FClipDownloader::PollTorrentStatus()
{
	using namespace libtorrent; 

	std::auto_ptr<alert> Alert;
	Alert = m_TorrentSession->pop_alert();
	FDownloadItem* pItem = NULL; 

	while (Alert.get())
	{
		SynchronizeThread(m_Sync); 

		if (torrent_finished_alert* p = dynamic_cast<torrent_finished_alert*>(Alert.get()))
		{
			pItem = FindByTorrentHandle(p->handle); 
			if (NULL == pItem)
				ATLASSERT(pItem != NULL);
			else
			{
				pItem->m_SeedStarted = time(NULL); 
				SendNotifyAlert(pItem, ALERT_DOWNLOAD_FINISHED, S_OK); 
				WriteResume(pItem); 
				if (g_LogTrackerMessages) _DBGAlert("FClipDownloader: torrent-finished-alert: %s\n", Alert->msg().c_str()); 
			}
		}
		else if (tracker_alert* p = dynamic_cast<tracker_alert*>(Alert.get()))
		{
			pItem = FindByTorrentHandle(p->handle); 
			if (NULL == pItem)
				ATLASSERT(pItem != NULL);
			else
			{
				SendNotifyAlert(pItem, ALERT_TRACKER_REPLY, p->status_code); 
				if (g_LogTrackerMessages) _DBGAlert("FClipDownloader: tracker-alert: status_code=%d; msg=%s; \n", p->status_code, Alert->msg().c_str());
			}
		}
		else if (tracker_reply_alert* p = dynamic_cast<tracker_reply_alert*>(Alert.get()))
		{

			pItem = FindByTorrentHandle(p->handle); 
			
			if (NULL == pItem)
				ATLASSERT(pItem != NULL);
			else
			{
				if (!pItem->IsFinished())
					WriteResume(pItem); 

				SendNotifyAlert(pItem, ALERT_TRACKER_REPLY, S_OK); 
				try
				{
					torrent_status ts = p->handle.status();
					if (g_LogTrackerMessages) _DBGAlert("FClipDownloader: tracker-reply: %s; seeds=%d; peers=%d; complete sources=%d; incomplete=%d; announce-interval: %d;\n", 
						p->msg().c_str(), ts.num_seeds, ts.num_peers, ts.num_complete, ts.num_incomplete, ts.announce_interval / 1000000 ); 
				}
				catch(...){}
			}
		}

		else if (peer_error_alert* p = dynamic_cast<peer_error_alert*>(Alert.get()))
		{
			_DBGAlert("FClipDownloader: peer-error: %s\n", p->msg().c_str());
		}
		else if (tracker_announce_alert* p = dynamic_cast<tracker_announce_alert*>(Alert.get()))
		{
			pItem = FindByTorrentHandle(p->handle); 

			if (NULL != pItem)
			{
				//check if event = stopped
				if (p->msg().find("event=stopped") != std::string::npos)
				{
					SendNotifyAlert(pItem, ALERT_ITEM_REMOVED, S_OK);  
					RemoveClip(pItem); 
				}
				if (p->msg().find("event") != std::string.npos)
				{
					if (g_LogTrackerMessages) _DBGAlert("FClipDownloader: tracker-announce: %s\n", p->msg().c_str()); 
				}
			}
		}
		else if (file_error_alert* p = dynamic_cast<file_error_alert*>(Alert.get()))
		{
			_DBGAlert("FClipDownloader: file-error-alert: %s\n", p->msg().c_str());
		}
		else if (listen_failed_alert* p = dynamic_cast<listen_failed_alert*>(Alert.get()))
		{
			_DBGAlert("FClipDownloader: listen-failed: %s\n", p->msg().c_str()); 
			FString StrMsg; 
			StrMsg.Format("Could not listen. Please check settings: %s", p->msg().c_str()); 
			
			MessageBox(NULL, StrMsg, "Listen Error", MB_OK | MB_ICONERROR); 
		}
		else if (fastresume_rejected_alert* p = dynamic_cast<fastresume_rejected_alert*>(Alert.get()))
		{
			_DBGAlert("FClipDownloader: fast resume-rejected: %s\n", p->msg().c_str()); 
		}
		else if (invalid_request_alert* p = dynamic_cast<invalid_request_alert*>(Alert.get()))
		{
			_DBGAlert("FClipDownloader: invalid-request: %s\n", p->msg().c_str());
		}
		else if (tracker_warning_alert* p = dynamic_cast<tracker_warning_alert*>(Alert.get()))
		{
			_DBGAlert("FClipDownloader: tracker-warning: %s\n", p->msg().c_str());
		}
		else if (url_seed_alert* p = dynamic_cast<url_seed_alert*>(Alert.get()))
		{
			_DBGAlert("FClipDownloader: url-seed: %s\n", p->msg().c_str()); 
		}
		Alert = m_TorrentSession->pop_alert();
	}
}

BOOL FClipDownloader::PauseResume(FDownloadItem* pItem, BOOL bResume)
{
	SynchronizeThread(m_Sync); 
	if (!IsValidClip(pItem))
		return FALSE; 

	if (pItem)
	{
		if (pItem->m_DownloadType.CompareNoCase("torrent") == 0 && pItem->m_TorrentHandle.is_valid())
		{
			if (!bResume)
				pItem->m_TorrentHandle.pause();
			else
				pItem->m_TorrentHandle.resume(); 
		}
		else
		if (pItem->m_DownloadType.CompareNoCase("http") == 0)
		{
			if (!bResume)
				g_Objects._HttpDownloader.Pause(pItem->m_DownloadHandle); 
			else
				g_Objects._HttpDownloader.Resume(pItem->m_DownloadHandle); 
		}
		return TRUE; 
	}
	return FALSE; 
}

BOOL FClipDownloader::PauseResumeAll(BOOL bResume)
{
	SynchronizeThread(m_Sync); 
	POSITION myPos = m_Clips.GetHeadPosition(); 
	while(myPos != 0)
	{
		PauseResume(m_Clips.GetNext(myPos), bResume); 
	}	
	return TRUE; 
}


