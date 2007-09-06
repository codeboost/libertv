#include "stdafx.h"
#include "shellapi.h"
#include "StorageManager.h"
#include <assert.h>
#include "GlobalObjects.h"


#define STORAGE_INDEX_FILE "storage.i"

#if 0
bool FDeleteDirectory(LPCTSTR lpszDir, bool noRecycleBin = true)
{
	size_t len = _tcslen(lpszDir);

	TCHAR *pszFrom = new TCHAR[len+2];
	_tcscpy(pszFrom, lpszDir);
	pszFrom[len] = 0;
	pszFrom[len+1] = 0;

	SHFILEOPSTRUCT fileop;
	fileop.hwnd   = NULL;    // no status display
	fileop.wFunc  = FO_DELETE;  // delete operation
	fileop.pFrom  = pszFrom;  // source file name as double null terminated string
	fileop.pTo    = NULL;    // no destination needed
	fileop.fFlags = FOF_NOCONFIRMATION|FOF_SILENT;  // do not prompt the user

	if(!noRecycleBin)
		fileop.fFlags |= FOF_ALLOWUNDO;

	fileop.fAnyOperationsAborted = FALSE;
	fileop.lpszProgressTitle     = NULL;
	fileop.hNameMappings         = NULL;

	int ret = SHFileOperation(&fileop);
	delete [] pszFrom;  
	return (ret == 0);
}
#endif

//Load Storage Index, check existing files, etc
bool FStorageManager::InitStorage(const char* pStrStorageRoot)
{

	char CurrentDir[MAX_PATH] =	{0};
	CurrentDir[0] = '.';
	GetCurrentDirectory(MAX_PATH, CurrentDir);
	PathCombine(m_StorageRoot.GetBufferSetLength(MAX_PATH), CurrentDir, pStrStorageRoot);
	CreateDirectory(m_StorageRoot, NULL);
	return true;
}


bool FStorageManager::GetFileName(FString& pFileName, vidtype nShowID, fsize_type nRequiredSize)
{
	pFileName.Format("%08u.vf", nShowID); 
	return true;
}

bool FStorageManager::CreateNewFileName(FString& pFileName, vidtype nShowID, fsize_type nRequiredSize, bool CreateFileName)
{
	pFileName.Format("%08u.vf", nShowID); 
	return true;
}

bool FStorageManager::CleanupStorage(fsize_type nReqSize)
{
	return true; 
}

bool FStorageManager::ReserveStorage(vidtype VideoID, fsize_type nReqSize, FString& pOutFileName)
{
	return GetFileName(pOutFileName, VideoID, nReqSize); 
}

fsize_type FStorageManager::GetDiskFreeSpace()
{
	ULARGE_INTEGER ulAvail; 
	::GetDiskFreeSpaceEx(m_StorageRoot, &ulAvail, NULL, NULL);
	fsize_type fs = ulAvail.HighPart; 
	return (fs << 32) | ulAvail.LowPart;
}


//////////////////////////////////////////////////////////////////////////

/*FString FStorageManager::FileName(const char* pRelFile)
{
    return _PathCombine(m_StorageRoot, pRelFile); 
}
*/


