#include "stdafx.h"
#include "FMediaPlayer.h"
#include "GlobalObjects.h"

void FMediaPlayer::OnSubtitleFinished(HRESULT hr, void* lpVoid)
{
	SubDownInfo* pInfo = (SubDownInfo*)lpVoid; 
	if (SUCCEEDED(hr))
	{
		if (!m_SubParser.SetSubtitle(pInfo->SubPath))
			hr = E_FAIL; 
	}

	m_pInfoBar.m_pBrowser.CallJScript("setSubtitle", ""); 

	if (FAILED(hr) && m_bHideInfoBarOnError)
	{
		ShowInfoBar(FALSE); 
	}
	delete pInfo;

}

HRESULT FMediaPlayer::SelectSubtitle(const tchar* szLang)
{
	if (strlen(szLang) == 0)
	{
		m_SubParser.Clear(); 
		m_bShowSub = FALSE; 
		m_pInfoBar.m_pBrowser.CallJScript("setSubtitle", "");
		return S_OK; 
	}

	if (strcmp(szLang, "*") == 0)	//Any language
	{
		m_SubParser.Clear(); 
		m_bShowSub = TRUE; 
		m_pInfoBar.m_pBrowser.CallJScript("setSubtitle", "");
		return S_OK; 
	}
		
	FString SubPath = _PathCombine(m_pVideo.m_Detail.m_DataPath, FormatString("%d.%s.sub", m_pVideo.m_Detail.m_VideoID, szLang));

	if (!PathFileExists(SubPath))
	{
		SubDownInfo* pInfo = new SubDownInfo; 
		pInfo->hWndNotify = m_hWnd; 
		pInfo->SubLang = szLang;
		pInfo->SubPath = SubPath; 
		pInfo->videoID = m_pVideo.GetVideoID(); 
		pInfo->SubURL = m_pVideo.m_Detail.m_SubURL;
		m_pInfoBar.m_pBrowser.CallJScript("setSubtitle", "Searching subtitles..."); 
		if (!g_Objects._SubManager.DownloadSubtitle(pInfo))
			return E_FAIL; 
	}
	else
	{
		m_SubParser.Clear(); 
		if (!m_SubParser.SetSubtitle(SubPath))
			return E_FAIL; 
	}

	m_bShowSub = TRUE; 
	return S_OK; 
}

void FMediaPlayer::UpdateSubtitle(ulong ulFrameNum)
{
	if (m_bShowSub && m_SubParser.HasSubs() && m_pInfoBar.IsWindow() && m_pInfoBar.IsWindowVisible())
	{
		FString fSub = m_SubParser.GetSubAt(ulFrameNum); 
		fSub.Replace('|', '\n');
		m_pInfoBar.m_pBrowser.CallJScript("setSubtitle", fSub);
	}
}

HRESULT FMediaPlayer::GetSubtitleFilename(char* pszSub, DWORD *dwLen)
{
	if (m_SubParser.HasSubs())
	{
		if (*dwLen > (DWORD)m_SubParser.m_SubFileName.GetLength() + 1)
		{
			strcpy(pszSub, m_SubParser.m_SubFileName); 
			*dwLen = m_SubParser.m_SubFileName.GetLength() + 1; 
			return S_OK; 
		}
		else
		{
			*dwLen = m_SubParser.m_SubFileName.GetLength() + 1;
			return E_POINTER;
		}
	}
	return E_FAIL; //No subtitles loaded
}

FString FMediaPlayer::ExtractSubLanguage(const tchar* pszFile)
{
	const tchar* pszDot = strchr(pszFile, '.'); 

	if (pszDot)
	{
		const tchar* pszLastDot = strrchr(pszDot + 1, '.'); 
		if (pszLastDot)
			return FString(pszDot + 1, (int)(pszLastDot - pszDot - 1)); 
	}
	return "";
}

void FMediaPlayer::FindExistingSubtitles()
{
	//Find all .sub files in the video directory for current video
	//Add them to the subtitles combo 
	WIN32_FIND_DATA   wFindData;

	FString FindFileName = _PathCombine(m_pVideo.m_Detail.m_DataPath, "*.sub");

	HANDLE hFindData = FindFirstFile(FindFileName, &wFindData); 

	if (hFindData != INVALID_HANDLE_VALUE)
	{
		for (;;)
		{
			if (!(wFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (wFindData.nFileSizeLow > 0))   
			{
				//FString SubFile = g_AppSettings.VideoDir(currentDownload, wFindData.cFileName); 
				//Read language from SubFile
				FString SubLang = ExtractSubLanguage(wFindData.cFileName); 
				if (SubLang.GetLength() > 0)
					m_pInfoBar.m_pBrowser.CallJScript("addSubLanguage", SubLang);
			}

			if (!FindNextFile(hFindData, &wFindData))
				break; 
		}
		FindClose(hFindData); 
	}
}

void FMediaPlayer::UpdateSubtitles()
{
	FindExistingSubtitles();

	if (m_SubParser.HasSubs())
	{
		FString curLang = ExtractSubLanguage(m_SubParser.m_SubFileName);
		m_pInfoBar.m_pBrowser.CallJScript("selectActiveLanguage", curLang); 
	}
	FString FVideoID; 
	FVideoID.Format("%u", m_pVideo.m_Detail.m_VideoID); 

	m_pInfoBar.m_pBrowser.CallJScript("refreshSubtitles", m_pVideo.m_Detail.m_SubURL, FVideoID);

}