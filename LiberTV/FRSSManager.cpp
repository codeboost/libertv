#include "stdafx.h"
#include "FRSSManager.h"
#include "AppSettings.h"
#include "GlobalObjects.h"
#include "FAsyncHttpDownload.h"
/*
RSS Manager contains an array of FRSSFeed objects.

FRSSFeed::LoadRSS() loads the actual RSS data; available in FRSSFeed::m_pChannel.

Autodownload
- when user clicks sets 'auto download' to true - all items are marked as 'old' - it's GUID (crc32) is added to the recent guids map.
- When a channel is updated, 
   if item doesn't exist in 'recent guids', we consider it new
   if collection permits (less than max-downloads-per-channel), we start the download
   if not - we save it to recent guids (keeping the NEW flag)
*/

FRSSManager::FRSSManager()
{
	m_pNotify = NULL; 
}

FRSSManager::~FRSSManager()
{
	Clear(); 
}

void FRSSManager::Clear(){
	SynchronizeThread(m_Sync); 
	m_UpdateQueue.clear();
	m_InProgress.clear(); 

	for (size_t k = 0; k < m_Channels.GetCount(); k++)
	{
		delete m_Channels[k];
	}
	m_Channels.RemoveAll(); 
	RSSIndex.Clear();
}

void FRSSManager::ClearChannels()
{
	SynchronizeThread(m_Sync); 
	for (size_t k = 0; k < m_Channels.GetCount(); k++)
	{
		m_Channels[k]->ReleaseChannel();
	}
}

void FRSSManager::SetNotify(IRSSInterface* pNotify)
{
	SynchronizeThread(m_Sync);
	m_pNotify = pNotify; 
}

/*
RSS Feeds are stored in rss.ini, which is located in the storage folder.
Each feed item contains: 
Feed name, 
Feed URL, 
XML file name, 
Flags
*/

BOOL FRSSManager::LoadIndex(const tchar* pszIndexDir)
{
	if (g_AppSettings.m_dwDebugFlags & START_FLAG_NO_LOAD_FEEDS)
		return FALSE; 

	SynchronizeThread(m_Sync); 
	Clear(); 
	m_IndexDir = pszIndexDir; 
	FString IndexPath = _PathCombine(pszIndexDir, "rss.ini"); 
	FString RecentItems = _PathCombine(pszIndexDir, "rss.uid"); 

	m_RecentItems.Load(RecentItems); 

	if (!RSSIndex.Load(IndexPath))
		return FALSE; 

	CAtlArray<FIniConfig::IniSection>& aSections = RSSIndex.GetSections(); 

	
	//size_t Count = 2; //aSections.GetCount() / 8;
	size_t Count = aSections.GetCount(); 
	for (size_t k = 0; k < Count; k++)
	{
		FRSSFeed* pFeed = new FRSSFeed(&m_GuidMap, &m_RecentItems); 
		if (pFeed->Load(aSections[k]))
		{
			m_Channels.Add(pFeed); 
		}
		else
			delete pFeed; 
	}
	return TRUE; 
}

BOOL FRSSManager::SaveIndex()
{
	SynchronizeThread(m_Sync); 
	
	for (size_t k = 0; k < m_Channels.GetCount(); k++)
	{
		FRSSFeed* pFeed = m_Channels[k];

		if (!RSSIndex.SectionExists(pFeed->m_SectionName))
			RSSIndex.AddSection(pFeed->m_SectionName); 
		pFeed->Save(RSSIndex); 
	}
	m_RecentItems.Save();
	return RSSIndex.Save();
}

BOOL FRSSManager::ChannelExists(const tchar* pszName)
{
	SynchronizeThread(m_Sync);
	for (size_t k = 0; k < m_Channels.GetCount(); k++)
	{
		if (m_Channels[k]->m_ChannelName.CompareNoCase(pszName) == 0)
			return TRUE; 
	}
	return FALSE;
}

FRSSFeed* FRSSManager::FindFeedById(DWORD dwChannelID)
{
	for (size_t k = 0; k < m_Channels.GetCount(); k++)
	{
		if (m_Channels[k]->m_dwChannelID == dwChannelID)
		{
			return m_Channels[k];
		}
	}
	return NULL;
}

FRSSFeed* FRSSManager::FindFeedByURL(const tchar* pszURL)
{
	for (size_t k = 0; k < m_Channels.GetCount(); k++)
	{
		if (m_Channels[k]->m_URL == pszURL)
			return m_Channels[k];
	}
	return NULL; 
}

FRSSItem* FRSSManager::FindByGuid(GuidType dwGuid, FRSSFeed** ppFeed)
{
	SynchronizeThread(m_Sync); 
	for (size_t k = 0; k < m_Channels.GetCount(); k++)
	{
		FRSSItem* pItem = m_Channels[k]->FindItemByGuid(dwGuid); 
		if (pItem)
		{
			if (ppFeed != NULL)
				*ppFeed = m_Channels[k];
			return pItem; 
		}
	}
	return NULL; 
}

DWORD FRSSManager::ChannelIdFromGuid(GuidType dwGuid)
{
	SynchronizeThread(m_Sync); 
	FRSSFeed* pFeed = NULL; 
	if (FindByGuid(dwGuid, &pFeed) != NULL)
	{
		return pFeed->m_dwChannelID; 
	}
	return 0; 
}

void FRSSManager::UpdateChannelVideoId(DWORD dwChannelID)
{
	SynchronizeThread(m_Sync); 
	FRSSFeed* pFeed = FindFeedById(dwChannelID); 
	if (pFeed)
		pFeed->UpdateVideoIds();

}

void FRSSManager::UpdateItemVideoId(GuidType dwGuid, DWORD dwVideoID)
{
	m_GuidMap.SetGuid(dwGuid, GuidInfo(dwVideoID)); 
	SynchronizeThread(m_Sync); 
	for (size_t k = 0; k < m_Channels.GetCount(); k++)
		m_Channels[k]->UpdateVideoIds(); 
}


DWORD FRSSManager::AddFeed(AddFeedParams& pParams)
{
	SynchronizeThread(m_Sync); 

	const char* pszURL = pParams.m_URL; 
	const char* pszName = pParams.m_Name; 
	DWORD dwFlags = pParams.m_dwFlags; 

	FRSSFeed* pFeed = FindFeedByURL(pszURL);
	if (pFeed)
	{
		if (strlen(pszName) > 0)
		{
			pFeed->m_ChannelName = pszName; 
			SaveIndex();
		}
		return pFeed->m_dwChannelID;
	}
	FRSSFeed* aFeed = new FRSSFeed (&m_GuidMap, &m_RecentItems); 
	aFeed->m_URL = pszURL; 
	aFeed->m_dwChannelID = RSSGuidFromString(pszURL);
	aFeed->m_SectionName = pszURL; 
	aFeed->m_ChannelName = pszName; 
	aFeed->m_FileName.Format("%08u.rss", RSSGuidFromString(aFeed->m_URL)); 
	aFeed->m_DefImage = pParams.m_DefImage; 
	aFeed->m_DetailURL = pParams.m_DetailsURL;

	if (aFeed->m_ChannelName.GetLength() == 0)
	{
		aFeed->m_dwFlags|=FEED_FLAG_SET_NAME; 
		aFeed->m_ChannelName = pszURL; 
	}
	m_Channels.Add(aFeed); 

	if (SaveIndex())
	{
		UpdateFeed(aFeed); 
		g_AppSettings.m_dwLastChannelId = aFeed->m_dwChannelID; 
		return aFeed->m_dwChannelID; 
	}
	return 0; 

}

DWORD FRSSManager::AddFeed(const tchar* pszURL, const tchar* pszName, DWORD dwFlags)
{
	AddFeedParams params; 
	params.m_URL = pszURL; 
	params.m_dwFlags = dwFlags; 
	params.m_Name = pszName; 
	return AddFeed(params);
}

void FRSSManager::RemoveFromQueue(DWORD dwChannelID)
{
	SynchronizeThread(m_Sync); 
	for (size_t j = 0; j < m_UpdateQueue.size(); j++)
	{
		if (m_UpdateQueue[j] == dwChannelID)
		{
			m_UpdateQueue.erase(m_UpdateQueue.begin()+ j);
			break; 
		}
	}
}

void FRSSManager::RemoveChannel(DWORD dwChannelID)
{
	SynchronizeThread(m_Sync);
	for (size_t k = 0; k < m_Channels.GetCount(); k++)
	{
		if (m_Channels[k]->m_dwChannelID == dwChannelID)
		{
			FRSSFeed* pFeed = m_Channels[k]; 
			pFeed->RemoveCachedImages(); 
			DeleteFile(_PathCombine(m_IndexDir, pFeed->m_FileName)); 
			RSSIndex.RemoveSection(pFeed->m_SectionName); 
			m_Channels.RemoveAt(k, 1); 
			RemoveFromQueue(dwChannelID); 
			delete pFeed; 
			SaveIndex();

			if (g_AppSettings.m_dwLastChannelId == dwChannelID)
				g_AppSettings.m_dwLastChannelId = 0; 
			break; 
		}
	}
}

BOOL FRSSManager::RemoveChannel(const char* pszURL)
{
	SynchronizeThread(m_Sync); 
	FRSSFeed* pFeed = FindFeedByURL(pszURL); 
	if (pFeed)
	{
		RemoveChannel(pFeed->m_dwChannelID);
		pFeed = NULL; 
		return TRUE; 
	}
	return FALSE;
}

BOOL FRSSManager::RefreshChannel(DWORD dwChannelID)
{
	SynchronizeThread(m_Sync); 
	FRSSFeed* pFeed = FindFeedById(dwChannelID);
	if (pFeed)
	{
		return UpdateFeed(pFeed);
	}
	return FALSE; 
}

/*
Feed update:
First we check if feed is fresh or not (now - last update > feed.ttl)
If feed must be updated, it's ID is added to the m_UpdateQueue

The RSS file is downloaded to a temporary file (feedId_rss.tmp)

*/
BOOL FRSSManager::UpdateFeed(FRSSFeed* pFeed)
{
	SynchronizeThread(m_Sync); 
	if (pFeed->m_FileName.GetLength() == 0)
		pFeed->m_FileName.Format("%s.rss", StringToFileName(pFeed->m_ChannelName)); 

	FDownloadInfo *pUrlInfo = new FDownloadInfo; 

	FString DownloadFile; 
	DownloadFile.Format("%08u_rss.tmp", pFeed->m_dwChannelID);

	pUrlInfo->m_DownloadFile = _PathCombine(m_IndexDir, DownloadFile);
	pUrlInfo->m_DownloadUrl  = pFeed->m_URL; 
	pUrlInfo->m_pv = (void*)pUrlInfo; 
	pUrlInfo->m_dwDownloadFlags|=HTTP_FLAG_NO_RESUME; 
	m_InProgress.push_back(pFeed->m_dwChannelID);

	if (m_pNotify)
		m_pNotify->OnChannelRefresh(pFeed->m_dwChannelID, FEED_REFRESH_STARTED, pFeed->m_NewItems);

	
	pFeed->m_DownloadHandle = g_Objects._HttpDownloader->DownloadURL(pUrlInfo, this); 
	
	if (pFeed->m_DownloadHandle == 0)
	{
		_DBGAlert("RSS manager: Feed refresh failed\n");
		m_InProgress.remove(pFeed->m_dwChannelID); 
		m_pNotify->OnChannelRefresh(pFeed->m_dwChannelID, FEED_REFRESH_ENDED, pFeed->m_NewItems); 
	}
	_DBGAlert("RSS manager: Feed refresh started\n");
	return pFeed->m_DownloadHandle != 0; 
}

void FRSSManager::Notify(FDownloadAlert& pAlert)
{
	DWORD dwChannelID = 0; 

	_DBGAlert("RSS manager: Notfiy: %x\n", pAlert.hr);
	
	FDownloadInfo* pDownInfo = (FDownloadInfo*)pAlert.m_Param1;
	if (pAlert.dwCode == ALERT_HTTP_DOWNLOAD_FINISHED)
	{
		SynchronizeThread(m_Sync); 
		for (size_t k = 0; k < m_Channels.GetCount(); k++)
		{
			FRSSFeed* pFeed = m_Channels[k];
			if (pAlert.m_DownloadHandle == pFeed->m_DownloadHandle)
			{
				dwChannelID = pFeed->m_dwChannelID; 

				if (SUCCEEDED(pAlert.hr))
				{
					if (pFeed->LoadRSS(TRUE, pDownInfo->m_DownloadFile))
					{
						FString NewFile = _PathCombine(g_AppSettings.m_IndexPath, pFeed->m_FileName);
						MoveFileEx(pDownInfo->m_DownloadFile, NewFile, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
					}
				}
				DeleteFile(pDownInfo->m_DownloadFile); 

				pFeed->m_dwFlags|=FEED_FLAG_REFRESHED;
				if (m_pNotify)
					m_pNotify->OnChannelRefresh(dwChannelID, FEED_REFRESH_ENDED, pFeed->m_NewItems);

				pFeed->m_LastUpdate = (DWORD)time(NULL); 
				m_InProgress.remove(dwChannelID); 
				
				_DBGAlert("Feed updated (0x%x): %s\n",  pAlert.hr, pFeed->m_URL); 
				break; 
			}
		}
		delete (FDownloadInfo*)pAlert.m_Param1;
		UpdateNextFeed();
	}
}

BOOL FRSSManager::DownloadItem(GuidType dwGuid, BOOL bStream)
{
	SynchronizeThread(m_Sync); 

	FRSSFeed* pFeed = NULL; 
	FRSSItem* pItem = FindByGuid(dwGuid, &pFeed); 
	if (pItem)
	{
		return pFeed->DownloadItem(pItem, FALSE, bStream);
	}
	return FALSE; 
}

FRSSFeed* FRSSManager::LockChannel(DWORD dwID)
{
	m_Sync.Lock(); 
	FRSSFeed* pFeed = FindFeedById(dwID);

	if (pFeed && pFeed->m_pChannel == NULL)
		pFeed->LoadRSS(FALSE); 
	return pFeed; 
	
}


const FArray<FRSSFeed*>& FRSSManager::LockChannels()
{
	m_Sync.Lock(); 
	for (size_t k = 0; k < m_Channels.GetCount(); k++)
	{
		if (IsInProgress(m_Channels[k]->m_dwChannelID))
			continue; 

		if (m_Channels[k]->m_pChannel)	//Already updated
			continue; 

		m_Channels[k]->LoadRSS(FALSE); 
	}
	return m_Channels; 
}

void FRSSManager::ReleaseChannelsLock()
{
	m_Sync.Unlock(); 
}

void FRSSManager::SetChannelAutoDownload(DWORD dwChannelID, BOOL bAutoDownload)
{
	if (bAutoDownload && !m_pNotify->OnSetAutoDownload())
		return ; 

	SynchronizeThread(m_Sync); 
	MarkAsViewed(dwChannelID);

	FRSSFeed* pFeed = FindFeedById(dwChannelID); 
	if (pFeed)
	{
		if (pFeed->m_dwFlags & FEED_FLAG_AUTO_DOWNLOAD)
			pFeed->m_dwFlags&=~FEED_FLAG_AUTO_DOWNLOAD;
		else
			pFeed->m_dwFlags|=FEED_FLAG_AUTO_DOWNLOAD;
		SaveIndex();
	}
}

BOOL FRSSManager::IsInUpdateQueue(DWORD dwChannelID)
{
	SynchronizeThread(m_Sync); 
	for (size_t j = 0; j < m_UpdateQueue.size(); j++){
		if (m_UpdateQueue[j] == dwChannelID)
			return TRUE; 
	}
	return FALSE; 
}

BOOL FRSSManager::IsInProgress(DWORD dwChannelID)
{
	SynchronizeThread(m_Sync); 
	BOOL bFound = FALSE; 
	std::list<DWORD>::iterator iter = m_InProgress.begin();
	while (iter != m_InProgress.end())
	{
		if (*iter == dwChannelID)
			return TRUE; 
		iter++;
	}
	return FALSE; 
}

void FRSSManager::CheckForUpdates(BOOL bForceUpdates){
	SynchronizeThread(m_Sync); 
	time_t now = time(NULL); 
	
	if (bForceUpdates)
		m_UpdateQueue.clear();

	for (size_t k = 0; k < m_Channels.GetCount(); k++)
	{
		FRSSFeed* pFeed = m_Channels[k]; 
		FString FeedFileName = _PathCombine(m_IndexDir, pFeed->m_FileName); 
		if (now - pFeed->m_LastUpdate > pFeed->m_UpdateInterval || bForceUpdates)
		{
			if (!IsInUpdateQueue(pFeed->m_dwChannelID) && !IsInProgress(pFeed->m_dwChannelID))
				m_UpdateQueue.push_back(pFeed->m_dwChannelID); 
		}
	}

	if (!m_UpdateQueue.empty() )
		UpdateNextFeed();

	_RegSetDword("LastFeedUpdate", (DWORD)time(NULL)); 
}

void FRSSManager::UpdateNextFeed()
{
	SynchronizeThread(m_Sync); 
	if (!m_UpdateQueue.empty())
	{
		if (!IsInProgress(m_UpdateQueue.front()))
			RefreshChannel(m_UpdateQueue.front());

		m_UpdateQueue.pop_front(); 
	} 
}

void FRSSManager::MarkAsViewed(DWORD dwChannelID)
{
	SynchronizeThread(m_Sync); 

	if (dwChannelID == 0)
	{
		for (size_t k = 0; k < m_Channels.GetCount(); k++)
			m_Channels[k]->MarkAllAsViewed();
	}
	else
	{
		FRSSFeed* pFeed = FindFeedById(dwChannelID); 	
		if (pFeed)
			pFeed->MarkAllAsViewed();
	}
	SaveIndex(); 
}

BOOL FRSSManager::SetChannelOptions(DWORD dwChannelID, ChannelOptions& pOpt)
{
	SynchronizeThread(m_Sync); 

	FRSSFeed* pFeed = FindFeedById(dwChannelID);
	if (pFeed)
	{
		if (pOpt.m_ChannelName.GetLength() > 0) 
		{
			//if channel name is changed, must clear the FEED_FLAG_SET_NAME flag, so
			//that the name is not reset when the feed is updated.
			pFeed->m_ChannelName = pOpt.m_ChannelName; 
			pFeed->m_dwFlags&=~FEED_FLAG_SET_NAME;
		}
		if (pOpt.m_dwMaxDownloadsPerFeed > 99)
			pOpt.m_dwMaxDownloadsPerFeed = 99;

		pFeed->m_MaxActiveDownloads = pOpt.m_dwMaxDownloadsPerFeed;
		BOOL bMustReload = FALSE; 
		if (pFeed->m_Filter != pOpt.m_Filter)
			bMustReload = TRUE; 

		pFeed->m_Filter = pOpt.m_Filter; 

		if (bMustReload)
		{
			pFeed->LoadRSS(FALSE);
		}
		
		SaveIndex(); 
	}
	return FALSE; 

}

BOOL FRSSManager::FeedExists(DWORD dwID)
{
	SynchronizeThread(m_Sync); 
	return FindFeedById(dwID) != NULL; 
}

FString FRSSManager::GetItemImageURL(DWORD dwChannelId, GuidType itemGuid)
{
	FRSSFeed* pFeed = LockChannel(dwChannelId);
	FString ItemImage; 
	if (pFeed && pFeed->m_pChannel)
	{
		
		FRSSItem* pItem = pFeed->FindItemByGuid(itemGuid);
		if (pItem)
			ItemImage = pItem->m_ItemImage; 
		if (ItemImage.GetLength() == 0)
			ItemImage = pFeed->m_pChannel->m_ChannelImage; 
	}	
	ReleaseChannelsLock();
	return ItemImage; 
}