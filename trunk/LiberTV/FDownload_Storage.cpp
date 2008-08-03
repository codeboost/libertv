#include "stdafx.h"
#include "FDownload.h"
#include "AppSettings.h"
#include "GlobalObjects.h"
#include "FVideoConv.h"

void FDownload_Storage::DeleteStorage(FDownloadEx* pDownload)
{

	_DBGAlert("DeleteStorage: Deleting %d - %s\n", pDownload->m_Detail.m_VideoID, pDownload->m_Detail.m_VideoName);
	for (size_t k = 0; k < pDownload->m_Clips.GetCount(); k++)
	{
		FDownloadItem* pItem = pDownload->m_Clips[k];

		pItem->m_TimeCompleted = time(NULL); 
		std::vector<FMediaFile> MediaFiles;
		pItem->GetMediaFiles(MediaFiles); 
		
		//Delete the 'data file'
		if (PathFileExists(pItem->m_DataFile))
		{
			if (!DeleteFile(pItem->m_DataFile))
			{
				_DBGAlert("DeleteStorage: Cannot delete data file: %s\n", pItem->m_DataFile);
			}
		}

		//Delete the torrent file & resume
		if (pItem->m_TorrentFile.GetLength() > 0)
		{
			if (!DeleteFile(pItem->m_TorrentFile))
			{
				_DBGAlert("DeleteStorage: Cannot delete data file %s. GetLastError() = %d\n", pItem->m_TorrentFile, GetLastError()); 
			}
			char pszMetaFile[MAX_PATH];
			strcpy(pszMetaFile, pItem->m_TorrentFile);
			PathRenameExtension(pszMetaFile, ".resume"); 
			DeleteFile(pszMetaFile); 
		}
		
		//Now delete the media files
		for (size_t j = 0; j < MediaFiles.size(); j++)
		{
			FMediaFile& pFile = MediaFiles[j];
			FString StrFile = pItem->DataPath(pFile.m_FileName);
			if (!DeleteFile(StrFile))
			{
				_DBGAlert("DeleteStorage: Cannot remove media file: %s (%d)\n", StrFile, GetLastError());
			}
		}
	}

	if (!DeleteFile(pDownload->m_Conf.m_FileName))
	{
		int nLastErr = GetLastError(); 
		if (nLastErr != ERROR_FILE_NOT_FOUND)
			_DBGAlert("DeleteStorage: Error deleting %s: %d\n", pDownload->m_Conf.m_FileName, nLastErr); 
	}
	else
	{
		_DBGAlert("[%d]: Storage Deleted\n", pDownload->GetVideoID()); 
	}
	FString IndexPath = pDownload->IndexPath(); 

	FString SubName; 
	SubName.Format("%d.*.sub", pDownload->GetVideoID()); 
	DeleteFilesMask(pDownload->IndexPath(), SubName); 
	DeleteFilesMask(pDownload->m_Detail.m_DataPath, SubName);


	RemoveDirectory(FormatString("%s\\%d", pDownload->m_Detail.m_DataPath, pDownload->m_Detail.m_VideoID));


	if (!RemoveDirectory(pDownload->m_Detail.m_DataPath))
	{
		_DBGAlert("DeleteStorage: Cannot remove data directory (%d): %s\n", GetLastError(), pDownload->m_Detail.m_DataPath); 
	}

	if (!RemoveDirectory(IndexPath))
	{
		_DBGAlert("DeleteStorage: Cannot remove directory (%d): %s\n", GetLastError(), IndexPath); 
	}
}

BOOL FDownload_Storage::LoadStorage(FArray<FDownloadEx*>& paDownloads)
{
	WIN32_FIND_DATA   wFindData;

	_DBGAlert("LoadStorageDir(): Starting\n");
	char Dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, Dir); 

	FString FindFileName = g_AppSettings.StorageDir("*.mtti"); 

	HANDLE hFindData = FindFirstFile(FindFileName, &wFindData); 

	int iLoaded = 0; 

	if (hFindData != INVALID_HANDLE_VALUE)
	{
		for (;;)
		{
			if (!(wFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				FString MTFileName = g_AppSettings.StorageDir(wFindData.cFileName); 

				FDownloadEx* paVideo = new FDownloadEx;

				if (paVideo->LoadFromDisk(MTFileName, ""))
				{
					paDownloads.Add(paVideo); 
					iLoaded++;
				}
				else 
					delete paVideo; 
			}
			if (!FindNextFile(hFindData, &wFindData))
				break; 
		}
		FindClose(hFindData); 
	}
	_DBGAlert("FDownloadMgr: LoadStorage() Done. %d items loaded\n", iLoaded);
	return TRUE; 
}

BOOL FDownload_Storage::DeleteStorage(BOOL bDeleteStorage)
{
	FArray<FDownloadEx*> aArray; 
	BOOL bRemoved = FALSE; 
	if (LoadStorage(aArray))
	{
		for (size_t k = 0 ; k < aArray.GetCount(); k++)
		{
			//Will only remove files relative to storage path. User may have external files which we don't want to delete
			DeleteStorage(aArray[k]);
			delete aArray[k];
		}

		bRemoved = TRUE; 

	}

	if (bDeleteStorage)
	{
		DeleteFile(g_AppSettings.StorageDir(LTV_APP_NAME".log"));
		DeleteFile(g_AppSettings.StorageDir("mediamgr.log"));
		DeleteFile(g_AppSettings.StorageDir("labels.ini"));
		DeleteFile(g_AppSettings.StorageDir("rss.ini"));
		DeleteFile(g_AppSettings.StorageDir("update\\ltv_setup.exe"));

		//remove cache\*.*; *.rss; *.uid; update\*.*
		DeleteFilesMask(g_AppSettings.StorageDir("cache"), "*.*");
		DeleteFilesMask(g_AppSettings.m_IndexPath, "*.rss");
		DeleteFilesMask(g_AppSettings.m_IndexPath, "*.uid");

		RemoveDirectory(g_AppSettings.StorageDir("update"));
		RemoveDirectory(g_AppSettings.StorageDir("cache"));
		RemoveDirectory(g_AppSettings.m_IndexPath);
	}

	return bRemoved; 
}


vidtype FDownload_Storage::InitStorage(FDownloadEx* pDownload, const tchar* pszOutDir)
{
	if (pDownload->m_Detail.m_VideoID == 0)
		pDownload->m_Detail.m_VideoID = g_AppSettings.GetRandomVideoID(); 

	pDownload->m_Detail.m_TimeAdded = time(NULL); 

	if (pDownload->m_Detail.m_DataPath.GetLength() == 0)
	{
		
		FString DirName;
		if (pDownload->m_Detail.m_VideoName.GetLength() > 0)
			 DirName = StringToFileName(pDownload->m_Detail.m_VideoName, 64);
		else
		{
			DirName.Format("%d", pDownload->m_Detail.m_VideoID);
		}
		
		pDownload->m_Detail.m_DataPath = _PathCombine(pszOutDir, DirName);
	}

	pDownload->m_Conf.m_FileName = g_AppSettings.MediaIndexPath(pDownload->m_Detail.m_VideoID); 

	FString IndexPath = pDownload->IndexPath(); 
	if (EnsureDirExists(IndexPath))
	{
		if (EnsureDirExists(pDownload->m_Detail.m_DataPath)){
			return pDownload->m_Detail.m_VideoID; 
		}
		else
		{
			_DBGAlert("ConvertDownload: Cannot create data directory: %s\n", pDownload->m_Detail.m_DataPath);
		}
	}
	else
	{
		_DBGAlert("ConvertDownload: Cannot create index directory: %s\n", IndexPath); 
	}

	for (size_t k = 0; k < pDownload->m_Clips.GetCount(); k++)
	{
		pDownload->m_Clips[k]->OnSerialized();
	}
	return pDownload->m_Detail.m_VideoID; 
}

vidtype FDownload_Storage::ConvertDownload(const tchar* pszOrgFile, const tchar* pszOutDir, FDownloadEx* pDownload)
{
	if (NULL == pszOutDir)
		pszOutDir = g_AppSettings.m_IndexPath;

	if (FVideoConv::ConvertMetafile(pszOrgFile, pDownload, pszOutDir) != ftUnknown )
	{
		return InitStorage(pDownload, pszOutDir); 
	}
	else
	{
		_DBGAlert("**ConvertDownload: Error converting file...\n"); 
		//		MessageBox(g_MainFrame, "Cannot load media. Unrecognized file format.\n", "Error", MB_OK | MB_ICONERROR); 
	}
	return 0; 
}

#if 0
BOOL LoadOldStorage()
{
	WIN32_FIND_DATA   wFindData;

	_DBGAlert("FDownloadMgr: LoadOldStorage(): Starting\n");
	char Dir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, Dir); 

	FString FindFileName = g_AppSettings.StorageDir("*.vi"); 

	HANDLE hFindData = FindFirstFile(FindFileName, &wFindData); 

	int iLoaded = 0; 

	if (hFindData != INVALID_HANDLE_VALUE)
	{
		for (;;)
		{
			if (!(wFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				FString MTFileName = g_AppSettings.StorageDir(wFindData.cFileName); 
				CAutoPtr<FDownloadEx> paVideo (new FDownloadEx); 

				FString StrNewFile; 

				if (ConvertAndLoad(MTFileName, NULL))
				{
					_DBGAlert("Converted: %s\n", MTFileName);
					DeleteFile(MTFileName); 
					iLoaded++;
				}
			}
			if (!FindNextFile(hFindData, &wFindData))
				break; 
		}
		FindClose(hFindData); 
	}
	_DBGAlert("FDownloadMgr: LoadOldStorage() Done. %d items loaded\n", iLoaded);
	m_bStorageLoaded = TRUE; 
	return TRUE; 

}

BOOL FDownloadMgr::LoadStorageAsync()
{
	FAlertStructBase *LoadAlert = new FAlertStructBase; 
	LoadAlert->dwCode = ALERT_LOAD_STORAGE; 
	return m_Alerts.Push(LoadAlert) ? TRUE: FALSE; 
}
#endif