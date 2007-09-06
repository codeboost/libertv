#include "stdafx.h"
#include "FIniFile.h"
#include "strsafe.h"
#include "rc4.h"
#include <ctype.h>
#define INI_BUF_SIZE 4096


///-----

//Advances pPtr to the first non-whitespace character. Returns false if end-of-line has been reached (or end-of-string)
static inline bool SkipWhitespaces(const tchar*& pPtr)
{
    while (*pPtr == ' ' || *pPtr == '\t' && *pPtr!='\n' && *pPtr!= '\r' && *pPtr != '\0')
        pPtr++;

    if (*pPtr == '\n' || *pPtr == '\r' || *pPtr == '\0')
        return false; 
    return true; 
}

static inline bool SkipWhitespacesNewlines(const tchar*& pPtr, const tchar* pHigh)
{
	while ((*pPtr == ' ' || *pPtr == '\t' || *pPtr=='\n' || *pPtr== '\r') && *pPtr != '\0' && pPtr < pHigh)
		pPtr++;

	if (*pPtr == '\0' || pPtr >= pHigh)
		return false; 

	return true; 
}

static inline bool HasInvalidChars(const tchar* pPtr, const tchar* pHigh)
{
	while (pPtr < pHigh)
	{
		if (*pPtr < ' ' || *pPtr > 126)
			return true; 
		pPtr++; 
	}
	return false; 	
}

//Skips newline and space characters backwards, until a non-whitespace char is found 
//pLowLimit is the base pointer for pPtr (pPtr>=pLowLimit)
static inline void SkipWhitespaceBack(const tchar*& pPtr, const tchar* pLowLimit)
{
    const tchar* pOrgPtr = pPtr; 
    while (*pPtr == ' ' || *pPtr == '\n' || *pPtr == '\r' || *pPtr == '\0' && 
        pPtr > pLowLimit)
    {
        pPtr--;
    }

    //if pPtr has been altered (skipped bad chars), we need to increase it by one so that it points to the first 'bad' character
    if (pPtr < pOrgPtr)
        pPtr++; 
}
//////////////////////////////////////////////////////////////////////////      

//Parse a line from the ini file. Type is set to:
//LINE_BLANK if blank or contains just a '\n'
//LINE_COMMENT if starts with #, ; or does not contain a '='
//LINE_DATA if a valid ValName/Value pair is found (line like Name=Value)
void FIniConfig::IniLine::SetLine(const tchar* pszLine, size_t nLen)
{
    m_ValName = "";
    m_Value = "";

    
    m_dwFlags&=~INI_LINE_TYPE; 
    if (0 == nLen)
    {
        m_dwFlags |= INI_LINE_BLANK;
        return; 
    }


    if (*pszLine == '#' || *pszLine == ';')
    {
        m_ValName.SetString(pszLine, (int)nLen); 
        m_dwFlags |= INI_LINE_COMMENT;
        return; 
    }

    const tchar* szEq = strchr(pszLine, '=');
    size_t szEqL = szEq - pszLine; 
    if (szEqL >= nLen || szEqL < 3)
    {
        //= not found - set type to COMMENT
        m_dwFlags = INI_LINE_COMMENT;
        m_ValName.SetString(pszLine, (int)nLen); 
    }   
    else
    {
        m_dwFlags |= INI_LINE_DATA; 
        m_ValName.SetString(pszLine, (int)szEqL);
        m_Value.SetString(szEq + 1, (int)(nLen - szEqL - 1)); 
        m_ValName.Trim(); 
        m_Value.Trim(); 
        m_Value.Trim('\"');
        m_Value.Trim(); 
    }
}
FIniConfig::IniSection& FIniConfig::IniSection::operator = (const FIniConfig::IniSection& aNew)
{
	if (this != &aNew)
	{
		Clear(); 
		m_dwFlags = aNew.m_dwFlags; 
		m_Name = aNew.m_Name; 
		for (size_t k = 0; k < aNew.m_Lines.GetCount(); k++)
			m_Lines.Add(aNew.m_Lines[k]); 
	}
	return *this; 
}

FIniConfig::IniSection::IniSection(const IniSection& aNew)
{
	m_dwFlags = aNew.m_dwFlags; 
	m_Name = aNew.m_Name; 
	for (size_t k = 0; k < aNew.m_Lines.GetCount(); k++)
		m_Lines.Add(aNew.m_Lines[k]); 
}

void FIniConfig::IniSection::SetSection(const tchar* pszSection)
{
    if (NULL == pszSection)
        return; 

    const tchar* pszEnd = NULL; 

    if (*pszSection == '[')
    {
        pszSection++;
        pszEnd = strchr(pszSection, ']');
    }
    else
        pszEnd = pszSection + strlen(pszSection);

    if (NULL == pszEnd)
        return ; 

    m_Name.SetString(pszSection, (int)(pszEnd - pszSection)); 
}

FIniConfig::IniLine* FIniConfig::IniSection::FindLine(const tchar* pszValName)
{
    IniLine* pLine = NULL; 

    for (size_t k = 0; k < m_Lines.GetCount(); k++){
        if ((m_Lines[k].m_dwFlags & INI_LINE_TYPE) == INI_LINE_DATA)
        {
            if (m_Lines[k].m_ValName.CompareNoCase(pszValName) == 0)
                return &m_Lines[k];
        }
    }
    return NULL; 
}

void FIniConfig::IniLine::Escape()
{
    
}

void FIniConfig::IniSection::ModifyValue(const tchar* pszValueName, const tchar* pszValue, BOOL bAdd)
{
    IniLine* pLine = FindLine(pszValueName); 

    if (NULL != pLine)
    {
        pLine->m_Value = pszValue; 
    }
    else
    {
        if (bAdd)
        {
            m_Lines.Add();
            pLine = &m_Lines[m_Lines.GetCount() - 1];
            pLine->m_ValName = pszValueName; 
            pLine->m_Value = pszValue; 
            pLine->m_dwFlags = INI_LINE_DATA; 
            pLine->m_ValName.Trim();
            pLine->m_Value.Trim(); 
            pLine->m_ValName.Replace("=", "");
        }
    }
}

FString FIniConfig::IniSection::GetValue(const tchar* pszValue)
{
   IniLine* pLine = FindLine(pszValue); 
   if (NULL == pLine)
       return "";

   return pLine->m_Value;
}

DWORD FIniConfig::IniSection::GetValueDWORD(const tchar* pszValue, int radix)
{
    FString Value = GetValue(pszValue); 
    return strtoul(Value, NULL, radix); 
}

//////////////////////////////////////////////////////////////////////////
void FIniConfig::Clear()
{
    m_CurSection = 0; 
    for (size_t i = 0; i < m_Sections.GetCount(); i++)
        m_Sections[i].Clear(); 
	m_Sections.RemoveAll(); 
}

void FIniConfig::ParseLine(const tchar* pszLine, size_t len)
{
    ATLASSERT(pszLine != NULL); 

    if (NULL == pszLine || len == 0)
        return; 

    if (*pszLine == '[')
    {
		FString aStr(pszLine, len);
        AddSection(aStr); 
        return; 
    }

    if (m_Sections.GetCount() == 0)
    {
        m_CurSection = 0; 
        m_Sections.Add();
    }

    CAtlArray<IniLine>& Lines = m_Sections[m_CurSection].m_Lines; 
    Lines.Add();
    Lines[Lines.GetCount() - 1].SetLine(pszLine, len); 

}
              
void FIniConfig::AddSection(const tchar* pszSection)
{
	FIniConfig::IniSection* pSection = FindSection(pszSection);
	if (NULL == pSection)
	{
	    m_Sections.Add();
		m_CurSection = m_Sections.GetCount() - 1;
		m_Sections[m_CurSection].SetSection(pszSection); 
	}
	else
	{
		
	}
}

FIniConfig::IniSection* FIniConfig::FindSection(const tchar* pszName)
{
    for (size_t k = 0; k < m_Sections.GetCount(); k++)
    {
        if (m_Sections[k].m_Name.CompareNoCase(pszName) == 0)
            return &m_Sections[k];
    }
    return NULL;
}

BOOL FIniConfig::ModifyValue(const tchar* pszSection, const tchar* pszValueName, const tchar* pszValue, BOOL bAdd)
{
    IniSection* pSection = FindSection(pszSection); 
    if (NULL == pSection)
	{
		if (bAdd)
		{
			AddSection(pszSection); 
			pSection = FindSection(pszSection); 
			if (NULL == pszSection)
				return FALSE; 
		}
		else
			return FALSE; 
	}

	pSection->ModifyValue(pszValueName, pszValue, bAdd); 
    return TRUE; 
}
BOOL FIniConfig::ModifyValueDWORD(const tchar* pszSection, const tchar* pszValueName, DWORD dwValue, int radix, BOOL bAdd)
{
    char szValue[16]; 
    ultoa(dwValue, szValue, radix); 
    return ModifyValue(pszSection, pszValueName, szValue, bAdd); 
}

BOOL FIniConfig::ModifyValueUINT64(const tchar* pszSection, const tchar* pszValueName, UINT64 uiValue, int radix /* = 10 */, BOOL bAdd /* = TRUE */)
{
	char szValue[32]; 
	StringCbPrintf(szValue, 32, "%I64d", uiValue); 
	return ModifyValue(pszSection, pszValueName, szValue, bAdd); 
}


FString FIniConfig::GetValue(const tchar* pszSection, const tchar* pszValue)
{
    IniSection* pSection = FindSection(pszSection); 
    if (NULL == pSection)
        return ""; 

    return pSection->GetValue(pszValue); 
}

DWORD FIniConfig::GetValueDWORD(const tchar* pszSection, const tchar* pszValue, int radix /* = 10 */)
{
    IniSection* pSection = FindSection(pszSection); 
    if (NULL == pSection)
        return 0; 

    return pSection->GetValueDWORD(pszValue, radix); 
}

unsigned __int64 FIniConfig::GetValueUINT64(const tchar* pszSection, const tchar* pszValue, int radix /* = 10 */)
{
	IniSection* pSection = FindSection(pszSection); 
	if (NULL == pSection)
		return 0; 

	FString Value = pSection->GetValue(pszValue); 
	return _strtoui64(Value, NULL, radix); 
}

void FIniConfig::ExchangeValueDWORD(const tchar* pszSection, const tchar* pszValue, DWORD& dwVal, int radix, DWORD dwFlags /* = 0 */)
{
	if (dwFlags & FLAG_PUTVAL)
	{
		ModifyValueDWORD(pszSection, pszValue, dwVal, radix, dwFlags & FLAG_ADDIFNEW); 
	}
	else
	{
		dwVal = GetValueDWORD(pszSection, pszValue, radix);
	}
}

void FIniConfig::ExchangeValueStr(const tchar* pszSection, const tchar* pszValue, FString& pStr, DWORD dwFlags)
{
	if (dwFlags & FLAG_PUTVAL)
	{
		ModifyValue(pszSection, pszValue, pStr, dwFlags & FLAG_ADDIFNEW); 
	}
	else
	{
		pStr = GetValue(pszSection, pszValue); 
	}
}

void FIniConfig::ExchangeValueUINT64(const tchar* pszSection, const tchar* pszValue, unsigned __int64 &ui64Val, int radix, DWORD dwFlags /* = 0 */)
{
	if (dwFlags & FLAG_PUTVAL)
	{
		ModifyValueUINT64(pszSection, pszValue, ui64Val, radix, dwFlags & FLAG_ADDIFNEW); 
	}
	else
	{
		ui64Val = GetValueUINT64(pszSection, pszValue, radix);
	}
}

BOOL FIniConfig::SectionExists(const tchar* pszSection)
{
	return NULL != FindSection(pszSection); 
}

BOOL FIniConfig::ValueExists(const tchar* pszSection, const tchar* pszValueName)
{
	IniSection* pSec = FindSection(pszSection);
	IniLine* pLine = NULL; 
	if (pSec)
	{
		pLine = pSec->FindLine(pszValueName); 
	}
	return pLine != NULL; 
}


class FStrList{
public:

	CAtlList<FString> m_Lines; 
	
	void AddLineFmt(const tchar* pszFmt, ...)
	{
		va_list paramList;
		va_start(paramList, pszFmt);

		FString aStr; 
		aStr.FormatV(pszFmt, paramList); 

		m_Lines.AddTail(aStr); 
		va_end(paramList);
	}

	void AddLine(const tchar* pszLine){
		m_Lines.AddTail(pszLine); 
	}

	/*void AddLine(const tchar* pszLine)
	{
		m_Lines.AddTail(pszLine); 
	}
	*/
};


int FIniConfig::SaveToBuffer(char*& aBuffer, BOOL bIgnoreCrypt)
{
	FStrList aLst; 

	for (size_t iSection = 0; iSection < m_Sections.GetCount(); iSection++)
	{
		IniSection& Section = m_Sections[iSection]; 

		if (Section.m_Name.GetLength() > 0)
			aLst.AddLineFmt("[%s]\r\n", Section.m_Name);

		ulong ulType = 0; 
		for (size_t k = 0;  k < Section.m_Lines.GetCount(); k++)
		{
			IniLine& Line = Section.m_Lines[k];

			ulType = Line.m_dwFlags & INI_LINE_TYPE; 

			if (ulType == INI_LINE_DATA)
			{
				//Do not write directives which don't have a value
				//Clogs the ini file and will give same results anyway
				if (Line.m_Value.GetLength() > 0)
				{
					BOOL bHasSpace = FALSE; 
					bHasSpace = Line.m_Value.Find(" ") != -1;
					aLst.AddLine(Line.m_ValName); 
					aLst.AddLine(" = ");

					if (bHasSpace)							
						aLst.AddLine("\"");

					aLst.AddLine(Line.m_Value); 

					if (bHasSpace)							
						aLst.AddLine("\"");

					aLst.AddLine("\r\n"); 
				}
			}
			else
			{
				if (ulType ==  INI_LINE_BLANK)
					aLst.AddLine("\r\n"); 
				else
					if (ulType == INI_LINE_COMMENT)
					{
						aLst.AddLine(Line.m_ValName); 
						aLst.AddLine("\r\n"); 
					}
			}
		}
	}

	POSITION pos = aLst.m_Lines.GetHeadPosition();

	int nSize = 0; 
	while (pos != 0)
	{
		FString& aStr = aLst.m_Lines.GetNext(pos);
		nSize+=aStr.GetLength(); 
	}

	BOOL bMustEncrypt = m_IsEncrypted && !bIgnoreCrypt;

	if (nSize > 0)
	{
		if (bMustEncrypt)
		{
			nSize+=6; //header size
		}
		aBuffer = new char[nSize + 1];
		char* pBuf = aBuffer;

		if (bMustEncrypt)
		{
			strcpy(pBuf, "MTTI01"); 
			pBuf+=6;
		}

		pos = aLst.m_Lines.GetHeadPosition();
		while (pos != 0)
		{
			FString& aStr = aLst.m_Lines.GetNext(pos);
			strcpy(pBuf, aStr); 
			pBuf+=aStr.GetLength(); 
		}

		if (bMustEncrypt)
		{
			pBuf = aBuffer; 
			RC4_KEY rc4Key; 
			char pswd[32] = {0}; 
			strcpy(pswd, "libertv@nsa.gov");
			prepare_key((unsigned char*)pswd, strlen(pswd), &rc4Key); 
			rc4((unsigned char*)&pBuf[6], nSize - 6, &rc4Key); 
		}
	}
	return nSize ; 
}

BOOL FIniConfig::Save(const tchar* pszFileName, BOOL bIgnoreCrypt)
{
    if (NULL == pszFileName)
        pszFileName = m_FileName;

	if (NULL == pszFileName || strlen(pszFileName) == 0)
		return FALSE; 

	char* aBuffer = NULL; 
	FILE* f = fopen (pszFileName, "wb"); 
	if (NULL != f)
	{
		int nBufLen = SaveToBuffer(aBuffer, bIgnoreCrypt);
		if (nBufLen > 0)
		{
			fwrite(aBuffer, nBufLen, 1, f); 
		}
		delete[] aBuffer; 
		fclose(f); 
		return TRUE; 
	}
    return FALSE;
}

BOOL FIniConfig::ParseBuffer(tchar* pBuf, DWORD dwSize)
{
	const tchar* pLast = pBuf; 
	const tchar* pBufEnd = pBuf + dwSize; 
	int Lines = 0; 
	int BlankLines = 0; 

	if (!SkipWhitespacesNewlines(pLast, pBufEnd))
		return FALSE; 

	for (;pLast < pBufEnd;)
	{
		const tchar* pszEndP = strnchr(pLast, '\n', dwSize - (pLast - pBuf)); 
		if (pszEndP == NULL)
			pszEndP = pBufEnd; 

		const tchar* pszEnd = pszEndP; 

		SkipWhitespaceBack(pszEnd, pLast);

		if (pszEnd - pLast > 0 && pszEnd - pLast <= 2 )
			BlankLines++; //prevent from adding more than 5 blank lines.
		else
			BlankLines = 0; 

		if (BlankLines < 5 && !HasInvalidChars(pLast, pszEnd))
		{
			ParseLine(pLast, pszEnd - pLast); 
			Lines++; 
		}

		pLast = pszEndP + 1; 
		if (!SkipWhitespacesNewlines(pLast, pBufEnd))
			break; 
	}
	return Lines > 0; 
}

BOOL FIniConfig::LoadFromBuffer(char* pBuf, DWORD size)
{
	if (pBuf[0] == 'M' && pBuf[1] == 'T' && pBuf[2] == 'T' && pBuf[3] == 'I' && pBuf[4] == '0' && 
		pBuf[5] == '1')
	{
		size-=6; 
		pBuf+=6;
		m_IsEncrypted = TRUE; 
		RC4_KEY rc4Key; 
		char pswd[32] = {0}; 
		strcpy(pswd, "libertv@nsa.gov");
		prepare_key((unsigned char*)pswd, strlen(pswd), &rc4Key); 
		rc4((unsigned char*)pBuf, size, &rc4Key); 

		//TODO: Scan and detect non-alpha chars and null-terminate it
		pBuf[size] = 0;
	}
	return ParseBuffer(pBuf, size); 
}

BOOL FIniConfig::Load(const tchar* pszFileName)
{
    Clear(); 
	m_IsEncrypted = FALSE; 
	m_FileName = pszFileName;
    
	FILE* f = fopen(pszFileName, "rb");

    if (NULL == f)
        return FALSE; 

	fseek(f, 0, SEEK_END); 
	int size = ftell(f); 
	fseek(f, 0, SEEK_SET); 

	if (size == 0)
	{
		fclose(f); 
		return TRUE; 
	}

	char* pBuf = new char[size + 1];
	fread(pBuf, size, 1, f); 
	fclose(f); 
	pBuf[size] = 0; 
	BOOL bLoaded = LoadFromBuffer(pBuf, size);
	delete[] pBuf; 
	return bLoaded; 
}

void FIniConfig::AddSection(IniSection& aSection)
{
	IniSection* pSection = FindSection(aSection.m_Name); 
	if (NULL == pSection)
	{
		m_Sections.Add(aSection); 
	}
	else
	{
		*pSection = aSection; 
	}
}

void FIniConfig::CopySection(FIniConfig& aConfSource, const tchar* szSectionSource) 
{
	IniSection* pSection = aConfSource.FindSection(szSectionSource); 
	if (NULL != pSection)
		AddSection(*pSection); 
}

void FIniConfig::RemoveSection(const tchar* pszSectionName)
{
	for (size_t k =0 ; k < m_Sections.GetCount(); k++)
	{
		if (m_Sections[k].m_Name == pszSectionName)
		{
			m_Sections.RemoveAt(k, 1); 
			break; 
		}
	}
}

FIniConfig::FIniConfig()
{
	m_CurSection = 0; 
	m_IsEncrypted = FALSE; 
}

FIniConfig::FIniConfig(const FIniConfig& aConf)
{
	m_FileName = aConf.m_FileName; 
	m_CurSection = aConf.m_CurSection; 
	for (size_t k = 0; k < aConf.m_Sections.GetCount(); k++)
	{
		m_Sections.Add(aConf.m_Sections[k]); 
	}
}

FIniConfig& FIniConfig::operator = (const FIniConfig& aConf)
{
	if (this != &aConf)
	{
		Clear(); 
		m_IsEncrypted = aConf.m_IsEncrypted; 
		m_FileName = aConf.m_FileName; 
		m_CurSection = aConf.m_CurSection; 
		for (size_t k = 0; k < aConf.m_Sections.GetCount(); k++)
		{
			m_Sections.Add(aConf.m_Sections[k]); 
		}
	}
	return *this; 
}


