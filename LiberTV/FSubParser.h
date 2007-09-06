#pragma once


#include "Utils.h"


class FSubParser
{
    struct SubDef
    {
        ulong   m_FrameStart;
        FString m_sText;
        ulong   m_FrameEnd;
    };
    CAtlArray<SubDef> m_Subs;
    BOOL        ParseSubFile(const tchar* pFileName); 
    void        ParseLine(const tchar* szLine); 
public:
    FString     m_LastReturn; 
	FString		m_SubFileName; 
    BOOL        SetSubtitle(const tchar* pFileName); 
    FString     GetSubAt(ulong ulFrameNum); 
    BOOL        IsNewSub(ulong ulFrameNum); 
    BOOL        HasSubs(){return m_Subs.GetCount() > 0;}
    void        Clear();
};

