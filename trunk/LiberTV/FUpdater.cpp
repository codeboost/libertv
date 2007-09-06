#include "stdafx.h"
#include "FUpdater.h"
#include "vslVersion.h"
#include "AppSettings.h"
#include "GlobalObjects.h"
#include <wininet.h>
bool WINAPI Callback(DWORD bytes, DWORD objPtr)
{
	FUpdater* pThis = (FUpdater*)objPtr; 
	return !pThis->m_bCancelled;
}

FUpdater::FUpdater()
{
	m_bCancelled = false; 
	m_Work.Create(true); 
	m_UpdateThread.Create(this);
}

FUpdater::~FUpdater()
{
}

void FUpdater::Stop()
{
	m_bCancelled = true; 	
	if (m_hDownload != 0)
		g_Objects._HttpDownloader->StopDownload(m_hDownload);

	m_UpdateThread.StopThread(); 
	m_UpdateThread.Wait(INFINITE); 
}

BOOL FUpdater::CheckConnectable(HWND hWndNotify)
{
	unsigned short listen_port = (unsigned short)g_AppSettings.m_ListenPort;
	FSessionInfo pInfo; 
	if (g_Objects._ClipDownloader->GetSessionInfo(&pInfo))
	{
		listen_port = pInfo.m_ListenPort ; 
	}
	CAtlHttpClient Client;

	WPARAM wParam = 0; 
	FString sUrl;

	FString ConnectableURL = g_AppSettings.GetConnectableURL(); 
	if (!PathIsURL(ConnectableURL))
	{
		_DBGAlert("FATAL: Cannot get ConnectableURL from registry\n"); 
		return FALSE; 
	}

	sUrl.Format("%s?port=%d&version=%s", ConnectableURL, listen_port, g_AppSettings.m_AppVersion); 

	if (Navigate(Client, sUrl) != ATL_INVALID_STATUS)
	{
		FString SConnectable; 
		if (Client.GetHeaderValue("LTV_CONNECTABLE", SConnectable))
		{
			SConnectable.MakeLower(); 
			SConnectable.Trim();
			if (SConnectable == "no")
			{
				wParam = 1; //notify parent that we're not connectible
				PostMessage(hWndNotify, WM_LTV_CONNECTABLE, wParam, 0); 
			}
		}
	}
	return (wParam == 0);
}

int FUpdater::Navigate(CAtlHttpClient& Client, const tchar* pszUrl)
{
	CAtlNavigateData Data; 
	Data.SetReadStatusCallback(Callback, (DWORD_PTR)this); 
	Data.dwTimeout = 15000; //15 seconds good enough ?

	FString ProxyAddr, ProxyUser, ProxyPass; 
	unsigned short ProxyPort; 

	if (g_AppSettings.GetProxy(ProxyAddr, ProxyPort, ProxyUser, ProxyPass))
	{
		Client.SetProxy(ProxyAddr, ProxyPort);
	}

	if (Client.Navigate(pszUrl, &Data))
		return Client.GetStatus();
	return ATL_INVALID_STATUS; 
}

static BOOL bFirstTime = TRUE;

void FUpdater::UpdateThread::Thread(void* pObj)
{
	FUpdater* pThis = (FUpdater*)pObj;

	for (;;)
	{
		int nRes = pThis->m_Work.Wait(60 * 60 * 1000); 

		if (nRes == WAIT_OBJECT_0 || nRes == WAIT_TIMEOUT)
		{
			pThis->CheckNewVersionSync(bFirstTime ? UPDATE_EVENT_STARTING : UPDATE_EVENT_TICK); 
			bFirstTime = FALSE; 
			pThis->m_Work.ResetEvent();
		}
		else
			break; 
	}
}

bool FUpdater::CheckNewVersionSync(DWORD dwEventType)
{
	SynchronizeThread(m_Sync);
	CAtlHttpClient Client;
	FString sUrl;

	FString VersionURL = g_AppSettings.GetVersionURL();
	if (!PathIsURL(VersionURL))
	{
		_DBGAlert("FATAL: Version URL cannot be found. Update thread terminating\n");
		return FALSE; 
	}


	FString StrCurVersion = g_AppSettings.m_AppVersion; 

	if (FUpdater::IsValidUpdate(_RegStr("Version")))
	{
		StrCurVersion = _RegStr("UpdateVersion"); 
	}
	
	if (StrCurVersion == "")
		StrCurVersion = g_AppSettings.m_AppVersion; 
	else
	{
		
		vslVersion aV = StrCurVersion; 
		vslVersion bV = g_AppSettings.m_AppVersion; 
		if (bV > aV)
			StrCurVersion = g_AppSettings.m_AppVersion; 
	}


	sUrl.Format("%s?version=%s&uGuid=%s&event=%d", VersionURL, StrCurVersion, g_AppSettings.m_uGuid, dwEventType); 


	if (Navigate(Client, sUrl) != ATL_INVALID_STATUS)
	{
		_DBGAlert("Update request sent: %d\n", dwEventType);
		if (dwEventType == UPDATE_EVENT_STOPPING)
			return TRUE; 

		FString SVersion; 
		if (Client.GetHeaderValue("LTV_VERSION", SVersion))
		{
			vslVersion aV = StrCurVersion; 
			vslVersion bV = SVersion; 
			if (aV < bV)	
			{
				//Current version is lower then server version
				//Perform the update

				m_UpdateVersion = SVersion; 
				FString StrUpdateUrl;
				if (Client.GetHeaderValue("LTV_UPDATE_URL", StrUpdateUrl) && PathIsURL(StrUpdateUrl))
				{
					if (EnsureDirExists(g_AppSettings.StorageDir("update")))
					{
						FString SetupFilename = _PathCombine(g_AppSettings.m_IndexPath, "update\\ltv_setup.exe"); 
						DownloadUpdate(StrUpdateUrl, SetupFilename);
					}
				}
			}
		}
		return TRUE;
	}	
	return FALSE; 
	
}

bool FUpdater::CheckNewVersion(HWND hWndNotify)
{
	if (m_Work.Wait(0) == WAIT_TIMEOUT)
	{
		//Event is not set - no download in progress
		m_hWndNotify = hWndNotify; 
		m_Work.SetEvent(); 
		return true; 
	}
	return false;
}

bool FUpdater::DownloadUpdate(const tchar* pszURL, const tchar* pszSaveFileName)
{
	FDownloadInfo *pInfo = new FDownloadInfo; 
	pInfo->m_ContentType = "";
	pInfo->m_DownloadFile = pszSaveFileName;
	pInfo->m_DownloadUrl = pszURL; 
	pInfo->m_MediaName = "App Update";
	pInfo->m_pv = NULL; 
	DeleteFile(pszSaveFileName); 
	m_hDownload = g_Objects._HttpDownloader->DownloadURL(pInfo, this);
	return (m_hDownload != 0);
}

void FUpdater::Notify(FDownloadAlert &alert)
{
	if (alert.dwCode == ALERT_HTTP_DOWNLOAD_FINISHED && SUCCEEDED(alert.hr) && !m_bCancelled)
	{
		FDownloadInfo* pInfo = (FDownloadInfo*)alert.m_Param1; 
		if (pInfo->m_dwStatusCode == 200)
		{
			if (PathFileExists(pInfo->m_DownloadFile) && GetFileSize(pInfo->m_DownloadFile) == pInfo->m_ContentLength)
			{
				_DBGAlert("Downloaded update: %s\n", pInfo->m_DownloadFile); 
				_RegSetStr("UpdateFile", pInfo->m_DownloadFile); 
				_RegSetDword("UpdateFileSize", (dword)pInfo->m_ContentLength); 
				_RegSetStr("UpdateVersion", m_UpdateVersion);
				PostMessage(m_hWndNotify, WM_LTV_VERSION_UPDATE, 1, 0); //New version is available.
			}
		}
	}
}

BOOL FUpdater::IsValidUpdate(const char* pszCurVersion)
{
	FString StrUpdatePath = _RegStr("UpdateFile");
	DWORD dwFileSize = _RegDword("UpdateFileSize"); 
	FString StrUpdateVer = _RegStr("UpdateVersion"); 

	vslVersion aV = StrUpdateVer; 
	vslVersion bV = pszCurVersion; 
	if (aV > bV)
	{
		if (dwFileSize > 0 && GetFileSize(StrUpdatePath) == dwFileSize)
		{
			return TRUE; 
		}
	}
	return FALSE; 
}

BOOL FUpdater::ClearUpdateInfo()
{
	SHDeleteValue(LTV_REG_ROOT,LTV_REG_KEY, "UpdateFile");
	SHDeleteValue(LTV_REG_ROOT,LTV_REG_KEY, "UpdateFileSize");
	SHDeleteValue(LTV_REG_ROOT,LTV_REG_KEY, "UpdateVersion");
	return TRUE; 
}

























