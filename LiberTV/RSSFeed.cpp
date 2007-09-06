#include "stdafx.h"
#include "RSSFeed.h"
#include "GlobalObjects.h"


FRSSFeed::FRSSFeed(FRSSGuidMap* pGuidMap, FRSSGuidMap* pRecentItems)
{
	m_pVideoMap = pGuidMap;
	m_RecentGuids = pRecentItems; 
	m_dwFlags = 0; 
	m_dwChannelID = 0; 
	m_pChannel = NULL; 
	m_LastUpdate = 0; 
	m_DownloadHandle = 0; 
	m_UpdateInterval = 3600; //every hour
	m_NewItems = 0; 
	m_MaxActiveDownloads = g_AppSettings.m_MaxDownloadsPerFeed; 
}

FRSSFeed::~FRSSFeed()
{
	ReleaseChannel();
}

void FRSSFeed::ReleaseChannel()
{
	if (m_pChannel)
	{
		g_Objects._RSSReader.FreeChannel(m_pChannel); 
		m_pChannel = NULL; 
	}
}

BOOL FRSSFeed::Load(FIniConfig::IniSection& pSection)
{
	m_SectionName = pSection.m_Name; 
	m_URL = pSection.GetValue("URL"); 
	m_dwChannelID = RSSGuidFromString(m_URL); 
	if (m_dwChannelID == 0)
		m_dwChannelID = GetTickCount(); 

	m_FileName = pSection.GetValue("FileName"); 
	m_ChannelName = pSection.GetValue("Name"); 
	m_dwFlags = pSection.GetValueDWORD("Flags"); 
	m_LastUpdate = pSection.GetValueDWORD("LastUpdate"); 
	m_DetailURL = pSection.GetValue("DetailURL"); 
	m_DefImage = pSection.GetValue("DefImage");
	m_Filter.m_Contains = pSection.GetValue("FilterContains"); 
	m_Filter.m_NotContains = pSection.GetValue("FilterNotContains"); 
	if (m_FileName == "")
		m_FileName.Format("%08u.rss", m_dwChannelID); 

	return TRUE; 
}

BOOL FRSSFeed::Save(FIniConfig& pConf)
{
	pConf.ModifyValue(m_SectionName, "URL", m_URL); 
	pConf.ModifyValue(m_SectionName, "Name", m_ChannelName); 
	pConf.ModifyValue(m_SectionName, "FileName", m_FileName); 
	pConf.ModifyValueDWORD(m_SectionName, "Flags", m_dwFlags); 
	pConf.ModifyValueDWORD(m_SectionName, "LastUpdate", m_LastUpdate); 
	pConf.ModifyValue(m_SectionName, "DetailURL", m_DetailURL); 
	pConf.ModifyValue(m_SectionName, "DefImage", m_DefImage);
	pConf.ModifyValue(m_SectionName, "FilterContains", m_Filter.m_Contains); 
	pConf.ModifyValue(m_SectionName, "FilterNotContains", m_Filter.m_NotContains); 
	return TRUE; 
}

void FRSSFeed::UpdateVideoIds()
{
	if (m_pChannel != NULL)		
	{
		for (size_t k = 0; k < m_pChannel->m_Items.GetCount(); k++)
		{
			FRSSItem* pItem = m_pChannel->m_Items[k];
			pItem->m_videoID = m_pVideoMap->FindGuid(pItem->m_Guid).m_dwValue; 
		}
	}
}

void FRSSFeed::RSSItemToDownload(FDownloadEx& aDownload, FRSSItem* pItem, BOOL bStream)
{
	aDownload.m_Detail.m_VideoName		= pItem->m_Title;
	aDownload.m_Detail.m_VideoID		= g_AppSettings.GetRandomVideoID(); 
	aDownload.m_Detail.m_TotalSize		= pItem->m_EnclosureSize == -1 ? 0 : pItem->m_EnclosureSize;
	aDownload.m_Detail.m_DetailURL			= pItem->m_EnclosureURL; //ItemURL
	aDownload.m_Detail.m_TimePublished	= FileTimeToUnixTime((LARGE_INTEGER*)&pItem->m_ftPublished); 
	aDownload.m_Detail.m_ImageURL		= PathIsURL(pItem->m_ItemImage) ? pItem->m_ItemImage : m_pChannel->m_ChannelImage;
	aDownload.m_RSSInfo.m_Guid			= pItem->m_Guid; 
	aDownload.m_RSSInfo.m_RSSName		= m_ChannelName;
	aDownload.m_RSSInfo.m_RSSURL		= m_URL; 
	aDownload.m_RSSInfo.m_dwFlags		= RSS_FLAG_FROM_RSS;
	aDownload.m_RSSInfo.m_ItemURL		= pItem->m_EnclosureURL;

	FDownloadItem* pClip = aDownload.AddNewClip();
	if (pItem->m_EnclosureType.Find("bittorrent") != -1)
		pClip->m_DownloadType = "torrent";
	else{
		BOOL bShift = ( GetKeyState(VK_SHIFT) & 0x8000) != 0;
		if (bShift || bStream)
			pClip->m_DownloadType = "stream";
		else
			pClip->m_DownloadType = "http";
	}
	pClip->m_Href = pItem->m_EnclosureURL; 
	pClip->m_Description = pItem->m_Description; 
}

BOOL FRSSFeed::DownloadItem(FRSSItem* pItem, BOOL IsAutomatic, BOOL bStream)
{

	pItem->m_dwFlags|=FEED_ITEM_DOWNLOADED; 
	BOOL bStarted = FALSE; 

	//Do not auto-subscribe to stuff
	if (pItem->m_EnclosureType.Find("xml") != -1 || pItem->m_EnclosureType.Find("rss") != -1)
		return FALSE; 

	if (pItem->m_EnclosureType.CompareNoCase("video/mtti") == 0)
	{
		FDownloadInfo pInfo;
		pInfo.m_RSSInfo.m_Guid = pItem->m_Guid; 
		pInfo.m_RSSInfo.m_RSSName = m_ChannelName;
		pInfo.m_RSSInfo.m_RSSURL = m_URL; 
		pInfo.m_RSSInfo.m_RSSGuid = m_dwChannelID; 
		pInfo.m_RSSInfo.m_ItemURL = pItem->m_EnclosureURL;
		pInfo.m_RSSInfo.m_dwFlags = RSS_FLAG_FROM_RSS;
		if (IsAutomatic) pInfo.m_RSSInfo.m_dwFlags|=RSS_FLAG_FROM_AUTODOWNLOAD;
		pInfo.m_DownloadUrl = pItem->m_EnclosureURL; 
		bStarted =  g_Objects._DownloadDispatcher.OpenMTTFromHTTP(pInfo); 
	}
	else
	{
		//Create a temporary FDownload (MTTI) object; then load it as if it was downloaded from HTTP
		//I hate the complexity; but it's currently the easiest way
		//TODO: Implement a more elegant method
		FDownloadEx aDownload; 
		RSSItemToDownload(aDownload, pItem, bStream); 
		aDownload.m_RSSInfo.m_RSSGuid = m_dwChannelID;
		aDownload.m_Detail.m_DetailURL = m_DetailURL; 

		if (IsAutomatic)
			aDownload.m_RSSInfo.m_dwFlags|=RSS_FLAG_FROM_AUTODOWNLOAD;
		aDownload.m_Conf.m_FileName = _PathCombine(g_AppSettings.m_IndexPath, FormatString("%d.tmp.mtti", aDownload.m_Detail.m_VideoID));
		aDownload.SaveToDisk();

		FDownloadInfo pInfo;
		pInfo.m_RSSInfo = aDownload.m_RSSInfo; 
		pInfo.m_DownloadUrl = pItem->m_EnclosureURL; 
		pInfo.m_DownloadFile = aDownload.m_Conf.m_FileName; 
		g_Objects._DownloadDispatcher.LoadDownloadAsync(pInfo);
		bStarted = TRUE; 
	}

	if (bStarted)
	{
		if (pItem->m_dwFlags & FEED_ITEM_NEW)
			m_NewItems--; 		

		//set as 'viewed'
		pItem->m_dwFlags &= ~FEED_ITEM_NEW;
		m_RecentGuids->SetGuid(pItem->m_Guid, GuidInfo(pItem->m_dwFlags | FEED_ITEM_EXISTS)); 
	}

	return bStarted;
}

void FRSSFeed::OnFeedLoaded(BOOL bIsAutoDownload)
{
	if (!(m_dwFlags & FEED_FLAG_AUTO_DOWNLOAD) || !m_pChannel)
		return ; 

	//We need to update the videoIds for this channel so that we can see how many items
	//we are already downloading on this channel.
	UpdateVideoIds();

	size_t dwItemCount = m_pChannel->m_Items.GetCount();

	//We need to check current number of active downloads from this channel.
	//We can only do an approximation, because downloads are added asynchronously
	//and we can't use RSSChannelMap for the exact number of active downloads from this channel
	//We take current value and then increase it as we add downloads. This should be good enough.
	
	DWORD dwCrc32 = RSSGuidFromString(m_URL); 
	GuidInfo pInfo = g_Objects._RSSChannelMap.FindGuid(dwCrc32);

	DWORD dwCurrentActive = pInfo.m_dwValue; 

	if (dwCurrentActive >= m_MaxActiveDownloads)
		return ;

	for (size_t k = 0; k < dwItemCount; k++)
	{
		FRSSItem* pItem = m_pChannel->m_Items[k]; 
		if (pItem->m_dwFlags & FEED_ITEM_NEW)
		{
			if (g_Objects._VideoHistory.FindGuid(pItem->m_Guid).m_dwValue == 0)
			{
				if (DownloadItem(pItem, bIsAutoDownload, FALSE))
				{
					if (++dwCurrentActive >= m_MaxActiveDownloads)
						break; 
				}
			}
		}
	}
}

//Parses the RSS file, 
//checks item GUIDs against GUIDS in the RecentGuids list,
//marks items as new if they are not found in the list

BOOL FRSSFeed::LoadRSS(BOOL bAutoUpdate, const char* pszFileName)
{
	FRSSChannel* pNewChannel = NULL; 
	FString FileName; 
	if (pszFileName == NULL)
		FileName = _PathCombine(g_AppSettings.m_IndexPath, m_FileName); 
	else
		FileName = pszFileName; 

	if (!PathFileExists(FileName))
		return FALSE; 

	BOOL bFirstRefresh = (m_dwFlags & FEED_FLAG_REFRESHED) == 0;

	if (SUCCEEDED(g_Objects._RSSReader.ParseRSS(FileName, &pNewChannel, &m_Filter)))
	{
		ReleaseChannel();
		m_pChannel = pNewChannel; 
		m_dwFlags |= FEED_FLAG_REFRESHED;
	}
	else
		return FALSE; 

	m_NewItems = 0; 

	
	if (m_pChannel)
	{
		if (m_dwFlags & FEED_FLAG_SET_NAME)
		{
			m_ChannelName = m_pChannel->m_Title;
		}

		FRSSChannel* pChannel = m_pChannel; 
		DWORD dwFlags = 0; 
		//check for new items in the channel
		//look through the guid map and get each item's flags
		for (size_t kItem = 0; kItem < pChannel->m_Items.GetCount(); kItem++)
		{
			FRSSItem* pItem = pChannel->m_Items[kItem];
			if (pItem->m_ItemImage.GetLength() == 0 && m_DefImage.GetLength() > 0)
				pItem->m_ItemImage = m_DefImage; 

			GuidInfo pInfo = m_RecentGuids->FindGuid(pItem->m_Guid);
			if (pInfo.m_dwValue & FEED_ITEM_EXISTS)
			{
				pItem->m_dwFlags = pInfo.m_dwValue; 
			}
			else
			{
				//Set new items only when updating; not when the item is added.
				if (!bFirstRefresh)
					pItem->m_dwFlags |= FEED_ITEM_NEW; 

				m_RecentGuids->SetGuid(pItem->m_Guid, GuidInfo(pItem->m_dwFlags | FEED_ITEM_EXISTS)); 
			}

			if (pItem->m_dwFlags & FEED_ITEM_NEW)
				m_NewItems++; 
		}

		UpdateVideoIds();

		OnFeedLoaded(bAutoUpdate); 

		if (m_pChannel->m_dwTimeToLive)
		{
			m_UpdateInterval = m_pChannel->m_dwTimeToLive * 60;	//TTL is expressed in minutes
		}
		//Free the memory used by the channel.
		if (bAutoUpdate)
			ReleaseChannel();

		return TRUE; 
	}
	return FALSE; 
}

void FRSSFeed::MarkAllAsViewed()
{
	m_NewItems = 0; 
	FRSSChannel* pChannel = m_pChannel;
	if (pChannel)
	{
		for (size_t i = 0; i < pChannel->m_Items.GetCount(); i++)
		{
			FRSSItem* pItem = pChannel->m_Items[i]; 
			pItem->m_dwFlags&=~FEED_ITEM_NEW; 
			m_RecentGuids->SetGuid(pItem->m_Guid, GuidInfo(pItem->m_dwFlags | FEED_ITEM_EXISTS)); 
		}
	}
}

FRSSItem* FRSSFeed::FindItemByGuid(GuidType guid)
{
	FRSSChannel* pChannel = m_pChannel;
	if (pChannel)
	{
		for (size_t i = 0; i < pChannel->m_Items.GetCount(); i++)
		{
			FRSSItem* pItem = pChannel->m_Items[i]; 
			if (pItem->m_Guid == guid)
				return pItem; 
		}
	}
	return NULL; 
}

void FRSSFeed::RemoveCachedImages()
{
	if (!m_pChannel)
	{
		if (!LoadRSS(FALSE, m_FileName) || !m_pChannel)
			return; 
	}

	ATLASSERT(m_pChannel != NULL); 

	for (size_t k = 0; k < m_pChannel->m_Items.GetCount(); k++)
	{
		if (PathIsURL(m_pChannel->m_Items[k]->m_ItemImage))
			g_Objects._ImageCache.RemoveImage(m_pChannel->m_Items[k]->m_ItemImage);
	}
	g_Objects._ImageCache.RemoveImage(m_pChannel->m_ChannelImage);
	g_Objects._ImageCache.RemoveImage(m_DefImage);
}

//////////////////////////////////////////////////////////////////////////
