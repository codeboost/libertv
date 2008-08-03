#ifndef __APPSETTINGS_H__
#define __APPSETTINGS_H__

#include "Utils.h"
#include "FDownloadInc.h"
#include "IAppManager.h"
#include "FCmdLine.h"

#define PLAYER_REG_KEY		LTV_REG_KEY"\\PlayerSettings"
#define TORRENT_REG_KEY		LTV_REG_KEY"\\DownloadEngine"
#define DECODER_REG_KEY		LTV_REG_KEY"\\Decoder"	


//Start with windows
#define FLAG_AUTO_START		0x01

#define FLAG_DEFAULT		FLAG_AUTO_START | PFLAG_RESUME_VIDEOS | PFLAG_SHOW_LAST_VIDEO | PFLAG_CONFIRM_REMOVE | PFLAG_AUTO_SUBTITLES | PFLAG_DONT_SHOW_INFO_DLG 

//When user clicks on a video link, it is added to the download queue.
//If this flag is set, will switch to Player view and show 'buffering'.
//Otherwise, the download status window will be shown.

#define PFLAG_STREAMING_PLAYBACK	0x04

//Load last video when opening player
#define PFLAG_SHOW_LAST_VIDEO		0x08

//Resume from last position
#define PFLAG_RESUME_VIDEOS			0x10

#define PFLAG_ESC_FULLSCREEN		0x20

//Show quick bar
#define PFLAG_QUICKBAR_ON			0x40

//Confirm video removal
#define PFLAG_CONFIRM_REMOVE        0x80

//Auto subtitles
#define PFLAG_AUTO_SUBTITLES		0x100

//Ask for folder when downloading
//#define PFLAG_ASK_FOLDER			0x200

//Jump to my collection when starting download
#define PFLAG_JUMP_COLLECTION		0x400

//Do not Show 'Download Started' info dialog
#define PFLAG_DONT_SHOW_INFO_DLG	0x800

//Install updates automatically
#define PFLAG_AUTO_UPDATE			0x1000


#define CODEC_FLAG_USE_QUICKTIME	0x01
#define CODEC_FLAG_USE_WMP			0x02


//Debug flags - if 
#define START_FLAG_NO_LOAD_STORAGE	0x01	//Do not load downloads
#define START_FLAG_NO_LOAD_FEEDS	0x02	//Do not load RSS feeds

typedef unsigned long vidtype;



struct FChannelStruct{
	FString m_Name; 
	FString m_Description; 
	FString m_GuideURL; 
	FString m_ConnectableURL; 
	FString m_SiteURL; 
};


struct AppSettings : IAppSettings
{
	int			m_Volume; 

	FString		m_AppVersion; 
	FString		m_IndexPath; 
	FString		m_ExePath; 
	FCmdLine	m_CmdLine; 
	dword		m_MaxKBDown; 
	dword		m_MaxKBUp; 
	dword		m_MaxConnections; 
	dword		m_MaxDownloads; //Max number of active video downloads 
	dword		m_ListenPort;	//BitTorrent listen port
	dword		m_RandomPort;	//Will listen on a random port each time
	dword		m_TorrentLogLevel;
	dword		m_MaxShareAge; 
	dword		m_MaxTorrents; 
	dword		m_SeedTime;		//Seed time, in seconds, for torrent rotation
	dword		m_MinSeeds;		//Minimum number of seeds 
	dword		m_MinRatio;		//Ratio, in percent, which the torrent must complete
	dword		m_MaxDownloadsPerFeed; //Maximum number of downloads from a feed
	
	FString		m_Proxy; 
	FString		m_ProxyA;
	FString		m_DefSubLang; 
	FString		m_Vout; //video ouptut module
	
	dword		m_Flags; 
	dword		m_dwAnimTime; //animation timeout
	dword		m_AddTorrentTimeout; 
	dword		m_dwHideMouse; 

	bool		SaveSettings(); 
	bool		LoadSettings();
	void		LoadDecoderSettings(); 
	void		ProcessAutoStart(); 
	bool		LoadGuides(const tchar* pszGuidesFileName, BOOL bAddToList);


	//////////////////////////////////////////////////////////////////////////
	dword		m_NumVideosInArchive; 
	dword		m_MaxStorageDays;
	dword		m_MinSpaceOnDriveMB;	
	FString		m_LastVideoID;
	dword		m_LastSection; 
	FString	    m_AppDirectory; 
//	FString		m_ChannelGuideURL; 
	FString		m_LastGuideURL;
	FString		m_DownloadsFolder; 
    FString     m_uGuid;            //User GUID
	FString		m_LogFile; 
	DWORD		m_dwCodecFlags;
	FString		m_WMCodec; 
	BOOL		m_LogEnabled; 
	DWORD		m_dwLastChannelId; //feeds last channel id
	std::vector<FChannelStruct> m_Channels; 
	DWORD		m_dwDebugFlags; 
	
public:
	void        SetLastVideoID(vidtype videoID);
	vidtype     GetLastVideoID();
    FString     AppDir(const tchar* StrRelativeDir); 
	FString		SkinPath(const tchar* pszRelative);
    FString     StorageDir(const tchar* StrRelativeDir, const tchar* pszDir = NULL); 
	FString		ChannelGuideUrl(const tchar* StrRealtive); 
	FString		MetafilePath(vidtype videoID, size_t itemIndex, const tchar* pszDir); 
	FString		MediaIndexPath(vidtype videoID);	//returns %Storage%\\videoID.mtti
	FString		FormatStoragePath(const tchar* pszDir, const tchar* pszFormat, ...);
	FString		CombineURL(const tchar* pszRoot, const tchar* pszRelative); 
	BOOL		ParseCommonJS();
	vidtype		GetRandomVideoID(); 
	FString		GetLastSelectedFolder(); 
	FString		GetVersionURL();	//Url of the version script on the server
	FString		GetConnectableURL(); //Url of the connectable script on server
	FString		GetSiteURL(const tchar* pszRelative); 
	BOOL		QueryUserProxy(FString& aProxy, FString& aUser, FString& aPass, FString& aBypass);	//Detects global IE proxy set in the system. Returns TRUE if proxy exists.
	
	//Returns FALSE if no proxy exists
	BOOL		GetProxy(FString& ProxyAddress, unsigned short& pProxyPort, FString& ProxyUser, FString& ProxyPass); 
	

    int         SelectIndexPath();

	bool		bDeveloperMode; 

	AppSettings()
	{
		bDeveloperMode = false; 
		m_dwDebugFlags = 0; 
	}

	void		FillConf(FClipDownloadConfig& aConf); 

	FString		AppName(FString& appendString);
};

extern AppSettings g_AppSettings; 
#endif //__APPSETTINGS_H__