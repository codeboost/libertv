#ifndef __MZREGMATCH_H__
#define __MZREGMATCH_H__

//Simple Regular Expressions Matcher

class mzRegMatch 
{


        static bool  MatchStar(const char* szPattern, const char* szText); 
    public:

        //PatternEndState -- value to return if pattern ends before szText (eg. AB*CD <=>ABCDE)
        static bool  IsMatch(const char* szPattern, const char* szText, bool PatternEndState=true); 

		//Matches a URL (ignores http:// and port if it exists in url)
		static bool  IsUrlMatch(const char* szPattern, const char* szUrl); 
		static const char* GetAdjustedURL(const char* szUrl); 

		/*
		Matches szPattern in szText. Can force start and last stars.
		If our pattern is 'hello', we can force it to look like '*hello', 'hello*' or '*hello*'.
		Use ForceLastStar/ForceFirstStar to obtain this.
		If pattern already contains starting or ending *, the Forcelast/Forcefirst parameters are ignored.
		*/
		static bool MatchStarEx(const char* szPattern, const char* szText, bool bForceLastStar, bool bForceStar);
		static bool IsMatchEx(const char* szPattern, const char* szText, bool ForceLastStar, bool ForceFirstStar);

};

#endif //__MZREGMATCH_H__