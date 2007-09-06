#include "stdafx.h"
#include "FDownloadMgr.h"
#include "FMainFrame.h"
#include "GlobalObjects.h"
#include "FJScriptArrayBuilderA.h"

extern FAutoWindow<FMainFrame> g_MainFrame; 

BOOL FDownloadMgr::_GetItemDownloadStatus(FDownload* pDownload, IStatusArrayBuilder* pJSArray, FDownloadProgress& ItemProgress, DWORD dwGetFlags)
{
	if (pDownload->m_dwFlags & FLAG_DOWNLOAD_DELETED)
		return FALSE; 


	FDownloadEx* pExDownload = (FDownloadEx*)pDownload;

	size_type aCurClipSize = 0; 

	FString aStatus; 
	int nStatus = 1;	
	double fTotalUp = 0.0f;
	double fTotalDown = 0.0f;

	long lLoaded = 0; 

	if (g_MainFrame.IsObjectAndWindow())
		lLoaded = g_MainFrame->GetActiveVideo() == pDownload->GetVideoID();

	BOOL bPlaying = 0; 

	if (lLoaded != 0)
	{
		IVideoPlayer* pPlayer = g_MainFrame->GetPlayer();
		if (pPlayer)
			bPlaying = pPlayer->IsPlaying(); 
	}

	size_t iIndex = pDownload->m_CurClipIndex > 0 ? pDownload->m_CurClipIndex - 1 : 0;

	FDownloadItem* curClip = pDownload->m_Clips[iIndex];
	FDownloadProgress OldProgress;
	FDownloadProgress TotalDownProgress; 
	FDownloadProgress DownProgress;

	//Compute total progress
	typedef CAtlMap<FDownloadHandle, FDownloadProgress>::CPair FProgressPair; 
	for (size_t k = 0; k < pDownload->m_Clips.GetCount(); k++)
	{
		FProgressPair *aPair = m_Progress.Lookup(pDownload->m_Clips[k]->m_DownloadHandle);
		if (aPair != NULL)
		{
			TotalDownProgress+=aPair->m_value; 
			if (k == iIndex)
				DownProgress = aPair->m_value; 
		}
	}

	if (curClip->m_DownloadType == "http" && DownProgress.m_bytesTotal > 0)
	{
		if (curClip->m_FileSize < DownProgress.m_bytesTotal || pDownload->m_Detail.m_TotalSize < DownProgress.m_bytesTotal)
		{
			curClip->m_FileSize = DownProgress.m_bytesTotal;
			pDownload->m_Detail.m_TotalSize = pDownload->ComputeSizeFromClips();
			SaveDownload(pDownload);
		}
	}

	if (pExDownload->m_LastProgress.m_bytesTotal == 0 && pExDownload->m_LastProgress.m_bytesDownloaded == 0)
	{
		pExDownload->m_LastProgress = TotalDownProgress; 
		pExDownload->m_LastProgress.m_LastTick = GetTickCount(); 
	}

	DWORD dwTicked = GetTickCount() - pExDownload->m_LastProgress.m_LastTick;

	fTotalDown = (double)(TotalDownProgress.m_bytesDownloaded - pExDownload->m_LastProgress.m_bytesDownloaded);
	fTotalUp = (double)(TotalDownProgress.m_bytesUploaded - pExDownload->m_LastProgress.m_bytesUploaded);

	if (dwTicked > 1000)
	{
		 float fSecs = dwTicked / 1000.0f;
		 fTotalUp = fTotalUp / fSecs; 
		 fTotalDown = fTotalDown / fSecs; 
	}

	if (dwTicked > 10000)
	{
		pExDownload->m_LastProgress = TotalDownProgress;
		pExDownload->m_LastProgress.m_LastTick = GetTickCount();
	}

	size_type totalSize = pDownload->m_Detail.m_TotalSize; 

	if (totalSize == 0)
		totalSize = pDownload->ComputeSizeFromClips();

	if (totalSize < TotalDownProgress.m_bytesDownloaded)
	{
		pDownload->m_Detail.m_TotalSize = TotalDownProgress.m_bytesTotal;
		totalSize = TotalDownProgress.m_bytesTotal;
	}

	if (totalSize == 0)
		totalSize = TotalDownProgress.m_bytesTotal; 
	else
	{
		if (totalSize > TotalDownProgress.m_bytesTotal && TotalDownProgress.m_bytesTotal > 0 && pDownload->m_Clips.GetCount() == 1)
			pDownload->m_Detail.m_TotalSize = TotalDownProgress.m_bytesTotal; 
	}

	REFERENCE_TIME rtAvail = pDownload->GetAvailDuration();

	rtAvail*=DURATION_MULT;

	//----Adjustments
	if (fTotalDown < 1024.0f)
		fTotalDown = 0.0f; 

	if (fTotalUp < 1024.0f * 10)	//10 K min
		fTotalUp = 0.0f;

	int iCanPlay = iIndex > 0; 

	BOOL bIsFinished = (pDownload->m_dwFlags & FLAG_DOWNLOAD_FINISHED) != 0; 
	if ( bIsFinished && pDownload->m_StatusStr != "Finished")
		pDownload->m_StatusStr = "Finished";

	aStatus = pDownload->m_StatusStr; 

	if (pDownload->m_Clips.GetCount() > 0 && pDownload->m_Clips[0]->m_DownloadType == "stream")
	{
		aStatus = "Stream ready";
		iCanPlay = 1; 
	}

	if (bIsFinished)
	{
		//Do not send download speed/download size for finished downloads
		//This is usually a residual speed caused by bittorrent/tracker interaction
		iCanPlay = 1; 
	}

	//--------------------------------
	FString videoName = pDownload->m_Detail.m_VideoName; 
	videoName.Replace(',','-');

	pJSArray->NewElement(); 

	pJSArray->AddValue("videoID", "%u", pDownload->GetVideoID()); //				
	pJSArray->AddValueStr("videoName", videoName);
	pJSArray->AddValueStr("videoStatusStr",	aStatus); 
	pJSArray->AddValueUINT64("videoTotalSize", totalSize); 
	pJSArray->AddValueUINT64("videoDownloaded",	pDownload->m_Detail.m_SizeDownloaded);
	pJSArray->AddValueUINT64("videoTotalDuration", pDownload->m_Detail.m_TotalDurationMS * DURATION_MULT); 
	pJSArray->AddValueUINT64("videoAvailDuration", rtAvail); 
	pJSArray->AddValueUINT("videoTimeAdded", (UINT)pDownload->m_Detail.m_TimeAdded); 
	pJSArray->AddValueUINT("videoTimeCompleted", (UINT)pDownload->m_Detail.m_TimeCompleted); 
	pJSArray->AddValueUINT("videoNumClips",	(UINT)pDownload->m_Clips.GetCount()); 
	pJSArray->AddValueDouble("videoTotalUpSpeed", fTotalUp);
	pJSArray->AddValueDouble("videoTotalDownSpeed", fTotalDown); 
	pJSArray->AddValueUINT("videoIsLoaded",	lLoaded);
	pJSArray->AddValueUINT("videoIsPlaying", bPlaying);
	pJSArray->AddValueUINT("videoFlags", pDownload->m_dwFlags); 
	pJSArray->AddValueUINT64("videoWatched", pDownload->m_Detail.m_Watched); 
	pJSArray->AddValueUINT("episodeID",	pDownload->m_Detail.m_EpisodeID); 
	pJSArray->AddValueUINT("showID", pDownload->m_Detail.m_ShowID); 
	pJSArray->AddValueUINT("videoCanPlay", iCanPlay);
	pJSArray->AddValueUINT("curClipID",	(UINT)pDownload->m_CurClipIndex - 1); 
	pJSArray->AddValueUINT64("curClipSize",	curClip->m_FileSize); 
	pJSArray->AddValueUINT64("curClipDownloaded", DownProgress.m_bytesDownloaded); 
	pJSArray->AddValueDouble("curClipDownSpeed", DownProgress.m_bytesPerSec);
	pJSArray->AddValueDouble("curClipUpSpeed", DownProgress.m_upBytesPerSec > 10 * 1024.0 ? DownProgress.m_upBytesPerSec : 0.0f);
	pJSArray->AddValueUINT("curClipSeeds", 0); //DownProgress.m_Seeds
	pJSArray->AddValueUINT("curClipPeers",	0); //DownProgress.m_Peers 
	pJSArray->AddValueUINT("curClipComplete",	1); 
	pJSArray->AddValueUINT("curClipIncomplete",	1); 
	pJSArray->AddValueUINT("curClipState",	curClip->m_DownloadStatus);
	pJSArray->AddValueUINT("RSSFlags",	pDownload->m_RSSInfo.m_dwFlags); 
	pJSArray->AddArrayFromString(pDownload->m_Detail.m_Labels.GetLabelStr());

	if (dwGetFlags & STATUS_FLAG_FIELD_DESCRIPTION)
	{
		pJSArray->AddValueStr("VideoDescription", pDownload->m_Detail.m_Description); 
	}
	ItemProgress = DownProgress; 
	return TRUE; 
}

BOOL FDownloadMgr::GetDownloadStatusString(vidtype videoID, IStatusArrayBuilder* pJSArray)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDownload = FindByID(videoID); 
	if (NULL == pDownload)
		return FALSE;
	FDownloadProgress ItemProgress; 
	return _GetItemDownloadStatus(pDownload, pJSArray, ItemProgress, 0); 
}


enum SortKeys
{
	byNone, 
	byDate, 
	byName, 
	byStatus
};

static SortKeys g_SortKey = byDate; 
static int		g_SortDir = 0; //0 ascending; 1 = descending


double SafeDiv(size_type size1, size_type size2, double dblZero = 0.0)
{
	if (size2 > 0)
		return (double)size1 / (double)size2; 
	return dblZero; 
}

int CompareFn( const void *arg1, const void *arg2 )
{
	FDownloadEx* pItem1 = NULL; 
	FDownloadEx* pItem2 = NULL; 
	pItem1 = *(FDownloadEx**)arg1; 
	pItem2 = *(FDownloadEx**)arg2; 

	int nCmp = 0; 
	switch(g_SortKey)
	{
	case byName:
		nCmp = _stricmp(pItem1->m_Detail.m_VideoName, pItem2->m_Detail.m_VideoName);
		break; 
	case byStatus:
		{
			size_type diff = pItem1->m_Detail.m_TotalSize -
				 pItem2->m_Detail.m_TotalSize;
			nCmp = diff > 0 ? 1 : -1;
		}
		break;
	default:
		nCmp = (int)(pItem2->m_Detail.m_TimeAdded - pItem1->m_Detail.m_TimeAdded);
		break; 
	}	
	//0 = asc; 1 = desc
	return g_SortDir == 0 ? -nCmp : nCmp; 
}

BOOL FDownloadMgr::_GetDownloadStatus(DownloadStatusOptions& pOptions, IStatusArrayBuilder* pJSArray)
{
	SynchronizeThread(m_Sync);

	const char* pszSort = pOptions.m_Sort; 
	const char* pszSortDir = pOptions.m_SortDir; 
	const char* pszName = pOptions.m_Filter; 
	DWORD dwFlags = pOptions.m_dwFlags; 
	const char* pszLabel = pOptions.m_Label; 

	m_ItemSums.Clear(); 
	//Copy all download pointers to a temporary array so that we can sort the items 

	size_t itemCount = m_Downloads.GetCount(); 
	FDownloadEx** Downloads = new FDownloadEx* [itemCount]; 

	for (size_t k = 0; k < itemCount; k++)
		Downloads[k] = m_Downloads[k];

	if (pszSort != NULL && strlen(pszSort) > 0)
	{
		g_SortKey = byDate; 
		if (stricmp(pszSort, "name") == 0)
			g_SortKey = byName; 
		else
			if (stricmp(pszSort, "status") == 0)
				g_SortKey = byStatus;

		if (pszSortDir != NULL && stricmp(pszSortDir, "desc") == 0)
			g_SortDir = 1; //descending
		else
			g_SortDir = 0; //ascending

		qsort(&Downloads[0], itemCount, sizeof(FDownloadEx*), CompareFn);
	}

	size_t nameLen = pszName ? strlen(pszName) : 0;
	size_t labelLen = pszLabel ? strlen(pszLabel) : 0; 

	if (labelLen == 0)
	{
		pszLabel = "all";
		labelLen = 3; 
	}

	int nCount = g_Objects._ClipDownloader->GetTotalProgress(m_Progress); 

	pJSArray->New();
	size_type totalFileSize = 0; 
	size_t    nWatched = 0; 
	size_t	  numItems = 0; 
	size_t	  numItemsAdded = 0; 
	FDownloadProgress TotalProgress; 

	for (size_t k = 0; k < itemCount; k++)
	{
		FDownload* pDownload = Downloads[k];

		BOOL bGetStatus = !(pDownload->m_dwFlags & FLAG_DOWNLOAD_DELETED);

		if (bGetStatus && labelLen > 0)
		{
			//Counts; 

			m_ItemSums.m_dwAll++; 

			if (pDownload->m_dwFlags & FLAG_DOWNLOAD_QUEUED)
				m_ItemSums.m_dwQueued++; 

			if (pDownload->m_Detail.m_Watched == 0 && pDownload->IsDownloadFinished())
				m_ItemSums.m_dwUnwatched++; 

			if (pDownload->m_dwFlags & FLAG_DOWNLOAD_ACTIVE)
				m_ItemSums.m_dwInProgress++;

			if (pDownload->m_RSSInfo.m_dwFlags & RSS_FLAG_FROM_RSS)
				m_ItemSums.m_dwSubscribed++; 
			
			if (pDownload->IsStream())
				m_ItemSums.m_dwStreams++; 

			//Labels
			if (stricmp (pszLabel, "all") == 0)
			{
				bGetStatus = TRUE; 

				if (dwFlags & STATUS_FLAG_HIDE_QUEUED && (pDownload->m_dwFlags & FLAG_DOWNLOAD_QUEUED))
					bGetStatus = FALSE; 
				else
				if (dwFlags & STATUS_FLAG_HIDE_INPROGRESS && !pDownload->IsDownloadFinished() && !(pDownload->m_dwFlags & FLAG_DOWNLOAD_QUEUED))
					bGetStatus = FALSE; 
			}
			else
			if (stricmp(pszLabel, "queued") == 0)
				bGetStatus = pDownload->m_dwFlags & FLAG_DOWNLOAD_QUEUED; 
			else
			if (stricmp(pszLabel, "new") == 0)
				bGetStatus = pDownload->m_Detail.m_Watched == 0 && pDownload->IsDownloadFinished();
			else
			if (stricmp(pszLabel, "unlabeled") == 0)
				bGetStatus = pDownload->m_Detail.m_Labels.GetItemCount() == 0; 
			else
			if (stricmp(pszLabel, "downloading") == 0)
				bGetStatus = (pDownload->m_dwFlags & FLAG_DOWNLOAD_ACTIVE) && !(pDownload->m_dwFlags & FLAG_DOWNLOAD_QUEUED);
			else
			if (stricmp(pszLabel, "downloaded") == 0)
				bGetStatus = (pDownload->m_dwFlags & FLAG_DOWNLOAD_FINISHED) != 0; 
			else
			if (stricmp(pszLabel, "subscribed") == 0)
				bGetStatus = (pDownload->m_RSSInfo.m_dwFlags & RSS_FLAG_FROM_RSS) != 0; 
			else
			if (stricmp(pszLabel, "streams") == 0)
				bGetStatus = pDownload->IsStream(); 
			else
				bGetStatus = pDownload->m_Detail.m_Labels.HasLabel(pszLabel); 

			//bGetStatus = mzRegMatch::IsMatchEx(pszLabel, pDownload->m_Detail.m_Labels, true, true);
		}

		if (bGetStatus && nameLen > 0)
		{
			bGetStatus	 = (StrStrI(pDownload->m_Detail.m_VideoName, pszName) != NULL);
		}

		if (bGetStatus)
		{
			numItems++; 
			size_type itemSize = pDownload->m_Detail.m_TotalSize; 
			if (itemSize == 0)
				itemSize = 	pDownload->ComputeSizeFromClips();
			totalFileSize+= itemSize; 
			if (pDownload->m_Detail.m_Watched > 0)
				nWatched++; 


			if (numItems >= (size_t)pOptions.m_StartIndex)
			{
				if (pOptions.m_Count > 0 && numItemsAdded >= (size_t)pOptions.m_Count)
					continue; //we don't break, because we want to find compute the total count
				FDownloadProgress ItemProgress; 
				_GetItemDownloadStatus(pDownload, pJSArray, ItemProgress, dwFlags);
				TotalProgress+=ItemProgress; 
				numItemsAdded++; 
			}
		}

	}

	delete[] Downloads; 

	pOptions.m_TotalCount = numItems; 

	if (dwFlags & STATUS_FLAG_COLLECTION)
	{
		FString StatusText; 
		StatusText.Format("%d videos (%d unwatched); %s total used space\n", numItems, numItems - nWatched, SizeString(totalFileSize));
		//StatusText.Format("%d active (%d max)", m_ActiveDownloads, g_AppSettings.m_MaxDownloads);
		if (m_pStatusBar)
			m_pStatusBar->SetStatusText(0, StatusText, VIEW_STATUS); 
	}
	return TRUE;
}


BOOL FDownloadMgr::GetDownloadProgress(vidtype videoID, FDownloadProgress& aProgress)
{
	return g_Objects._ClipDownloader->GetTotalProgress(videoID, aProgress);
}

BSTR FDownloadMgr::GetDownloadStatusStr(const tchar* pszNameFilter, const tchar* pszLabelFilter, const tchar* pszSort, const tchar* pszSortDir)
{
	return NULL; 
}

BOOL FDownloadMgr::GetDownloadStatus(DownloadStatusOptions& pOptions, IStatusArrayBuilder* pOutArray)
{
	if (m_bStopping)
		return FALSE; 

	SynchronizeThread(m_Sync); 
	return _GetDownloadStatus(pOptions, pOutArray);
}

FDownload FDownloadMgr::GetDownloadInfo(vidtype videoID)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDownload = FindByID(videoID); 
	if (NULL != pDownload)
	{
		return FDownload(*pDownload);
	}
	return FDownload(); 
}

FIniConfig FDownloadMgr::GetDownloadConf(vidtype videoID)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDownload = FindByID(videoID); 
	if (pDownload)
	{
		FDownloadEx* pDownEx = (FDownloadEx*)(pDownload);
		return pDownEx->m_Conf;
	}
	return FIniConfig();
}
