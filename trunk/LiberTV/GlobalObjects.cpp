#include "stdafx.h"
#include "GlobalObjects.h"
#include "FClipDownloaderEx.h"


bool GlobalObjects::Init()
{
	_RSSManager = new FRSSManager; 
	_DownloadDispatcher.Init(); 
	_VideoHistory.Load(_PathCombine(g_AppSettings.m_IndexPath, "VideoHistory.uid"));
	_LabelManager.Load(_PathCombine(g_AppSettings.m_IndexPath, "labels.ini")); 
	_ClipDownloader = new FClipDownloaderEx;
	_HttpDownloader = new FAsyncDownload;
	_SubManager.Init();
	
	FString CacheDir = _PathCombine(g_AppSettings.m_IndexPath, "cache");
	if (!PathIsDirectory(CacheDir))
		CreateDirectory(CacheDir, NULL); 

	_ImageCache.Init(CacheDir); 

	FString RSSFileName = _PathCombine(g_AppSettings.m_IndexPath, "rss.ini");
	if (!PathFileExists(RSSFileName))
	{
		//if rss.ini is not found in the index directory, copy the file from the default rss.ini, the one
		//that came with the installation. (if user removes all feeds, rss.ini exists, but is blank, so this shouldn't
		//happen when user removes all feeds).
		FString OrgFileName = _PathCombine(g_AppSettings.m_AppDirectory, "rss.ini");
		CopyFile(OrgFileName, RSSFileName, TRUE); 
	}

	_RSSManager->LoadIndex(g_AppSettings.m_IndexPath);
	 
	return true; 
}

void GlobalObjects::Stop()
{
	_HttpDownloader->Stop();
	_ClipDownloader->Stop(); 
	_RSSManager->SaveIndex();
	_VideoHistory.Save();
	delete _ClipDownloader;
	delete _HttpDownloader;
	delete _RSSManager;
}



