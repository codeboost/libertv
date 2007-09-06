#pragma once

#include "FDownloadInc.h"
#include "FGlobals.h"
#include "LTV_RSS.h"
#include "FGuids.h"
#include "StringList.h"
#include "FIniFile.h"
#include "FDownload.h"

//Channel flags
#define FEED_FLAG_AUTO_DOWNLOAD 0x01	//Channel is set to 'auto download'
#define FEED_FLAG_UID_SAVED		0x02	//.uid file has been created for this channel
#define FEED_FLAG_SET_NAME		0x04	//Get channel name from feed
#define FEED_FLAG_REFRESHED		0x08	//The feed has been added; until this flag is set, it means that no RSS has been loaded

//Item flags
#define FEED_ITEM_NEW			0x01	//New item
#define FEED_ITEM_DOWNLOADED	0x02	//Item has been downloaded
#define FEED_ITEM_NOTFOUND		0x04	//Item not found in recent guids
#define FEED_ITEM_EXISTS		0x100	//Item exists (not 0)

struct FRSSFeed {
	FString			m_URL;					//Feed (Channel) URL
	FString			m_ChannelName;			//Name given by user (or updated from server)
	FString			m_SectionName;			//Section name in the ini file
	FString			m_DefImage;				//Default item image
	FString			m_DetailURL;			//Details URL in channel guide
	FRSSChannel*	m_pChannel;				//Pointer to the actual channel data (items, elements, etc)
	FString			m_FileName;				//RSS XML file (IndexPath\ChannelName.rss)
	DWORD			m_dwChannelID;			//Channel ID; randomly generated each time app is started
	DWORD			m_dwFlags;				//Flags (Channel flags above)
	DWORD			m_LastUpdate;			//Last time this feed has been updated
	DHANDLE			m_DownloadHandle;		//Used for HTTP downloads
	DWORD			m_UpdateInterval;		//Update interval for this channel
	DWORD			m_NewItems;				//Number of new items in this channel
	FRSSGuidMap*	m_pVideoMap;			//Item Guid (crc32) to videoID map
	FRSSGuidMap*	m_RecentGuids; 
	int				m_iVideos;				//active videos for this channel
	DWORD			m_MaxActiveDownloads;	//maximum number of active downloads from this channel
	FRSSFilter		m_Filter; 
	

	FRSSFeed(FRSSGuidMap* pGuidMap, FRSSGuidMap* pRecentItems);
	~FRSSFeed();

	BOOL	Load(FIniConfig::IniSection& pSection); 
	BOOL	Save(FIniConfig& pFile); 
	void	OnFeedLoaded(BOOL bIsAutoDownload);
	BOOL	DownloadItem(FRSSItem* pItem, BOOL IsAutomatic, BOOL bStream);
	BOOL	LoadRSS(BOOL bAutoUpdate, const char* pszFileName = NULL); 
	void	UpdateVideoIds(); //Updates Video IDs for all items 
	void	MarkAllAsViewed();
	void	ReleaseChannel(); //releases m_pChannel and frees up memory
	void    RSSItemToDownload(FDownloadEx& aDownload, FRSSItem* pItem, BOOL bStream);
	void	RemoveCachedImages(); 

	FRSSItem* FindItemByGuid(GuidType guid); 
};