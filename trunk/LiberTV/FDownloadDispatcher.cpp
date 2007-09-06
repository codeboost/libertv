#include "stdafx.h"
#include "FDownloadDispatcher.h"
#include "FMainFrame.h"
#include "FGlobals.h"
#include "GlobalObjects.h"
#include "FDlgSaveVideo.h"
extern FAutoWindow<FMainFrame> g_MainFrame; 


void FDownloadDispatcher::FDispThread::Thread(void* p)
{
	FDownloadDispatcher* pThis = (FDownloadDispatcher*)p;
	for (;;)
	{
		FDownloadInfo di; 
		bool bPopped = false; 
		int nRes = pThis->m_QueuedDownloads.PopWait(di, bPopped); 
		if (nRes != WAIT_OBJECT_0)
			break; 
		if (bPopped)
		{
			if (di.m_dwDownloadFlags & HTTP_FLAG_DOWNLOAD_URL)
				g_Objects._HttpDownloader->DownloadURL(new FDownloadInfo(di), pThis); 
			else
			{
				pThis->LoadDownload(di);
				DeleteFile(di.m_DownloadFile); 
			}
		}
	}
}


FDownloadDispatcher::FDownloadDispatcher()
{
}

BOOL FDownloadDispatcher::Init()
{
	m_QueuedDownloads.SetSize(128);
	m_DispThread.Create(this); 
	return TRUE; 
}

FDownloadDispatcher::~FDownloadDispatcher()
{
	m_QueuedDownloads.Clear(); 
	m_DispThread.StopThread(); 
}

BOOL FDownloadDispatcher::LoadDownloadAsync(FDownloadInfo& pInfo)
{
	return m_QueuedDownloads.Push(pInfo);
}

BOOL FDownloadDispatcher::DownloadMTT(FDownloadInfo* pUrlInfo)
{
	ATLASSERT(pUrlInfo != NULL); 
	if (NULL == pUrlInfo)
		return FALSE;

	srand((unsigned int)time(NULL));
	FString vFileName; 
	vFileName.Format("%u%u.tmp", g_AppSettings.GetRandomVideoID(), GetCurrentThreadId()); 
	pUrlInfo->m_DownloadFile = _PathCombine(g_AppSettings.m_IndexPath, vFileName);
	pUrlInfo->m_ContentType = "text/html, text/plain, application/x-bittorrent, application/x-mtti"; 
	_DBGAlert("Starting temp download: for URL %s into %s\n", pUrlInfo->m_DownloadUrl, pUrlInfo->m_DownloadFile); 
	//return g_Objects._HttpDownloader->DownloadURL(pUrlInfo, this) > 0;
	pUrlInfo->m_dwDownloadFlags|=HTTP_FLAG_DOWNLOAD_URL;
	return m_QueuedDownloads.Push(*pUrlInfo); 
}

BOOL FDownloadDispatcher::OpenMTTFromHTTP(FDownloadInfo& pDownloadInfo)
{
	FString aVid; 
	if (ExtractQueryParam(pDownloadInfo.m_DownloadUrl, "videoID", aVid))
	{
		vidtype videoID = strtoul(aVid, NULL, 10); 
		if (g_Objects._DownloadManager.IsDownload(videoID))
		{
			if (g_MainFrame.IsObjectAndWindow())
				g_MainFrame->SetActiveView(VIEW_STATUS); 
			return TRUE; 
		}
	}
	if (g_MainFrame.IsObjectAndWindow())
		g_MainFrame->PostMessage(WM_MTTI_DOWNLOAD, 0, 0); 

	FDownloadInfo *pNewUrlInfo = new FDownloadInfo(pDownloadInfo); //deleted in WM_HTTP_DOWNLOAD_COMPLETE

	if (!DownloadMTT(pNewUrlInfo))
	{
		delete pNewUrlInfo;
		return FALSE; 
	}
	return TRUE; 
}

DWORD FDownloadDispatcher::LoadDownload(FDownloadInfo& pUrlInfo)
{
	BOOL bLoadVideo = TRUE; 

	FString StoreFolder = g_AppSettings.GetLastSelectedFolder(); 
	BOOL bCtrl  = ( GetKeyState(VK_CONTROL) & 0x8000) != 0;
	BOOL bShift = ( GetKeyState(VK_SHIFT) & 0x8000) != 0;

	FIniConfig aConf; 
	FString StrDownloadType;
	if (GetFileSize(pUrlInfo.m_DownloadFile) < 8192 && 
		aConf.Load(pUrlInfo.m_DownloadFile))
	{
		if (aConf.SectionExists("Subscribe")) //Subscribe to FEED
		{
			FString FeedUrl = aConf.GetValue("Subscribe", "FeedURL");
			FString FeedName = aConf.GetValue("Subscribe", "FeedName"); 
			if (PathIsURL(FeedUrl))
			{
				g_MainFrame->m_Container->SwitchActiveView(VIEW_FEEDS); 
				g_MainFrame->AddFeedDialog(FeedUrl, FeedName); 
				g_MainFrame->m_Container->SwitchActiveView(VIEW_FEEDS); 
				return 1; 
			}

		}
		else
			StrDownloadType = aConf.GetValue("Clip 0", "DownloadType");
	}

	if (StoreFolder.GetLength() == 0 || !PathIsDirectory(StoreFolder) || bCtrl || bShift)
	{
		if (StrDownloadType != "stream")
		{
			FDlgSaveVideo aSaveVideo; 
			FDlgSaveVideo::SaveDlgOpenParams Params(&g_AppSettings);
			
			int nRes = aSaveVideo.Open(g_MainFrame, Params); 
			if (nRes  == IDOK)
			{
				StoreFolder = aSaveVideo.m_StrStorage;
				g_AppSettings.SaveSettings();
			}
			else
				bLoadVideo = FALSE; 
		}
		else
			StoreFolder = g_AppSettings.m_IndexPath;
	}
	vidtype videoID = 0; 
	if (bLoadVideo)
	{
		videoID = g_Objects._DownloadManager.ConvertAndLoad(&pUrlInfo, StoreFolder);
		if (g_MainFrame.IsObjectAndWindow())
			g_MainFrame->PostMessage(WM_DOWNLOAD_LOADED, (WPARAM)videoID, 0); 
	}

	return videoID; 
}

void FDownloadDispatcher::Notify(FDownloadAlert& alert)
{
	if (alert.dwCode == ALERT_HTTP_DOWNLOAD_FINISHED)
	{

		FDownloadInfo* pUrlInfo = (FDownloadInfo*)alert.m_Param1;

		if (SUCCEEDED(alert.hr))
		{
			LoadDownload(*pUrlInfo); 
			DeleteFile(pUrlInfo->m_DownloadFile); 
		}

		if (g_MainFrame.IsObjectAndWindow())
			g_MainFrame->PostMessage(WM_MTTI_DOWNLOAD, 1, (LPARAM)alert.hr);
		delete pUrlInfo;
	}


	//hr = E_NOINTERFACE : Content type mismatch. Create a fake mtti and download the file.
	/*
	pUrlInfo->m_MediaName = pUrlInfo->m_DownloadUrl;
	FDownloadEx aMTT; 
	aMTT.CreateFromURL(g_AppSettings.GetRandomVideoID(), pUrlInfo->m_MediaName, pUrlInfo->m_DownloadUrl, pUrlInfo->m_ContentType); 
	aMTT.m_Conf.m_FileName.Format("%u.tmp", GetTickCount()); 
	aMTT.m_Conf.m_FileName = g_AppSettings.StorageDir(aMTT.m_Conf.m_FileName);
	if (aMTT.m_Conf.Save())
	{
	ConvertAndLoad(aMTT.m_Conf.m_FileName, pUrlInfo->m_DownloadUrl);
	DeleteFile(aMTT.m_Conf.m_FileName); 
	}
	*/
}