#pragma once
#include "StorageManager.h"
#include "AppSettings.h"
#include "FDownloadInc.h"
#include "FDownloadMgr.h"
#include "FSubManager.h"
#include "FLabelMgr.h"
#include "FVideoHistory.h"
#include "FRSSManager.h"
#include "FDownloadDispatcher.h"
#include "FImageCache.h"


struct GlobalObjects
{
	FStorageManager			_Storage; 
	IHttpDownloader*		_HttpDownloader; 
    FDownloadMgr			_DownloadManager; 
	FSubManager				_SubManager; 
	FLabelManager			_LabelManager; 
	IClipDownloader*		_ClipDownloader; 
	FRSSManager*			_RSSManager; 
	FRSSGuidMap				_VideoHistory;	//Stores GUIDs for items that have been downloaded (or queued) in collection.
	FRSSGuidMap				_RSSChannelMap; //Stores a map of channel url and number of active downloads for that channel
	FRSSReader				_RSSReader; 
	FDownloadDispatcher		_DownloadDispatcher;
	FRSSGuidMap				_RSSGuidMap; 
	FVideoImageCache		_ImageCache; 

	bool	bNoLoadState;			//Don't load torrent state on startup
	DWORD	dwRestartOnClose;		//1 = Restart app; 2 = Restart app silently

	GlobalObjects()
	{
		bNoLoadState = false; 
		dwRestartOnClose = 0; 
	}
	bool Init(); 
	void Stop(); 
};



extern GlobalObjects g_Objects; 
