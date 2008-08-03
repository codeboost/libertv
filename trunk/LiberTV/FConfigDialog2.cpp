#include "stdafx.h"
#include "FConfigDialog2.h"
#include "AppSettings.h"
#include "GlobalObjects.h"
#include "FDlgSaveVideo.h"
#include "FMainFrame.h"

extern FAutoWindow<FMainFrame> g_MainFrame; 

void FConfigDialog2::OnCreated()
{
	SetMinSize(600, 480); 
	m_AppSettings = g_AppSettings; 
	m_CurrentSection = 0; 
	SetIcon(LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_MAIN)));

	if (FAILED(m_pW.Navigate(g_AppSettings.AppDir("data/settings/settings.htm"))))
	{
		MessageBox("Cannot load settings.htm. Please reinstall.", "Error", MB_OK | MB_ICONERROR);
		DestroyWindow();
	}
}

LRESULT FConfigDialog2::OnDestroy(UINT u, WPARAM w, LPARAM l, BOOL& b)
{
	FIEDialog::OnDestroy(u, w, l, b); 
	return 0; 
}

void FConfigDialog2::OnUnload(long lSection)
{
	FDocument& aDoc = m_pW.m_pBrowser; 

	ATLASSERT(aDoc.pDocument);

	if (!aDoc.pDocument)
		return ; 


	switch(lSection)
	{
	case 0:
		{
			if (aDoc.GetCheck("General", "AutoStart"))
				m_AppSettings.m_Flags|=FLAG_AUTO_START;
			else
				m_AppSettings.m_Flags&=~FLAG_AUTO_START;

			if (aDoc.GetCheck("General", "AutoUpdate"))
				m_AppSettings.m_Flags|=PFLAG_AUTO_UPDATE;
			else
				m_AppSettings.m_Flags&=~PFLAG_AUTO_UPDATE;

			m_AppSettings.m_NumVideosInArchive = aDoc.GetDword("General", "ArchiveMaxVideos");
			m_AppSettings.m_MinSpaceOnDriveMB = aDoc.GetDword("General", "MinimumSpace");
			//m_AppSettings.m_MaxStorageDays = aDoc.QueryHtmlElementValueListBox()
			//m_AppSettings.m_DownloadsPath = aDoc.CallScriptStr("GetStoragePath", ""); 

			if (aDoc.GetCheck("General", "JumpCollection"))
				m_AppSettings.m_Flags|=PFLAG_JUMP_COLLECTION; 
			else
				m_AppSettings.m_Flags&=~PFLAG_JUMP_COLLECTION; 
		}
		break; 
	case 1:
		{
			aDoc.GetDword("Network", "MaxKBUp",			m_AppSettings.m_MaxKBUp);
			aDoc.GetDword("Network", "MaxKBDown",		m_AppSettings.m_MaxKBDown);
			aDoc.GetDword("Network", "MaxDownloads",	m_AppSettings.m_MaxDownloads);
			aDoc.GetDword("Network", "MaxConnections",	m_AppSettings.m_MaxConnections); 
			aDoc.GetDword("Network", "ListenPortMin",	m_AppSettings.m_ListenPort);
			m_AppSettings.m_RandomPort = aDoc.GetCheck("Network", "RandomPort") ? 1 : 0;

			m_AppSettings.m_Proxy = aDoc.CallScriptStr("getProxy", "");
			m_AppSettings.m_ProxyA = aDoc.CallScriptStr("getProxyAuth", "");

		}
		break; 
	case 2:
		{
			aDoc.GetDword("Player", "HideCursor",	m_AppSettings.m_dwHideMouse);

			if (aDoc.GetCheck("Player", "SavePosition"))
				m_AppSettings.m_Flags|=PFLAG_RESUME_VIDEOS;
			else
				m_AppSettings.m_Flags&=~PFLAG_RESUME_VIDEOS;

			if (aDoc.GetCheck("Player", "StreamPlay"))
				m_AppSettings.m_Flags|=PFLAG_STREAMING_PLAYBACK;
			else
				m_AppSettings.m_Flags&=~PFLAG_STREAMING_PLAYBACK; 

			if (aDoc.GetCheck("Player", "ShowVideo"))
				m_AppSettings.m_Flags|=PFLAG_SHOW_LAST_VIDEO;
			else
				m_AppSettings.m_Flags&=~PFLAG_SHOW_LAST_VIDEO;

			if (aDoc.GetCheck("Player", "EscToggle"))
				m_AppSettings.m_Flags|=PFLAG_ESC_FULLSCREEN;
			else
				m_AppSettings.m_Flags&=~PFLAG_ESC_FULLSCREEN; 

            if (aDoc.GetCheck("Player", "ConfirmRemove"))
                m_AppSettings.m_Flags|=PFLAG_CONFIRM_REMOVE;
            else
                m_AppSettings.m_Flags&=~PFLAG_CONFIRM_REMOVE;

			if (aDoc.GetCheck("Player", "autoSubs"))
				m_AppSettings.m_Flags|=PFLAG_AUTO_SUBTITLES;
			else
				m_AppSettings.m_Flags&=~PFLAG_AUTO_SUBTITLES;

			m_AppSettings.m_DefSubLang = aDoc.CallScriptStr("getDefSubLang", ""); 
		}
		break;
	case 3:
		{
			if (aDoc.GetCheck("Codecs", "UseQuicktime"))
				m_AppSettings.m_dwCodecFlags|=CODEC_FLAG_USE_QUICKTIME;
			else
				m_AppSettings.m_dwCodecFlags&=~CODEC_FLAG_USE_QUICKTIME;

			if (aDoc.GetCheck("Codecs", "UseWMP"))
				m_AppSettings.m_dwCodecFlags|=CODEC_FLAG_USE_WMP;
			else
				m_AppSettings.m_dwCodecFlags&=~CODEC_FLAG_USE_WMP;

			m_AppSettings.m_Vout = aDoc.CallScriptStr("getVout", ""); 
		}
	}
}

void FConfigDialog2::OnLoad(long lSection)
{
	m_CurrentSection = lSection; 

	FDocument& aDoc = m_pW.m_pBrowser; 
	
	ATLASSERT(aDoc.pDocument);

	if (!aDoc.pDocument)
		return ; 

	m_pW.SetFocus();
	CComPtr<IHTMLWindow2> pWnd; 
	if (SUCCEEDED(aDoc.	pDocument->get_parentWindow(&pWnd)) && pWnd)
	{
		pWnd->focus();
	}
	
	switch(lSection)
	{
		case 0:
		{
			aDoc.SetCheck("General", "AutoStart",		(m_AppSettings.m_Flags & FLAG_AUTO_START) != 0);
			aDoc.SetCheck("General", "AutoUpdate",		(m_AppSettings.m_Flags & PFLAG_AUTO_UPDATE) != 0);
			aDoc.SetDword("General", "ArchiveMaxVideos", m_AppSettings.m_NumVideosInArchive);
			aDoc.SetDword("General", "MinimumSpace",	m_AppSettings.m_MinSpaceOnDriveMB);
			aDoc.CallJScript("SetStoragePath",			m_AppSettings.m_DownloadsFolder); 
			aDoc.SetCheck("General", "JumpCollection",	(m_AppSettings.m_Flags & PFLAG_JUMP_COLLECTION) != 0); 
		}
		break;
		case 1:
		{
			aDoc.SetDword("Network", "MaxKBUp",			m_AppSettings.m_MaxKBUp);
			aDoc.SetDword("Network", "MaxKBDown",		m_AppSettings.m_MaxKBDown);
			aDoc.SetDword("Network", "MaxDownloads",	m_AppSettings.m_MaxDownloads);
			aDoc.SetDword("Network", "MaxConnections",	m_AppSettings.m_MaxConnections); 
			aDoc.SetDword("Network", "ListenPortMin",	m_AppSettings.m_ListenPort);
			aDoc.SetCheck("Network", "RandomPort",		m_AppSettings.m_RandomPort != 0? 1 : 0);
			
			aDoc.CallJScript("setProxy", m_AppSettings.m_Proxy); 
			aDoc.CallJScript("setProxyAuth", m_AppSettings.m_ProxyA);

		}
		case 2:
		{
			aDoc.SetCheck("Player", "SavePosition", (m_AppSettings.m_Flags & PFLAG_RESUME_VIDEOS) != 0);
			aDoc.SetCheck("Player", "StreamPlay",	(m_AppSettings.m_Flags & PFLAG_STREAMING_PLAYBACK) != 0); 
			aDoc.SetCheck("Player", "ShowVideo",    (m_AppSettings.m_Flags & PFLAG_SHOW_LAST_VIDEO) != 0);
			aDoc.SetCheck("Player", "EscToggle",	(m_AppSettings.m_Flags & PFLAG_ESC_FULLSCREEN) != 0); 
            aDoc.SetCheck("Player", "ConfirmRemove", (m_AppSettings.m_Flags & PFLAG_CONFIRM_REMOVE) != 0); 
			aDoc.SetCheck("Player", "autoSubs", (m_AppSettings.m_Flags & PFLAG_AUTO_SUBTITLES) != 0); 
			aDoc.SetDword("Player", "HideCursor",	m_AppSettings.m_dwHideMouse);
			aDoc.CallJScript("setDefSubLang", m_AppSettings.m_DefSubLang); 
			
		}
		break; 
		case 3:
			{
				aDoc.SetCheck("Codecs", "UseQuicktime", (m_AppSettings.m_dwCodecFlags & CODEC_FLAG_USE_QUICKTIME) != 0);
				aDoc.SetCheck("Codecs", "UseWMP", (m_AppSettings.m_dwCodecFlags & CODEC_FLAG_USE_WMP) != 0);
				aDoc.CallJScript("setVout", m_AppSettings.m_Vout); 
			}
		break; 
	}
}

void FConfigDialog2::OnCancel()
{
	m_pW.m_pBrowser.SetExternalDispatch(NULL); 
	DestroyWindow();
}

void FConfigDialog2::OnSave()
{
    BOOL bRestartApp = FALSE; 

	 OnUnload(m_CurrentSection);	//Save current settings
	 int nRes = g_pAppManager->OnSettingsChanged(&m_AppSettings); 
	 if (nRes == 1)
     {
         MessageBox(LTV_APP_NAME" will now restart to apply the new settings.", "LiberTV Settings", MB_OK | MB_ICONINFORMATION);
		 bRestartApp = TRUE; 
     }

   OnCancel();
   if (bRestartApp)
       g_pAppManager->RestartApp();
}



int FConfigDialog2::BrowseForFolder(const tchar* pStrCurrent, FString& StrOut)
{
   FDlgSaveVideo SelectStorageDlg;

   FDlgSaveVideo::SaveDlgOpenParams Params(&m_AppSettings); 
   int nRes = SelectStorageDlg.Open(m_hWnd, Params); 
   StrOut = m_AppSettings.m_DownloadsFolder; 
   return nRes; 
}

//////////////////////////////////////////////////////////////////////////

















