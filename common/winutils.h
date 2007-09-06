#pragma once

#include <atlctrls.h>
#include <atlmisc.h>
#include "utils.h"
#include <dshow.h>

#define LTV_REG_ROOT HKEY_CURRENT_USER
#define LTV_REG_KEY  "Software\\LiberTV"
#define LTV_REG_SUBKEY_STRLEN 64

typedef WTL::CRect FRect; 

//Loads a string from the string table
LTVUTILDLL_API FString LoadLangString(int idiString); 
#define _Lang(x) LoadLangString((x))

LTVUTILDLL_API FRect ScreenCenteredRect(int nReqWidth, int nReqHeight);
//Registry functions
LTVUTILDLL_API FString _RegStr(const tchar* pValueName, const tchar* pSubKey = LTV_REG_KEY, HKEY hKeyRoot = LTV_REG_ROOT ); 
LTVUTILDLL_API dword   _RegDword(const tchar* pValueName, const tchar* pSubKey = LTV_REG_KEY, dword dwDefault = 0, HKEY hKeyRoot = LTV_REG_ROOT); 
LTVUTILDLL_API bool	_RegSetDword(const tchar* pValueName, dword dwValue, const tchar* pSubKey = LTV_REG_KEY, HKEY hKeyRoot = LTV_REG_ROOT); 
LTVUTILDLL_API bool	_RegSetStr(const tchar* pValueName, const tchar* pValue, const tchar *pSubKey = LTV_REG_KEY, HKEY hKeyRoot = LTV_REG_ROOT); 

//Windowing utilities

//Saves window position to registry
LTVUTILDLL_API bool	_SaveWindowPosition(HWND hWnd, const tchar* WindowName);
//Loads window position from registry. Returns 0 = failure, 1 = ok, 2 = maximized
LTVUTILDLL_API int	_LoadWindowPosition(const tchar* WindowName, RECT& rcWindow);

LTVUTILDLL_API void	LimitMinWndSize(int MinWidth, int MinHeight, WPARAM wParam, LPARAM lParam);
LTVUTILDLL_API FString	GetWinUserDirectory(const tchar* pszSubDir = ""); 
LTVUTILDLL_API FString _PathCombine(const tchar* pszDir, const tchar* pszFile);
LTVUTILDLL_API fsize_type GetFileSize(const tchar* pszFileName); 
LTVUTILDLL_API fsize_type GetSpaceLeft(const tchar* pszFolder);
LTVUTILDLL_API PCHAR* CommandLineToArgvA(PCHAR CmdLine, int* _argc );
//Recursively deletes directory!
LTVUTILDLL_API bool FDeleteDirectory(LPCTSTR lpszDir, bool noRecycleBin = true);
LTVUTILDLL_API BOOL FDeleteDirectoryEx (const tchar* pszPath); 

//Detele files based on mask: DeleteFiles("E:\\Storage\\1000", "1000*.sub"); 
LTVUTILDLL_API int DeleteFilesMask(const tchar* pszPath, const tchar* pszMask); 
LTVUTILDLL_API BOOL		PathIsRelativeTo(const tchar* pszParent, const tchar* pszSubDir); 
//Checks if dir exists; creates it if it doesn't, checks if path is writable; returns TRUE on success
LTVUTILDLL_API BOOL EnsureDirExists(const tchar* pszName);

//Removes the 'root path' from a path:
//path = C:\\MyPath\SomePath; pszRoot = C:\\MyPath; result = SomePath
//Only works if pszPath is a subdirectory of pszRoot
LTVUTILDLL_API FString	PathRemoveRoot(const tchar* pszPath, const tchar* pszRoot); 
//returns ERROR_SUCCESS if writable, or GetLastError() if not
LTVUTILDLL_API int IsPathWriteable(const tchar* pszPath);
LTVUTILDLL_API time_t FileTimeToUnixTime(LARGE_INTEGER *ltime);


//Returns a valid filename from an arbitrary string.
//Replaces all invalid chars with _
//Filename is truncated to MaxLen chars
LTVUTILDLL_API FString StringToFileName(const tchar* pszString, int nMaxLen = 64);

