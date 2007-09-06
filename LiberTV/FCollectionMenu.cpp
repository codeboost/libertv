#include "stdafx.h"
#include "GlobalObjects.h"
#include "FCollectionMenu.h"
#include "FMenu.h"
#include "FMainFrame.h"
#include "FDlgInputBox.h"

extern FAutoWindow<FMainFrame> g_MainFrame; 

HMENU FCollectionMenu::CreateLabelsSubmenu(FLabels& Labels, int nStartID)
{
	HMENU hMenu = CreateMenu(); 

	int nID = nStartID; 

	_InsertMenuItem(hMenu, 0, nID++, "New Label..."); 

	m_zeLabels.RemoveAll(); 

	g_Objects._LabelManager.GetLabels(m_zeLabels);

	if (m_zeLabels.GetCount() > 0)
	{
		_InsertMenuSeparator(hMenu, 1); 

		for (size_t k = 0; k < m_zeLabels.GetCount(); k++)
		{
			FString &StrLabel = m_zeLabels[k];
			BOOL bChecked = Labels.HasLabel(StrLabel); 
			_InsertCheckedMenuItem(hMenu, k + 2, nID++, bChecked, StrLabel); 
		}
	}
	return hMenu;
}

enum MenuItems
{
	miNone,
	miPlay, 
	miSuspend, 
	miDelete, 
	miRevealFiles,
	miRename, 
	miRevealMTTI, 
	miStart, 
	miInfo
};

void CopyLabels(FLabels& Res, FLabels& Source)
{
	Res+=Source; 
}

size_t BuildDownloadArray(FArray<FDownload> &DownArray, const FArray<FString>& aVids)
{
	for (size_t k = 0; k < aVids.GetCount(); k++)
	{
		vidtype videoID = strtoul(aVids[k], NULL, 10);
		FDownload pVideo = g_Objects._DownloadManager.GetDownloadInfo(videoID);
		if (pVideo.IsValid())
		{
			DownArray.Add(pVideo);
		}
	}
	return DownArray.GetCount(); 
}

/*
Creates a Union between all the labels found in DownArray.
Returns All labels in AllLabels and labels which match for all videos in CommonLabels
*/

void BuildLabelsUnion(FArray<FDownload>& DownArray, FLabels& AllLabels, FLabels& CommonLabels)
{
	for (size_t k = 0; k < DownArray.GetCount(); k++)
	{
		CopyLabels(AllLabels, DownArray[k].m_Detail.m_Labels); 
	}

	size_t j = 0; 
	for (size_t k = 0; k < AllLabels.GetItemCount(); k++)
	{
		const FString& StrLabel = AllLabels.GetLabelAt(k); 
		for ( j = 0; j < DownArray.GetCount(); j++)
		{
			if (!DownArray[j].m_Detail.m_Labels.HasLabel(StrLabel))
				break; 
		}

		if (j == DownArray.GetCount())
		{
			CommonLabels.AddLabel(StrLabel);
		}
	}
}

int FCollectionMenu::ShowContextMenu(HWND hWndParent, const FArray<FString> &aVids)
{
	FArray<FDownload> aVideos; 
	size_t videoCount = BuildDownloadArray(aVideos, aVids); 

	if (videoCount == 0)
		return 0; 


	vidtype videoID = aVideos[0].m_Detail.m_VideoID; 
	FDownload &pVideo = aVideos[0];

	HMENU hmenu = CreatePopupMenu();
	int i = 0; 

	if (videoCount == 1 )
	{
		if (pVideo.m_dwFlags & FLAG_DOWNLOAD_FINISHED)
			_InsertMenuItem(hmenu, i++, miPlay, "Play");

		if (!pVideo.IsDownloadFinished())
		{
			if (pVideo.m_dwFlags & FLAG_DOWNLOAD_QUEUED )
				_InsertMenuItem(hmenu, i++, miStart, "Start Download");
			else
				_InsertMenuItem(hmenu, i++, miSuspend, pVideo.IsPaused() ? "Resume Download" : "Suspend Download");
		}
		else
		{
			if (pVideo.IsDownloadableStream())
				_InsertMenuItem(hmenu, i++, miStart, "Download Video");
		}

		_InsertMenuItem(hmenu, i++, miInfo, "Info...");
		_InsertMenuItem(hmenu, i++, miRename, "Rename");
	}

	_InsertMenuItem(hmenu, i++, miDelete, "Delete");
	_InsertMenuSeparator(hmenu, i++); 


	FLabels aAllLabels; 
	FLabels aCommonLabels; 
	BuildLabelsUnion(aVideos, aAllLabels, aCommonLabels); 

	HMENU hSub = CreateLabelsSubmenu(aCommonLabels, 100); 

	_InsertSubMenu(hmenu, i++, "Label", hSub); 

	if (videoCount == 1)
		_InsertMenuItem(hmenu, i++, miRevealFiles, "Reveal Files"); 

	BOOL bShowReveal = g_AppSettings.m_LogEnabled; 
#ifdef _DEBUG
	bShowReveal = TRUE; 
#endif

	if (bShowReveal)
		_InsertMenuItem(hmenu, i++, miRevealMTTI, "Reveal MTTI");

	POINT pt;
	GetCursorPos(&pt);

	int nCmd = TrackPopupMenu(hmenu, TPM_RETURNCMD, pt.x, pt.y, 0, hWndParent, NULL);

	if (nCmd < 100)
	{
		switch(nCmd)
		{
		case miPlay:
			g_MainFrame->PlayMediaFile(videoID); 
			break; 
		case miRename:
			{
				FString StrNewName = ShowRenameDlg(hWndParent, pVideo.m_Detail.m_VideoName);
				if (StrNewName != "")
				{
					pVideo.m_Detail.m_VideoName = StrNewName;
					g_Objects._DownloadManager.SaveVideoDetails(pVideo.GetVideoID(), pVideo.m_Detail); 
				}
			}
			break;

		case miSuspend:
			for (size_t k = 0; k < aVideos.GetCount(); k++)
			{
				FDownload& pVideo = aVideos[k];
				if (!pVideo.IsPaused())
					g_Objects._DownloadManager.PauseDownload(videoID); 
				else
					g_Objects._DownloadManager.ResumeDownload(videoID);
			}
			break; 
		case miDelete:
			for (size_t k = 0; k < aVideos.GetCount(); k++)
			{
				g_MainFrame->RemoveVideo(aVideos[k].m_Detail.m_VideoID);
				//g_Objects._DownloadManager.RemoveDownload(aVideos[k].m_Detail.m_VideoID); 
			}
			break; 
		case miRevealFiles:
			{
				FString StrExec;
				if (pVideo.m_Clips.GetCount() > 0)
					StrExec	= pVideo.m_Clips[0]->m_DataPath; 
				ShellExecute(NULL, "open", StrExec, "", "", SW_SHOW);
			}
			break; 
		case miRevealMTTI:
			{
				FString MTTFileName = g_Objects._DownloadManager.GetDownloadMTTI(videoID);
				ShellExecute(NULL, "open", "notepad", MTTFileName, "", SW_SHOW);
			}
			break; 
		case miStart:
			{
				IVideoPlayer* pPlayer = g_MainFrame->GetPlayer(); 
				if (pPlayer)
				{
					if (pPlayer->GetVideoID() == videoID)
						pPlayer->Stop(); 
				}
				g_Objects._DownloadManager.StartQueuedDownload(videoID); 
			}
			break; 
		case miInfo:
			{
				//Navigate to details if episodeID exists.
				//If it's a RSS Feed, GoToChannelById()
				//If it's neither, open the folder
				g_MainFrame->EpisodeDetails(videoID); 

			}
			break; 
		}
	}

	if (nCmd >= 100)
	{
		int idStr = nCmd - 100; 
		if (idStr == 0)
		{
			FString StrLabel = ShowAddLabelDlg(hWndParent, ""); 

			if (StrLabel.GetLength() > 0)
			{
				g_Objects._LabelManager.AddLabel(StrLabel);
				for (size_t k = 0; k < aVideos.GetCount(); k++)
				{
					g_Objects._DownloadManager.AddLabel(aVideos[k].m_Detail.m_VideoID, StrLabel);
				}
			}
			nCmd = 0; 
		}
		else
			if (idStr > 0 && idStr <= (int)m_zeLabels.GetCount())
			{
				FString SelectedLabel = m_zeLabels[idStr - 1];
				UINT uState = GetMenuState(hSub, idStr + 100, MF_BYCOMMAND);
				if (uState != (UINT)-1)
				{
					for (size_t k = 0; k < aVideos.GetCount(); k++)
					{
						if (uState == MF_CHECKED)
							g_Objects._DownloadManager.RemoveLabel(aVideos[k].m_Detail.m_VideoID, SelectedLabel); 
						else
							g_Objects._DownloadManager.AddLabel(aVideos[k].m_Detail.m_VideoID, SelectedLabel); 
					}
				}
			}
	}


	DestroyMenu(hSub);
	DestroyMenu(hmenu);

	return nCmd; 
}


FString FCollectionMenu::ShowAddLabelDlg(HWND hWndParent, const tchar* pszLabel)
{
	FDlgInputBox addLabelDlg; 

	addLabelDlg.m_StrCaption = "Add Label";
	addLabelDlg.m_StrPrompt = "Enter a label:";
	addLabelDlg.m_StrDlgResult = pszLabel; 
	if (IDOK == addLabelDlg.DoModal(hWndParent))
		return addLabelDlg.m_StrDlgResult.Left(20); 
	return "";
}

FString FCollectionMenu::ShowRenameDlg(HWND hWndParent, const tchar* pszName)
{
	FDlgInputBox addLabelDlg; 

	addLabelDlg.m_StrCaption = "Rename";
	addLabelDlg.m_StrPrompt = "Name:";
	addLabelDlg.m_StrDlgResult = pszName; 
	if (IDOK == addLabelDlg.DoModal(hWndParent))
		return addLabelDlg.m_StrDlgResult; 
	return "";
}