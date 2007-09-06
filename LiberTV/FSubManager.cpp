#include "stdafx.h"
#include "FSubManager.h"
#include "GlobalObjects.h"
#include "StrSafe.h"

FSubManager::FSubManager()
{
}

BOOL FSubManager::Init()
{
	return m_DownloadNotify.Init(this); 
}

void FSubManager::Notify(FDownloadAlert& alert)
{
	OnHttpDownloadComplete(alert.hr, (FDownloadInfo*)alert.m_Param1);
}

BOOL FSubManager::DownloadSubtitle(SubDownInfo* pInfo) 
{
    FString FStrUrl; 
    FStrUrl.Format("%s?videoID=%u&lang=%s", pInfo->SubURL, pInfo->videoID, pInfo->SubLang); 
	
	if (!PathIsURL(FStrUrl))
	{
		return FALSE; 
	}

	FDownloadInfo* pUrlInfo = new FDownloadInfo; 

	pUrlInfo->m_DownloadUrl = FStrUrl; 
	pUrlInfo->m_DownloadFile = pInfo->SubPath; 
	pUrlInfo->m_ContentType = "text/html, text/plain";
	pUrlInfo->m_pv = (void*)pInfo; 

    return g_Objects._HttpDownloader->DownloadURL(pUrlInfo, &m_DownloadNotify);
}

void FSubManager::OnHttpDownloadComplete(HRESULT hr, FDownloadInfo* pUrlInfo)
{
    SubDownInfo *pInfo = (SubDownInfo*)pUrlInfo->m_pv; 
    if (IsWindow(pInfo->hWndNotify))
	    PostMessage(pInfo->hWndNotify, WM_SUBTITLE_FINISHED, (WPARAM)hr, (LPARAM)pUrlInfo->m_pv); 
    else
        delete pInfo;

	delete pUrlInfo; 
}