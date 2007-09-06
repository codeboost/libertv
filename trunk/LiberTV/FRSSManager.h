#pragma  once
#include "FIniFile.h"
#include "FDownloadInc.h"
#include "IRSSInterface.h"
#include "RSSFeed.h"


struct AddFeedParams{
	FString m_URL; 
	FString m_Name; 
	FString	m_DefImage; 
	FString m_DetailsURL; 
	DWORD	m_dwFlags; 
};

struct ChannelOptions{
	DWORD		m_dwMaxDownloadsPerFeed;
	FString		m_ChannelName; 
	FRSSFilter	m_Filter; 
	ChannelOptions(){
		m_dwMaxDownloadsPerFeed = 3; 
	}
};
class FRSSManager : IClipDownloadNotify
{
	FArray<FRSSFeed*>	 m_Channels; 
	mz_Sync				 m_Sync; 
	FIniConfig			 RSSIndex; 
	FString				 m_IndexDir; 
	IRSSInterface*		 m_pNotify; 
	FRSSGuidMap			 m_GuidMap;		//Global item guid -> VideoID map
	FRSSGuidMap			 m_RecentItems; 
	BOOL				 m_bChannelRSSLoaded;
	
	BOOL				 ChannelExists(const tchar* pszName); 

	//Called when RSS file has been parsed
	BOOL				 UpdateFeed(FRSSFeed* pFeed);
	FRSSItem*			 FindByGuid(GuidType dwGuid, FRSSFeed** pFeed = NULL); 
	FRSSFeed*			 FindFeedById(DWORD dwID); 
	FRSSFeed*			 FindFeedByURL(const tchar* pszURL); 
	void				 UpdateNextFeed(); 
	void				 RemoveFromQueue(DWORD dwChannelID); 
	BOOL				 IsInUpdateQueue(DWORD dwChannelID); 
	BOOL				 IsInProgress(DWORD dwChannelID);
	
	std::deque<DWORD> m_UpdateQueue; 
	std::list<DWORD>  m_InProgress; 
public:
	~FRSSManager(); 
	FRSSManager();

	void	Clear();
	BOOL LoadIndex(const tchar* pszIndexDir); 
	BOOL SaveIndex(); 

	const FArray<FRSSFeed*>& LockChannels(); 
	void	ReleaseChannelsLock(); 
	BOOL	UpdateFeeds(); 
	DWORD	AddFeed(const tchar* pszURL, const tchar* pszName, DWORD dwFlags);
	DWORD	AddFeed(AddFeedParams& pParams);
	void	RemoveChannel(DWORD dwChannelID); 
	BOOL	RemoveChannel(const char* pszURL);
	BOOL	RefreshChannel(DWORD dwChannelID); 
	void	UpdateChannelVideoId(DWORD dwChannelID); 
	void	SetChannelAutoDownload(DWORD dwChannelID, BOOL bAutoDownload); 
	void	SetNotify(IRSSInterface* pNotify);
	void	CheckForUpdates(BOOL bForceUpdates = FALSE); 
	void	MarkAsViewed(DWORD dwChannelID); //Mark all items as viewed
	void	UpdateItemVideoId(GuidType dwGuid, DWORD dwVideoID); 
	FString	GetItemImageURL(DWORD dwChannelId, GuidType guid);
	
	DWORD	ChannelIdFromGuid(GuidType dwGuid); 
	BOOL	SetChannelOptions(DWORD dwChannelID, ChannelOptions& pOpt);

	BOOL	DownloadItem(GuidType dwGuid, BOOL bStream); 
	

	//Locks the FEEDS array and returns a pointer to the feed
	//Loads the RSS file, if it exists
	FRSSFeed* LockChannel(DWORD dwID);
	BOOL	FeedExists(DWORD dwID);

	//IClipDownloadNotify
	void Notify(FDownloadAlert& pAlert);
	void	ClearChannels(); //releases all RSS files loaded in the channels
	
};