#ifndef f__UTILS_H__
#define f__UTILS_H__

#pragma once

#include <atlstr.h>
#include "mz_Synchro.h"
#include "mz_Thread.h"
#include "FTemplates.h"
#include "atlcoll.h"

#pragma  warning(disable:4251)

#ifndef min
	#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
	#define max(a,b)    (((a) > (b)) ? (a) : (b))
#endif


template <class T>
class FArray : public ATL::CAtlArray<T>
{
public:
	FArray& operator=(const FArray& rv)
	{
		if (this != &rv)
		{
			RemoveAll();
			for (size_t k = 0; k < rv.GetCount(); k++)
				Add(rv[k]);
		}
		return *this; 
	}
};
//typedef WTL::CRect CRect; 



typedef ATL::CString	FString; 

#define FList  ATL::CAtlList	 

typedef __int64  fsize_type;
typedef unsigned long FShowIDType; 
typedef TCHAR tchar; 


struct LTVUTILDLL_API LogSettings{
	FString m_LogFileName; 
	BOOL	m_LogEnabled; 
	LogSettings(){
		m_LogEnabled = FALSE; 
	}
};



LTVUTILDLL_API void _DBGAlert (const tchar* Mask, ...);
LTVUTILDLL_API void _DBGAlertM (const tchar* Mask, ...);
extern LTVUTILDLL_API LogSettings g_LogSettings; 
//Functions -- in Utils.cpp

LTVUTILDLL_API void _DBGAlert(const tchar*, ...); 
LTVUTILDLL_API std::string add_suffix(float val);
LTVUTILDLL_API std::string to_string(float v, int width, int precision = 3);

LTVUTILDLL_API bool f_WriteDword(FILE* f, dword dwVal); 
LTVUTILDLL_API bool f_LoadDword (FILE* f, dword& dwVal); 
LTVUTILDLL_API bool f_WriteString(FILE* f, const char* pStr); 
LTVUTILDLL_API bool f_LoadString(FILE* f, FString& aStrOut); 
LTVUTILDLL_API bool f_TextFileToString(const char* pText, FString& aStrOut); 


//String formatting routines
LTVUTILDLL_API FString	SizeString(LONGLONG lsize);			//adds bytes, KB, MB
LTVUTILDLL_API FString	SpeedString(LONGLONG lSpeed, const tchar* PerSecond = "/s");	//adds KB/s
LTVUTILDLL_API FString	PercentString(double val1, double val2);	//returns a string representing the percentual value of val1 of val2

LTVUTILDLL_API BOOL		ExtractQueryParam(const tchar* Query, const tchar* Param, FString& aOut);
//Escape for javascript
LTVUTILDLL_API void		EscapeJString(const char *svStr, FString& svOutStr);
LTVUTILDLL_API bool		StrSplit(const tchar* src, std::string& aLeft, std::string& aRight);
LTVUTILDLL_API BOOL		StrSplit(const tchar* pszSrc, FString& aLeft, FString& aRight, const char SplitChar = ':');
LTVUTILDLL_API FString	ExtractDomainFromUrl(const char* pStrUrl, int UpToLevel);
LTVUTILDLL_API BOOL		IsSameDomain(const char* pszUrl1, const char* pszUrl2); 
LTVUTILDLL_API void		StrReplaceChar(char* szStr, char ch, char chrepl); 
LTVUTILDLL_API FString	FormatString(const tchar* szFmt, ...);


//String utils
LTVUTILDLL_API void * __cdecl mzmemchr (const void* buf, int chr, size_t cnt);
LTVUTILDLL_API const char* stristr (const char* haystack, const char* needle); //case insensitive strstr
LTVUTILDLL_API const char* strinstre (const char* haystack, const char* needle, const char* end); //with end limit
LTVUTILDLL_API const char* strinstr (const char* haystack, const char* needle, int size); //with size
LTVUTILDLL_API const char* strnchre (const char* pStr, char pChar, const char* pEnd); 
LTVUTILDLL_API const char* strnchr (const char* pStr, char pChar, const int pLen);
LTVUTILDLL_API int mzstrcnt (const char* str, const char* lex);
//Returns the filename part from the URL. Returns "" if filename couldn't be extracted
LTVUTILDLL_API FString FileNameFromURL(const tchar* pszURL);
LTVUTILDLL_API FString LongToStr(long lVal);
LTVUTILDLL_API FString ULongToStr(unsigned long lVal);
LTVUTILDLL_API void StripHTMLTags(LPTSTR pszBuffer);


//FProfile
class FProfile
{
public:
	DWORD	m_dwStart; 
	FString m_StrFunction; 
	FProfile(const tchar* pszFunctionName)
	{
		m_StrFunction = pszFunctionName; 
		m_dwStart = GetTickCount(); 
	}

	~FProfile()
	{
		_DBGAlert("%s : %d milliseconds\n", m_StrFunction, GetTickCount() - m_dwStart); 
	}
};

#endif //__UTILS_H__