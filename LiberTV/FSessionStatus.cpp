#include "stdafx.h"
#include "FSessionStatus.h"
#include "GlobalObjects.h"
#include "FMainFrame.h"

extern FAutoWindow<FMainFrame> g_MainFrame; 
extern FString GetStatusStr(unsigned int flags);
extern FString GetNetStatus(unsigned int flags);

LRESULT FDlgSessionStatus::OnInitDialog(UINT , WPARAM , LPARAM , BOOL& )
{
	DlgResize_Init();
	CenterWindow(GetParent());

	m_lvSessionStatus.Attach(GetDlgItem(IDC_SESSION));
	m_lvPeers.Attach(GetDlgItem(IDC_PEERS));


	m_lvSessionStatus.SetViewType(LVS_REPORT );
	m_lvPeers.SetViewType(LVS_REPORT ); 

	FRect rDlg; 
	GetClientRect(&rDlg); 

	FRect rStatus, rPeers; 

	m_lvSessionStatus.GetClientRect(&rStatus); 
	m_lvPeers.GetClientRect(&rPeers); 

	m_lvSessionStatus.AddColumn("Variable", 0);
	m_lvSessionStatus.AddColumn("Value", 1);

	int VarWidth = 150; 
	m_lvSessionStatus.SetColumnWidth(0, VarWidth); 
	m_lvSessionStatus.SetColumnWidth(1, rStatus.Width() - VarWidth); 
	

	m_lvPeers.AddColumn("Peer Ip", 0); 
	m_lvPeers.AddColumn("Status", 1); 
	m_lvPeers.AddColumn("Flags", 2); 
	m_lvPeers.AddColumn("Down Speed", 3); 
	m_lvPeers.AddColumn("Downloaded", 4); 
	m_lvPeers.AddColumn("Up Speed", 5); 
	m_lvPeers.AddColumn("Uploaded", 6); 
	m_lvPeers.AddColumn("Client", 7); 

	m_lvPeers.SetColumnWidth(0, 150); 
	for (size_t k = 1; k < 8; k++)
		m_lvPeers.SetColumnWidth((int)k, 100); 

	SetTimer(0, 1000); 
	return 0; 
}

LRESULT FDlgSessionStatus::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	UpdateSessionStatus(); 
	UpdatePeers(); 
	return 0; 
}

void FDlgSessionStatus::UpdateSessionStatus()
{
	if (m_lvSessionStatus.GetItemCount() == 0)
	{
		m_lvSessionStatus.InsertItem(0, "Torrents"); 
		m_lvSessionStatus.InsertItem(1, "Peers"); 
		m_lvSessionStatus.InsertItem(2, "Upload Rate"); 
		m_lvSessionStatus.InsertItem(3, "Download Rate"); 
		m_lvSessionStatus.InsertItem(4, "Total Upload"); 
		m_lvSessionStatus.InsertItem(5, "Total Download"); 
	}

	libtorrent::session_status status = g_Objects._ClipDownloader->GetSessionStatus();

	std::vector<torrent_handle> torrents = g_Objects._ClipDownloader->GetTorrents();

	m_lvSessionStatus.SetItem(0, 1, LVIF_TEXT, FormatString("%d", torrents.size()), 0, 0, 0, 0); //Torrents
	m_lvSessionStatus.SetItem(1, 1, LVIF_TEXT, FormatString("%d", status.num_peers), 0, 0, 0, 0); //Peers
	m_lvSessionStatus.SetItem(2, 1, LVIF_TEXT, SpeedString((LONGLONG)status.upload_rate), 0, 0, 0, 0); //Upload Rate
	m_lvSessionStatus.SetItem(3, 1, LVIF_TEXT, SpeedString((LONGLONG)status.download_rate), 0, 0, 0, 0); //Download Rate
	m_lvSessionStatus.SetItem(4, 1, LVIF_TEXT, SizeString(status.total_upload), 0, 0, 0, 0); //Total Upload
	m_lvSessionStatus.SetItem(5, 1, LVIF_TEXT, SizeString(status.total_download), 0, 0, 0, 0); //Total Download
}

void FDlgSessionStatus::UpdatePeers()
{
	if (!g_MainFrame->m_Container->m_pStatus.IsObjectAndWindow())
		return ; 

	FString StrVideoID = g_MainFrame->m_Container->m_pStatus->m_pBrowser.CallScriptStr("getSelectedVideoId", "");
	vidtype videoid = strtoul(StrVideoID, NULL, 10);

	if (m_LastVideo != videoid)
	{
		
		m_LastVideo = videoid; 
		if (videoid == 0)
		{
			SetDlgItemText(IDC_VIDEO_NAME, "<Select video in My Collection>"); 
			return ; 
		}
	}
	m_lvPeers.DeleteAllItems(); 
	
	FDownload pDown = g_Objects._DownloadManager.GetDownloadInfo(videoid);

	SetDlgItemText(IDC_VIDEO_NAME, pDown.m_Detail.m_VideoName); 

	for (size_t i = 0; i < pDown.m_Clips.GetCount(); i++)
	{
		libtorrent::torrent_handle handle = g_Objects._ClipDownloader->GetTorrent(pDown.m_Clips[i]->m_DownloadHandle); 
		if (handle.is_valid())
		{
			std::vector<libtorrent::peer_info> peers;
			handle.get_peer_info(peers);

			//jsArray.AddValue("info-hash", "%s", handle.info_hash().)
			for(size_t k = 0; k < peers.size(); k++)
			{
				libtorrent::peer_info &peer = peers[k];
				std::string strIP = peer.ip.address().to_string();

				LVFINDINFO fi = {0}; 
				fi.flags = LVFI_STRING ; 
				fi.psz = strIP.c_str(); 
				fi.vkDirection = VK_NEXT ;
				
				int nIndex = m_lvPeers.FindItem(&fi, 0); 

				if (nIndex == -1)
				{
					nIndex = m_lvPeers.InsertItem(m_lvPeers.GetItemCount(), strIP.c_str()); 
				}

				m_lvPeers.SetItem(nIndex, 1, LVIF_TEXT, GetNetStatus(peer.flags), 0, 0, 0, 0);
				m_lvPeers.SetItem(nIndex, 2, LVIF_TEXT, GetStatusStr(peer.flags), 0, 0, 0, 0); 
				m_lvPeers.SetItem(nIndex, 3, LVIF_TEXT, SpeedString((LONGLONG)peer.down_speed), 0, 0, 0, 0); 
				m_lvPeers.SetItem(nIndex, 4, LVIF_TEXT, SizeString(peer.total_download), 0, 0, 0, 0); 
				m_lvPeers.SetItem(nIndex, 5, LVIF_TEXT, SpeedString((LONGLONG)peer.up_speed), 0, 0, 0, 0); 
				m_lvPeers.SetItem(nIndex, 6, LVIF_TEXT, SizeString(peer.total_upload), 0, 0, 0, 0); 
				m_lvPeers.SetItem(nIndex, 7, LVIF_TEXT, peer.client.c_str(), 0, 0, 0, 0); 

/*
				libtorrent::peer_info &peer = peers[k];
				jsArray.NewElement("|");
				jsArray.AddValue("peer_ip", "%s", );
				jsArray.AddValue("status", "%s", GetNetStatus(peer.flags));
				jsArray.AddValue("flags", "%s", GetStatusStr(peer.flags)); 
				jsArray.AddValue("download_speed", "%.2f", peer.down_speed);
				jsArray.AddValue("upload_speed", "%.2f", peer.up_speed); 
				jsArray.AddValue("total_down", "%I64d", peer.total_download);
				jsArray.AddValue("total_up", "%I64d", peer.total_upload); 
				jsArray.AddValue("client", "%s", peer.client.c_str()); 
				*/
			}
		}
	}



}