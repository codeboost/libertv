#include "stdafx.h"
#include "FSubParser.h"


void FSubParser::ParseLine(const tchar* szLine)
{
    int nLen = (int)strlen(szLine); 
    if (nLen > 7)   //must have at least 7 chars: {1}{2}A
    {
        m_Subs.Add();
        SubDef& subLine = m_Subs.GetAt(m_Subs.GetCount() - 1 ); 
        tchar* psz = subLine.m_sText.GetBufferSetLength(nLen + 1); 
        int nScanned = sscanf(szLine, "{%d}{%d}%[^\0]", &subLine.m_FrameStart, &subLine.m_FrameEnd, psz);

        if (nScanned < 3)
            m_Subs.RemoveAt(m_Subs.GetCount() - 1);
    }
}

BOOL FSubParser::ParseSubFile(const tchar* pFileName)
{
    FILE* f = fopen(pFileName, "r"); 
    if (NULL == f)
        return FALSE; 


	fseek(f, 0, SEEK_END); 
	int fileSize = ftell(f); 
	if (fileSize == 0)
	{
		fclose(f); 
		return FALSE; 
	}
	fseek(f, 0, SEEK_SET); 

    const int subBufferLen = 2048;
    FString SubBuffer;
    char* pBuf = SubBuffer.GetBufferSetLength(subBufferLen);

    while (!feof(f))
    {
        fgets(pBuf, subBufferLen, f); 

        tchar* nl = strpbrk(pBuf, "\r\n");
        if (nl) *nl = 0; 

        ParseLine(pBuf); 
    }
	fclose(f);
    return TRUE; 
}

BOOL FSubParser::SetSubtitle(const tchar* pFileName)
{
    Clear(); 

    if (ParseSubFile(pFileName))
	{
		m_SubFileName = pFileName;
		return TRUE; 
	}
	return FALSE; 
}

BOOL FSubParser::IsNewSub(ulong ulFrameNum)
{
    for (size_t k = 0; k < m_Subs.GetCount(); k++)
    {
        if (ulFrameNum >= m_Subs[k].m_FrameStart && 
            ulFrameNum <  m_Subs[k].m_FrameEnd)
        {
            if (m_LastReturn != m_Subs[k].m_sText)
            {
                return TRUE; 
            }
        }
    }
    return m_LastReturn == "";
}

FString FSubParser::GetSubAt(ulong ulFrameNum)
{
    for (size_t k = 0; k < m_Subs.GetCount(); k++)
    {
        if (ulFrameNum >= m_Subs[k].m_FrameStart && 
            ulFrameNum <  m_Subs[k].m_FrameEnd)
        {
            if (m_LastReturn != m_Subs[k].m_sText)
                m_LastReturn = m_Subs[k].m_sText; 

            return m_LastReturn; 
        }
    }
    m_LastReturn = "";
    return m_LastReturn;
}

void FSubParser::Clear()
{
    m_Subs.RemoveAll();
	m_SubFileName = "";
}


