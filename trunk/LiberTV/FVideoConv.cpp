#include "stdafx.h"
#include "FVideoConv.h"
#include <iostream>
#include "Metatorrent.h"
#include "GlobalObjects.h"
#include "winutils.h"
using namespace std;
BOOL TI2FClip(TorrentInfo& ti, FDownloadItem* pItem)
{
	return FALSE; 
}

BOOL FVideoConv::MTT2Video(const tchar* pszMttFileName, FDownloadEx* pVideo)
{

#if 0
	Metatorrent mtFile; 

	ATLASSERT(!pVideo->IsValid()); 

	BOOL bSuccess = FALSE;

	if (mtFile.Load(pszMttFileName))
	{
		pVideo->m_Detail.m_VideoName = 	mtFile.GetMT().mtName.c_str(); 
		pVideo->m_Detail.m_TotalDurationMS = (duration_type)mtFile.GetMT().duration; 
		pVideo->m_Detail.m_TotalSize = mtFile.GetMT().GetTotalSize(); 
		pVideo->m_Detail.m_VideoID = mtFile.GetMT().videoID; 

		vidtype videoID = mtFile.GetMT().videoID; 

		//Create the storage directory and spit each .torrent file into it
		FString StrDir = pVideo->IndexPath();
		if (EnsureDirExists(StrDir))
		{
			//Now convert each clip
			for (size_t k =0 ; k < mtFile.GetMT().torrents.size(); k++)
			{
				TorrentInfo& ti = mtFile.GetMT().torrents[k];
				FString StrTFile = g_AppSettings.MetafilePath(videoID, k, ""); 
				try{

					FString StrTracker; 
					StrTracker.Format("%s?videoID=%u&clipID=%d", mtFile.GetMT().mtTracker.c_str(), mtFile.GetMT().videoID, k); 
					ti.t.add_tracker((const char*)StrTracker);

					entry e = ti.t.create_torrent(); 
					std::ofstream out(StrTFile, std::ios_base::binary); 
					bencode(ostream_iterator<char>(out), e); 
					out.close();

					FDownloadItem* pItem = pVideo->AddNewClip();
					pItem->m_ClipIndex = k; 
					pItem->m_DownloadType = "torrent";
					//OLD Items - this format is no longer used and is only valid for old mtt files.
					//hence the hard coded URLs
					pItem->m_Href.Format("http://www.libertv.ro/guide/data/storage/%u_%d.torrent", videoID, k); 
					pItem->m_TorrentFile = StrTFile; 
					pItem->m_DurationMS = (duration_type)ti.duration; 
					pItem->m_FileSize = ti.GetTotalSize(); 
					pItem->m_HrefSize = GetFileSize(StrTFile); 
					pItem->m_UrlSeed = "http://beta.libertv.ro/storage/";
					pItem->m_UrlSeed1 = "http://libertv.dyndns.org/";

					pVideo->m_Clips.Add(pItem); 
				}
				catch(std::exception& e)
				{
					_DBGAlert("**MTT2Video: Exception: %s\n", e.what());
					break; 
				}
			}
		}
		return TRUE;
	}
#endif
	return FALSE; 
}

BOOL FVideoConv::T2Video(const tchar* pszTorrent, FDownloadEx* pVideo, const tchar* pszOutDir)
{
	try
	{
		std::ifstream in(pszTorrent, std::ios_base::binary);
		in.unsetf(std::ios_base::skipws);
		entry e = bdecode(std::istream_iterator<char>(in), std::istream_iterator<char>());
		torrent_info tinfo(e);	

		vidtype videoID = pVideo->m_Detail.m_VideoID; 
		if (videoID == 0) videoID = g_AppSettings.GetRandomVideoID(); 


		pVideo->m_Detail.m_VideoName = 	tinfo.name().c_str();
		pVideo->m_Detail.m_VideoID   =  videoID; 
		FString DirName = StringToFileName(pVideo->m_Detail.m_VideoName, 64);
		pVideo->m_Detail.m_DataPath = _PathCombine(pszOutDir, DirName);

		FDownloadItem* pItem = pVideo->AddNewClip(); 

		pItem->m_DownloadType = "torrent"; 
		pItem->m_HrefSize = GetFileSize(pszTorrent);
		pItem->m_Href = pItem->m_TorrentFile; 
		pItem->m_FileSize = tinfo.total_size();
		return TRUE; 
	}
	catch (std::exception& ex)
	{
		_DBGAlert("**T2Video exception for %s : %s", pszTorrent, ex.what());
	}

	return FALSE; 
}

FileTypes FVideoConv::GetMetafileType(const tchar* pszFileName)
{
	//Is MTT?
	const uint mtMagic = 0x4043; 
	FILE* pFile = fopen(pszFileName, "rb"); 

	if (pFile == NULL)
		return ftInvalid; 

	char header[16];

	fread(header, 16, 1, pFile);
	fclose(pFile); 

	uint& mtHeader = (uint&)header; 

	if (mtHeader == mtMagic)
		return ftMtt;

	//Mtti detection
	if (strnicmp(header, "[Video]", 7) == 0)
		return ftMtt2;

	if (strnicmp(header, "MTTI01", 6) == 0)
		return ftMtt3; 

	//Torrent detection
	try{
		std::ifstream in(pszFileName, std::ios_base::binary);
		in.unsetf(std::ios_base::skipws);
		entry e = bdecode(std::istream_iterator<char>(in), std::istream_iterator<char>());
		return ftTorrent;
	}
	catch(...)
	{

	}

	return ftUnknown; 
}

FileTypes FVideoConv::ConvertMetafile(const tchar* pszFileName, FDownloadEx* pVideo, const tchar* pszOutDir)
{
	FileTypes ft = GetMetafileType(pszFileName); 

	if (ft == ftMtt2 || ft == ftMtt3)
	{
		//Load from pszFileName and call SaveToDisk() which creates the storage and returns a new file name
		if (pVideo->LoadFromDisk(pszFileName, pszOutDir))
		{
			return ft; 
		}
		else
			_DBGAlert("**MTT2 load failed\n"); 
	}
	else
	if (ft == ftMtt)
	{
		if (MTT2Video(pszFileName, pVideo))
		{
			_DBGAlert("FDownloadMgr: MTT to MTT2 converted successfully\n");
			return ftMtt; 
		}
	}
	else
	if (ft == ftTorrent)
	{
		if (T2Video(pszFileName, pVideo, pszOutDir))
		{
			if (EnsureDirExists(pVideo->m_Detail.m_DataPath))
			{
				if (EnsureDirExists(pVideo->IndexPath()))
				{
					if (CopyFile(pszFileName,pVideo->m_Clips[0]->m_TorrentFile, FALSE))
					{
						return ftTorrent;
					}
				}
			}
		}
		
	}
	return ftUnknown;
}


