#include "stdafx.h"
#include "FFileCrypt.h"
#include "mzdetour.h"

int HandleException(LPCSTR lpErrorMsg, int nCode){
	return EXCEPTION_CONTINUE_SEARCH;
}



typedef BOOL (WINAPI ReadFileDetour)(
									  HANDLE hFile,                // handle to file
									  LPVOID lpBuffer,             // data buffer
									  DWORD nNumberOfBytesToRead,  // number of bytes to read
									  LPDWORD lpNumberOfBytesRead, // number of bytes read
									  LPOVERLAPPED lpOverlapped    // overlapped buffer
									  );

typedef HANDLE (WINAPI CreateFileDetour)(
									  LPCTSTR lpFileName,                         // file name
									  DWORD dwDesiredAccess,                      // access mode
									  DWORD dwShareMode,                          // share mode
									  LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
									  DWORD dwCreationDisposition,                // how to create
									  DWORD dwFlagsAndAttributes,                 // file attributes
									  HANDLE hTemplateFile                        // handle to template file
									  );

typedef LPTSTR (WINAPI GetCommandLineDetour)();




static ReadFileDetour* g_pRealReadFile = NULL; 
static CreateFileDetour* g_pRealCreateFile = NULL; 
static GetCommandLineDetour* g_pGetCommandLine = NULL; 
extern FFileCrypt* g_pFileCrypt;

BOOL WINAPI FReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	/*
	SynchronizeThread(g_pFileCrypt->m_CryptSync); 
	for (size_t k = 0; k < g_pFileCrypt->m_Items.GetCount(); k++)
	{
		FCryptItem& pItem = g_pFileCrypt->m_Items[k];
		if (pItem.m_FileHandle == hFile)
		{
			//_DBGAlert("%s - Reading %d bytes\n", pItem.m_FileName, nNumberOfBytesToRead); 
		}
	}
	*/
	return g_pRealReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped); 
}

HANDLE FCreateFile(
				   LPCTSTR lpFileName,                         // file name
				   DWORD dwDesiredAccess,                      // access mode
				   DWORD dwShareMode,                          // share mode
				   LPSECURITY_ATTRIBUTES lpSecurityAttributes, // SD
				   DWORD dwCreationDisposition,                // how to create
				   DWORD dwFlagsAndAttributes,                 // file attributes
				   HANDLE hTemplateFile                        // handle to template file
				   )
{

//	SynchronizeThread(g_pFileCrypt->m_CryptSync); 
	HANDLE hFile = g_pRealCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
//	Outp("CreateFile(): %s\n", lpFileName); 
	/*
	for (size_t k = 0; k < g_pFileCrypt->m_Items.GetCount(); k++)
	{
		if (g_pFileCrypt->m_Items[k].m_FileName.CompareNoCase(lpFileName) == 0)
		{
//			_DBGAlert("CreateFile(): Found crypt item: %s - 0x%x\n", lpFileName, hFile); 
			g_pFileCrypt->m_Items[k].m_FileHandle = hFile; 
		}
	}
	*/
	return hFile;
}


LPTSTR FGetCommandLine(){
	return "";
}

HRESULT HijackCommandLine()
{
	//SynchronizeThread(g_pFileCrypt->m_CryptSync); 
	HRESULT hr = E_FAIL; 
	__try
	{

		if (g_pGetCommandLine == NULL)
		{
			hr = mz_DetourFn(GetModuleHandle("kernel32.dll"), "GetCommandLineA", (VOID*)FGetCommandLine, (VOID**)&g_pGetCommandLine);
		}
		
		/*
		if (g_pRealReadFile == NULL)
		{
			hr = mz_DetourFn(GetModuleHandle("kernel32.dll"), "ReadFile", (VOID*)FReadFile, (VOID**)&g_pRealReadFile);
			if (SUCCEEDED(hr))
			{
				hr = mz_DetourFn(GetModuleHandle("kernel32.dll"), "CreateFileA", (VOID*)FCreateFile, (VOID**)&g_pRealCreateFile);
			}
		}
		*/

	} __except(HandleException("Detour_UnhijackAll()", GetExceptionCode())){}

	return hr; 
}

HRESULT FFileCrypt::Init()
{
	//return HijackFiles();
	return E_FAIL; 
}

HRESULT FFileCrypt::AddCryptItem(FCryptItem& pItem)
{
	SynchronizeThread(m_CryptSync); 
	m_Items.Add(pItem); 
	return S_OK; 
}