#pragma once

#include "FDownloadInc.h"

class IDownloadDispatcher 
{
public:
	virtual BOOL	OpenMTTFromHTTP(FDownloadInfo& pInfo) = 0;
	virtual DWORD	LoadDownload(FDownloadInfo& pInfo) = 0; 
};


class FDownloadDispatcher : public IDownloadDispatcher, IClipDownloadNotify
{
protected:
	BOOL	DownloadMTT(FDownloadInfo* pInfo); 
	FFastSyncSemaStack<FDownloadInfo> m_QueuedDownloads;
	class FDispThread : public mz_Thread //Queued Downloads dispatcher thread
	{void Thread(void* p);};
	friend class FDispThread;
	FDispThread m_DispThread; 
	mz_Sync		m_Sync; 
public:

	FDownloadDispatcher();
	~FDownloadDispatcher(); 
	BOOL	Init(); 
	BOOL	OpenMTTFromHTTP(FDownloadInfo& pInfo);
	DWORD	LoadDownload(FDownloadInfo& pInfo);
	//Load download asynchronously
	BOOL	LoadDownloadAsync(FDownloadInfo& pInfo); 
	//IClipDownloadNotify
	void	Notify(FDownloadAlert& alert);
};