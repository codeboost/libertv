#include "stdafx.h"
#include "mzRegMatch.h"

bool mzRegMatch::MatchStar(const char* szPattern, const char* szText)
{

    while (*szPattern=='?' || *szPattern == '*')
    {
        if (*szPattern == '?' && 0 == *szText)
            return false; //end of text, but pattern requires another char

        szPattern++;
    }

    if (0 == *szPattern)
        return true; //end of pattern, everything is ok

    char cNext = *szPattern; 

    if (cNext == '\\')
    {
        cNext = szPattern[1];
        if (0 == cNext)
            return false; //invalid pattern, ends on escape
    }

    bool res = false; 

    for(;;)
    {
        if (cNext == *szText)
            res = IsMatch(szPattern, szText); 

        if (res || 0 == *szText++)
            break;
    }
    return res; 
}

 bool mzRegMatch::IsMatch(const char* szPattern, const char* szText, bool PatternEndState)
{
    
    for (; *szPattern; szPattern++, szText++)
    {
        if (0 == *szText)
            return (*szPattern == '*');
        switch(*szPattern)
        {
            case '?':
                break;
            case '*':
                return MatchStar(szPattern, szText); 
            case '\\':  //quoted char, move to the next and don't treat as special character
                szPattern++;
                if (0 == *szPattern)
                    return PatternEndState; // pattern ends on a '\', invalid
                break;
            default:
                if (*szPattern != *szText)
                    return false; 
        }
    }

    if (*szText != 0)
        return false; //no match

    return true; 
}

 const char* mzRegMatch::GetAdjustedURL(const char* szUrl)
 {
	 if (NULL == szUrl)
		 return NULL; 

		size_t len = strlen(szUrl); 

		//Check for "http://"
		if (len > 7 && 
			tolower(szUrl[0]) == 'h' && 
			tolower(szUrl[1]) == 't' && 
			tolower(szUrl[2]) == 't' &&
			tolower(szUrl[3]) == 'p' && 
			szUrl[4] == ':' &&
			szUrl[5] == '/' && 
			szUrl[6] == '/')
		{
			//check for "http://www.
			if (len > 11 && 
				tolower(szUrl[7]) == 'w' && 
				tolower(szUrl[8]) == 'w' && 
				tolower(szUrl[9]) == 'w' &&
				szUrl[10] == '.')
			{
				//http://www.url
				return szUrl+11;
			}
			//http://url
			return szUrl+7; 
		}

		if (len > 4 &&
			tolower(szUrl[7]) == 'w' && 
			tolower(szUrl[8]) == 'w' && 
			tolower(szUrl[9]) == 'w' &&
			szUrl[10] == '.')
		{
			return szUrl+4; 
		}

		return szUrl; 


 }
/*

  Ignores http:// and/or www. in the URL
  Note that http://user:password@host:port/ is not handled here
  Used with 'Type to find' edits in the interface

*/
bool mzRegMatch::IsUrlMatch(const char* szPattern, const char* szUrl)
{
	
	if (szUrl && szPattern)
	{
		const char* szUrlAdjusted = GetAdjustedURL(szUrl); 

		return IsMatch(szPattern, szUrlAdjusted);
	}

	return false; 
}


//////////////////////////////////////////////////////////////////////////

bool mzRegMatch::MatchStarEx(const char* szPattern, const char* szText, bool bForceLastStar, bool bForceStar)
{

    while (*szPattern == '*' || bForceStar)
    {
        if (0 == *szText)
            return false; //end of text, but pattern requires another char

		if (bForceStar)
			bForceStar = false; 
		else
			szPattern++;
    }

    if (0 == *szPattern)
        return true; //end of pattern, everything is ok

    char cNext = *szPattern; 

    if (cNext == '\\')
    {
        cNext = szPattern[1];
        if (0 == cNext)
            return false; //invalid pattern, ends on escape
    }

    bool res = false; 

    for(;;)
    {
        if (cNext == *szText)
            res = IsMatchEx(szPattern, szText, bForceLastStar, bForceStar); 

        if (res || 0 == *szText++)
            break;
    }
    return res; 
}



bool mzRegMatch::IsMatchEx(const char* szPattern, const char* szText, bool ForceLastStar, bool ForceFirstStar)
{

	if (*szPattern == '*')
		ForceFirstStar = false; 

//	if (szPattern[strlen(szPattern)-1] == '*')
//		ForceLastStar = false; 

	for (; *szPattern; szPattern++, szText++)
    {
        if (0 == *szText)
		{
			//we have a match if pattern ends in '*'
            return (*szPattern == '*' && *(szPattern + 1) == '\0');
		}


        switch(*szPattern)
        {
//            case '?':
  //              break;
            case '*':
                return MatchStarEx(szPattern, szText, ForceLastStar, false); 
            case '\\':  //quoted char, move to the next and don't treat as special character
                szPattern++;
                if (0 == *szPattern)
                    return ForceLastStar; // pattern ends on a '\', invalid
                break;
            default:
				if (ForceFirstStar)
				{
					return MatchStarEx(szPattern, szText, ForceLastStar, true); 
				}
                if (*szPattern != *szText)
                    return false; 
        }
    }

    if (*szText != 0)
	{
		if (*szPattern == 0)
			return ForceLastStar; 

        return false; //no match
	}

    return true; 
}