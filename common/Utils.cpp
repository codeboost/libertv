#include "stdafx.h"
#include <sstream>
#include <string>
#include <math.h>
#include "Utils.h"
#include "StrSafe.h"
#include "winutils.h"
LogSettings g_LogSettings; 

std::string to_string(float v, int width, int precision)
{
	std::stringstream s;
	s.precision(precision);
	s.flags(std::ios_base::right);
	s.width(width);
	s.fill(' ');
	s << v;
	return s.str();
}

std::string add_suffix(float val)
{
	const char* prefix[] = {"B", "kB", "MB", "GB", "TB"};
	const int num_prefix = sizeof(prefix) / sizeof(const char*);
	for (int i = 0; i < num_prefix; ++i)
	{
		if (fabs(val) < 1000.f)
			return to_string(val, i==0?5:4) + prefix[i];
		val /= 1000.f;
	}
	return to_string(val, 6) + "PB";
}


FString SizeString(LONGLONG aSize)
{
	const int bS = 64; 
	tchar aBuf[bS];
	StrFormatByteSize64(aSize, aBuf, bS);
	return aBuf; 
}

FString SpeedString(LONGLONG aSpeed, const tchar* PerSecondStr)
{
	FString aSize = SizeString(aSpeed); 
	aSize+=PerSecondStr; 
	return aSize; 
}

FString PercentString(double val1, double val2)
{
	double fPercent = 0.0; 
	if (val2 > 0)
	{
		fPercent = (val1 / val2) * 100.0;
	}
	FString aStr; 
	aStr.Format("%.1f%%", fPercent); 
	return aStr; 
}

//_DbgAlert is in FTrayWnd.cpp
void x_DBGAlert (const tchar* Mask, ...)
{
	va_list paramList;
	va_start(paramList, Mask);
	char Temp[4096];
	StringCbVPrintf (Temp, 4096, Mask, paramList); 
	OutputDebugString(Temp); 

	va_end(paramList);
}

bool f_WriteDword(FILE* f, dword dwVal)
{
	bool bOk = false; 
	if (f != NULL)
	{
		bOk = (1 == fwrite(&dwVal, sizeof(dword), 1, f));
	}
	return bOk; 
}

bool f_WriteString(FILE* f, const char* pStr)
{
	bool bOk = false; 
	if (NULL != f)
	{
		dword nLen = pStr ? (dword)strlen(pStr) : 0;

		bOk = f_WriteDword(f, nLen); 

		if (bOk && NULL != pStr)
		{
			bOk = (1 == fwrite(pStr, nLen, 1, f)); 
		}
	}
	return bOk; 
}

bool f_LoadDword(FILE* f, dword& dwVal)
{
	bool bOk = false;
	if (NULL != f)
	{
		bOk = (1 == fread(&dwVal, sizeof(dword), 1, f));
	}
	return bOk;
}

#define MAX_STRING_SIZE 16384
bool f_LoadString(FILE* f, FString& aOutStr)
{
	bool bOk = false; 
	aOutStr = ""; 
	if (NULL != f)
	{
		dword dwLen = 0; 
		if (f_LoadDword(f, dwLen))
		{
			if (dwLen > 16384)
			{
				assert(false);
				return false; 
			}
			char* pStr = aOutStr.GetBufferSetLength(dwLen); 
			bOk = (1 == fread(pStr, dwLen, 1, f));
		}
	}
	return bOk;
}

bool f_TextFileToString(const char* pTextFile, FString& aStrOut)
{
	FILE* pf = fopen(pTextFile, "rb");
	bool bDone = false; 

	if (NULL != pf)
	{
		fseek(pf, 0, SEEK_END); 
		int aSize = ftell(pf); 
		if (aSize < 65535)	//protect from extreme sizes
		{
			fseek(pf, 0, SEEK_SET); 
			char* aBuffer = aStrOut.GetBufferSetLength(aSize + 1); 
			fread(aBuffer, aSize, 1, pf); 
			aBuffer[aSize] = 0;	 
			bDone = true;
		}
		fclose(pf); 
	}
	return bDone; 
}



FRect ScreenCenteredRect(int nReqWidth, int nReqHeight)
{
	RECT rc = {0};
	rc.left = GetSystemMetrics(SM_CXSCREEN) / 2 - nReqWidth / 2; 
	rc.top = GetSystemMetrics(SM_CYSCREEN) / 2 - nReqHeight / 2; 
	rc.right = rc.left + nReqWidth; 
	rc.bottom = rc.top + nReqHeight; 
	return rc; 
}

//////////////////////////////////////////////////////////////////////////

FString _RegStr(const tchar* pValueName, const tchar* pSubKey, HKEY hKeyRoot)
{
	DWORD dwType = REG_SZ;
	DWORD dwLen = 256; 
	tchar rStrbuf[256];
	DWORD dwErr = SHGetValue(hKeyRoot, pSubKey, pValueName, &dwType, rStrbuf, &dwLen);
	if (dwErr == ERROR_SUCCESS)
		return FString(rStrbuf); 
	return FString(""); 
}

dword _RegDword(const char* pValueName, const tchar* pSubKey, dword dwDefault, HKEY hKeyRoot)
{
	DWORD dwType = REG_SZ;
	DWORD dwLen = 256; 
	DWORD dwValOut; 

	DWORD dwErr = SHGetValue(hKeyRoot, pSubKey, pValueName, &dwType, &dwValOut, &dwLen);

	if (dwErr == ERROR_SUCCESS)
		return dwValOut; 

	return dwDefault; 
}

bool _RegSetStr(const tchar* pValueName, const tchar* pValue, const tchar *pSubKey, HKEY hKeyRoot)
{
	assert(NULL != pValueName);
	assert(NULL != pValue);

	return ERROR_SUCCESS == 
		SHSetValue(hKeyRoot, pSubKey, pValueName, REG_SZ, pValue, (DWORD)strlen(pValue));
}

bool _RegSetDword(const tchar* pValueName, dword dwValue, const tchar* pSubKey, HKEY hKeyRoot)
{
	return ERROR_SUCCESS == 
		SHSetValue(hKeyRoot, pSubKey, pValueName, REG_DWORD, &dwValue, sizeof(dword));
}

bool _SaveWindowPosition(HWND hWnd, const tchar* WindowName)
{
	char _ConvBuffer[64];
	RECT rcWindow = {0}; 
	if (GetWindowLong(hWnd, GWL_STYLE) & WS_MAXIMIZE)
		_RegSetStr(WindowName, "Maximized", LTV_REG_KEY"\\wPositions");
	else
	if (GetWindowRect(hWnd, &rcWindow))
	{

		StringCbPrintf(_ConvBuffer, 64, "%d,%d,%d,%d", rcWindow.left, rcWindow.top, rcWindow.right, rcWindow.bottom);
		return _RegSetStr(WindowName, _ConvBuffer, LTV_REG_KEY"\\wPositions");
	}
	return false; 
}

bool _CheckRect(RECT& rcWnd)
{
	if (rcWnd.left >= 0 && rcWnd.right  >= rcWnd.left && 
		rcWnd.top >= 0  && rcWnd.bottom >= rcWnd.top)
	{
		if (rcWnd.right - rcWnd.left <= GetSystemMetrics(SM_CXSCREEN) &&
			rcWnd.bottom - rcWnd.top <= GetSystemMetrics(SM_CYSCREEN))
		{
			return true; 
		}	
	}
	return false; 
}

int _LoadWindowPosition(const tchar* WindowName, RECT& rcWindowOut)
{
	FString StrPositions = _RegStr(WindowName, LTV_REG_KEY"\\wPositions");
	if (StrPositions.GetLength() > 0)
	{
		if (StrPositions == "Maximized")
			return 2; 
        FRect rcWindow;
		if (4 == sscanf(StrPositions, "%d,%d,%d,%d", &rcWindow.left, &rcWindow.top, &rcWindow.right, &rcWindow.bottom) &&
			_CheckRect(rcWindow))
        {
            rcWindowOut = rcWindow; 
            return 1; 
        }
	}
	return 0; 
}


void LimitMinWndSize(int MinWidth, int MinHeight, WPARAM wSide, LPARAM lParam)
{
	RECT *rc = (RECT*)lParam; 
	
	if (rc->right - rc->left < MinWidth)
	{
		if (wSide == WMSZ_BOTTOMRIGHT || wSide == WMSZ_RIGHT || wSide == WMSZ_TOPRIGHT)
			rc->right = rc->left +	MinWidth; 
		else
			rc->left = rc->right - MinWidth; 
	}

	if (rc->bottom - rc->top < MinHeight)
	{
		if (wSide == WMSZ_BOTTOMRIGHT || wSide == WMSZ_BOTTOM || wSide == WMSZ_BOTTOMLEFT)
			rc->bottom = rc->top + MinHeight; 
		else
			rc->top = rc->bottom - MinHeight; 
	}
}


FString GetWinUserDirectory(const tchar* pszSubDir)
{
	static char szPath[MAX_PATH];
//	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, pszSubDir, szPath); 
	SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, szPath); 
	PathCombine(szPath, szPath, pszSubDir); 
	return szPath; 
}

FString _PathCombine(const tchar* pszDir, const tchar* pszFile)
{                             
    ATLASSERT(pszFile != NULL); 
    ATLASSERT(pszDir != NULL); 
    char szPath[MAX_PATH];
    PathCombine(szPath, pszDir, pszFile);
	PathRemoveBackslash(szPath); 
	StrReplaceChar(szPath, '/', '\\');
	PathMakePretty(szPath);
    return FString(szPath);
}

BOOL ExtractQueryParam(const tchar* Query, const tchar* Param, FString& aOut)
{
	if (NULL != Query)
	{
		const tchar* pMatch = strstr(Query, Param); 
		if (pMatch)
		{
			pMatch+=strlen(Param); 

			while (*pMatch && *pMatch == ' ') pMatch++;

			if (*pMatch == '=')
			{
				pMatch++;
				while (*pMatch && *pMatch == ' ') pMatch++;	
				//read until end of string, space or &

				const tchar* pEnd = pMatch; 
				while (*pEnd && *pEnd != '&')
					pEnd++;

				aOut.SetString(pMatch, (int)(pEnd - pMatch)); 
				aOut.Trim();
				return TRUE; 
			}
		}
	}

	return FALSE; 
}

fsize_type GetFileSize(const tchar *fileName)
{
	BOOL                        fOk;
	WIN32_FILE_ATTRIBUTE_DATA   fileInfo;

	if (NULL == fileName)
		return 0;

	fOk = GetFileAttributesEx(fileName, GetFileExInfoStandard, (void*)&fileInfo);
	if (!fOk)
	{
		int err = GetLastError();
		_DBGAlert("GetFileSize error: %d\n", err); 
		return 0;
	}

	fsize_type hiPart = fileInfo.nFileSizeHigh;
	return (hiPart << 32) | fileInfo.nFileSizeLow;
}

/* What EscapeString returns must be FREEd. */
void EscapeJString(const char *svStr, FString& svOutStr)
{
	if (NULL == svStr)
	{
		svOutStr = ""; 
		return; 
	}

	/* Determine final size */
	size_t nEscapes,i,j,count;
	char *svOutBuf;

	nEscapes=0;
	count = strlen(svStr);
	for(i=0;i<count;i++) {
		if(svStr[i]=='\a') nEscapes++;
		else if(svStr[i]=='\b') nEscapes++;
		else if(svStr[i]=='\f') nEscapes++;
		else if(svStr[i]=='\n') nEscapes++;
		else if(svStr[i]=='\r') nEscapes++;
		else if(svStr[i]=='\t') nEscapes++;
		else if(svStr[i]=='\v') nEscapes++;
		else if(svStr[i]=='\\') nEscapes++;
		else if(svStr[i]=='\'') nEscapes++; 
		else if(svStr[i]<' ' || svStr[i]>'~') nEscapes+=3;
	}

	/* Allocate output buffer */

	int nBufferLen = (int)(strlen(svStr) + nEscapes + 1);
	svOutBuf = svOutStr.GetBufferSetLength( nBufferLen );


	// Escape things
	j=0;
	for(i=0;i<count;i++) {
		char c;
		c=svStr[i];
		if(c>=' ' && c<='~' && c != '\'') { svOutBuf[j]=c; j++; }
		else if(c=='\a') { svOutBuf[j]='\\'; svOutBuf[j+1]='a'; j+=2; }
		else if(c=='\b') { svOutBuf[j]='\\'; svOutBuf[j+1]='b'; j+=2; }
		else if(c=='\f') { svOutBuf[j]='\\'; svOutBuf[j+1]='f'; j+=2; }
		else if(c=='\n') { svOutBuf[j]='\\'; svOutBuf[j+1]='n'; j+=2; }
		else if(c=='\r') { svOutBuf[j]='\\'; svOutBuf[j+1]='r'; j+=2; }
		else if(c=='\t') { svOutBuf[j]='\\'; svOutBuf[j+1]='t'; j+=2; }
		else if(c=='\v') { svOutBuf[j]='\\'; svOutBuf[j+1]='v'; j+=2; }
		else if(c=='\\') { svOutBuf[j]='\\'; svOutBuf[j+1]='\\'; j+=2; }
		else if(c=='\'') { svOutBuf[j]='\\'; svOutBuf[j+1]='\''; j+=2;}
		else {
			StringCbPrintf(svOutBuf + j, nBufferLen, "\\x%1.1X%1.1X", (c>>4), c&15);
			j+=4;
		}
	}
	svOutBuf[j]='\0';
}

PCHAR*
CommandLineToArgvA(
				   PCHAR CmdLine,
				   int* _argc
				   )
{
	PCHAR* argv;
	PCHAR  _argv;
	ULONG   len;
	ULONG   argc;
	CHAR   a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = (ULONG)strlen(CmdLine);
	i = ((len+2)/2)*sizeof(PVOID) + sizeof(PVOID);

	argv = (PCHAR*)GlobalAlloc(GMEM_FIXED,
		i + (len+2)*sizeof(CHAR));

	_argv = (PCHAR)(((PUCHAR)argv)+i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while( a = CmdLine[i] ) {
		if(in_QM) {
			if(a == '\"') {
				in_QM = FALSE;
			} else {
				_argv[j] = a;
				j++;
			}
		} else {
			switch(a) {
				case '\"':
					in_QM = TRUE;
					in_TEXT = TRUE;
					if(in_SPACE) {
						argv[argc] = _argv+j;
						argc++;
					}
					in_SPACE = FALSE;
					break;
				case ' ':
				case '\t':
				case '\n':
				case '\r':
					if(in_TEXT) {
						_argv[j] = '\0';
						j++;
					}
					in_TEXT = FALSE;
					in_SPACE = TRUE;
					break;
				default:
					in_TEXT = TRUE;
					if(in_SPACE) {
						argv[argc] = _argv+j;
						argc++;
					}
					_argv[j] = a;
					j++;
					in_SPACE = FALSE;
					break;
			}
		}
		i++;
	}
	_argv[j] = '\0';
	argv[argc] = NULL;

	(*_argc) = argc;
	return argv;
}

//Extracts parts from strings of the form 'string:value': aLeft='string'; aRight='value';

bool StrSplit(const tchar* src, std::string& aLeft, std::string& aRight)
{
	if (src)
	{
		const tchar* pP = strchr(src, ':');
		if (pP && pP > src)
		{
			aLeft.assign(src, pP - src); 
			aRight.assign(pP + 1, strlen(src) - aLeft.length() - 1);
			return true; 
		}
	}
	return false;
}

BOOL StrSplit(const tchar* pszSrc, FString& aLeft, FString& aRight, const char SplitChar)
{
	if (pszSrc)
	{
		const tchar* pP = strchr(pszSrc, SplitChar);
		if (pP && pP > pszSrc)
		{
			int LeftLen = (int)(pP - pszSrc);
			aLeft.SetString(pszSrc, LeftLen);
			aRight.SetString(pP + 1, (int)strlen(pszSrc) - LeftLen - 1);
			return TRUE; 
		}
	}
	return FALSE; 
}

BOOL IsDots(const TCHAR* str) {
	if(_tcscmp(str,".") && _tcscmp(str,"..")) return FALSE;
	return TRUE;
}

#pragma warning(disable: 4995)
BOOL FDeleteDirectoryEx(const TCHAR* sPath) {
	HANDLE hFind;  // file handle
	WIN32_FIND_DATA FindFileData;

	TCHAR DirPath[MAX_PATH];
	TCHAR FileName[MAX_PATH];

	_tcscpy(DirPath,sPath);
	_tcscat(DirPath,"\\*");    // searching all files
	_tcscpy(FileName,sPath);
	_tcscat(FileName,"\\");

	hFind = FindFirstFile(DirPath,&FindFileData); // find the first file
	if(hFind == INVALID_HANDLE_VALUE) return FALSE;
	_tcscpy(DirPath,FileName);

	bool bSearch = true;
	while(bSearch) { // until we finds an entry
		if(FindNextFile(hFind,&FindFileData)) {
			if(IsDots(FindFileData.cFileName)) continue;
			_tcscat(FileName,FindFileData.cFileName);
			if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

				// we have found a directory, recurse
				if(!FDeleteDirectoryEx(FileName)) { 
					FindClose(hFind); 
					return FALSE; // directory couldn't be deleted
				}
				RemoveDirectory(FileName); // remove the empty directory
				_tcscpy(FileName,DirPath);
			}
			else {
//				if(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
//					_chmod(FileName, _S_IWRITE); // change read-only file mode
				if(!DeleteFile(FileName)) {  // delete the file
					FindClose(hFind); 
					return FALSE; 
				}                 
				_tcscpy(FileName,DirPath);
			}
		}
		else {
			if(GetLastError() == ERROR_NO_MORE_FILES) // no more files there
				bSearch = false;
			else {
				// some error occured, close the handle and return FALSE
				FindClose(hFind); 
				return FALSE;
			}

		}

	}
	FindClose(hFind);  // closing file handle

	return RemoveDirectory(sPath); // remove the empty directory

}

int DeleteFilesMask(const tchar* pszPath, const tchar* pszMask)
{
	WIN32_FIND_DATA   wFindData;


	FString FullPath = _PathCombine(pszPath, pszMask); 
	
	HANDLE hFindData = FindFirstFile(FullPath, &wFindData); 

	int iDeleted = 0; 

	if (hFindData != INVALID_HANDLE_VALUE)
	{
		for (;;)
		{
			if (!(wFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				FString FilePath = _PathCombine(pszPath, wFindData.cFileName); 
				_DBGAlert("Deleting: %s\n", FilePath); 
				
				if (DeleteFile(FilePath))
					iDeleted++;
				else
				{
					_DBGAlert("DeleteFilesMask error: %d\n", GetLastError()); 
				}
			}

			if (!FindNextFile(hFindData, &wFindData))
				break; 
		}
		FindClose(hFindData); 
	}
	return iDeleted; 
}

bool FDeleteDirectory(LPCTSTR lpszDir, bool noRecycleBin )
{
	size_t len = _tcslen(lpszDir);

	TCHAR *pszFrom = new TCHAR[len+2];
	StringCbCopy(pszFrom, len+2, lpszDir);
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

	if (ret != 0)
	{
		_DBGAlert("SHFileOperation return = %d\n", ret); 
	}
	return (ret == 0);
}



#define MAX_DOMAIN_LEVELS 2

//Sets pStart to the start of the domain, pLen to the length of the domain substring
bool Ptr_ExtractDomainFromUrl(const char* url, int UpToLevel, const char*& pStart, size_t& pLen)
{
	//Look for protocol start
	const char* d_Start	= strstr(url, "://"); 

	if (NULL == d_Start)
	{
		//Not found, use URL start (domain.com case)
		d_Start = url;
	}
	else
	{
		d_Start += 3;	//skip "://"
	} 

	//Find path start
	const char* d_End	= strchr(d_Start, '/'); 

	//Find Query Start
	const char* d_Qry  = strchr(d_Start, '?');

	//If query string exists, check if it starts before path
	//If this is the case, it is a malformatted url (domain.com?query), but we aren't gonna cry,
	//we'll use it as domain end (if no port is present).
	//Also, if Path (/) is not found and Query (?) is found, use it as end.
	if (d_Qry != NULL && (d_Qry < d_End || NULL == d_End))
		d_End = d_Qry;

	if (NULL == d_End)
	{
		//No path or query, use url end 
		d_End = url + strlen(url);
	}
	else
	{
		//path or query found, we are pointing at it, point to previouse caracter, as this is 
		//where the domain ends
		d_End--;
	}

	//Do we have user information, eg http://user:password@url.com?
	const char* p_Start	= strchr(url, '@'); 

	if (NULL != p_Start && p_Start > d_Start && p_Start < d_End) //see if it starts withing protocol && '/'
	{
		//use the first char after '@' as domain start, like in user:password@domain.com
		d_Start = p_Start + 1;
	} 

	//Skip any whitespace. This is a REALLY wrong URL, but we'll keep going
	while (*d_Start && *d_Start==' ' && d_Start < d_End)
		d_Start++; 


	//Now look for : . We want to see the port, but this may also be a funky URL like about:blank
	//we'll check for that in a sec
	const char* n_Domn	= strchr(d_Start, ':'); 

	if (NULL != n_Domn && n_Domn > d_Start && n_Domn < d_End)	//Found anything ?
	{

		const char* pPort = n_Domn + 1; 
		//Skip whitespace. This should never happen, but we'll do it just in case.
		while (*(pPort) && *(pPort)== ' ') 
			pPort++; 

		//if char following ':' is not a digit (aka port), then we consider this to be a 
		//funky domain, like 'about:blank' or 'javascript:whatever'.
		if (isdigit(*(pPort)))
		{
			//Set d_End to port start, excluding :
			d_End = n_Domn - 1;
		}
		else
			n_Domn = d_End; //Funky domain, use the whole URL

	}
	else
	{
		n_Domn = d_End;
	}

	const char* n_End	= d_End;

	// Is it an IP address?
	if (*n_End && *n_End < 'A')
	{
		pStart = d_Start;
		pLen = n_End - d_Start + 1;
		return true; 

	} 

	int lvl	= 0; 
	if (UpToLevel <= 0)
	{
		UpToLevel = 32768;
	}

	for (int k = 1; k <= UpToLevel; k++)
	{
		const char* last_Dmn= --n_Domn; 
		lvl++; 
		while (n_Domn-- > d_Start && *n_Domn != '.');
		//some kind of handling of .co.uk and the like:
		//If current subdomain length is 2 (like .uk or .us), go up one more domain
		if (UpToLevel == MAX_DOMAIN_LEVELS && lvl == 2 && last_Dmn - n_Domn <= 3)   	
			UpToLevel++; 

		if (n_Domn <= d_Start)
			break;
	}

	if (n_Domn == d_Start)
	{
		pStart = d_Start; 
		pLen = n_End - d_Start + 1;
		return true;
	} 

	//return mzString(n_Domn + 1, n_End - n_Domn);
	pStart = n_Domn + 1; 
	pLen = n_End - n_Domn; 
	return true; 
}

bool Buf_ExtractDomainFromUrl(const char* pStrUrl, char* pBuffer, int UpToLevel)
{

	const char* pStart = NULL; 
	size_t nLen = 0; 

	if (Ptr_ExtractDomainFromUrl(pStrUrl, UpToLevel, pStart, nLen))
	{
		strncpy(pBuffer, pStart, nLen); 
		pBuffer[nLen] = 0; 
		return true; 
	}
	return false; 
}

FString ExtractDomainFromUrl(const char* pStrUrl, int UpToLevel)
{
	FString S; 

	const char* pStart = NULL; 
	size_t nLen = 0; 

	if (Ptr_ExtractDomainFromUrl(pStrUrl, UpToLevel, pStart, nLen))
	{
		return FString(pStart, (int)nLen);
	}
	return "";
}

BOOL IsSameDomain(const char* pszUrl1, const char* pszUrl2)
{
	FString Domain1 = ExtractDomainFromUrl(pszUrl1, MAX_DOMAIN_LEVELS); 
	FString Domain2 = ExtractDomainFromUrl(pszUrl2, MAX_DOMAIN_LEVELS); 
	return (Domain1.CompareNoCase(Domain2) == 0);
}

void StrReplaceChar(char* szStr, char ch, char chr)
{
	size_t len = strlen(szStr); 
	for (size_t k = 0; k < len; k++)
	{
		if (*szStr == ch)
			*szStr = chr; 
		szStr++;
	}
}


BOOL PathAdjust(char* pszOut, const tchar* pszPath)
{
	if (PathCanonicalize(pszOut, pszPath))
	{
		PathUnquoteSpaces(pszOut); 
		StrReplaceChar(pszOut, '/', '\\');
		return TRUE; 
	}
	return FALSE; 
}


BOOL PathIsRelativeTo(const tchar* pszParent, const tchar* pszCheck)
{
	char Path1[MAX_PATH];
	char Path2[MAX_PATH];
	if (PathAdjust(Path1, pszParent) && PathAdjust(Path2, pszCheck))
	{
		strlwr(Path2);
		strlwr(Path1); 
		return (strstr(Path2, Path1) != NULL);
	}
	return FALSE; 
}

FString PathRemoveRoot(const tchar* pszPath, const tchar* pszRoot)
{
	ATLASSERT(pszPath != NULL); 
	ATLASSERT(strlen(pszPath) > 0);

	char Path[MAX_PATH];
	char Root[MAX_PATH];
	FString Res; 
	if (PathAdjust(Path, pszPath) && PathAdjust(Root, pszRoot))
	{
		strlwr(Path);
		strlwr(Root); 
		const tchar* psz = strstr(Path, Root); 
		if (NULL == psz)
			Res = Path;
		else
		{
			psz+=strlen(Root);
			//Substring must start with '\'
			if (*psz=='\\')
			{
				*psz++;
				Res = FString(psz);
			}
			else
				Res = pszPath; 
		}
		PathMakePretty(Res.GetBuffer());
	}
	else
		Res = pszPath; 
	return Res; 
}

FString FormatString(const tchar* szFmt, ...)
{
	FString Str; 
	va_list paramList;
	va_start(paramList, szFmt);
	Str.FormatV(szFmt, paramList);
	va_end(paramList); 
	return Str; 
}

int IsPathWriteable(const tchar* pszPath)	//returns last error (ERROR_SUCCESS if writable)
{
	FString StorageFile = _PathCombine(pszPath, "temp.ltv");
	HANDLE hFile = CreateFile(StorageFile, 0, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_DELETE_ON_CLOSE, NULL);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		return GetLastError(); 
	}
	CloseHandle(hFile); 
	return ERROR_SUCCESS; 
}

void * __cdecl mzmemchr (const void* buf, int chr, size_t cnt)
{
	while (cnt && (*(unsigned char *) buf != (unsigned char) chr))
	{
		buf = (unsigned char *) buf + 1;
		cnt--;
	}

	return(cnt ? (void *) buf : NULL);
}

const char* stristr (const char* haystack, const char* needle)
{
	const char* n, * tmp;

	for (; *haystack; haystack++)
	{
		for (tmp = haystack, n = needle; *tmp && *n && mztolower(*tmp) == mztolower(*n); tmp++, n++)
			;
		if (!*n)
			return haystack;
	}

	return NULL;
}

const char* strinstre (const char* haystack, const char* needle, const char* end)
{
	const char* n, * tmp;
	for (; haystack >= end || *haystack; haystack++)
	{
		for (tmp = haystack, n = needle; tmp < end && *tmp && *n && mztolower(*tmp) == mztolower(*n); tmp++, n++)
			;
		if (tmp > end)
			return NULL; 
		if (!*n)
			return haystack;
	}
	return NULL;
}

const char* strinstr (const char* haystack, const char* needle, int size)
{
	return strinstre(haystack, needle, haystack + size);
}

const char* strnchre (const char* pStr, char pChar, const char* pEnd)
{
	return (const char *) mzmemchr(pStr, pChar, pEnd - pStr);
}

const char* strnchr (const char* pStr, char pChar, const int pLen)
{
	return (const char *) mzmemchr(pStr, pChar, pLen);
}

int mzstrcnt (const char* str, const char* lex)
{
	int cnt			= 0; 
	const char* lst	= str; 
	size_t lexlen		= strlen(lex);
	for (; ;)
	{
		const char* tmp	= strstr(lst, lex);
		if (tmp)
			cnt++;
		else
			break;
		lst = tmp + lexlen;
	}

	return cnt;
}

FString StringToFileName(const tchar* pszString, int MaxLen)
{
	if (MaxLen >= MAX_PATH)
		MaxLen = MAX_PATH;
	if (MaxLen <= 0)
		MaxLen = 32; 

	CAutoPtr<char> newFileName; 
	newFileName.Attach(new char[MaxLen + 1]);

	const tchar* pszBadChars = "*!<>[]=+\"\\/,.:;%#@^&$?";

	char* pszNewFile = newFileName;

	for (int k = 0; k < MaxLen; k++)
	{
		if (*pszString == 0)
			break; 
		
		if (strchr(pszBadChars, *pszString) == 0)
			*pszNewFile = *pszString; 
		else
			*pszNewFile = '-';

		pszString++; 
		pszNewFile++; 
	}

	*pszNewFile = 0; 

	FString StrFileName = newFileName;
	StrFileName.Trim(); 
	return StrFileName; 
}


BOOL _DBGNow(FILE* f)
{
	if (f)
	{
		SYSTEMTIME Time; 
		GetLocalTime(&Time); 
		char TimeBfr[64];
		StringCbPrintf(TimeBfr, 64, "[%d/%d/%d %d:%d:%d] ", Time.wMonth, Time.wDay, Time.wYear, Time.wHour, Time.wMinute, Time.wSecond);
		return (fputs(TimeBfr, f) != EOF); 
	}
	return FALSE; 
}

void _DBGAlert (const tchar* Mask, ...)
{
	if (!g_LogSettings.m_LogEnabled)
		return; 

	va_list paramList;
	va_start(paramList, Mask);
	char Temp[4096];
	StringCbVPrintf (Temp, 4096, Mask, paramList); 
	OutputDebugString(Temp); 

	if (1)
	{
		FILE* f = fopen(g_LogSettings.m_LogFileName, "a");

		if (f)
		{
			_DBGNow(f); //write time
			fputs(Temp, f); 
			fclose(f); 
		}
	}
	va_end(paramList);
}

void _DBGAlertM (const tchar* Mask, ...)
{
	va_list paramList;
	va_start(paramList, Mask);
	char Temp[4096];
	StringCbVPrintf (Temp, 4096, Mask, paramList); 
	OutputDebugString(Temp); 
	FILE* f = fopen(g_LogSettings.m_LogFileName, "a");
	if (f)
	{	
		_DBGNow(f); //write time
		fputs(Temp, f); 
		fclose(f); 
	}

	va_end(paramList);
}

time_t FileTimeToUnixTime(LARGE_INTEGER *ltime)
{
	FILETIME filetime, localfiletime;
	SYSTEMTIME systime;
	struct tm utime;
	filetime.dwLowDateTime=ltime->LowPart;
	filetime.dwHighDateTime=ltime->HighPart;
	FileTimeToLocalFileTime(&filetime, &localfiletime);
	FileTimeToSystemTime(&localfiletime, &systime);
	utime.tm_sec=systime.wSecond;
	utime.tm_min=systime.wMinute;
	utime.tm_hour=systime.wHour;
	utime.tm_mday=systime.wDay;
	utime.tm_mon=systime.wMonth-1;
	utime.tm_year=systime.wYear-1900;
	utime.tm_isdst=-1;
	return(mktime(&utime));
}

fsize_type GetSpaceLeft(const tchar* pszDirectory)
{
	FString Path = pszDirectory; 
	PathStripToRoot(Path.GetBuffer()); 

	ULARGE_INTEGER ulAvail; 
	if (::GetDiskFreeSpaceEx(Path, &ulAvail, NULL, NULL))
	{
		fsize_type fs = ulAvail.HighPart; 
		return (fs << 32) | ulAvail.LowPart;
	}
	return 0; 
}

FString FileNameFromURL(const tchar* pszURL)
{
	FString FName = pszURL; 

	int nPos = FName.Find("?");
	if (nPos != -1)
		FName = FName.Left(nPos);

	nPos = FName.ReverseFind('/');
	if (nPos != -1)
		FName = FName.Mid(nPos + 1); 
	return FName; 
}

FString LongToStr(long lVal)
{
	char szVal[16];
	ltoa(lVal, szVal, 10); 
	return szVal;
}

FString ULongToStr(unsigned long lVal)
{
	char szVal[16];
	ultoa(lVal, szVal, 10); 
	return szVal; 
}

void StripHTMLTags(LPTSTR pszBuffer)
{
	BOOL bInTag = FALSE;
	LPTSTR pszSource = pszBuffer;
	LPTSTR pszDest = pszBuffer;

	while (*pszSource != '\0')
	{
		if (bInTag)
		{
			if (*pszSource == '>')
				bInTag = FALSE;
			pszSource++;
		}
		else
		{
			if (*pszSource == '<')
				bInTag = TRUE;
			else
			{
				*pszDest = *pszSource;
				pszDest++;
			}
			pszSource++;
		}
	}
	*pszDest = '\0';
}

BOOL EnsureDirExists(const tchar* pszName)
{
	if (NULL == pszName)
	{
		_DBGAlert("***NULL dir name passed\n");
		return FALSE; 
	}

	if (PathIsDirectory(pszName))
		return TRUE; 

	if (CreateDirectory(pszName, NULL))
		return TRUE; 
	
	_DBGAlert("Cannot create directory: %s\n", pszName); 
	return FALSE; 
}