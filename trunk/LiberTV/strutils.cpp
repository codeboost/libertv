#include "StdAfx.h"
#include "strutils.h"


int SplitStringToArray(const tchar* pszStr, FArray<FString> &aRes, const tchar* pSep )
{

	if (NULL == pszStr)
		return 0; 

	size_t len = strlen(pszStr); 

	if (len == 0)
		return 0; 

	ATLASSERT(pSep != NULL); 

	const tchar* pszCur = pszStr; 
	const tchar* pszLast = pszStr; 
	const tchar* pszEnd = pszStr + len; 

	for (;;)
	{
		while (pszLast < pszEnd && (*pszLast == ' ' || *pszLast == '\t' || *pszLast == '\n'))
			pszLast++;

		if (pszLast >= pszEnd)
			break; 

		pszCur = strstr(pszLast, pSep);

		if (NULL == pszCur)
			pszCur = pszEnd; 

		if (pszCur - pszLast > 1)
		{
			FString Str(pszLast,(int)(pszCur - pszLast));
			Str.Trim(); 
			aRes.Add(Str); 
		}
		pszLast = pszCur + 1; 
	}
	return (int)aRes.GetCount(); 
}
