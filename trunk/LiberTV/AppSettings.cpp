#include "stdafx.h"
#include "AppSettings.h"
#include "StrSafe.h"
#include "shellapi.h"
#include "FDlgSelectFolder.h"
#include "wininet.h"

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif


#define MEGABYTE 1024 * 1024 



bool AppSettings::SaveSettings()
{
	_RegSetDword("GeneralFlags",	m_Flags);
	_RegSetDword("MaxStorageDays",  m_MaxStorageDays); 
	_RegSetDword("MinSpaceOnDriveMB", m_MinSpaceOnDriveMB);
	_RegSetDword("NumVideosInArchive", m_NumVideosInArchive); 
	_RegSetStr("DownloadsFolder", m_DownloadsFolder);
	_RegSetDword("FeedsLastChannelId", m_dwLastChannelId); 

	_RegSetStr("IndexPath", m_IndexPath); 

	_RegSetDword("VolumeEx",		m_Volume,			PLAYER_REG_KEY);
	_RegSetDword("AnimTimeout",		m_dwAnimTime,		PLAYER_REG_KEY); 
	_RegSetStr("LastVideoID",		m_LastVideoID,		PLAYER_REG_KEY); 
	_RegSetDword("LastSection",		m_LastSection,		PLAYER_REG_KEY); 
	_RegSetDword("HideMouseTimeout", m_dwHideMouse,		PLAYER_REG_KEY); 
	_RegSetStr("DefSubLang",		m_DefSubLang,		PLAYER_REG_KEY); 
	_RegSetDword("CodecFlags",		m_dwCodecFlags,		PLAYER_REG_KEY); 
	_RegSetStr("Vout",				m_Vout,				PLAYER_REG_KEY);

	_RegSetDword("MaxKbDown",		m_MaxKBDown,		TORRENT_REG_KEY); 
	_RegSetDword("MaxKbUp",			m_MaxKBUp,			TORRENT_REG_KEY); 
	_RegSetDword("MaxDownloads",	m_MaxDownloads,		TORRENT_REG_KEY); 
	_RegSetDword("MaxConnections",	m_MaxConnections,	TORRENT_REG_KEY); 
	_RegSetDword("ListenPortMin",	m_ListenPort,		TORRENT_REG_KEY); 
	_RegSetDword("RandomPort",		m_RandomPort,		TORRENT_REG_KEY);
	_RegSetDword("AddTorrentTimeout", m_AddTorrentTimeout, TORRENT_REG_KEY); 
	_RegSetDword("LogLevel", m_TorrentLogLevel, TORRENT_REG_KEY); 
	_RegSetDword("MaxTorrents", m_MaxTorrents, TORRENT_REG_KEY); 
	_RegSetDword("SeedTime", m_SeedTime, TORRENT_REG_KEY); 
	_RegSetDword("MinRatio", m_MinRatio, TORRENT_REG_KEY); 
	_RegSetDword("MinSeeds", m_MinSeeds, TORRENT_REG_KEY); 



	if (m_Proxy.GetLength())
	{
		_RegSetStr("HttpProxy",			m_Proxy,			TORRENT_REG_KEY);
		_RegSetStr("HttpProxyA",		m_ProxyA,			TORRENT_REG_KEY); 
	}
	else
	{
		SHDeleteValue(LTV_REG_ROOT, TORRENT_REG_KEY, "HttpProxy"); 
		SHDeleteValue(LTV_REG_ROOT, TORRENT_REG_KEY, "HttpProxyA");
	}


	ProcessAutoStart();

	
	return true; 
}
FString ExtractVarValue(const tchar* pszBuf, const tchar* pszVar)
{
	const tchar* pszPtr = strstr(pszBuf, pszVar); 
	if (pszPtr == NULL)
		return "";

	pszPtr+=strlen(pszVar);

	const tchar* pszLineEnd = strchr(pszPtr, '\n'); 
	if (NULL == pszLineEnd) 
		pszLineEnd = pszBuf + strlen(pszBuf); 


	const tchar* pszVarStart = strchr(pszPtr, '\"');

	if (NULL == pszVarStart || pszVarStart > pszLineEnd)
		pszVarStart = pszPtr; 

	pszVarStart++;

	if (pszVarStart>=pszLineEnd)
		return "";

	const tchar* pszVarEnd = strchr(pszVarStart, '\"'); 
	if (NULL == pszVarEnd)
		pszVarEnd = pszLineEnd; 

	FString Str(pszVarStart, (int)(pszVarEnd - pszVarStart)); 

	Str.Trim(); 

	return Str; 
}


bool AppSettings::LoadSettings()
{
	//General

	m_MaxDownloadsPerFeed = 5; 

	m_dwDebugFlags = 0; //START_FLAG_NO_LOAD_STORAGE | START_FLAG_NO_LOAD_FEEDS;

	if (m_CmdLine.HasParam("/devel"))
	{
		m_LogEnabled = TRUE; 
	}

	//Try 'all users'
	m_AppVersion = _RegStr("Version", LTV_REG_KEY);

	if (m_AppVersion.GetLength() == 0)	//try current user
		m_AppVersion = _RegStr("Version", LTV_REG_KEY); 


	m_Flags	=				_RegDword("GeneralFlags",	LTV_REG_KEY,	FLAG_DEFAULT);
	m_MaxStorageDays=		_RegDword("MaxStorageDays", LTV_REG_KEY,	0); 
	m_MinSpaceOnDriveMB=	_RegDword("MinSpaceOnDriveMB", LTV_REG_KEY, 350); 
	m_NumVideosInArchive=	_RegDword("NumVideosInArchive", LTV_REG_KEY, 10); 
	m_dwLastChannelId =		_RegDword("FeedsLastChannelId"); //will be returned to ExecCmd("GetLastChannelId")

	m_IndexPath = _RegStr("IndexPath", LTV_REG_KEY); 
	if (m_IndexPath.GetLength() == 0 || !PathIsDirectory(m_IndexPath))
	{
		//Older versions
		m_IndexPath =			_RegStr("StoragePath", LTV_REG_KEY); 

		if (m_IndexPath.GetLength() == 0 || !PathIsDirectory(m_IndexPath))
		{
			m_IndexPath = GetWinUserDirectory("LiberTV Data");
			if (!EnsureDirExists(m_IndexPath))
			{
				if (SelectIndexPath() == -1)
				{
					return false; 
				}
			}
			_RegSetStr("IndexPath", m_IndexPath);
		}

	}

	m_DownloadsFolder = _RegStr("DownloadsFolder");
	if (m_DownloadsFolder.GetLength() == 0 || !PathIsDirectory(m_DownloadsFolder))
		m_DownloadsFolder =  _RegStr("LastSelectedFolder", LTV_REG_KEY); 

	m_LastGuideURL		 =  _RegStr("LastChannelGuide", LTV_REG_KEY); 
	m_LogFile = _PathCombine(m_IndexPath, "libertv.log");

	
	//Player
	m_Volume =			_RegDword("VolumeEx",		PLAYER_REG_KEY, 50);
	m_dwAnimTime =		_RegDword("AnimTime",		PLAYER_REG_KEY, 350); 
	m_LastVideoID=		_RegStr("LastVideoID",		PLAYER_REG_KEY); 
	m_LastSection=		_RegDword("LastSection",	PLAYER_REG_KEY, 0); 
	m_dwHideMouse=		_RegDword("HideMouseTimeout", PLAYER_REG_KEY, 5); 
	m_DefSubLang=		_RegStr("DefSubLang",		PLAYER_REG_KEY);
	m_dwCodecFlags=		_RegDword("CodecFlags",		PLAYER_REG_KEY, CODEC_FLAG_USE_WMP);
	m_Vout=				_RegStr("Vout", PLAYER_REG_KEY); 

	if (m_Vout == "")
		m_Vout = "direct3d";

	//Network
	m_MaxKBUp =			_RegDword("MaxKbUp",		TORRENT_REG_KEY, 0); 
	m_MaxKBDown =		_RegDword("MaxKbDown",		TORRENT_REG_KEY, 0); 
	m_MaxDownloads =	_RegDword("MaxDownloads",	TORRENT_REG_KEY, 5); 
	m_MaxConnections =	_RegDword("MaxConnections", TORRENT_REG_KEY, 50); 
	m_ListenPort =		_RegDword("ListenPortMin",	TORRENT_REG_KEY, 3900); 
	m_RandomPort =		_RegDword("RandomPort",		TORRENT_REG_KEY, 0); 
	m_Proxy =			_RegStr("HttpProxy",		TORRENT_REG_KEY); 
	m_ProxyA =			_RegStr("HttpProxyA",		TORRENT_REG_KEY); 
	m_AddTorrentTimeout=_RegDword("AddTorrentTimeout", TORRENT_REG_KEY, 100); 
	m_TorrentLogLevel = _RegDword("LogLevel",		TORRENT_REG_KEY, 0);
	m_MaxShareAge	  = _RegDword("ShareAge",		TORRENT_REG_KEY, 5 * 24 * 3600);
	m_MaxTorrents	  = _RegDword("MaxTorrents",	TORRENT_REG_KEY, 50); 
	m_SeedTime		  = _RegDword("SeedTime",		TORRENT_REG_KEY, 3600); 
	m_MinSeeds		  = _RegDword("MinSeeds",		TORRENT_REG_KEY, 0); 
	m_MinRatio		  = _RegDword("MinRatio",		TORRENT_REG_KEY, 0); 

	if (m_MaxDownloads == 0)
		m_MaxDownloads = 5; 

	if (m_RandomPort > 0)
	{
		srand((int)time(NULL));
		m_ListenPort = 3000 + rand();
	}


    //Get User GUID
    m_uGuid = _RegStr("uGuid", LTV_REG_KEY); 

    if (m_uGuid.GetLength() == 0 || m_uGuid[0] != '{')
    {
        USES_CONVERSION; 
        GUID Guid;
        CoCreateGuid(&Guid); 
        OLECHAR oStrUID[256]; 
        int nLen = StringFromGUID2(Guid, oStrUID, 256);
        if (nLen > 0)
        {
            m_uGuid = OLE2T(oStrUID); 
            _RegSetStr("uGuid", m_uGuid, LTV_REG_KEY);
        }

    }

	LoadDecoderSettings();
	ProcessAutoStart();

	//Register handler and icon for .mtt files
	FString RegVal = m_ExePath + " \"%1\"";
    FString KeyName = "LiberTV.Metafile";
    SHSetValue(HKEY_CLASSES_ROOT, ".mtt", "", REG_SZ, KeyName.GetBuffer(), KeyName.GetLength()); 
    SHSetValue(HKEY_CLASSES_ROOT, ".vi", "", REG_SZ, KeyName.GetBuffer(), KeyName.GetLength()); 
	SHSetValue(HKEY_CLASSES_ROOT, ".mtti", "", REG_SZ, KeyName.GetBuffer(), KeyName.GetLength()); 

    SHSetValue(HKEY_CLASSES_ROOT, KeyName + "\\shell\\open\\command", "", REG_SZ, RegVal.GetBuffer(), RegVal.GetLength()); 
    SHSetValue(HKEY_CLASSES_ROOT, KeyName + "\\shell\\DefaultIcon", "", REG_SZ, m_ExePath.GetBuffer(), m_ExePath.GetLength()); 


	if (m_LogEnabled)
	{
		g_LogSettings.m_LogEnabled = TRUE; 
		g_LogSettings.m_LogFileName = StorageDir("libertv.log");
	}
	return true; 
}

int AppSettings::SelectIndexPath()
{
	FSelectFolder aSelector; 
	const tchar* pszMsg = "Select a folder for temporary files. LiberTV will place index files and resume data in this folder.";
	return aSelector.SelectFolder(m_IndexPath, NULL, pszMsg, 10 * MEGABYTE);
}   

void AppSettings::LoadDecoderSettings()
{
	if (0 == _RegDword("Supported_4CC", DECODER_REG_KEY))
	{
		_RegSetDword("Supported_4CC", 7, DECODER_REG_KEY); 
	}
}

void AppSettings::ProcessAutoStart()
{
   if (m_Flags & FLAG_AUTO_START)
   {
	   FString ExeName = m_ExePath; 
	   ExeName = ExeName + " /silent";

	   SHSetValue(LTV_REG_ROOT, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", "LiberTV", REG_SZ, ExeName, ExeName.GetLength());  
   }
   else
   {
	   SHDeleteValue(LTV_REG_ROOT, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", "LiberTV"); 
   }
}

void AppSettings::SetLastVideoID(vidtype videoID)
{
	m_LastVideoID.Format("%u", videoID); 
}

vidtype AppSettings::GetLastVideoID()
{
	return strtoul(m_LastVideoID, NULL, 10);
}

FString AppSettings::AppDir(const tchar* StrRelativeDir)
{
    return _PathCombine(m_AppDirectory, StrRelativeDir); 
}

FString AppSettings::SkinPath(const tchar* pszRelative)
{
	FString SkinDir;
	SkinDir.Format("Skins\\%s", _RegStr("Skin"));
	SkinDir = _PathCombine(m_AppDirectory, SkinDir); 
	return _PathCombine(SkinDir, pszRelative); 
}

FString AppSettings::StorageDir(const tchar* StrRelativeDir, const tchar* pszStorage)
{
	if (NULL == pszStorage)
		pszStorage = m_IndexPath; 
    
	return _PathCombine(pszStorage, StrRelativeDir);
}


FString AppSettings::CombineURL(const tchar* pszRoot, const tchar* pszRelative)
{
	FString UrlCombined;
	DWORD dwLen = 0; 
	UrlCombine(pszRoot, pszRelative, NULL, &dwLen, URL_ESCAPE_PERCENT | URL_ESCAPE_UNSAFE); 
	if (dwLen > 0)
	{
		LPSTR pBuf = UrlCombined.GetBufferSetLength(dwLen + 1); 
		if (S_OK == UrlCombine(pszRoot, pszRelative, pBuf, &dwLen, URL_ESCAPE_PERCENT | URL_ESCAPE_UNSAFE))
			return UrlCombined; 
	}

	return FString(pszRoot) + FString(pszRelative); 
}

FString AppSettings::ChannelGuideUrl(const tchar* StrRealtive)
{
	if (StrRealtive != NULL && strlen(StrRealtive) > 0)
		return CombineURL(_RegStr("ChannelGuideURL"), StrRealtive); 
	return _RegStr("ChannelGuideURL"); 
}

FString AppSettings::MediaIndexPath(vidtype videoID)
{
	return FormatStoragePath(m_IndexPath, "%u.mtti", videoID);
}

FString AppSettings::MetafilePath(vidtype videoID, size_t itemIndex, const tchar* pszDir)
{
	if (NULL == pszDir || strlen(pszDir) == 0)
		pszDir = m_IndexPath; 

	return FormatStoragePath(pszDir, "%u.vf\\%u_%d.vf", videoID, videoID, itemIndex); 
}

FString AppSettings::FormatStoragePath(const tchar* pszFolder, const tchar* pszFormat, ...)
{
	va_list paramList;
	va_start(paramList, pszFormat);

	char pszPath[MAX_PATH] = {0};
	StringCbVPrintf(pszPath, MAX_PATH, pszFormat, paramList); 
	va_end(paramList); 

	return StorageDir(pszPath, pszFolder); 
}

vidtype AppSettings::GetRandomVideoID()
{
	vidtype RandVID = (vidtype)(rand() / 100 + 1) * GetTickCount();
	Sleep(50); //let GetTickCount() tick for 3 ms..
	return RandVID;
}

FString AppSettings::GetLastSelectedFolder()
{
	if (!PathIsDirectory(m_DownloadsFolder))
		return "";

	return m_DownloadsFolder;
}

FString AppSettings::GetVersionURL()
{
	FString SiteURL = _RegStr("SiteURL"); 

	if (SiteURL.GetLength() == 0)
		SiteURL = _RegStr("ChanelGuideURL");

	return CombineURL(SiteURL, "version.php"); 
}

FString AppSettings::GetConnectableURL()
{
	return _RegStr("CheckConnectURL"); 
}

FString AppSettings::GetSiteURL(const tchar* pszRelative)
{
	FString SiteURL = _RegStr("SiteURL"); 
	if (SiteURL.GetLength() > 0)
	{
		return CombineURL(SiteURL, pszRelative); 
	}
	return pszRelative; 
}

void AppSettings::FillConf(FClipDownloadConfig& aConf)
{
	aConf.m_MaxKBDown = m_MaxKBDown; 
	aConf.m_MaxKBUp = m_MaxKBUp;
	aConf.m_MaxConnections = m_MaxConnections; 
	aConf.m_MaxDownloads =  m_MaxDownloads;
	aConf.m_MaxUploads =  8;	//magic of magic :)
	aConf.m_TorrentLogLevel = 0;
	aConf.m_MaxShareAge =  m_MaxShareAge;
	aConf.m_MaxTorrents =  m_MaxTorrents;
	aConf.m_Proxy =  m_Proxy;
	aConf.m_ProxyA = m_ProxyA;
	aConf.m_AppVersion = m_AppVersion; 
	aConf.m_LPortMin = m_ListenPort;
	aConf.m_LPortMax = m_ListenPort; 
	aConf.m_IndexPath = m_IndexPath;
}

BOOL AppSettings::GetProxy(FString& ProxyAddress, unsigned short& pProxyPort, FString& ProxyUser, FString& ProxyPass)
{
	std::string porttmp; 
	FString proxy_port; 

	if (m_Proxy.GetLength() == 0)
		return FALSE; 

	if (StrSplit(m_Proxy, ProxyAddress, proxy_port, ':'))
	{
		pProxyPort = (unsigned short)StrToInt(proxy_port); 
		if (m_ProxyA.GetLength() > 0)
			StrSplit(m_ProxyA, ProxyUser, ProxyPass);

		return TRUE; 
	}
	return FALSE; 
}


BOOL QueryInternetOption(DWORD dwOption, FString& StrOption)
{
	char* pBuffer = NULL; 
	DWORD dwLen = 0; 

	if (!InternetQueryOption(NULL, dwOption, pBuffer, &dwLen) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		CAutoPtr<char> autoBuf (new char [dwLen + 1]);
		pBuffer = autoBuf.m_p;

		if (InternetQueryOption(NULL, dwOption, pBuffer, &dwLen))
		{
			pBuffer[dwLen] = 0; 
			StrOption = pBuffer; 
			return TRUE; 
		}
	}
	return FALSE; 
}


BOOL AppSettings::QueryUserProxy(FString& aProxy, FString& aUser, FString& aPass, FString& aBypass)
{
	char *aProxyInfo = NULL; 
	DWORD dwLen = 0;

	if (!InternetQueryOption(NULL, INTERNET_OPTION_PROXY, aProxyInfo, &dwLen) && GetLastError() == ERROR_INSUFFICIENT_BUFFER)
	{
		CAutoPtr<char> autoBuf (new char [dwLen + 1]);
		aProxyInfo = autoBuf.m_p;
		if (InternetQueryOption(NULL, INTERNET_OPTION_PROXY, aProxyInfo, &dwLen))
		{
			INTERNET_PROXY_INFO* pInfo = (INTERNET_PROXY_INFO*)aProxyInfo; 

			if (pInfo->dwAccessType == INTERNET_OPEN_TYPE_PROXY)
			{
				aProxy = pInfo->lpszProxy;
				aBypass = pInfo->lpszProxyBypass;

				QueryInternetOption(INTERNET_OPTION_PROXY_USERNAME, aUser);
				QueryInternetOption(INTERNET_OPTION_PROXY_PASSWORD, aPass);
				return TRUE; 
			}
		}
	}


	return FALSE; 
}
