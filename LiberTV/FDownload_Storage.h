#ifndef __FDOWNLOAD_STORAGE_H__
#define __FDOWNLOAD_STORAGE_H__
#include "FDownload.h"


class FDownload_Storage
{
public:
	BOOL	LoadStorage(BOOL bRemoveStorage = FALSE); 
	void	DeleteStorage(FDownloadEx* pDownload); 
	BOOL	DeleteStorage(BOOL bDeleteFolder); 
	BOOL	LoadOldStorage(); 
	BOOL	LoadStorage(FArray<FDownloadEx*>& pDownloads); 
	static vidtype ConvertDownload(const tchar* pszOrgFile, const tchar* pszOutDir, FDownloadEx* pDownloadOut);
	static vidtype InitStorage(FDownloadEx* pDownload, const tchar* pszDataDir); 
};


#endif //__FDOWNLOAD_STORAGE_H__