#include "stdafx.h"
#include "FVideoOptionsDlg.h"
#include "GlobalObjects.h"
#include "FMainFrame.h"

extern FAutoWindow<FMainFrame> g_MainFrame; 

FString ExtractSubLanguage(const tchar* pszFile)
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

void FVideoOptionsDlg::OnCreated()
{
	if (g_MainFrame.IsObjectAndWindow())
		g_MainFrame->EnableWindow(FALSE);

	SetMinSize(320, 200); 
	SetIcon(LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_MAIN)));
	
	SetWindowText("Video Options"); 
	

	if (FAILED(m_pW.Navigate(g_AppSettings.AppDir("data/videoOptions.html"))))
	{
		MessageBox("Sorry, no video options dialog today. Cannot load videoOptions.html. Please reinstall.", "Error", MB_OK | MB_ICONSTOP); 
		DestroyWindow();
	}
}

LRESULT FVideoOptionsDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if (g_MainFrame.IsObjectAndWindow())
	{
		g_MainFrame->EnableWindow(TRUE); 
		::SetForegroundWindow(g_MainFrame);
		g_MainFrame->SetFocus(); 
		FIEDialog::OnDestroy(uMsg, wParam, lParam, bHandled); 
	}
	return 0; 
}


void FVideoOptionsDlg::FindExistingSubtitles()
{
	//Find all .sub files in the video directory for current video
	//Add them to the subtitles combo 
	WIN32_FIND_DATA   wFindData;

	IVideoPlayer* pPlayer = g_MainFrame->GetPlayer(); 
	if (pPlayer == NULL)
		return ; 

	FDownload pDown = g_Objects._DownloadManager.GetDownloadInfo(pPlayer->GetVideoID());

	
	if (!pDown.IsValid())
		return; 
	
	FString FindFileName = _PathCombine(pDown.m_Detail.m_DataPath, "*.sub");

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
					m_pW.m_pBrowser.CallJScript("addSubLanguage", SubLang);

			}

			if (!FindNextFile(hFindData, &wFindData))
				break; 
		}
		FindClose(hFindData); 
	}
}

void FVideoOptionsDlg::OnLoadComplete()
{

	IVideoPlayer* pPlayer = g_MainFrame->GetPlayer(); 

	if (!pPlayer)
		return; 

	FindExistingSubtitles();
	char curSubFile[MAX_PATH];
	DWORD dwLen = MAX_PATH; 

	if (SUCCEEDED(pPlayer->GetSubtitleFilename(curSubFile, &dwLen)))
	{
		PathStripPath(curSubFile); 
		FString curLang = ExtractSubLanguage(curSubFile);
		m_pW.m_pBrowser.CallJScript("selectActiveLanguage", curLang); 
	}
	m_pW.m_pBrowser.CallJScript("refreshSubtitles");
}