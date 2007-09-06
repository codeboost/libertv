#ifndef __FUPDATER_H__
#define __FUPDATER_H__

#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#define max(a, b)  (((a) > (b)) ? (a) : (b))

#include "Utils.h"
#include "atlhttp.h"
#include "FDownloadInc.h"


#define UPDATE_EVENT_STARTING 1
#define UPDATE_EVENT_TICK 2
#define UPDATE_EVENT_STOPPING 3

class FUpdater : public IClipDownloadNotify
{
	class UpdateThread : public mz_Thread
	{
		void Thread(void*);
	};
	UpdateThread	m_UpdateThread; 
	mz_Event		m_Work; 

public:
	HWND	m_hWndNotify; 
	bool	m_bCancelled; 
	DHANDLE m_hDownload; 
	FString m_UpdateVersion; 
	mz_Sync	m_Sync; 


	FUpdater(); 
	~FUpdater(); 
	bool CheckNewVersion(HWND hWndNotify); 
	bool CheckNewVersionSync(DWORD dwEventType); 
	void Stop(); 
	bool DownloadUpdate(const tchar* pszURL, const tchar* pszSaveFileName); 
	void Notify(FDownloadAlert &alert);
	int Navigate(CAtlHttpClient& aClient, const tchar* pszUrl);
	BOOL CheckConnectable(HWND hWndNotify); 
	static BOOL IsValidUpdate(const char* pszCurrentVersion); 
	static BOOL ClearUpdateInfo(); 
};
#endif //__FUPDATER_H__