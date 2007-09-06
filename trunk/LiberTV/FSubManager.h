#pragma once

#include "Utils.h"
#include "AppSettings.h"
#include "FAsyncHttpDownload.h"
#include "FSubParser.h"


struct SubDownInfo
{
    HWND        hWndNotify; 
    vidtype     videoID;
    FString     SubPath; 
    FString     SubLang; 
	FString		Charset; 
	FString		SubURL; 
};


//Player (or whatever) calls DownloadSubtitle

class FSubManager : public IClipDownloadNotify
{
	FDownloadNotify m_DownloadNotify; 
public:
	FSubManager(); 
	BOOL	Init(); 
    int     DownloadSubtitle(SubDownInfo* pInfo); 
    void    OnHttpDownloadComplete(HRESULT hr, FDownloadInfo* pInfo);
	void	Notify(FDownloadAlert& alert); 
};