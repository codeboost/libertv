#include "stdafx.h"
#include "FDownload.h"
#include "AppSettings.h"
#include "atlutil.h"
#include <fstream>
#include "FGuids.h"
#include "FXMLUtil.h"

#ifndef _SCL_SECURE_NO_DEPRECATE
#define _SCL_SECURE_NO_DEPRECATE
#endif
#ifndef _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_DEPRECATE
#endif

extern int GetMediaType(FMediaFile* mf);

FString GetUrlExtension(const tchar* pszURL)
{
	CUrl url; 
	url.CrackUrl(pszURL); 
	FString aPath; 
	aPath.SetString(url.GetUrlPath(), url.GetUrlPathLength()); 

	const tchar* pszLast = strrchr(aPath, '.'); 

	if (pszLast != NULL)
	{
		pszLast++; 
		return FString(pszLast, strlen(pszLast)); 
	}
	return ""; 
}

BOOL RenameExtension(FMediaFile& aMediaFile, const tchar* pszExt)
{
	FString CurrentExt = PathFindExtension(aMediaFile.m_FileName); 
	if (CurrentExt.CompareNoCase(pszExt) != 0)
	{
		FString OrgFileName = aMediaFile.m_FileName; 
		if (PathRenameExtension(aMediaFile.m_FileName.GetBuffer(), pszExt))
		{
			DeleteFile(aMediaFile.m_FileName); //Delete the destination file, if it exists
			if (!MoveFile(OrgFileName, aMediaFile.m_FileName))
			{
				_DBGAlert("Cannot rename file from %s to %s\n", OrgFileName, aMediaFile.m_FileName);
				aMediaFile.m_FileName = OrgFileName;
				return FALSE;
			}
			return TRUE ;
		}
	}
	return FALSE; 
}


BOOL FDownload::RenameClipExtension(FMediaFile& pFile, const tchar* pszNewExtension)
{
	for (size_t k = 0; k < m_Clips.GetCount(); k++)
	{
		FDownloadItem* pClip = m_Clips[k]; 
		if (pClip->m_DataFile == pFile.m_FileName)
		{
			if (RenameExtension(pFile, pszNewExtension))
				pClip->m_DataFile = pFile.m_FileName; 
		}
	}
	return FALSE; 
}

FDownload& FDownload::operator =(const FDownload& rVid)
{
	if (this != &rVid)
		CopyFrom(rVid); 		
	return *this; 
}

FDownload::FDownload(const FDownload& rVid)
{
	CopyFrom(rVid);
}

void FDownload::CopyFrom(const FDownload& rVid)
{
	m_Detail = rVid.m_Detail; 
	m_RSSInfo = rVid.m_RSSInfo; 
	m_CurClipIndex = rVid.m_CurClipIndex; 
	m_dwFlags = rVid.m_dwFlags; 
	RemoveClips(); 
	for (size_t k = 0 ; k < rVid.m_Clips.GetCount(); k++)
	{
		FDownloadItem* pNewClip = new FDownloadItem(*rVid.m_Clips[k]); 
		pNewClip->m_Video = this; 
		m_Clips.Add(pNewClip); 
	}
}

void FVideoDetail::Reset()
{
	m_VideoID = 0; 
	m_ShowID = 0; 
	m_EpisodeID = 0; 
	m_TotalSize = 0; 
	m_TotalDurationMS = 0; 
	m_TimeAdded = 0; 
	m_TimeCompleted = 0; 
	m_SizeDownloaded = 0; 
	m_PlaybackPosMS = 0 ;
	m_VideoName.ReleaseBuffer();
	m_DataPath.ReleaseBuffer();
	m_Labels.Clear(); 
	m_Watched = 0; 
	m_VideoFlags = 0; 
	m_TimePublished = 0;
}

FString FDownloadItem::DataPath(const tchar* pszRelative)
{
	ATLASSERT(m_DataPath.GetLength() > 0); 
	
	return _PathCombine(m_DataPath, pszRelative);
}

void AddFromFile(std::vector<FMediaFile>& Where, const tchar* pszFileName, const tchar* pszDescription, const tchar* pszContentType)
{
	FMediaFile aMediaFile; 
	aMediaFile.m_FileName = pszFileName; 
	aMediaFile.m_FileSize = GetFileSize(pszFileName); 
	aMediaFile.m_Description = pszDescription; 
	StripHTMLTags(aMediaFile.m_Description.GetBuffer()); 
	aMediaFile.m_Description.Trim();
	aMediaFile.m_Index = Where.size(); 
	aMediaFile.m_MediaType = pszContentType;
	Where.push_back(aMediaFile); 
}

BOOL IsMediaFile(const tchar* pszFileName)
{
	const char* gExtensions[] = {
	".avi", ".mpg", ".mpeg", ".ts" , ".mkv", ".vob",  ".dv",
	".mov", ".m4v", ".mp4" , ".mpa", ".m4a", ".moov", ".qt",
	".wmv", ".asf", ".wma" , ".m2v", ".m2p", ".m1v",  ".mpv", ".mp1", ".mp2",
	".flv", ".swf", ".mp3" , ".wav", ".mp2", ".ac3",  ".pcm", ".aac", ".mpa", ".aif", ".aiff",
	".rar", ".vf",  ".divx", ".ogm", ".rm", ".ram",
	NULL
	};

	FString Ext = PathFindExtension(pszFileName);
	Ext.MakeLower();

	size_t k = 0; 
	while (gExtensions[k] != NULL)
	{
		if (Ext == gExtensions[k])
			return TRUE; 
		k++; 
	}
	return FALSE; 
}

size_t FDownloadItem::GetMediaFiles(std::vector<FMediaFile>& MediaFiles)
{
	size_t nAdded = 0; 	
	if (m_DownloadType == "http" || m_DownloadType == "none" || m_DownloadType == "")
	{
		AddFromFile(MediaFiles, m_DataFile, m_Description, m_ContentType);
		nAdded++; 
	}
	else
	if (m_DownloadType == "torrent" && PathFileExists(m_TorrentFile))
	{
		try
		{
			
			std::ifstream in_t(m_TorrentFile, std::ios_base::binary);
			if (in_t.is_open())
			{
				in_t.unsetf(std::ios_base::skipws);
				entry e = bdecode(std::istream_iterator<char>(in_t), std::istream_iterator<char>()); 
				in_t.close();
				torrent_info ti (e);
				for (size_t k = 0; k < (size_t)ti.num_files(); k++)
				{
					FMediaFile aMediaFile; 
					const file_entry& ft = ti.file_at((int)k);
					if (IsMediaFile(ft.path.string().c_str()))
					{
						AddFromFile(MediaFiles, DataPath(ft.path.string().c_str()), m_Description, m_ContentType);
						nAdded++; 
					}
				}
			}

		} catch(...){}
		return nAdded; 
	} 
	else
	if (m_DownloadType == "stream")
	{
		FMediaFile aMediaFile; 
		aMediaFile.m_FileName = m_Href; 
		aMediaFile.m_Description = m_Description; 

		
		if (m_StreamType.GetLength() > 0)
			aMediaFile.m_MediaType = m_StreamType; 
		else
		{
			FString ext = GetUrlExtension(aMediaFile.m_FileName); 
			if (ext.GetLength() > 0)
				aMediaFile.m_MediaType = ext.MakeLower();
		}
		MediaFiles.push_back(aMediaFile); 
		nAdded++;
	}
	return nAdded; 
}

void FDownload::RemoveClips()
{
	for (size_t k = 0 ; k < m_Clips.GetCount(); k++)
	{
		delete m_Clips[k];
	}
	m_Clips.RemoveAll(); 
}

BOOL FDownload::Serialize(FIniConfig& Conf, BOOL bPut)
{
	DWORD dwFlags = bPut ? FLAG_PUTVAL | FLAG_ADDIFNEW: 0;

	if (!bPut && !Conf.SectionExists("Video"))
		return FALSE; 

	FString StrLabels;
	if (bPut)
	{
		StrLabels = m_Detail.m_Labels.GetLabelStr(); 
	}

	Conf.AddSection("Video"); 
	Conf.ExchangeValueStr("Video",	 "Name", m_Detail.m_VideoName, dwFlags); 
	Conf.ExchangeValueStr("Video",	 "Labels", StrLabels, dwFlags); 
	Conf.ExchangeValueDWORD("Video", "VideoID", m_Detail.m_VideoID, 10, dwFlags); 
	Conf.ExchangeValueDWORD("Video", "EpisodeID", m_Detail.m_EpisodeID, 10, dwFlags);
	Conf.ExchangeValueUINT64("Video","Size", (UINT64&)m_Detail.m_TotalSize, 10, dwFlags);
	Conf.ExchangeValueUINT64("Video","DurationMS", (duration_type&)m_Detail.m_TotalDurationMS, 10, dwFlags);
	Conf.ExchangeValueDWORD("Video", "TimeAdded", (DWORD&)m_Detail.m_TimeAdded, 10, dwFlags);
	Conf.ExchangeValueDWORD("Video", "TimeCompleted", (DWORD&)m_Detail.m_TimeCompleted, 10, dwFlags);
	Conf.ExchangeValueDWORD("Video", "DatePublished", (DWORD&)m_Detail.m_TimePublished, 10, dwFlags);
	Conf.ExchangeValueUINT64("Video","PlaybackPos", (duration_type&)m_Detail.m_PlaybackPosMS, 10, dwFlags); 
	Conf.ExchangeValueUINT64("Video","Watched", (duration_type&)m_Detail.m_Watched, 10, dwFlags); 
	Conf.ExchangeValueDWORD("Video", "dwFlags", m_dwFlags, 16, dwFlags);
	Conf.ExchangeValueDWORD("Video", "VideoFlags", m_Detail.m_VideoFlags, 10, dwFlags); 
	Conf.ExchangeValueStr("Video",	 "SubURL", m_Detail.m_SubURL, dwFlags); 
	Conf.ExchangeValueStr("Video", "DataPath", m_Detail.m_DataPath, dwFlags); 
	Conf.ExchangeValueStr("Video", "DetailsURL", m_Detail.m_DetailURL, dwFlags);
	Conf.ExchangeValueStr("Video", "ImageURL", m_Detail.m_ImageURL, dwFlags);
	Conf.ExchangeValueStr("Video", "Description", m_Detail.m_Description, dwFlags); 

//	if (m_Detail.m_DataPath == "")
//		m_Detail.m_DataPath = g_AppSettings.m_IndexPath; 

	//RSS Stuff
	Conf.ExchangeValueDWORD("RSS", "GUID", m_RSSInfo.m_Guid, 10, dwFlags); 
	Conf.ExchangeValueDWORD("RSS", "ChannelGUID", m_RSSInfo.m_RSSGuid, 10, dwFlags); 
	Conf.ExchangeValueStr("RSS", "FeedName", m_RSSInfo.m_RSSName, dwFlags); 
	Conf.ExchangeValueStr("RSS", "FeedURL", m_RSSInfo.m_RSSURL, dwFlags); 
	Conf.ExchangeValueStr("RSS", "ItemURL", m_RSSInfo.m_ItemURL, dwFlags); 
	Conf.ExchangeValueDWORD("RSS", "RSSFlags", m_RSSInfo.m_dwFlags, 10, dwFlags); 
	
	
	if (!bPut)
	{
		m_Detail.m_Labels.ParseLabelStr(StrLabels); 
		if (m_RSSInfo.m_RSSGuid == 0 && m_RSSInfo.m_RSSURL.GetLength() > 0)
			m_RSSInfo.m_RSSGuid = RSSGuidFromString(m_RSSInfo.m_RSSURL);
	}

	return TRUE; 
}

BOOL FDownloadItem::Serialize(FIniConfig& Conf, const tchar* SectionName, BOOL bPut)
{
	DWORD dwFlags = bPut ? FLAG_PUTVAL | FLAG_ADDIFNEW: 0;

	if (!bPut && !Conf.SectionExists(SectionName))
		return FALSE; 

	FString StrMetaFile; 
	Conf.AddSection(SectionName); 
	Conf.ExchangeValueStr(SectionName, "DownloadType", m_DownloadType, dwFlags); 
	Conf.ExchangeValueStr(SectionName, "Description", m_Description, dwFlags); 
	Conf.ExchangeValueUINT64(SectionName, "MetaSize", (UINT64&)m_HrefSize, 10, dwFlags); 
	Conf.ExchangeValueStr(SectionName, "Href", m_Href, dwFlags); 
	Conf.ExchangeValueDWORD(SectionName, "TimeAdded", (DWORD&)m_TimeAdded, 10, dwFlags); 
	Conf.ExchangeValueDWORD(SectionName, "TimeCompleted", (DWORD&)m_TimeCompleted, 10, dwFlags); 
	Conf.ExchangeValueUINT64(SectionName, "Size", (UINT64&)m_FileSize, 10, dwFlags); 
	Conf.ExchangeValueUINT64(SectionName, "DurationMS", (duration_type&)m_DurationMS, 10, dwFlags); 
	Conf.ExchangeValueStr(SectionName, "Url-Seed", m_UrlSeed, dwFlags); 
	Conf.ExchangeValueStr(SectionName, "Url-Seed1", m_UrlSeed1, dwFlags); 
	Conf.ExchangeValueDWORD(SectionName, "Flags", m_dwFlags, 10, dwFlags); 
	FString DataFile = m_DataFile;

	Conf.ExchangeValueStr(SectionName, "DataFile", DataFile, dwFlags); 

	Conf.ExchangeValueStr(SectionName, "StreamType", m_StreamType, dwFlags); 
	Conf.ExchangeValueStr(SectionName, "Tracker", m_TrackerURL, dwFlags); 
	Conf.ExchangeValueStr(SectionName, "Content-Type", m_ContentType, dwFlags); 
	
	return TRUE; 
}

FDownloadItem* FDownload::AddNewClip()
{
	m_Clips.Add(new FDownloadItem(this));
	FDownloadItem* pItem = m_Clips[m_Clips.GetCount() - 1];

	pItem->m_videoID = m_Detail.m_VideoID;
	pItem->m_ClipIndex = m_Clips.GetCount() - 1; 
	pItem->m_DataPath = m_Detail.m_DataPath; 
	pItem->m_TorrentFile = g_AppSettings.MetafilePath(m_Detail.m_VideoID, pItem->m_ClipIndex, NULL); 
	pItem->m_DownloadType.MakeLower();
	return pItem; 
}


void FDownloadItem::ConvertToBittorrent()
{
	m_DownloadType = "torrent";
	m_TorrentFile = m_DataFile; 
	m_DataFile = "";
}

void FDownloadItem::OnSerialized()
{
	if (m_DownloadType == "http" && m_DataFile == "")
	{
		
		FString FileName = FileNameFromURL(m_Href);

		FileName.Trim();
		BOOL bHacked = FALSE;  //I know, I know.. I fucking hate this too...
		if (FileName.GetLength() > 0)
		{
			if (FileName.Find(".") == -1)
				FileName.Append(".vf");
		}
		else
		{
			//Older versions bug hack - filename saved as Movie Name\.vf
			FString TempPath = _PathCombine(m_DataPath, ".vf"); 
			if (PathFileExists(TempPath))
			{
				m_DataFile = TempPath; 
				bHacked = TRUE; 
			}
		}

		if (!bHacked)
		{
			FString DataFile = _PathCombine(m_DataPath, FileName); 

			if (FileName.GetLength() == 0 || PathFileExists(DataFile))
			{
				DataFile.Format("%s_%d.vf", StringToFileName(m_Video->m_Detail.m_VideoName), m_ClipIndex);
				m_DataFile = _PathCombine(m_DataPath, DataFile); 
			}
			else
			{
				m_DataFile = DataFile;
			}
		}
	}

	if (m_DownloadType == "torrent")
		m_DataFile = "";
	else
		m_TorrentFile = "";

	if (m_DownloadType == "stream")
		m_DataFile = m_Href; 
}

void FDownload::AdjustDataPath(FString& DataPath)
{
	if (m_Detail.m_DataPath.GetLength() == 0)
	{
		if (DataPath.GetLength() == 0)
			DataPath = g_AppSettings.m_DownloadsFolder; 
		if (m_RSSInfo.m_RSSName.GetLength() > 0)
			m_Detail.m_DataPath = _PathCombine(DataPath, StringToFileName(m_RSSInfo.m_RSSName));
		else
		{
			if (m_Detail.m_VideoName.GetLength() == 0)
				m_Detail.m_VideoName = "Untitled Video";

			m_Detail.m_DataPath = _PathCombine(DataPath, StringToFileName(m_Detail.m_VideoName)); 
		}
	}
}

BOOL FDownload::LoadConf(FIniConfig &Conf)
{
	if (Conf.IsLoaded())
	{

		size_t dwNumClips = 64; 
		FString DataPath = m_Detail.m_DataPath; 

		Serialize(Conf, FALSE); 

		AdjustDataPath(DataPath); 

		for (size_t k = 0; k < dwNumClips; k++)
		{
			FString SectionName; 
			SectionName.Format("Clip %d", k); 

			if (Conf.SectionExists(SectionName)){
				FDownloadItem* pItem = AddNewClip();
				if (pItem && pItem->Serialize(Conf, SectionName, FALSE))
					pItem->OnSerialized();
				else
				{
					//????? ERROR HANDLING ???
				}
			}
			else
				break; 
		}
		if (m_Clips.GetCount() == 0)
			m_StatusStr = "Load error";
		else
			m_StatusStr = "Loaded";
		return m_Clips.GetCount() > 0; 
	}
	return FALSE; 
}

BOOL FDownload::SaveConf(FIniConfig &Conf)
{
	if (Conf.IsLoaded() || 1)
	{
		Serialize(Conf, TRUE);
		Conf.ModifyValueDWORD("Video", "Clips", (DWORD)m_Clips.GetCount(), 10, TRUE);

		for (size_t k = 0; k < m_Clips.GetCount(); k++)
		{
			FDownloadItem& pItem = *m_Clips[k];
			FString SectionName; 
			SectionName.Format("Clip %d", k); 
			pItem.Serialize(Conf, SectionName, TRUE); 
		}
		return Conf.Save(); 
	}

	return FALSE; 
}

duration_type FDownload::GetAvailDuration(){

	duration_type rtAvail = 0; 
	for (size_t k = 0; k < m_Clips.GetCount(); k++)
	{
		FDownloadItem* pItem = m_Clips[k];
		if (pItem->IsFinished())
		{
			rtAvail+=pItem->m_DurationMS; 
		}
	}
	return rtAvail; 
}

size_t FDownload::GetAvailMedia(FMediaPlaylist &PlayList)
{
	size_t numAdded = 0; 
	PlayList.Clear(); 
	PlayList.m_VideoName = m_Detail.m_VideoName;

	for (size_t k = 0; k < m_Clips.GetCount(); k++)
	{
		FDownloadItem* pItem = m_Clips[k];
		if (pItem->IsFinished() && !(pItem->m_dwFlags & CLIP_NOT_FOUND))
		{
			pItem->GetMediaFiles(PlayList.m_Files); 
			PlayList.m_TotalDurationMS+=pItem->m_DurationMS;
			numAdded++; 
		}
	}
	return numAdded; 
}

size_t FDownload::GetAvailMediaCount()
{
	FMediaPlaylist aPlaylist; 
	GetAvailMedia(aPlaylist);
	return aPlaylist.m_Files.size(); 

}

size_type FDownload::ComputeSizeFromClips()
{
	size_type totalSize = 0;  
	for (size_t k = 0; k < m_Clips.GetCount(); k++)
	{
		FDownloadItem* pItem = m_Clips[k];
		totalSize+=pItem->m_FileSize;
	}
	return totalSize; 
}


BOOL FMediaPlaylist::GetTimeIndex(duration_type rtTime, size_t& nIndex, duration_type& rtOffset)
{
	size_t count = m_Files.size(); 

	duration_type rtTotal = 0; 
	//fix for 0-duration files
	if (count == 1)
	{
		nIndex = 0; 
		rtOffset = rtTime;
		return TRUE; 
	}
	for (size_t k = 0; k < count; k++)
	{
		if (rtTotal + m_Files[k].m_DurationMS >= rtTime)
		{
			rtOffset = rtTime - rtTotal;
			nIndex = k;
			return TRUE; 
		}
		else
			rtTotal+=m_Files[k].m_DurationMS; 
	}
	return FALSE; 
}

BOOL FDownload::IsDownloadableStream()
{
	return	m_Clips.GetCount() > 0 && 
			m_Clips[0]->m_DownloadType == "stream" && 
			m_Clips[0]->m_Href.Find("http://") == 0; //make sure it's not rtsp:// or something; must start with http://
}

BOOL FDownload::IsStream()
{
	return m_Clips.GetCount() > 0 &&
			m_Clips[0]->m_DownloadType == "stream";
}

//////////////////////////////////////////////////////////////////////////

BOOL FDownloadEx::SaveToDisk()
{
	return SaveConf(m_Conf); 
}

BOOL FDownloadEx::LoadFromDisk(const tchar* pszFileName, const tchar* pszDataDir)
{
	if (m_Conf.Load(pszFileName))
	{
		m_Detail.m_DataPath = pszDataDir; 
		return LoadConf(m_Conf); 
	}
	return FALSE; 
}

BOOL FDownloadEx::CreateDownload(FDownloadCreateInfo& pInfo)
{
	m_Detail.m_VideoName = pInfo.m_VideoName;
	m_Detail.m_VideoID = pInfo.videoID; 
	m_Detail.m_TotalSize = pInfo.m_Size;
	m_Detail.m_EpisodeID = 0;
	m_Detail.m_TimeAdded = time(NULL); 
	m_Detail.m_TimePublished = pInfo.m_TimePublished; 
	m_RSSInfo.m_Guid = pInfo.m_GUID; 
	
	FDownloadItem* pItem = AddNewClip();
	pItem->m_DownloadType = pInfo.m_DownloadType; 
	pItem->m_Href = pInfo.m_URL; 
	pItem->m_Description = pInfo.m_Description; 
	StripHTMLTags(pItem->m_Description.GetBuffer()); 
	pItem->m_Description.Replace("\"", "'");
	pItem->m_Description.Trim();

	FString mtti;
	mtti.Format("%u_tmp.mtti", m_Detail.m_VideoID);
	m_Conf.m_FileName = _PathCombine(g_AppSettings.m_IndexPath, mtti); 
	return SaveConf(m_Conf); 
}

FString FDownload::IndexPath()
{
	FString IndexPath; 
	IndexPath.Format("%u.vf", m_Detail.m_VideoID);
	return _PathCombine(g_AppSettings.m_IndexPath, IndexPath); 
}

BOOL FDownloadEx::CreateFromFileName(const tchar* pszFileName)
{
	_DBGAlert("Creating from filename: %s\n", pszFileName); 
	FString VideoName = pszFileName; 
	PathStripPath(VideoName.GetBuffer()); 

	if (PathIsDirectory(VideoName))
		return FALSE; 

	m_Detail.m_VideoName = VideoName; 
	m_Detail.m_VideoID = g_AppSettings.GetRandomVideoID();
	m_Detail.m_DataPath = pszFileName; 
	PathRemoveFileSpec(m_Detail.m_DataPath.GetBuffer()); 

	FDownloadItem* pItem = new FDownloadItem(this); 
	pItem->m_Href = pszFileName; 
	m_Clips.Add(pItem); 
	return FALSE; 
}

FDownloadEx::FDownloadEx( )
{
	m_ProgressCount = 0;
}
//////////////////////////////////////////////////////////////////////////

#ifdef USE_XML

HRESULT FDownload::LoadFromXML(IXMLDOMNode* pTag)
{
	HRESULT hr = E_FAIL; 
	if (pTag)
	{
		m_pXMLNode = pTag; 
		hr = S_OK; 
		FXMLNode pNode = pTag;

		m_Detail.m_VideoID =			g_AppSettings.GetRandomVideoID(); 
		m_Detail.m_EpisodeID =			pNode.GetAttribute("EPISODEID").ToULong();
		m_Detail.m_VideoName =			pNode.GetChild("TITLE").GetText().ToString(); 
		m_Detail.m_DetailURL =			pNode.GetChild("MOREINFO").GetAttribute("HREF").ToString(); 
		//Labels will be stored in Labels.xml
		m_Detail.m_TotalDurationMS =	pNode.GetChild("DURATION").GetAttribute("VALUE").ToUINT64(); 
		m_Detail.m_DataPath	=			pNode.GetChild("STORAGE").GetAttribute("PATH").ToString(); 
		m_Detail.m_TotalSize =			pNode.GetChildEx("STORAGE").GetAttribute("TOTALSIZE").ToUINT64(); 
		m_Detail.m_ImageURL =			pNode.GetChild("IMAGE").GetAttribute("THUMBNAIL").ToString(); 
		FXMLNode pDateNode =			pNode.GetChild("DATE");
		if (pDateNode)
		{
			m_Detail.m_TimeAdded =			pDateNode.GetAttribute("ADDED").ToULong();
			m_Detail.m_TimePublished =		pDateNode.GetAttribute("PUBLISHED").ToULong();
			m_Detail.m_TimeCompleted =		pDateNode.GetAttribute("COMPLETED").ToULong();
		}
		m_Detail.m_PlaybackPosMS =		pNode.GetChild("PLAYBACK").GetAttribute("STOPPED").ToUINT64(); 
		m_Detail.m_Watched		 =		pNode.GetChild("PLAYBACK").GetAttribute("WATCHED").ToUINT64(); 
		m_dwFlags				 =		pNode.GetChild("FLAGS").GetAttribute("VALUE").ToULong(10); 
		m_Detail.m_VideoFlags	 =		pNode.GetChild("FLAGS").GetAttribute("VIDEOFLAGS").ToULong(10); 
		m_Detail.m_SubURL		 =		pNode.GetChild("SUBTITLES").GetAttribute("HREF").ToString(); 
		m_Detail.m_Description	 =		pNode.GetChild("ABSTRACT").GetText().ToString();

		FXMLNode pRssInfo		 =		pNode.GetChild("RSS");
		m_RSSInfo.m_dwFlags		 =		pRssInfo.GetAttribute("FLAGS").ToULong(10);
		m_RSSInfo.m_Guid		 =		pRssInfo.GetChild("ITEM").GetAttribute("GUID").ToULong(10); 
		m_RSSInfo.m_ItemURL		 =		pRssInfo.GetChild("ITEM").GetAttribute("URL").ToString(); 
		m_RSSInfo.m_RSSGuid		 =		pRssInfo.GetChild("FEED").GetAttribute("GUID").ToULong(10); 
		m_RSSInfo.m_RSSURL		 =		pRssInfo.GetChild("FEED").GetAttribute("HREF").ToString(); 
		m_RSSInfo.m_RSSName		 =		pRssInfo.GetChild("FEED").GetAttribute("NAME").ToString(); 

		CComPtr<IXMLDOMNodeList> pEntries; 
		hr = pTag->selectNodes(L"ENTRY", &pEntries);
		if (pEntries)
		{
			hr = LoadEntries(pEntries);
		}
	}
	return hr; 
}

HRESULT LoadUrlSeeds(IXMLDOMNode* pRef, FDownloadItem* pClip)
{
	HRESULT hr = E_FAIL; 
	if (pRef)
	{
		CComPtr<IXMLDOMNodeList> pUrlSeeds;
		hr = pRef->selectNodes(L"URLSEED", &pUrlSeeds);
		if (pUrlSeeds)
		{
			int count = 0; 
			CComPtr<IXMLDOMElement> pUrlSeed; 
			for (int count = 0; count < 2; count++)
			{
				FXMLUtil::GetNextElement(pUrlSeeds, pUrlSeed);
				if (pUrlSeed)
				{
					FXMLNode pUrlSeedNode = pUrlSeed; 
					FString& StrOut = count == 0 ? pClip->m_UrlSeed: pClip->m_UrlSeed1;
					StrOut = pUrlSeedNode.GetAttribute("HREF").ToString(); 
				}
			}
		}
	}
	return hr; 
}

HRESULT FDownload::LoadEntries(IXMLDOMNodeList* pEntries)
{
	HRESULT hr = E_FAIL; 

	if (pEntries)
	{
		for (;;)
		{
			CComPtr<IXMLDOMElement> pEntry; 
			hr = FXMLUtil::GetNextElement(pEntries, pEntry);
			if (pEntry)
			{
				LoadEntry(pEntry); 
			}
			else
				break; 
		}
	}
	return hr; 
}

HRESULT FDownload::LoadEntry(IXMLDOMNode* pEntry)
{
	HRESULT hr = E_FAIL; 
	if (pEntry)
	{
		FXMLNode pEntryNode = pEntry; 
		CComPtr<IXMLDOMNodeList> pRefs; 
		hr = pEntry->selectNodes(L"REF", &pRefs);
		if (pRefs)
		{
			for (;;)
			{
				CComPtr<IXMLDOMElement> pRef; 
				FXMLUtil::GetNextElement(pRefs, pRef);
				if (pRef)
				{
					FXMLNode pRefNode = pRef; 

					FDownloadItem* pClip = AddNewClip();
					pClip->m_pXMLNode = pRef; 
					pClip->m_DownloadType = pRefNode.GetAttribute("TYPE").ToString(); 
					pClip->m_Description  = pEntryNode.GetChild("ABSTRACT").GetText().ToString();
					pClip->m_Href		  = pRefNode.GetAttribute("HREF").ToString(); 
					pClip->m_TorrentFile  = pRefNode.GetAttribute("LOCALFILE").ToString(); 
					pClip->m_HrefSize     = pRefNode.GetAttribute("SIZE").ToUINT64();
					pClip->m_TimeCompleted= pRefNode.GetChild("DATE").GetAttribute("COMPLETED").ToULong(10);
					pClip->m_TimeAdded	  = pRefNode.GetChild("DATE").GetAttribute("ADDED").ToULong(10); 
					pClip->m_DurationMS	  = pRefNode.GetChild("DURATION").GetAttribute("VALUE").ToUINT64(10); 
					pClip->m_dwFlags	  = pRefNode.GetAttribute("FLAGS").ToULong(10); 
					pClip->m_StreamType	  = pRefNode.GetAttribute("MEDIATYPE").ToString();
					pClip->m_TrackerURL	  = pRefNode.GetChild("TRACKER").GetAttribute("HREF").ToString();
					LoadUrlSeeds(pRef, pClip); 
					pClip->OnSerialized(); 
				} else
					break; 
			}
		}
	}
	return hr; 
}


HRESULT SaveClip(FDownloadItem* pClip)
{
	FXMLNode pNode = pClip->m_pXMLNode;
	if (pNode)
	{
		pNode.SetAttribute("TYPE", pClip->m_DownloadType);
		pNode.SetAttribute("HREF", pClip->m_Href); 
		pNode.SetAttribute("LOCALFILE", pClip->m_TorrentFile);
		pNode.SetAttribute("FLAGS", pClip->m_dwFlags); 
		pNode.SetAttribute("SIZE", (UINT64)pClip->m_HrefSize);
		FXMLNode pDateNode = pNode.GetChildEx("DATE");
		pDateNode.SetAttribute("COMPLETED", (ULONG)pClip->m_TimeCompleted); 
		pDateNode.SetAttribute("ADDED", (ULONG)pClip->m_TimeAdded); 
		pNode.GetChildEx("DURATION").SetAttribute("VALUE", pClip->m_DurationMS);
		pNode.SetAttribute("MEDIATYPE", pClip->m_StreamType); 

	}
	return S_OK; 
}

HRESULT FDownload::SaveToXML(IXMLDOMNode* pTag, BOOL bOutputAllNodes)
{
	HRESULT hr = E_FAIL; 
	
	if (pTag)
	{
		
		FXMLNode pNode = pTag; 
		
		pNode.SetAttribute("EPISODEID", m_Detail.m_EpisodeID); 
		pNode.GetChildEx("TITLE").SetText(m_Detail.m_VideoName); 
		pNode.GetChildEx("DURATION").SetAttribute("VALUE", m_Detail.m_TotalDurationMS); 
		pNode.GetChildEx("STORAGE").SetAttribute("PATH", m_Detail.m_DataPath); 
		pNode.GetChildEx("STORAGE").SetAttribute("TOTALSIZE", (UINT64)m_Detail.m_TotalSize);
		pNode.GetChildEx("ABSTRACT").SetText(m_Detail.m_Description); 
		pNode.GetChildEx("IMAGE").SetAttribute("THUMBNAIL", m_Detail.m_ImageURL); 
		if (bOutputAllNodes)
		{
			pNode.GetChildEx("MOREINFO").SetAttribute("HREF", m_Detail.m_DetailURL); 
		}
		//MOREINFO (Description) is constant
		FXMLNode pDateNode = pNode.GetChildEx("DATE");
		if (pDateNode)
		{
			pDateNode.SetAttribute("ADDED",		(ULONG)m_Detail.m_TimeAdded); 
			pDateNode.SetAttribute("COMPLETED", (ULONG)m_Detail.m_TimeCompleted); 
			//PUBLISHED is constant
		}
		pNode.GetChildEx("PLAYBACK").SetAttribute("STOPPED", m_Detail.m_PlaybackPosMS); 
		pNode.GetChildEx("PLAYBACK").SetAttribute("WATCHED", m_Detail.m_Watched); 
		pNode.GetChild("FLAGS").SetAttribute("VALUE", m_dwFlags);
		pNode.GetChild("FLAGS").SetAttribute("VIDEOFLAGS", m_Detail.m_VideoFlags); 

		FXMLNode pRssInfo = pNode.GetChildEx("RSS");
		if (pRssInfo)
		{
			pRssInfo.SetAttribute("FLAGS", m_RSSInfo.m_dwFlags);
			pRssInfo.GetChildEx("ITEM").SetAttribute("GUID", m_RSSInfo.m_Guid); 
			pRssInfo.GetChildEx("ITEM").SetAttribute("URL",  m_RSSInfo.m_ItemURL); 
			pRssInfo.GetChildEx("FEED").SetAttribute("GUID", m_RSSInfo.m_RSSGuid);
			pRssInfo.GetChildEx("FEED").SetAttribute("HREF", m_RSSInfo.m_RSSURL);
			pRssInfo.GetChildEx("FEED").SetAttribute("NAME", m_RSSInfo.m_RSSName);
		}

		FXMLNode pEntry = pNode.GetChildEx("ENTRY");
		

		for (size_t k = 0; k < m_Clips.GetCount(); k++)
		{
			if (m_Clips[k]->m_pXMLNode == NULL)
			{
				CComPtr<IXMLDOMElement> pElement; 
				CComPtr<IXMLDOMDocument> pDoc; 
				if (SUCCEEDED(pTag->get_ownerDocument(&pDoc)))
				{
					if (SUCCEEDED(pDoc->createElement(L"REF", &pElement)))
					{
						if (SUCCEEDED(pEntry.m_pNode->appendChild(pElement, NULL)))
							m_Clips[k]->m_pXMLNode = pElement; 
						
					}
					else
						continue; 
				}
			}
			if (m_Clips[k]->m_pXMLNode)
				SaveClip(m_Clips[k]); 
		}

		hr = S_OK; 
	}
	return hr; 
}

#endif