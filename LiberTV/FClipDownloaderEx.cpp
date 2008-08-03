#include "stdafx.h"
#include "FClipDownloaderEx.h"
#include <libtorrent/alert_types.hpp>
#include <fstream>
#include <boost/filesystem/path.hpp>
#include "hasher.hpp"
#include "libtorrent/storage.hpp"
#include <libtorrent/extensions/metadata_transfer.hpp>
#include <libtorrent/extensions/ut_pex.hpp>
#include "IAppManager.h"

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif

extern IAppManager* g_pAppManager;
using namespace libtorrent; 

BOOL g_LogTrackerMessages = TRUE; 

#define _LogMessage _DBGAlert


FClipDownloaderEx::FClipDownloaderEx()
{
	m_TorrentSession = NULL; 
	m_pNotify = NULL; 
	m_pHttpDownloader = NULL; 
	m_CurHandle = 0; 
}

FClipDownloaderEx::~FClipDownloaderEx()
{
	//Session must be destroyed in Stop()
	ATLASSERT(m_TorrentSession == NULL); 
}	

BOOL FClipDownloaderEx::UpdateConfig(FClipDownloadConfig& aConf)
{
	SynchronizeThread(m_Sync); 
	m_Config = aConf; 
	if (LoadConfig() && m_TorrentSession)
	{
		unsigned short listen_port = m_TorrentSession->listen_port();
		if (listen_port != m_Config.m_LPortMin) 
		{
			m_TorrentSession->listen_on(std::make_pair(m_Config.m_LPortMin, m_Config.m_LPortMax)) ? TRUE: FALSE; 
		}
		return TRUE; 
	}
	return FALSE; 
}

BOOL FClipDownloaderEx::StartDHT()
{
	boost::filesystem::path dht_path = _PathCombine(m_Config.m_IndexPath, ".dht_State");

	boost::filesystem::ifstream dht_state_file(dht_path, std::ios_base::binary);
	dht_state_file.unsetf(std::ios_base::skipws);
	entry dht_state;
	try
	{
		dht_state = bdecode(std::istream_iterator<char>(dht_state_file), std::istream_iterator<char>());
	}
	catch (std::exception&) {
		return FALSE; 
	}
	m_TorrentSession->start_dht(dht_state);
	m_TorrentSession->add_dht_router(std::make_pair(std::string("router.bittorrent.com"), 6881));
	m_TorrentSession->add_dht_router(std::make_pair(std::string("router.utorrent.com"), 6881));
	m_TorrentSession->add_dht_router(std::make_pair(std::string("router.bitcomet.com"), 6881));
	m_TorrentSession->add_dht_router(std::make_pair(std::string("dht.aelitis.com"), 6881));
	
	
	return TRUE; 
}

BOOL FClipDownloaderEx::LoadConfig()
{
	//////////////////////////////////////////////////////////////////////////

	int iKBDown = (int)m_Config.m_MaxKBDown * 1024; if (iKBDown == 0)	iKBDown = -1; 
	int iKBUp   = (int)m_Config.m_MaxKBUp * 1024; if (iKBUp == 0) iKBUp = -1; 

	m_TorrentSession->set_download_rate_limit(iKBDown); 
	m_TorrentSession->set_upload_rate_limit(iKBUp); 
	m_TorrentSession->set_max_connections(m_Config.m_MaxConnections); 
	m_TorrentSession->set_max_uploads(m_Config.m_MaxUploads); 

	//HTTP Settings
	session_settings sset;
	sset.stop_tracker_timeout = 10; 
	sset.tracker_completion_timeout  = 60; 
	sset.user_agent = "uTorrent/1610"; 
	
	//Proxy stuff
	
#if 1
	libtorrent::proxy_settings proxy; 
	std::string porttmp; 
	if (StrSplit(m_Config.m_Proxy, proxy.hostname, porttmp))
	{
		proxy.port = StrToInt(porttmp.c_str()); 
		proxy.type = libtorrent::proxy_settings::http; 
		StrSplit(m_Config.m_ProxyA, proxy.username, proxy.password);
		m_TorrentSession->set_tracker_proxy(proxy); 
		m_TorrentSession->set_web_seed_proxy(proxy); 
	}
#else
	std::string porttmp; 
	if (StrSplit(m_Config.m_Proxy, sset.proxy_ip, porttmp))
	{
		sset.proxy_port = StrToInt(porttmp.c_str()); 
		StrSplit(m_Config.m_ProxyA, sset.proxy_login, sset.proxy_password);
	}
#endif
	
	sset.urlseed_pipeline_size = 20; 
	sset.allow_multiple_connections_per_ip = false; 
	sset.use_dht_as_fallback = false; 
	

	m_TorrentSession->add_extension(&create_metadata_plugin);
	m_TorrentSession->add_extension(&create_ut_pex_plugin);
	m_TorrentSession->set_max_half_open_connections(-1);

	m_TorrentSession->set_settings(sset); 
	StartDHT();
	return TRUE; 
}

BOOL FClipDownloaderEx::Init(FClipDownloadConfig& aConfig)
{
	ATLASSERT(m_pHttpDownloader != NULL); 
	ATLASSERT(m_pNotify != NULL);
	ATLASSERT(m_TorrentSession == NULL); 

	boost::filesystem::path::default_name_check(boost::filesystem::native); 
	m_TorrentSession = new libtorrent::session;
	
	m_TorrentSession->start_upnp(); 
	m_TorrentSession->start_natpmp(); 
	m_TorrentSession->start_lsd();

	m_Config = aConfig; 

	if (!LoadConfig())
	{
		delete m_TorrentSession; 
		m_TorrentSession = NULL; 
		return FALSE; 
	}

	m_TorrentSession->set_severity_level(alert::info);

	if (!m_AlertThread.Create(this, false, 1))
		return FALSE; 

	if (!m_QueueThread.Create(this))
		return FALSE; 
	
	m_HttpNotify.m_HttpAlerts.SetSize(128);

	m_TorrentSession->listen_on(std::make_pair(m_Config.m_LPortMin, m_Config.m_LPortMax)); 
	return TRUE; 
}

BOOL FClipDownloaderEx::Stop()
{
	_LogMessage("ClipDownloaderEx:: Stop()\n");
	m_HttpNotify.m_HttpAlerts.Clear(); 

	m_AlertThread.StopAndWait(INFINITE);

	if (m_TorrentSession == NULL)
		return FALSE; 

	{
		SynchronizeThread(m_Sync); 
		//Clear the queues
		m_CQueue.clear();
		m_IQueue.clear(); 

		POSITION myPos = m_Downloads.GetStartPosition(); 
		POSITION oldPos = myPos; 
		while(myPos != NULL)
		{
			oldPos = myPos; 
			FPair* p = m_Downloads.GetNext(myPos);
			FDownloadPair* pDownload = p->m_value; 
			RemoveDownload(pDownload->m_Handle);
		}
		m_Downloads.RemoveAll();
	}
	
	m_QueueThread.StopThread(); 

	m_pHttpDownloader = NULL;
	m_pNotify = NULL; 

	SynchronizeThread(m_Sync); 

	std::vector<libtorrent::torrent_handle> handles = m_TorrentSession->get_torrents();

	if (handles.size() > 0)
	{
		_LogMessage("WARNING: Torrent session still contains torrents\n");
		for (size_t k = 0; k < handles.size(); k++)
			StopTorrent(handles[k]); 
	}
	
	m_TorrentSession->stop_dht();
	delete m_TorrentSession;
	m_TorrentSession = NULL; 
	_LogMessage("ClipDownloaderEx:: Stop() finished\n");
	return TRUE; 
}

BOOL FClipDownloaderEx::QueueDownload(FDownloadPair* pPair)
{
	SynchronizeThread(m_Sync); 

	return FALSE; 
}

BOOL FClipDownloaderEx::StopTorrent(libtorrent::torrent_handle handle)
{
	try{
		handle.pause();
		WriteResume(handle); 
		m_TorrentSession->remove_torrent(handle);
		for (;;)
		{
			if (!handle.is_valid())
				break; 
			if (WAIT_IO_COMPLETION == SleepEx(100, TRUE))
				break; 		
		}
	} catch(std::exception& e)
	{
		_LogMessage("StopTorrent exception: %s\n", e.what()); 
		return FALSE; 
	}
	return TRUE; 
}

BOOL FClipDownloaderEx::RemoveQueueHandle(std::deque<FDownloadHandle>& queue, FDownloadHandle handle)
{
	for (std::deque<FDownloadHandle>::iterator i = queue.begin(); i != queue.end(); i++)
	{
		if (*i == handle)
		{
			queue.erase(i); 
			return TRUE; 
		}
	}
	return FALSE; 
}

BOOL FClipDownloaderEx::RemoveDownload(FDownloadHandle handle)
{
	SynchronizeThread(m_Sync); 

	RemoveQueueHandle(m_CQueue, handle);
	RemoveQueueHandle(m_IQueue, handle);
		
	

	FPair* pPair = m_Downloads.Lookup(handle);
	
	if (NULL != pPair)
	{
		FDownloadPair* pDownload = pPair->m_value; 

		pDownload->m_bRemoved = TRUE; 
		if (pDownload->m_pInfo.m_DownloadType == Torrent)
		{
			if (pDownload->m_torrentHandle.is_valid())
			{
				ATLASSERT(pDownload->m_DownloadStatus != EMetaData);
				StopTorrent(pDownload->m_torrentHandle);
			}
			else
			{
				m_pHttpDownloader->StopDownload(pDownload->m_httpHandle); 
			}
			SendAlert(pDownload, ALERT_ITEM_REMOVED, E_ABORT); 
		}
		else
		{
			m_pHttpDownloader->StopDownload(pDownload->m_httpHandle); 
			SendAlert(pDownload, ALERT_ITEM_REMOVED, E_ABORT); 
		}
		m_Downloads.RemoveKey(handle); 
		delete pDownload;
	}
	return TRUE; 
}

FDownloadHandle FClipDownloaderEx::AddDownload(FClipDownloadInfo& pInfo)
{
	SynchronizeThread(m_Sync); 

	FDownloadHandle handle = ++m_CurHandle;
	FDownloadPair* pItem = new FDownloadPair(handle, pInfo); 

	m_Downloads.SetAt(handle, pItem);

	if (pItem->m_pInfo.m_Href.GetLength() == 0)
	{
		pInfo.m_bMetadataComplete = TRUE; 
		OnMetadataComplete(pItem, S_OK, FALSE); 
		return handle; 
	}

	FString &Href = pItem->m_pInfo.m_Href; 
	FString &DataFile = pItem->m_pInfo.m_DataFile;
	fsize_type hrefSize = pItem->m_pInfo.m_HrefSize; 
	if (PathIsURL(Href))
	{
		if (DataFile == "")
		{
			ATLASSERT(FALSE); 
			return 0; 
		}

		if (hrefSize > 0 && GetFileSize(DataFile) == hrefSize)
		{
			pInfo.m_bMetadataComplete = TRUE; 
			OnMetadataComplete(pItem, S_OK, FALSE); 
			return handle; 
		}

		ATLASSERT(m_pHttpDownloader != NULL); 

		FDownloadInfo* pInfo = new FDownloadInfo; 
		pInfo->m_DownloadUrl = Href; 
		pInfo->m_DownloadFile = DataFile; 
		pInfo->m_pv = (void*) handle; 
		pItem->m_httpHandle = m_pHttpDownloader->DownloadURL(pInfo, &m_HttpNotify);

		if (pItem->m_httpHandle != 0)
		{
			_LogMessage("QueueDownload: %d: Queued HTTP download: %s\n", pItem->m_pInfo.m_videoID, Href); 
			return handle; 
		}
		else
		{
			delete pInfo; 
		}
	}
	else
	{
		if (PathIsFileSpec(Href) == FALSE)
		{
			if (PathFileExists(Href))
			{
				pItem->m_pInfo.m_DataFile = DataFile; 
				pItem->m_pInfo.m_HrefSize = GetFileSize(DataFile);
				pInfo.m_bMetadataComplete = TRUE; 
				OnMetadataComplete(pItem, S_OK, FALSE); 
				return handle; 
			}
		}
	
	}
	return 0; 
}

void FClipDownloaderEx::OnMetadataComplete(FDownloadPair* pPair, HRESULT hr, BOOL bSendAlert)
{
	if (SUCCEEDED(hr))
	{
		if (pPair->m_pInfo.m_DownloadType == Torrent)
		{
			pPair->m_DownloadStatus = EDownloading; 
			if (pPair->m_pInfo.m_dwFlags & CFLAG_COMPLETE)
			{
				m_CQueue.push_back(pPair->m_Handle);
			}
			else
			{
				m_IQueue.push_back(pPair->m_Handle); 
			}
			if (bSendAlert) SendAlert(pPair, ALERT_ITEM_QUEUED, hr); 
		}
		else
			if (bSendAlert) SendAlert(pPair, ALERT_DOWNLOAD_FINISHED, hr); 
	}
	else
		if (bSendAlert) SendAlert(pPair, ALERT_DOWNLOAD_FINISHED, hr); 
}

void FClipDownloaderEx::OnHttpDownloadComplete(HRESULT hr, FDownloadInfo* pUrlInfo)
{
	SynchronizeThread(m_Sync); 
	FDownloadHandle pHandle = (FDownloadHandle)pUrlInfo->m_pv;
	FPair* p = m_Downloads.Lookup(pHandle); 

	_LogMessage("OnHttpDownloadComplete: hr = %d, handle = %d\n", hr, pHandle); 
	if (p != NULL)
	{
		FDownloadPair* pPair = p->m_value; 
		pPair->m_dwHttpCode = pUrlInfo->m_dwStatusCode; 
		OnMetadataComplete(pPair, hr); 
		if (pPair->m_pInfo.m_DownloadType == Http)
		{
			//Ugly shit
			//notify download manager that we have uncovered the content type of the clip
			
			pPair->m_ContentType = pUrlInfo->m_ContentType; 
			if (!pPair->m_bRemoved && SUCCEEDED(hr))
			{
				SendAlert(pPair, ALERT_CONTENT_TYPE, S_OK); 
			}
		}

		delete pUrlInfo; 
		
		if (pPair->m_bRemoved)
			m_Downloads.RemoveKey(pHandle); 
	}
}

BOOL FClipDownloaderEx::AddTorrent(FDownloadHandle handle)
{
	SynchronizeThread(m_Sync); 
	FPair* pPair = m_Downloads.Lookup(handle);
	if (NULL != pPair)
		return AddTorrent(pPair->m_value); 
	return FALSE; 
}

BOOL FClipDownloaderEx::AddTorrent(FDownloadPair* pPair)
{
	using namespace boost::filesystem;
	ATLASSERT(pPair != NULL); 
	if (NULL == pPair)
		return FALSE; 

	SynchronizeThread(m_Sync); 

	torrent_handle tHandle;
	try
	{
		path file = pPair->m_pInfo.m_DataFile; 
		std::ifstream in_t(file.string().c_str(), std::ios_base::binary);

		if (!in_t.is_open())
		{
			_LogMessage("**FClipDownloader::LoadTorrent: Cannot open file %s\n", file.string().c_str());
			return FALSE;
		}

		in_t.unsetf(std::ios_base::skipws);
		entry e = bdecode(std::istream_iterator<char>(in_t), std::istream_iterator<char>()); 
		in_t.close();

		torrent_info t(e);


		if (t.num_files() > 0)
		{

			if (pPair->m_pInfo.m_UrlSeed1.GetLength() > 0)
				t.add_url_seed(std::string(pPair->m_pInfo.m_UrlSeed1)); 

			if (pPair->m_pInfo.m_UrlSeed2.GetLength() > 0)
				t.add_url_seed(std::string(pPair->m_pInfo.m_UrlSeed2)); 

			if (pPair->m_pInfo.m_TrackerURL.GetLength() > 0)
			{
				t.add_tracker(std::string(pPair->m_pInfo.m_TrackerURL), 0); 
			}

			std::vector<announce_entry> const& tracks = t.trackers(); 

			//Load resume data
			entry resume_data;
			path resume_file = boost::filesystem::change_extension(file, ".resume");
			std::ifstream in(resume_file.string().c_str(), std::ios_base::binary); 
			if (in.is_open())
			{
				in.unsetf(std::ios_base::skipws);
				try{
					resume_data = bdecode(std::istream_iterator<char>(in), std::istream_iterator<char>());
				} catch(std::exception& e){
					_LogMessage("**FClipDownloaderEx::LoadTorrent exception: Cannot load resume file: %s\n", e.what()); 
				}
				in.close();
			}
			else
			{
				_LogMessage("FClipDownloaderEx::LoadTorrent: Cannot load resume file %s\n", resume_file.string().c_str());
			}
			//int blockSize = pPair->m_pInfo.m_UrlSeed1.GetLength() > 0 ? 512 * 1024 : 32 * 1024; 
			boost::filesystem::path StorageDir = pPair->m_pInfo.m_StorageFolder; 
			tHandle = m_TorrentSession->add_torrent(t, StorageDir, resume_data); 

			tHandle.set_max_uploads(-1);
			tHandle.set_ratio(1.1);
			tHandle.set_sequenced_download_threshold(15);
			tHandle.set_max_connections(60);
			

			SendAlert(pPair, ALERT_TORRENT_ADDED, S_OK); 
			if (pPair->m_pInfo.m_dwFlags & CFLAG_PAUSED)
				tHandle.pause();
		}
	}
	catch (std::exception& e)
	{
		_LogMessage("**FClipDownloaderEx::LoadTorrent: Exception: %s\n", e.what()); 
	}

	pPair->m_torrentHandle = tHandle; 
	return tHandle.is_valid(); 
}

BOOL FClipDownloaderEx::WriteResume(const tchar* pszResumeFile, torrent_handle h)
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
		_LogMessage("**FClipDownloader: WriteResume(): %s\n", e.what());
	}
	return FALSE; 
}

BOOL FClipDownloaderEx::WriteResume(FDownloadPair* pPair)
{
	using namespace boost::filesystem; 
	if (pPair->m_torrentHandle.is_valid())
	{
		path file = pPair->m_pInfo.m_DataFile;
		path resume_file = boost::filesystem::change_extension(file, ".resume");

		if (WriteResume(resume_file.string().c_str(), pPair->m_torrentHandle))
		{
			pPair->m_ResumeWritten = time(NULL); 
			return TRUE; 
		}
	}
	return FALSE; 
}

BOOL FClipDownloaderEx::WriteResume(libtorrent::torrent_handle handle)
{
	SynchronizeThread(m_Sync); 
	FDownloadPair* pPair = FindPairByTorrent(handle); 
	if (NULL != pPair)
		return WriteResume(pPair);

	return FALSE; 
}

FDownloadHandle FClipDownloaderEx::FindByTorrent(libtorrent::torrent_handle h)
{
	FDownloadPair* pPair = FindPairByTorrent(h); 
	if (NULL != pPair)
		return pPair->m_Handle; 
	return 0; 
}

FDownloadPair* FClipDownloaderEx::FindPairByTorrent(libtorrent::torrent_handle h)
{
	SynchronizeThread(m_Sync); 
	POSITION myPos = m_Downloads.GetStartPosition(); 
	while(myPos != NULL)
	{
		FPair* pPair = m_Downloads.GetNext(myPos);
		if (pPair->m_value->m_torrentHandle == h)
			return pPair->m_value; 
	}
	return NULL; 
}

void FClipDownloaderEx::RemoveCompletedTorrent(std::vector<torrent_handle>& torrent_array)
{
	SynchronizeThread(m_Sync); 
	for (size_t k = 0; k < torrent_array.size(); k++)
	{
		torrent_handle h = torrent_array[k]; 
		if (h.is_seed())
		{
			FDownloadHandle dhandle = FindByTorrent(h); 
			ATLASSERT(dhandle != 0);
			if (dhandle != 0)
			{
				m_CQueue.push_back(dhandle); 
				m_TorrentSession->remove_torrent(h); 
				return ; 
			}
		}
	}
}

BOOL FClipDownloaderEx::CheckIncompleteQueue()
{
	SynchronizeThread(m_Sync); 
	BOOL bAdded = FALSE; 

	if (!m_IQueue.empty())
	{
		std::vector<torrent_handle> torrent_array = m_TorrentSession->get_torrents(); 
		if (torrent_array.size() >= m_Config.m_MaxTorrents)
		{
			RemoveCompletedTorrent(torrent_array);
		}
		bAdded = AddTorrent(m_IQueue.front());
		m_IQueue.pop_front(); 
	}
	return bAdded;
}

BOOL FClipDownloaderEx::CheckCompleteQueue()
{
	SynchronizeThread(m_Sync); 
	BOOL bAdded = FALSE; 
	if (!m_CQueue.empty())
	{
		std::vector<torrent_handle> torrent_array = m_TorrentSession->get_torrents(); 
		if (torrent_array.size() < m_Config.m_MaxTorrents)
		{
			bAdded = AddTorrent(m_CQueue.front());
			m_CQueue.pop_front();
		}
		else
		{
			FDownloadHandle aHandle = m_CQueue.front();
			FPair* pNewPairP = m_Downloads.Lookup(aHandle);
			FDownloadPair* pNewPair = NULL; 
			if (pNewPairP)
			{
				pNewPair = pNewPairP->m_value; 
				time_t now = time(NULL); 
				for (size_t k = 0; k < torrent_array.size(); k++)
				{
					torrent_handle& h = torrent_array[k];
					if (h.is_seed())
					{
						FDownloadPair* pPair = FindPairByTorrent(h);
						if (pPair != NULL)
						{
							DWORD SeedTime = m_Config.m_SeedTime;

							if (pPair->m_pInfo.m_TimePublished > 0 && now - pPair->m_pInfo.m_TimePublished < m_Config.m_InitialSeedTime)
								SeedTime = max(m_Config.m_InitialSeedTime, m_Config.m_SeedTime); 

							if (pPair->m_SeedStarted > 0 && now - pPair->m_SeedStarted > SeedTime)
							{
								libtorrent::torrent_status tstatus = h.status(); 
								try
								{
									_LogMessage("Swapping torrents: %d (seeded) with %d (new)\n", pPair->m_Handle, m_CQueue.front());

									//Remove this torrent and add the new one
									m_TorrentSession->remove_torrent(h);
									pPair->m_SeedStarted = 0; 
									pNewPair->m_SeedStarted = 0; 

									bAdded = AddTorrent(aHandle); 
									m_CQueue.pop_front();
									m_CQueue.push_back(aHandle);
								} catch(...)
								{

								}
							}
						}
					}
				}
			}


			/*
				TODO: Find a suitable seeding torrent to remove:
				ratio is 1.0+
				seed time is > min seed time
			*/
		}
	}

	return bAdded; 
}

void FClipDownloaderEx::FQueueThread::Thread(void* p)
{
	FClipDownloaderEx* pThis = (FClipDownloaderEx*)p; 

	DWORD dwStartTime = GetTickCount(); 
	for (;;)
	{
		if (SleepEx(2000, TRUE) == WAIT_IO_COMPLETION)
			break; 
		else
		{
			if (pThis->CheckIncompleteQueue())
			{
				if (SleepEx(3000, TRUE) == WAIT_IO_COMPLETION)
					break; 
			}
			//A small delay before adding completed torrents
			if (GetTickCount() - dwStartTime > 30 * 1000)
			{
				if (pThis->CheckCompleteQueue())
				{
					if (SleepEx(3000, TRUE) == WAIT_IO_COMPLETION)
						break; 
				}
			}
		}
	}
}

BOOL FClipDownloaderEx::SetNotifySink(IClipDownloadNotify *pNotify)
{
	m_pNotify = pNotify; 
	return TRUE; 
}

BOOL FClipDownloaderEx::SetHttpDownloader(IHttpDownloader* pDownloader)
{
	m_pHttpDownloader = pDownloader; 
	return TRUE; 
}

void FClipDownloaderEx::SendAlert(libtorrent::torrent_handle h, DWORD dwCode, HRESULT hr)
{
	SynchronizeThread(m_Sync);
	FDownloadPair* pPair = FindPairByTorrent(h); 
	if (NULL != pPair)
		SendAlert(pPair, dwCode, hr); 
}

void FClipDownloaderEx::SendAlert(FDownloadPair* pPair, DWORD dwCode, HRESULT hr)
{
	ATLASSERT(m_pNotify != NULL); 
	FDownloadAlert alert; 
	alert.dwCode = dwCode; 
	alert.hr = hr; 
	if (NULL != pPair)
	{
		alert.m_clipIndex = pPair->m_pInfo.m_clipIndex; 
		alert.m_videoID = pPair->m_pInfo.m_videoID; 
		alert.m_Param1 = (void*)pPair->m_Handle;
		if (alert.dwCode == ALERT_CONTENT_TYPE)
		{
			char* pBuffer = new char[pPair->m_ContentType.GetLength() + 1];
			strcpy(pBuffer, pPair->m_ContentType); 
			alert.m_Param2 = pBuffer; 
		}
	}
	m_pNotify->Notify(alert); 
}

BOOL FClipDownloaderEx::PauseResumeDownload(FDownloadPair* pDownload, BOOL bResume)
{
	//Clear paused flag in case the download is queued.
	if (bResume)
	{
		pDownload->m_pInfo.m_dwFlags&=~CFLAG_PAUSED;
	}
	else
	{
		pDownload->m_pInfo.m_dwFlags|=CFLAG_PAUSED;
	}

	if (pDownload->m_pInfo.m_DownloadType == Torrent)
	{
		if (pDownload->m_torrentHandle.is_valid())
		{
			if (bResume)
			{
				pDownload->m_torrentHandle.resume(); 
			}
			else 
			{
				pDownload->m_torrentHandle.pause();
			}
		}
		else
		{
			if (bResume) 
			{
				m_pHttpDownloader->Resume(pDownload->m_httpHandle); 
			}
			else 
			{
				m_pHttpDownloader->Pause(pDownload->m_httpHandle); 
			}
		}
	}
	else
	{
		if (bResume) 
		{
			m_pHttpDownloader->Resume(pDownload->m_httpHandle); 
		}
		else 
		{
			m_pHttpDownloader->Pause(pDownload->m_httpHandle); 
		}
	}
	return TRUE; 
}

BOOL FClipDownloaderEx::PauseResume(FDownloadHandle handle, BOOL bResume)
{
	SynchronizeThread(m_Sync); 
	FPair* p = m_Downloads.Lookup(handle); 
	if (p != NULL)
	{
		FDownloadPair* pDownload = p->m_value; 
		return PauseResumeDownload(pDownload, bResume); 
	}
	return FALSE; 
}

BOOL FClipDownloaderEx::PauseResumeAll(BOOL bResume)
{
	SynchronizeThread(m_Sync); 
	POSITION myPos = m_Downloads.GetStartPosition(); 
	while(myPos != NULL)
	{
		FPair* pPair = m_Downloads.GetNext(myPos);
		PauseResumeDownload(pPair->m_value, bResume);
	}
	return TRUE; 
}

BOOL FClipDownloaderEx::PauseAll()
{
	return PauseResumeAll(FALSE); 
}

BOOL FClipDownloaderEx::ResumeAll()
{
	return PauseResumeAll(TRUE); 
}

void FClipDownloaderEx::OnTorrentFinished(libtorrent::torrent_handle& theHandle)
{
	SynchronizeThread(m_Sync); 
	FDownloadPair* pPair = FindPairByTorrent(theHandle); 
	if (pPair)
	{
		pPair->m_SeedStarted = time(NULL); 
	}
	else
	{
		
	}
}

//////////////////////////////////////////////////////////////////////////

void FClipDownloaderEx::ProcessAlerts()
{
	std::auto_ptr<alert> Alert;
	Alert = m_TorrentSession->pop_alert();

	while (Alert.get())
	{
		if (torrent_finished_alert* p = dynamic_cast<torrent_finished_alert*>(Alert.get()))
		{
			if (g_LogTrackerMessages) _LogMessage("FClipDownloaderEx: torrent-finished-alert: %s\n", Alert->msg().c_str()); 
			OnTorrentFinished(p->handle); 
			SendAlert(p->handle, ALERT_DOWNLOAD_FINISHED, S_OK); 
			WriteResume(p->handle); 
			

		}
		else if (tracker_alert* p = dynamic_cast<tracker_alert*>(Alert.get()))
		{
			if (g_LogTrackerMessages) _LogMessage("FClipDownloader: tracker-alert: status_code=%d; msg=%s; \n", p->status_code, Alert->msg().c_str());
		}
		else if (tracker_reply_alert* p = dynamic_cast<tracker_reply_alert*>(Alert.get()))
		{
			try
			{
				torrent_status ts = p->handle.status();
				if (g_LogTrackerMessages) _LogMessage("FClipDownloader: tracker-reply: %s; seeds=%d; peers=%d; complete sources=%d; incomplete=%d; announce-interval: %d;\n", 
					p->msg().c_str(), ts.num_seeds, ts.num_peers, ts.num_complete, ts.num_incomplete, ts.announce_interval / 1000000 ); 
			}
			catch(...){}
		}
		else if (peer_error_alert* p = dynamic_cast<peer_error_alert*>(Alert.get()))
		{
			_LogMessage("FClipDownloader: peer-error: %s\n", p->msg().c_str());
		}
		else if (tracker_announce_alert* p = dynamic_cast<tracker_announce_alert*>(Alert.get()))
		{
			if (p->msg().find("event=stopped") != std::string::npos)
			{
				SendAlert(p->handle, ALERT_ITEM_REMOVED, S_OK);  
			}

			if (p->msg().find("event") != std::string.npos)
			{
				if (g_LogTrackerMessages) _LogMessage("FClipDownloader: tracker-announce: %s\n", p->msg().c_str()); 
			}

		}
		else if (file_error_alert* p = dynamic_cast<file_error_alert*>(Alert.get()))
		{
			_LogMessage("FClipDownloader: file-error-alert: %s\n", p->msg().c_str());
			SendAlert(p->handle, ALERT_FILE_ERROR, E_FAIL); 
		}
		else if (listen_failed_alert* p = dynamic_cast<listen_failed_alert*>(Alert.get()))
		{
			_LogMessage("FClipDownloader: listen-failed: %s\n", p->msg().c_str()); 
			SendAlert(NULL, ALERT_LISTEN_FAILED, E_FAIL); 
		}
		else if (fastresume_rejected_alert* p = dynamic_cast<fastresume_rejected_alert*>(Alert.get()))
		{
			_LogMessage("FClipDownloader: fast resume-rejected: %s\n", p->msg().c_str()); 
		}
		else if (invalid_request_alert* p = dynamic_cast<invalid_request_alert*>(Alert.get()))
		{
			_LogMessage("FClipDownloader: invalid-request: %s\n", p->msg().c_str());
		}
		else if (tracker_warning_alert* p = dynamic_cast<tracker_warning_alert*>(Alert.get()))
		{
			_LogMessage("FClipDownloader: tracker-warning: %s\n", p->msg().c_str());
		}
		else if (url_seed_alert* p = dynamic_cast<url_seed_alert*>(Alert.get()))
		{
			_LogMessage("FClipDownloader: url-seed: %s\n", p->msg().c_str()); 
		}
		else if (portmap_alert* p = dynamic_cast<portmap_alert*>(Alert.get()))
		{
			_LogMessage("Port map: %s\n", p->msg().c_str()); 
			g_pAppManager->OnPortsMapped(TRUE);
		}
		else if (portmap_error_alert* p = dynamic_cast<portmap_error_alert*>(Alert.get()))
		{
			_LogMessage("Port map error: %s\n", p->msg().c_str()); 
			g_pAppManager->OnPortsMapped(TRUE);
		}
		
		/*
		else if (peer_blocked_alert* p = dynamic_cast<peer_blocked_alert*>(Alert.get()))
		{
			_LogMessage("FClipDownloader: peer-blocked-alert: %s\n", p->msg().c_str());
		}
		*/

		Alert = m_TorrentSession->pop_alert();
	}
}

libtorrent::session_status FClipDownloaderEx::GetSessionStatus()
{
	SynchronizeThread(m_Sync); 
	session_status status; 
	if (m_TorrentSession != NULL)
		status = m_TorrentSession->status();
	return status; 

}

torrent_status FClipDownloaderEx::GetTorrentStatus(FDownloadHandle handle)
{
	SynchronizeThread(m_Sync); 
	torrent_status status; 
	FPair* pPair = m_Downloads.Lookup(handle);
	if (NULL != pPair && pPair->m_value->m_torrentHandle.is_valid())
		status = pPair->m_value->m_torrentHandle.status(); 
	return status; 
}

std::vector<libtorrent::torrent_handle> FClipDownloaderEx::GetTorrents()
{
	SynchronizeThread(m_Sync); 
	if (m_TorrentSession != NULL)
		return m_TorrentSession->get_torrents(); 

	return std::vector<libtorrent::torrent_handle>();
}

libtorrent::torrent_handle FClipDownloaderEx::GetTorrent(FDownloadHandle handle)
{
	SynchronizeThread(m_Sync); 
	if (m_TorrentSession != NULL)
	{
		FPair* pPair = m_Downloads.Lookup(handle);
		if (NULL != pPair)
		{
			return pPair->m_value->m_torrentHandle; 
		}
	}
	return libtorrent::torrent_handle(); 
}

int FClipDownloaderEx::GetTotalProgress(CAtlMap<FDownloadHandle, FDownloadProgress> &aMap)
{
	SynchronizeThread(m_Sync); 
	POSITION myPos = m_Downloads.GetStartPosition(); 
	while(myPos != NULL)
	{
		FPair* pPair = m_Downloads.GetNext(myPos);
		FDownloadProgress Progress; 
		GetProgress(pPair->m_value, Progress); 
		aMap.SetAt(pPair->m_key, Progress); 
	}
	return aMap.GetCount();
}

BOOL FClipDownloaderEx::GetSessionProgress(FDownloadProgress& pProgress)
{
	SynchronizeThread(m_Sync);
	POSITION myPos = m_Downloads.GetStartPosition(); 
	while(myPos != NULL)
	{
		FPair* pPair = m_Downloads.GetNext(myPos);
		FDownloadProgress Progress; 
		GetProgress(pPair->m_value, Progress); 
		pProgress+=Progress;
	}
	return TRUE; 
}

BOOL FClipDownloaderEx::GetTotalProgress(vidtype videoID, FDownloadProgress& pProgress)
{
	SynchronizeThread(m_Sync); 
	POSITION myPos = m_Downloads.GetStartPosition(); 
	while(myPos != NULL)
	{
		FPair* pPair = m_Downloads.GetNext(myPos);
		if (pPair->m_value->m_pInfo.m_videoID == videoID)
		{
			GetProgress(pPair->m_value, pProgress); 
		}
	}
	return TRUE;
}

BOOL FClipDownloaderEx::GetProgress(FDownloadPair* pItem, FDownloadProgress& pProgress)
{
	if (pItem->m_pInfo.m_DownloadType == Torrent)
	{
		try{
			if (pItem->m_torrentHandle.is_valid())
			{
				torrent_status aStatus = pItem->m_torrentHandle.status();
				pProgress.m_bytesPerSec += aStatus.download_rate;
				pProgress.m_percentComplete += aStatus.progress;
				pProgress.m_upBytesPerSec += aStatus.upload_rate;
				pProgress.m_bytesDownloaded += aStatus.total_done; 
				pProgress.m_bytesUploaded += aStatus.total_upload; 
				pProgress.m_bytesTotal += aStatus.total_wanted; 
				pProgress.m_Seeds += aStatus.num_seeds;
				pProgress.m_Peers += aStatus.num_peers; 
			}
			return TRUE; 
		}
		catch(std::exception &){}

	}
	else
	{
		if (pItem->m_httpHandle != 0)
		{
			return m_pHttpDownloader->GetDownloadProgress(pItem->m_httpHandle, pProgress);
		}
	}
	return FALSE; 
}

BOOL FClipDownloaderEx::GetProgress(FDownloadHandle handle, FDownloadProgress& pProgress)
{
	SynchronizeThread(m_Sync); 
	FPair* pPair = m_Downloads.Lookup(handle);
	if (NULL != pPair)
	{
		return GetProgress(pPair->m_value, pProgress); 
	}

	return FALSE; 
}

BOOL FClipDownloaderEx::GetSessionInfo(FSessionInfo* pInfo)
{
	SynchronizeThread(m_Sync); 

	if (m_TorrentSession != NULL)
	{
		pInfo->m_IsListening = m_TorrentSession->is_listening();
		pInfo->m_ListenPort = m_TorrentSession->listen_port();
		return TRUE;
	}
	return FALSE; 
}

//////////////////////////////////////////////////////////////////////////



void FClipDownloaderEx::FAlertThread::Thread(void* p)
{
	FClipDownloaderEx* pThis = (FClipDownloaderEx*)p;

	for (;;)
	{
		bool bPopped = false; 
		FDownloadAlert alert; 
		int nRes = pThis->m_HttpNotify.m_HttpAlerts.PopWait(alert, bPopped, 500); 
		if (nRes == WAIT_OBJECT_0)
		{
			if (bPopped)
				pThis->OnHttpDownloadComplete(alert.hr, (FDownloadInfo*)alert.m_Param1);
		}
		else
		if (nRes == WAIT_TIMEOUT)
		{
			pThis->ProcessAlerts();
		}
		else
			break; 
	}
}

//////////////////////////////////////////////////////////////////////////
BOOL FClipDownloaderEx::HashFile(FHashFile& pHashFile)
{
	BOOL bSuccess = HashFileEx(pHashFile); 
	if (pHashFile.m_pProgress)
	{
		pHashFile.m_pProgress->OnHashFinished(bSuccess, pHashFile.m_TorrentFile); 
	}
	return bSuccess; 
}

BOOL FClipDownloaderEx::HashFileEx(FHashFile& pHashFile)
{
	try
	{
		const char* pszFileName = pHashFile.m_FileName.c_str(); 
		const tchar* pszTorrentFile = pHashFile.m_TorrentFile.c_str(); 
		std::vector<std::string>& trackers = pHashFile.m_Trackers;
		std::vector<std::string>& urlSeeds = pHashFile.m_UrlSeeds;

		boost::filesystem::path::default_name_check(boost::filesystem::native); 

		torrent_info t; 
		boost::filesystem::path filePath(pszFileName); 
		boost::filesystem::ifstream input(filePath, std::ios_base::binary);
		boost::filesystem::ofstream out(pszTorrentFile, std::ios_base::binary);

		input.unsetf(std::ios_base::skipws); 
		out.unsetf(std::ios_base::skipws);

		

		t.add_file(filePath.leaf(), file_size(filePath));
		const char* creatorStr = LTV_APP_NAME".AutoHasher";
		int piece_size = 256 * 1024; 
		t.set_piece_size(piece_size); 

		for (size_t k = 0; k < trackers.size(); k++)
		{
			t.add_tracker(trackers[k]);
		}


		int num = t.num_pieces();
		std::vector<char> buf(piece_size); 

		for (int i = 0; i < num; ++i)
		{
			input.read(&buf[0], (std::streamsize)t.piece_size(i));
			libtorrent::hasher h(&buf[0], (int)t.piece_size(i));
			t.set_hash(i, h.final()); 
			if (pHashFile.m_pProgress)
			{
				if (pHashFile.m_pProgress->OnHashProgress(pHashFile.m_FileName, i, num) == FALSE)
					return FALSE; 
			}
		}

		t.set_creator(creatorStr); 

		for (size_t k = 0; k < urlSeeds.size(); k++)
		{
			t.add_url_seed(urlSeeds[k]);
		}

		entry e = t.create_torrent();
		libtorrent::bencode(std::ostream_iterator<char>(out), e);

	}
	catch (std::exception& e)
	{
		_DBGAlert("Hash error: %s\n", e.what());
		return FALSE; 
	}
	return TRUE; 
}