// FHtmlEventDispatcher.cpp : Implementation of CFHtmlEventDispatcher

#include "stdafx.h"
#include "FHtmlEventDispatcher.h"
#include ".\fhtmleventdispatcher.h"
#include "GlobalObjects.h"
#include "FMainFrame.h"
#include "strutils.h"
#include "dispex.h"
#include "comdef.h"
#include <activscp.h>
#include "FJScriptArrayBuilderA.h"
#include "FRSSArrayBuilder.h"

extern FAutoWindow<FMainFrame> g_MainFrame; 


size_t VariantToUINTArray(VARIANT vtVar, FArray<FString>& aVids)
{
	USES_CONVERSION; 

	if (vtVar.vt == VT_BSTR)
	{
		const tchar* psz = OLE2T(vtVar.bstrVal); 
		if (psz != NULL)
		{
			if (NULL != strchr(psz, ','))
			{
				SplitStringToArray(psz, aVids); 
			}
			else
			{
				aVids.Add(psz);
			}
		}
	}
	else
	{
		FString StrVid;
		StrVid.Format("%u", VariantToUINT(vtVar));
		aVids.Add(StrVid); 
	}

	return aVids.GetCount(); 
}



// CFHtmlEventDispatcher
       
STDMETHODIMP CFHtmlEventDispatcher::DownloadMTT(BSTR Address)
{
	USES_CONVERSION; 
	if (NULL != Address)
	{
		g_pAppManager->OpenMediaFromURL(OLE2T(Address));
		return S_OK; 
	}
	return S_FALSE;
}

STDMETHODIMP CFHtmlEventDispatcher::MF_SetActiveSection(LONG lSection)
{
	if (g_MainFrame.IsObjectAndWindow())
		g_MainFrame->GetContainer().SwitchActiveView(lSection, TRUE); 

	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::PlayVideo(VARIANT varVideoID)
{
	USES_CONVERSION;
	if (g_MainFrame.IsObjectAndWindow())
	{
		vidtype videoID = VariantToUINT(varVideoID); 

		IVideoPlayer* pPlayer = g_MainFrame->GetPlayer(); 
		if (pPlayer == NULL || pPlayer->GetVideoID() != videoID)
		{
			g_MainFrame->PlayMediaFile(videoID);
		}
		else
		{
			g_MainFrame->m_Container->SwitchActiveView(VIEW_PLAYER, TRUE); 
			pPlayer->Play(); 
		}

	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::OnToolbarCommand(LONG idiControl, LONG wParam, LONG lParam)
{
	if (g_MainFrame.IsObjectAndWindow() && g_MainFrame->m_Container->m_pBrowser.IsObjectAndWindow())
	{
		g_MainFrame->m_Container->m_pBrowser->OnToolbarCommand(wParam); 
		//g_MainFrame->OnToolbarCommand(idiControl, wParam, lParam); 
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::RemoveVideo(VARIANT varVideoID, LONG* pvResult)
{
	USES_CONVERSION;											
	vidtype videoID = VariantToUINT(varVideoID); 

	*pvResult = 0; 
	if (g_MainFrame.IsObjectAndWindow())
	{
		if (g_MainFrame->RemoveVideo(videoID))
			*pvResult = 1; 
	}
	else
	{
		g_Objects._DownloadManager.RemoveDownload(videoID);
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::OnButtonClicked(LONG dialogID, LONG buttonID)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::Conf_OnLoad(LONG lPageID)
{
	g_pAppManager->Conf_OnLoad(lPageID); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::Conf_OnUnload(LONG lSectionID)
{
	g_pAppManager->Conf_OnUnload(lSectionID); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::Conf_OnCancel(LONG lSectionID)
{
	g_pAppManager->Conf_OnCancel();
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::Conf_OnSave(void)
{
	g_pAppManager->Conf_OnSave(); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::ControlBar_SetVolume(LONG lVolume)
{
	IVideoPlayer* pPlayer = g_MainFrame->GetPlayer(); 
	if (NULL != pPlayer)
	{
		g_AppSettings.m_Volume = lVolume; 
		pPlayer->SetVolume(lVolume); 
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::ControlBar_PlayButton(LONG lPlaying)
{
	IVideoPlayer* pPlayer = g_MainFrame->GetPlayer();
	if (pPlayer && pPlayer->GetVideoID() > 0)
	{
		if (pPlayer->IsPlaying())
			pPlayer->Pause(); 
		else
			pPlayer->Play(); 
	}
	else
	{
		g_MainFrame->PlayMediaFile(g_AppSettings.GetLastVideoID()); 
	}
	
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::PauseVideo(void)
{
	IVideoPlayer* pPlayer = g_MainFrame->GetPlayer();
	if (pPlayer && pPlayer->GetVideoID() > 0)
	{
		if (pPlayer->IsPlaying())
			pPlayer->Pause(); 
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::OutputDebugString(VARIANT vStr)
{
	USES_CONVERSION;
	g_pAppManager->OutputDebugString(OLE2T(vStr.bstrVal));
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::Seekbar_SetPosition(LONGLONG lPosition)
{
	IVideoPlayer* pPlayer = g_MainFrame->GetPlayer();
	if (pPlayer)
	{
		pPlayer->SetPosition(lPosition); 
	}

	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::PauseResume(VARIANT varVideoID)
{
	vidtype videoID = VariantToUINT(varVideoID); 

	if (g_Objects._DownloadManager.IsDownloadPaused(videoID))
		g_Objects._DownloadManager.ResumeDownload(videoID);
	else
		g_Objects._DownloadManager.PauseDownload(videoID); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::DebugPrintPeerList(VARIANT videoID)
{
	
	return S_OK;
}


STDMETHODIMP CFHtmlEventDispatcher::RestoreFullScreen(void)
{
	
	if (g_MainFrame.IsObjectAndWindow())
	{
		g_MainFrame->SendMessage(WM_PLAYER_MAX, 0, 0); 
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::PauseCurrentVideo(void)
{
	IVideoPlayer* pPlayer = g_MainFrame->GetPlayer();
	if (pPlayer)
	{
		pPlayer->Pause(); 
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::ReannounceAll(void)
{
//	g_Objects._DownloadManager.ReannounceVideo(0); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::ReannounceVideo(VARIANT varVideoID)
{
//	g_Objects._DownloadManager.ReannounceVideo(VariantToUINT(varVideoID));  
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::BrowseForFolder(VARIANT vtFolder, BSTR* vtOutFolder)
{
	USES_CONVERSION;
	FString FStrOut; 
	int nRes = IDCANCEL; 

	nRes = g_pAppManager->Conf_BrowseForFolder(OLE2T(vtFolder.bstrVal), FStrOut);
	if (nRes == -1)
	{
		//return initial path if user canceled the folder dialog
		FStrOut = OLE2T(vtFolder.bstrVal);
	}
	*vtOutFolder = FStrOut.AllocSysString();
	return S_OK; 
}

STDMETHODIMP CFHtmlEventDispatcher::OnAdComplete(void)
{
	// TODO: Add your implementation code here
	//g_MainFrame->m_Container->m_pMediaPlayer->OnAdComplete();
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::OpenSettings(void)
{
    if (g_MainFrame.IsObjectAndWindow())
        g_pAppManager->OpenSettings(g_MainFrame);

    return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::Search(BSTR searchString, LONG flag)
{
    USES_CONVERSION;
    if (g_MainFrame.IsObjectAndWindow())
    {
        g_MainFrame->Search(OLE2T(searchString), flag); 
    }
    return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::SelectSubtitle(BSTR subLang, ULONG videoID)
{
	USES_CONVERSION;
	if (g_MainFrame.IsObjectAndWindow())
	{
		g_MainFrame->m_Container->m_pMediaPlayer->SelectSubtitle(OLE2T(subLang)); 
	}

	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::GetPlayingVideoID(ULONG* videoID)
{
	*videoID = 0; 
	IVideoPlayer* pPlayer = g_MainFrame->GetPlayer();
	if (pPlayer)
	{
		*videoID = pPlayer->GetVideoID();
	}
		
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::GetVideoInfo(ULONG videoID, IDispatch* pDispScript, IDispatch* pDispOut)
{

	FJScriptArrayBuilder ItemStatus;
	ItemStatus.SetDispatch(pDispScript, pDispOut);
	ItemStatus.New(); 
	g_Objects._DownloadManager.GetDownloadStatusString(videoID, &ItemStatus);
	ItemStatus.End();
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::GetPlayerVersion(BSTR* version)
{
	*version = g_AppSettings.m_AppVersion.AllocSysString();

	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::AddEpisodeComment(ULONG videoID, ULONG episodeID)
{
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::EpisodeDetails(ULONG ulEpisodeID)
{
	g_MainFrame->EpisodeDetails(ulEpisodeID); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::VideoDetails(ULONG videoID)
{
	FDownload pDown = g_Objects._DownloadManager.GetDownloadInfo(videoID); 
	if (pDown.IsValid())
	{
		EpisodeDetails(pDown.m_Detail.m_EpisodeID); 
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::GetSearchString(BSTR* bstrSearchStr)
{
	// TODO: Add your implementation code here

	*bstrSearchStr = NULL;
	if (g_MainFrame.IsObjectAndWindow() && 
		g_MainFrame->m_TopBar != NULL	&& 
		g_MainFrame->m_TopBar->IsWindowVisible())
	{
		CComVariant vtRes; 
		if (SUCCEEDED(g_MainFrame->m_TopBar->m_pBrowser.CallJScript("getSearchString", &vtRes))){
			if (vtRes.vt == VT_BSTR){
				*bstrSearchStr = SysAllocString(vtRes.bstrVal);
				return S_OK;
			}
		}
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::PlayNextClip(void)
{

	IVideoPlayer* pPlayer = g_MainFrame->GetPlayer();

	if (pPlayer)
	{
		pPlayer->PlayNext(TRUE); 
	}

	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::PlayPrevClip(void)
{
	IVideoPlayer* pPlayer = g_MainFrame->GetPlayer();

	if (pPlayer)
	{
		pPlayer->PlayPrev(TRUE); 
	}

	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::PlayNextClipEx(LONG lClip)
{
	//PlayNextClip(); 
	IVideoPlayer* pPlayer = g_MainFrame->GetPlayer();

	if (pPlayer)
	{
		pPlayer->PlayClip(lClip);
	}

	return S_OK;
}


STDMETHODIMP CFHtmlEventDispatcher::ShowVideoOptions(ULONG videoID)
{
	if (g_MainFrame.IsObjectAndWindow())
		g_MainFrame->OpenVideoOptions();
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::ToggleSubtitles(ULONG videoID)
{
	if (g_MainFrame.IsObjectAndWindow() && 
		g_MainFrame->m_Container->m_pMediaPlayer.IsObjectAndWindow())
	{
		g_MainFrame->m_Container->m_pMediaPlayer->PostMessage(WM_KEYDOWN, 'S', 0); 
	}

	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::ToggleInfobar(void)
{
	if (g_MainFrame.IsObjectAndWindow() && 
		g_MainFrame->m_Container->m_pMediaPlayer.IsObjectAndWindow())
	{
		g_MainFrame->m_Container->m_pMediaPlayer->PostMessage(WM_KEYDOWN, 'I', 0); 
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::NavigateGuide(BSTR bstrURL, ULONG ulFlags)
{
	USES_CONVERSION; 
	if (g_MainFrame.IsObjectAndWindow())
	{
		g_MainFrame->NavigateGuide(OLE2T(bstrURL), ulFlags); 
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::SetLabel(VARIANT varVideoID, BSTR bstrLabel)
{
	USES_CONVERSION; 
	g_Objects._DownloadManager.SetLabel(VariantToUINT(varVideoID),  OLE2T(bstrLabel)); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::AddLabel(VARIANT varVideoID, BSTR bstrNewLabel)
{
	USES_CONVERSION; 

	FArray<FString> aVideoIDs; 
	VariantToUINTArray(varVideoID, aVideoIDs); 

	g_Objects._DownloadManager.AddLabel(aVideoIDs,  OLE2T(bstrNewLabel)); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::RemoveLabel(VARIANT varVideoID, BSTR bstrLabel)
{
	USES_CONVERSION;
	FArray<FString> aVideoIDs; 
	VariantToUINTArray(varVideoID, aVideoIDs); 
	g_Objects._DownloadManager.RemoveLabel(aVideoIDs, OLE2T(bstrLabel));
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::AddGlobalLabel(BSTR bstrLabel)
{
	USES_CONVERSION; 
	g_Objects._LabelManager.AddLabel(OLE2T(bstrLabel)); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::RemoveGlobalLabel(BSTR bstrLabel)
{
	USES_CONVERSION; 
	g_Objects._LabelManager.RemoveLabel(OLE2T(bstrLabel));
	g_Objects._DownloadManager.RemoveLabel(OLE2T(bstrLabel));
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::GetGlobalLabels(IDispatch* pDispArray)
{
	FString StrLabels = g_Objects._LabelManager.GetLabels(); 
	FJSArrayCRC32 pJSArray; 
	pJSArray.SetArray(pDispArray); 
	pJSArray.AddElementsFromString(StrLabels, ",");
	CComVariant vt = pJSArray.GetCRC32(); 
	pJSArray.AddProperty("crc32", &vt); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::ShowContextMenu(VARIANT wParam, VARIANT lParam, VARIANT* ppRes)
{
	ppRes->vt = VT_I4;
	ppRes->lVal = 0; 

	switch(wParam.ulVal)
	{
	case 1: //status right click
		{
			FArray<FString> aVideoIDs; 

			VariantToUINTArray(lParam, aVideoIDs);

			if (aVideoIDs.GetCount() > 0)
				ppRes->lVal = (LONG)g_MainFrame->m_Container->m_pStatus->ShowContextMenu(aVideoIDs); 
		}
		break; 
	case 2:	//channelguide right click
		{
			g_MainFrame->m_Container->m_pBrowser->ShowContextMenu();
		}
		break;
	case 10: //toolbar right click
		ppRes->lVal = g_MainFrame->ShowMenu();
		break; 
	
	}
	
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::ShowAddLabelDlg(BSTR bstrLabel, BSTR* bstrRetVal)
{
	USES_CONVERSION;
	FString StrRes = g_MainFrame->m_Container->m_pStatus->ShowAddLabelDlg(OLE2T(bstrLabel)); 
	*bstrRetVal = StrRes.AllocSysString(); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::PostPlayerMessage(ULONG uMsg, ULONG wParam, ULONG lParam)
{
	if (g_MainFrame.IsObjectAndWindow() && g_MainFrame->m_Container->m_pMediaPlayer.IsObjectAndWindow())
	{

		IVideoPlayer* pPlayer = g_MainFrame->GetPlayer();
		if (NULL != pPlayer)
		{
			IMediaPlayerNotify* pNotify = NULL; 
			if (SUCCEEDED(pPlayer->GetNotify(&pNotify)) && pNotify != NULL)
			{
				pNotify->NotifyPlayer((DWORD)wParam, (HRESULT)lParam);
			}
		}
	}

	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::GetDownloadStatusArray(IDispatch* pScript, IDispatch* pArray, BSTR bstrNameFilter, BSTR bstrLabel, BSTR bstrSort, BSTR bstrSortDir, ULONG dwFlags)
{
	FJScriptArrayBuilder JSArrayBuilder; 

	if (SUCCEEDED(JSArrayBuilder.SetDispatch(pScript, pArray)))
	{
		USES_CONVERSION;
		DownloadStatusOptions options; 
		options.m_Filter = OLE2T(bstrNameFilter);
		options.m_Label = OLE2T(bstrLabel);
		options.m_Sort = OLE2T(bstrSort);
		options.m_SortDir = OLE2T(bstrSortDir);
		options.m_dwFlags = dwFlags; 
		g_Objects._DownloadManager.GetDownloadStatus(options, &JSArrayBuilder);
		JSArrayBuilder.End();
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::GetDownloadStatus(IDispatch* pScript, IDispatch* pArray, IDispatch* pOptions)
{
	FJScriptArrayBuilder JSArrayBuilder; 

	if (SUCCEEDED(JSArrayBuilder.SetDispatch(pScript, pArray)))
	{
		USES_CONVERSION; 
		
		DownloadStatusOptions options; 
		FDispObject obj;
		obj.SetDispatch(pOptions); 

		options.m_Filter = obj.GetPropertyStr("filter");
		options.m_Label = obj.GetPropertyStr("label"); 
		options.m_Sort  = obj.GetPropertyStr("sortBy"); 
		options.m_SortDir = obj.GetPropertyStr("sortDir"); 
		options.m_dwFlags = obj.GetPropertyUINT("flags"); 
		options.m_StartIndex = obj.GetPropertyUINT("startIndex");
		options.m_Count = obj.GetPropertyUINT("itemCount"); 
		if (options.m_Count == 0) options.m_Count = -1; 
		
		g_Objects._DownloadManager.GetDownloadStatus(options, &JSArrayBuilder);

		FDispObject arrayProp; 
		arrayProp.SetDispatch(pArray); 
		arrayProp.AddPropertyUINT("totalCount", options.m_TotalCount);

		JSArrayBuilder.End();
	}

	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::_SendMessage(BSTR bstrWndName, ULONG ulMessage, ULONG wParam, ULONG lParam)
{
	USES_CONVERSION;
	FString WndName = OLE2T(bstrWndName); 
	
	if (WndName.CompareNoCase("QuickBar") == 0)
	{
		g_MainFrame->m_QuickBar.DestroyWindow(); 
	}
	else
	if (WndName.Find("ltv_AdWindow") != -1)
	{
		if (g_MainFrame->m_Container->m_pMediaPlayer.IsObjectAndWindow())
		{
			g_MainFrame->m_Container->m_pMediaPlayer->PostMessage(WM_ADWINDOW_BASE + ulMessage, wParam, lParam); 
		}
	}
	return S_OK; 
}

FString GetStatusStr(unsigned int flags)
{
	using namespace libtorrent; 
	FString res; 
	res+=(flags & peer_info::interesting)?"I":"_";
	res+=(flags & peer_info::choked)?"C":"_";
	res+=(flags & peer_info::remote_interested)?"i":"_";
	res+=(flags & peer_info::remote_choked)?"c":"_";
	res+=(flags & peer_info::supports_extensions)?"e":"_";
	res+=(flags & peer_info::local_connection)?"l":"r";
	return res; 
}

FString GetNetStatus(unsigned int flags)
{
	using namespace libtorrent; 
	FString res = "Downloading";
	if (flags & peer_info::queued)
		res = "Queued";
	else
	if (flags & peer_info::connecting)
		res = "Connecting";
	else
	if (flags & peer_info::handshake)
		res = "Handshake";
	return res; 
}

STDMETHODIMP CFHtmlEventDispatcher::GetSessionStatus(IDispatch* pStatus)
{
	FJSArrayCRC32 jsArray; 
	
	jsArray.SetArray(pStatus);
/*
	document.all.has_incomming_connections = aStatus[0];
	document.all.num_peers = aStatus[1];
	document.all.upload_rate = aStatus[2];
	document.all.download_rate = aStatus[3];
	document.all.total_download = aStatus[4];
	document.all.total_upload = aStatus[5];
	document.all.payload_download_rate = aStatus[6];
	document.all.payload_upload_rate = aStatus[7];
	document.all.total_payload_download = aStatus[8];
	document.all.total_payload_upload = aStatus[9];
*/
	libtorrent::session_status status = g_Objects._ClipDownloader->GetSessionStatus();

	std::vector<torrent_handle> torrents = g_Objects._ClipDownloader->GetTorrents(); 
	jsArray.AddProperty("hasIncommingConnections", &CComVariant(status.has_incoming_connections));
	jsArray.AddProperty("numPeers", &CComVariant(status.num_peers));
	jsArray.AddProperty("uploadRate", &CComVariant(status.upload_rate)); 
	jsArray.AddProperty("downloadRate", &CComVariant(status.download_rate)); 
	jsArray.AddProperty("totalDownload", &CComVariant((double)status.total_download)); 
	jsArray.AddProperty("totalUpload", &CComVariant((double)status.total_upload)); 
	jsArray.AddProperty("payloadDownloadRate", &CComVariant(status.payload_download_rate)); 
	jsArray.AddProperty("payloadUploadRate", &CComVariant(status.payload_upload_rate)); 
	jsArray.AddProperty("payloadDownload", &CComVariant((double)status.total_payload_download)); 
	jsArray.AddProperty("payloadUpload", &CComVariant((double)status.total_payload_upload));
	jsArray.AddProperty("numTorrents", &CComVariant((UINT)torrents.size()));

	if (g_MainFrame->m_Container->m_pStatus.IsObjectAndWindow())
	{
		FString StrVideoID = g_MainFrame->m_Container->m_pStatus->m_pBrowser.CallScriptStr("getSelectedVideoId", "");
		vidtype videoid = strtoul(StrVideoID, NULL, 10);

		FDownload info = g_Objects._DownloadManager.GetDownloadInfo(videoid); 
		jsArray.AddProperty("videoID", &CComVariant(videoid)); 
		jsArray.AddProperty("videoName", &CComVariant(info.m_Detail.m_VideoName));
		jsArray.AddProperty("numClips", &CComVariant((UINT)info.m_Clips.GetCount())); 
		jsArray.AddProperty("curClip", &CComVariant((UINT)info.m_CurClipIndex)); 
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::GetPeerInfo(VARIANT varVideoID, IDispatch* pScript, IDispatch* pPeerInfo)
{
	// TODO: Add your implementation code here

	vidtype videoid = VariantToUINT(varVideoID); 
	FJScriptArrayBuilder jsArray; 
	jsArray.SetDispatch(pScript, pPeerInfo); 
	jsArray.New();
	/*
		Output format:
		torrent_id, info-hash, peers | peer1 info1, info2, info3 | 	peer2 info1, info2, info3;
		torrent_id, info-hash, peers | peer1 info1, info2, info3 |  peer2 info1, info2, info3 | peer3, info3, info4;
		a. split by ; -> a1
		b. split each a1 by | -> a2
		c. split each a2 by , -> a3
		d. a3[0] = torrent_id_info, a3[1..n] = peers
		
	*/

	SynchronizeThread(g_Objects._DownloadManager.m_Sync); 
	if (videoid > 0)
	{
		try{
			
			FDownload* pDown = g_Objects._DownloadManager.FindByID(videoid); 
			if (pDown)
			{
				for (size_t i = 0; i < pDown->m_Clips.GetCount(); i++)
				{
					libtorrent::torrent_handle handle = g_Objects._ClipDownloader->GetTorrent(pDown->m_Clips[i]->m_DownloadHandle); 
					if (handle.is_valid())
					{
						std::vector<libtorrent::peer_info> peers;
						handle.get_peer_info(peers);

						jsArray.NewElement("");
						jsArray.GetArray().AddProperty("clipId", &CComVariant((UINT)i));
						jsArray.GetArray().AddProperty("numPeers", &CComVariant((UINT)peers.size())); 


						FJSArrayCRC32 pPeers; 
						CComPtr<IDispatch> pDispPeers; 
						jsArray.CreateJSArray(pDispPeers);						
						pPeers.SetArray(pDispPeers);
						jsArray.GetArray().AddProperty("m_peers", &CComVariant(pDispPeers));
						for(size_t k = 0; k < peers.size(); k++)
						{
							//Construct a new object
							FDispObject pPeer; 
							if (SUCCEEDED(pPeer.CreateObject(pScript)))
							{
								libtorrent::peer_info &peer = peers[k];
								pPeer.AddProperty("ip", &CComVariant(peer.ip.address().to_string().c_str()));
								pPeer.AddProperty("netStatus", &CComVariant(GetNetStatus(peer.flags)));
								pPeer.AddProperty("statusStr", &CComVariant(GetStatusStr(peer.flags))); 
								pPeer.AddProperty("downSpeed", &CComVariant(peer.down_speed));
								pPeer.AddProperty("upSpeed", &CComVariant(peer.up_speed)); 
								pPeer.AddProperty("totalDownload", &CComVariant((double)peer.total_download));
								pPeer.AddProperty("totalUpload", &CComVariant((double)peer.total_upload)); 
								pPeer.AddProperty("client", &CComVariant(peer.client.c_str())); 
								char country[3] = {0}; 
								strncpy(country, peer.country, 2); 
								pPeer.AddProperty("country", &CComVariant(country));

								//add it to the array
								pPeers.AddElement(&CComVariant(pPeer.m_pDisp));
							}
						}
						pPeers.ReleaseArray();
					}
				}
			}
		}
		catch(...)
		{
			_DBGAlert("Exception!\n");
		}
	}

	jsArray.End();

	//_DBGAlert("JSString: %s\n", jsArray.m_StrBuffer);
	//*bstrPeerInfo = jsArray.m_StrBuffer.AllocSysString(); 

	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::SetMainFrameFocus(void)
{
	g_MainFrame->SetFocus(); 
	return S_OK;
}

BOOL IsSetting(const FString& Setting, const tchar* pszSettingName)
{
	return Setting.CompareNoCase(pszSettingName) == 0; 
}

STDMETHODIMP CFHtmlEventDispatcher::SetEngineSetting(BSTR bstrName, BSTR bstrValue)
{
	USES_CONVERSION;

	FString StrSetting = OLE2T(bstrName);
	FString StrValue = OLE2T(bstrValue); 
	ulong ulValue = strtoul(StrValue, NULL, 10);
	if (IsSetting(StrSetting, "MaxShareAge") && ulValue > 0)
		g_AppSettings.m_MaxShareAge = ulValue; 
	else
	if (IsSetting(StrSetting, "MaxTorrents") && ulValue > 0)
		g_AppSettings.m_MaxTorrents = ulValue; 
	else
	if (IsSetting(StrSetting, "SeedTime") && ulValue > 0)
		g_AppSettings.m_SeedTime = ulValue; 
	else
	if (IsSetting(StrSetting, "MinSeeds") && ulValue > 0)
		g_AppSettings.m_MinSeeds = ulValue; 
	else
	if (IsSetting(StrSetting, "MinRatio") && ulValue > 0)
		g_AppSettings.m_MinRatio = ulValue; 
	else
	if (IsSetting(StrSetting, "ApplySettings"))
	{
		if (ulValue == 1)
		{
			FClipDownloadConfig aConfig; 
			g_AppSettings.FillConf(aConfig);
			g_Objects._ClipDownloader->UpdateConfig(aConfig); 
		} 
		g_AppSettings.SaveSettings();
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::IsClientConnectible(VARIANT_BOOL* bConnectible)
{
	if (g_pAppManager->IsConnectable())
		*bConnectible = VARIANT_TRUE; 
	else
		*bConnectible = VARIANT_FALSE; 
	
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::OpenSiteURL(BSTR bstrURL)
{
	USES_CONVERSION;
	FString FullUrl = g_AppSettings.GetSiteURL(OLE2T(bstrURL));
	ShellExecute(NULL, "open", FullUrl, "", "", SW_SHOW);
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::SetServer(BSTR bstrURL, BSTR bstrGuideURL, BSTR bstrLang, BSTR bstrExtra)
{
	USES_CONVERSION;
	FString StrChannelGuide = OLE2T(bstrGuideURL); 
	FString StrURL = OLE2T(bstrURL); 

	if (PathIsURL(StrChannelGuide))
	{
		_RegSetStr("ChannelGuideURL", StrChannelGuide); 
	}
	if (PathIsURL(StrURL))
	{
		_RegSetStr("SiteURL", StrURL); 
	}

	g_pAppManager->OnSetServer(); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::RSSGetChannels(IDispatch* pDispScript, BSTR bstrFilter, IDispatch* pDispArray)
{
	FJScriptArrayBuilder pArray; 
	pArray.SetDispatch(pDispScript, pDispArray);

	const FArray<FRSSFeed*>& aFeeds = g_Objects._RSSManager->LockChannels(); 

	for (size_t k = 0; k < aFeeds.GetCount(); k++)
	{
		FRSSFeed* pFeed = aFeeds[k];
		ATLASSERT(pFeed->m_dwChannelID != 0); 
		const FRSSChannel* pChannel = aFeeds[k]->m_pChannel; 
		pArray.NewElement("");
		ATLASSERT((int)pFeed->m_NewItems >= 0); 
		pArray.AddValueUINT("", pFeed->m_dwChannelID);
		pArray.AddValueStr("", pFeed->m_ChannelName);
		pArray.AddValueStr("", pFeed->m_URL);
		pArray.AddValueUINT("", pFeed->m_NewItems);
		pArray.AddValueStr("", pChannel?pChannel->m_ChannelImage:"");
		pArray.AddValueStr("", pChannel?pChannel->m_Description:"");
		pArray.AddValueUINT("", pFeed->m_dwFlags);
		pArray.AddValueUINT("", pFeed->m_MaxActiveDownloads);
		pArray.AddValueUINT("", pFeed->m_DetailURL.GetLength());
		pArray.AddValueStr("",  pFeed->m_Filter.m_Contains); 
		pArray.AddValueStr("", pFeed->m_Filter.m_NotContains); 
	}
	pArray.End();
	g_Objects._RSSManager->ReleaseChannelsLock();

	return S_OK;
}




STDMETHODIMP CFHtmlEventDispatcher::RSSGetItems(IDispatch* pDispScript, IDispatch* pDispOutArray, IDispatch* pDispOptions)
{
	USES_CONVERSION; 

	FRSSArrayBuilder aBuilder; 
	CountStruct Counters; 
	aBuilder.RSSGetItems(pDispScript, pDispOutArray, pDispOptions, Counters); 
	
	FString StatusText; 

	if (Counters.dwTotalItems == 0)
		StatusText.Format("No episodes."); 
	else
		StatusText.Format("%u episodes; %d new.", Counters.dwTotalItems, Counters.dwNewItems);

	FMainFrame::m_iStatusBar.SetStatusText(0, StatusText, VIEW_FEEDS); 
	
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::RSSDeleteChannel(VARIANT vtChannelID)
{
	g_Objects._RSSManager->RemoveChannel(VariantToUINT(vtChannelID));
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::RSSDownloadItem(UINT uiItemGuid)
{
	g_Objects._RSSManager->DownloadItem(uiItemGuid, FALSE); 
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::RSSAddFeed(IDispatch* pDisp, ULONG* plResult)
{
	USES_CONVERSION;

	FDispObject aDispObj;
	aDispObj.SetDispatch(pDisp);

	CComVariant vtUrl, vtImage;

	HRESULT hr = aDispObj.GetProperty("URL", &vtUrl);
	*plResult = 0; 
	if (FAILED(hr) || vtUrl.vt != VT_BSTR)
	{
		FString Name; 
		CComVariant vtName; 
		if (SUCCEEDED(aDispObj.GetProperty("Name", &vtName)) && vtName.vt == VT_BSTR)
			Name = OLE2T(vtName.bstrVal);

		*plResult = g_MainFrame->AddFeedDialog("", Name);
	}
	else
	{
		AddFeedParams params; 
		if (SUCCEEDED(aDispObj.GetProperty("defImage", &vtImage)) && vtImage.vt == VT_BSTR)
			params.m_DefImage = vtImage.bstrVal;
		params.m_URL = OLE2T(vtUrl.bstrVal); 
		params.m_dwFlags=FEED_FLAG_SET_NAME;
		params.m_Name = aDispObj.GetPropertyStr("Name");
		params.m_DetailsURL = aDispObj.GetPropertyStr("DetailsURL");

		DWORD dwChannelID = g_Objects._RSSManager->AddFeed(params);
		if (dwChannelID > 0)
		{
			g_AppSettings.m_dwLastChannelId = dwChannelID;
			_DBGAlert("Setting LastChannelId: %u\n", dwChannelID); 
			g_MainFrame->m_Container->SwitchActiveView(VIEW_FEEDS, TRUE); 
			g_MainFrame->m_Container->m_pFeeds->GoToChannel(dwChannelID);
		}
		*plResult = dwChannelID; 
	}
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::RSSRefreshChannel(VARIANT vtChannelID)
{
	g_Objects._RSSManager->RefreshChannel(VariantToUINT(vtChannelID));
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::RSSAddFeedEx(BSTR bstrURL, BSTR bstrName, BSTR bstrOptions, ULONG* ulChannelID)
{
	USES_CONVERSION;
	*ulChannelID = g_MainFrame->AddFeedDialog(OLE2T(bstrURL), OLE2T(bstrName));
	return S_OK;
}

STDMETHODIMP CFHtmlEventDispatcher::GetCollectionCounts(BSTR* bstrCounts)
{
	// TODO: Add your implementation code here

	USES_CONVERSION;

	FCollectionSums pSums = g_Objects._DownloadManager.GetCollectionCounts();

	FJScriptStringBuilder jsArray; 
	jsArray.New();

	jsArray.NewElement();
	jsArray.AddValue("Total Items", "%d", pSums.m_dwAll);
	jsArray.AddValue("New Items",   "%d", pSums.m_dwUnwatched);
	jsArray.AddValue("Queued",		"%d", pSums.m_dwQueued); 
	jsArray.AddValue("InProgress",	"%d", pSums.m_dwInProgress); 
	jsArray.AddValue("Subscribed",  "%d", pSums.m_dwSubscribed); 
	jsArray.AddValue("Streams",		"%d", pSums.m_dwStreams);

	*bstrCounts = jsArray.m_StrBuffer.AllocSysString();
	return S_OK;
}
//////////////////////////////////////////////////////////////////////////

STDMETHODIMP CFHtmlEventDispatcher::ExecCmd(BSTR bstrCmd, VARIANT vtParam1, VARIANT vtParam2, BSTR* bstrResult)
{

	USES_CONVERSION;
	FString StrCmd = OLE2T(bstrCmd);

	FString StrResult; 
	if (StrCmd == "GotoRSS")
	{		
		g_MainFrame->GoToRSS(VariantToUINT(vtParam1));
	}
	else
	if (StrCmd == "SelectServer")
		g_pAppManager->SelectServer(); 
	else
	if (StrCmd == "SetFeedAutoDownload")
		g_Objects._RSSManager->SetChannelAutoDownload(VariantToUINT(vtParam1), VariantToUINT(vtParam2) == 0 ? FALSE: TRUE); 
	else
	if (StrCmd == "SetFeedAsRead")
		g_Objects._RSSManager->MarkAsViewed(VariantToUINT(vtParam1)); 
	else
	if (StrCmd == "PauseResumeAll")
	{
		if (g_Objects._DownloadManager.IsAllPaused())
		{
			g_Objects._DownloadManager.ResumeAll();
			StrResult = "0";
		}
		else
		{
			g_Objects._DownloadManager.PauseAll();
			StrResult = "1";
		}
	}
	else
	if (StrCmd == "IsAllPaused")
	{
		StrResult = g_Objects._DownloadManager.IsAllPaused() ? "1" : "0";
	}
	else
	if (StrCmd == "GetSelectedChannelId")
	{
		StrResult.Format("%u", g_AppSettings.m_dwLastChannelId); 
	}
	else
	if (StrCmd == "RSSRemoveChannelByURL" && vtParam1.vt == VT_BSTR)
	{
		g_Objects._RSSManager->RemoveChannel(OLE2T(vtParam1.bstrVal));
		StrResult = "1";
	}
	else
	if (StrCmd == "SaveSubSize")
	{
		ULONG FontSize = VariantToUINT(vtParam1);
		if (FontSize > 0)
		{
			_RegSetStr("SubFontSize", ULongToStr(FontSize), PLAYER_REG_KEY);
		}
	}
	else
	if (StrCmd == "RSSSaveChannelSettings")
	{	//param1 = channelID, param2 = object 
		if (vtParam2.vt == VT_DISPATCH)
		{
			FDispObject pObj;
			pObj.SetDispatch(vtParam2.pdispVal);
			CComVariant vtMaxDownloads;
			CComVariant vtChannelName; 
			pObj.GetProperty("maxDownloads", &vtMaxDownloads);
			pObj.GetProperty("channelName", &vtChannelName); 

			ChannelOptions options; 
			options.m_dwMaxDownloadsPerFeed = vtMaxDownloads.uintVal;
			options.m_ChannelName = vtChannelName.bstrVal;
			options.m_Filter.m_Contains = pObj.GetPropertyStr("filterContains"); 
			options.m_Filter.m_NotContains = pObj.GetPropertyStr("filterNotContains"); 
			g_Objects._RSSManager->SetChannelOptions(VariantToUINT(vtParam1), options);
		}
	} 
	else
	if (StrCmd == "ShowCommentsWnd")
	{
		g_MainFrame->ShowCommentsWnd();
	}
	else
	if (StrCmd == "GetLastLabel")
	{
		StrResult = _RegStr("LastLabelStr");	//Saved by SetLastLabel
	}
	else
	if (StrCmd == "SetLastLabel")				//Used by GetLastLabel / CollectionWindow
	{
		_RegSetStr("LastLabelStr", OLE2T(vtParam1.bstrVal));
		g_MainFrame->m_Container->m_pStatus->OnLabelChanged(OLE2T(vtParam1.bstrVal)); 
	}
	else
	if (StrCmd == "SetCollectionFlags")		//Show Queued/In progress, etc
	{
		g_MainFrame->m_Container->m_pStatus->OnFlagsChanged(VariantToUINT(vtParam1));
	}
	else
	if (StrCmd == "SetCollectionSearchString")
	{
		g_MainFrame->m_Container->m_pStatus->OnSearchStringChanged(OLE2T(vtParam1.bstrVal));
	}
	else
	if (StrCmd == "GetSplitterPosition")
	{
		StrResult = _RegStr("FeedsSplitterPos", LTV_REG_KEY); //saved in  FFeedsDisplay::OnDestroy()
	}
	else
	if (StrCmd == "GetRSSItemsPerPage")
	{
		StrResult = "25";
	}
	else
	if (StrCmd == "RSSChannelDetails")
	{
		FRSSFeed* pChannel = g_Objects._RSSManager->LockChannel(VariantToUINT(vtParam1));
		FString NavURL; 
		if (pChannel && PathIsURL(pChannel->m_DetailURL))
		{	
			NavURL = pChannel->m_DetailURL; 
		}
		g_Objects._RSSManager->ReleaseChannelsLock();
		g_MainFrame->NavigateToDetails(NavURL); 
	}
	else
	if (StrCmd == "RSSStreamItem")
	{
		g_Objects._RSSManager->DownloadItem(VariantToUINT(vtParam1), TRUE); 
	}
	else 
	if (StrCmd == "GetItemImageURL")
	{
		StrResult = g_Objects._ImageCache.GetImageURL(OLE2T(vtParam1.bstrVal)); 
	}
	else
	if (StrCmd == "CycleCollectionView")
	{
		ULONG ulVal = VariantToUINT(vtParam1); 
		if (ulVal < 3)
		{
			g_MainFrame->m_Container->m_pStatus->SetView(ulVal);
		}
		StrResult = LongToStr(_RegDword("LastCollectionView"));		//saved by FCollection::SetView()
	}

	if (*bstrResult == NULL)
		*bstrResult = StrResult.AllocSysString(); 

	return S_OK;
}
