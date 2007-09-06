#include "stdafx.h"
#include "FMPlayerHTML.h"
#include "GlobalObjects.h"


void FMediaPlayerHTML::OnDocumentComplete(DWORD dwID, BOOL bMain)
{
//	m_pPlayerNotify->NotifyPlayer(PluginInitComplete, S_OK); 
	FIEWindow::OnDocumentComplete(dwID, bMain); 
}

BOOL FMediaPlayerHTML::OnNavigateError(DWORD dwID, BOOL bMainFrame, const tchar* pszURL)
{
	m_pPlayerNotify->NotifyPlayer(PluginInitComplete, E_FAIL); 
	return FIEWindow::OnNavigateError(dwID, bMainFrame, pszURL); 
}

HRESULT FMediaPlayerHTML::Init(HWND hWndParent, IMediaPlayerNotify* pNotify)
{
	m_pPlayerNotify = pNotify; 
	m_hWndNotify = hWndParent; 
	Create(hWndParent, rcDefault, NULL, WS_CHILD); 
	HRESULT hr = Navigate(g_AppSettings.AppDir("data\\htmlplayer.html"));
	return SUCCEEDED(hr) ? E_PENDING : hr;  
}

HRESULT FMediaPlayerHTML::ShowVideoWnd(int nShow)
{
	ShowWindow(nShow); 
	return S_OK; 
}

HRESULT FMediaPlayerHTML::LoadMedia(const tchar* pFileName, IMediaOptions* pOptions)
{
	FString StrOffs; 
	StrOffs.Format("%I64d", pOptions->rtOffset);
	HRESULT hr = m_pBrowser.CallJScript("ltv_loadMedia", pFileName, StrOffs);
	//return m_pBrowser.Navigate(pFileName); 
	return hr; 
}

HRESULT FMediaPlayerHTML::SetVideoRect(const RECT& rcV)
{
	MoveWindow(&rcV, TRUE); 
	FString S1, S2; 
	S1.Format("%d", rcV.right - rcV.left); 
	S2.Format("%d", rcV.bottom - rcV.top); 
	m_pBrowser.CallJScript("ltv_setSize", S1, S2); 
	return S_OK; 
}

HRESULT FMediaPlayerHTML::Stop()
{
	m_pBrowser.CallJScript("ltv_stop", "");
	return S_OK; 
}

HRESULT FMediaPlayerHTML::Play()
{
	return m_pBrowser.CallJScript("ltv_play"); 
}

HRESULT FMediaPlayerHTML::Pause()
{
	return m_pBrowser.CallJScript("ltv_pause"); 
}

VideoState FMediaPlayerHTML::GetVideoState()
{
	FString fState = m_pBrowser.CallScriptStr("ltv_getState", ""); 
	if (fState.CompareNoCase("playing") == 0)
		return vssPlaying; 
	else
	if (fState.CompareNoCase("paused") == 0)
		return vssPaused; 

	if (fState.CompareNoCase("buffering") == 0 || fState.CompareNoCase("connecting") == 0)
		return vssBuffering;

	if (fState.CompareNoCase("finished") == 0)
		return vssMediaEnded; 

	return vssStopped;
}

REFERENCE_TIME FMediaPlayerHTML::GetPosition()
{
   FString strPos = m_pBrowser.CallScriptStr("ltv_getPosition", ""); 
   unsigned __int64 uiPos = _strtoui64(strPos, NULL, 10); 
   return uiPos * DURATION_MULT;
}

HRESULT FMediaPlayerHTML::SeekPosition(REFERENCE_TIME rtPos)
{
    FString StrPos; 
    StrPos.Format("%I64d", rtPos / DURATION_MULT); 
    return m_pBrowser.CallJScript("ltv_setPosition", StrPos); 
}

REFERENCE_TIME FMediaPlayerHTML::GetDuration()
{
	FString strPos = m_pBrowser.CallScriptStr("ltv_getDuration", ""); 
	unsigned __int64 uiPos = _strtoui64(strPos, NULL, 10); 
	return uiPos * DURATION_MULT;
}

REFERENCE_TIME FMediaPlayerHTML::GetAvailDuration()
{
	FString strPos = m_pBrowser.CallScriptStr("ltv_getAvailDuration", ""); 
	unsigned __int64 uiPos = _strtoui64(strPos, NULL, 10); 
	return uiPos * DURATION_MULT;
}

void FMediaPlayerHTML::Clear()
{
	if (IsWindow())
		DestroyWindow(); 
}


HRESULT FMediaPlayerHTML::SetVolume(long lVol)
{
	FString StrVol;
	StrVol.Format("%d", lVol); 
	return m_pBrowser.CallJScript("ltv_setVolume", StrVol); 
}

long FMediaPlayerHTML::GetVolume()
{
	FString StrVol = m_pBrowser.CallScriptStr("ltv_getVolume", ""); 
	return strtoul(StrVol, NULL, 10); 
}

long FMediaPlayerHTML::GetMediaEvent()
{
	return EC_COMPLETE; 
}

//////////////////////////////////////////////////////////////////////////
HRESULT FMediaPlayerFLV::Init(HWND hWndParent, IMediaPlayerNotify* pNotify)
{
	m_pPlayerNotify = pNotify;
    m_hWndNotify = hWndParent; 
    Create(hWndParent, rcDefault, NULL, WS_CHILD | WS_VISIBLE); 
    HRESULT hr = Navigate(g_AppSettings.AppDir("data\\flvplayer.html"));
    return SUCCEEDED(hr) ? E_PENDING : hr;  
}

HRESULT FMediaPlayerHTML::GetBufferingProgress(long* lProgress)
{
	FString StrProgress;
	StrProgress = m_pBrowser.CallScriptStr("getBufferingProgress", "");
	StrProgress.Trim();
	if (StrProgress == "")
		return E_FAIL; 
	*lProgress = strtol(StrProgress, NULL, 10);
	return S_OK; 
}

HRESULT FMediaPlayerWMV::Init(HWND hWndParent, IMediaPlayerNotify* pNotify)
{
	m_pPlayerNotify = pNotify;
    m_hWndNotify = hWndParent; 
    Create(hWndParent, rcDefault, NULL, WS_CHILD | WS_VISIBLE); 
    HRESULT hr = Navigate(g_AppSettings.AppDir("data\\wmplayer.html"));
    return SUCCEEDED(hr) ? E_PENDING : hr;  
}

//////////////////////////////////////////////////////////////////////////

HRESULT FMediaPlayerMOV::Init(HWND hWndParent, IMediaPlayerNotify* pNotify)
{
	m_pPlayerNotify = pNotify;
	m_hWndNotify = hWndParent; 
	Create(hWndParent, rcDefault, NULL, WS_CHILD | WS_VISIBLE); 
	HRESULT hr = Navigate(g_AppSettings.AppDir("data\\movplayer.html"));
	return SUCCEEDED(hr) ? E_PENDING : hr;  
}

HRESULT FMediaPlayerMOV::LoadMedia(const tchar* pFileName, IMediaOptions* pOptions)
{
	FString StrOffs; 
	StrOffs.Format("%I64d", pOptions->rtOffset);
	HRESULT hr = m_pBrowser.CallJScript("ltv_loadMedia", pFileName, StrOffs);
	if (SUCCEEDED(hr))
		return E_PENDING; 

	return hr; 
}

//////////////////////////////////////////////////////////////////////////

HRESULT FMediaPlayerVLC::Init(HWND hWndParent, IMediaPlayerNotify* pNotify)
{
	m_pPlayerNotify = pNotify;
	m_hWndNotify = hWndParent; 
	Create(hWndParent, rcDefault, NULL, WS_CHILD | WS_VISIBLE); 
	HRESULT hr = Navigate(g_AppSettings.AppDir("data\\vlcplayer.html"));
	return SUCCEEDED(hr) ? E_PENDING : hr;  

}