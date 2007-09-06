#include "stdafx.h"
#include "StringList.h"


FStringList::FStringList()
{

}

FStringList::~FStringList()
{
	Clear();
}


void FStringList::AddString(const tchar* pszString)
{
	const tchar* pszEnd = strchr(pszString, '\n');
	if (pszEnd == NULL)
		pszEnd = pszString + strlen(pszString); 

	if (pszEnd - pszString > 0)
		m_Strings.AddTail(FString(pszString, (int)(pszEnd - pszString))); 
}

BOOL FStringList::Load(const tchar* pszFileName)
{
	m_FileName = pszFileName; 

	FILE* f = fopen(pszFileName, "r"); 

	if (f)
	{
		while (!feof(f))
		{
			char Buf[2048] = {0};
			if (NULL != fgets(Buf, 2048, f))
				AddString(Buf); 
		}
		fclose(f); 
		return TRUE; 
	}
	return FALSE; 
}

BOOL FStringList::Save(const tchar* pszFileName /* = NULL */)
{
	if (pszFileName == NULL)
		pszFileName = m_FileName; 

	FILE* f = fopen(pszFileName, "w"); 
	if (f)
	{
		POSITION pos = m_Strings.GetHeadPosition();
		while (pos != 0)
		{
			FString& aStr = m_Strings.GetNext(pos); 
			fputs(aStr, f); 
			fputs("\n", f); 
		}

		fclose(f); 
		return TRUE; 
	}
	return FALSE; 
}

BOOL FStringList::FindString(const tchar* pszString, BOOL bCaseSensitive)
{
	POSITION pos = m_Strings.GetHeadPosition();
	while (pos != 0)
	{
		FString& aStr = m_Strings.GetNext(pos); 
		if (bCaseSensitive)
		{
			if (aStr == pszString)
				return TRUE;
		}
		else
		{
			if (aStr.CompareNoCase(pszString) == 0)
				return TRUE; 
		}
	}
	return FALSE; 
}

void FStringList::RemoveString(const tchar* pszString, BOOL bCaseSensitive)
{
	POSITION pos = m_Strings.GetHeadPosition();
	
	while (pos != 0)
	{
		POSITION oldPos = pos; 
		FString& aStr = m_Strings.GetNext(pos); 
		if (bCaseSensitive)
		{
			if (aStr == pszString)
			{
				m_Strings.RemoveAt(pos);
				break; 
			}
		}
		else
		{
			if (aStr.CompareNoCase(pszString) == 0)
			{
				m_Strings.RemoveAt(oldPos);
				break; 
			}
		}
	}
}

void FStringList::Clear()
{
	m_Strings.RemoveAll(); 
}

size_t FStringList::GetItemCount()
{
	return m_Strings.GetCount(); 
}

const char* FStringList::GetFileName()
{
	return m_FileName; 
}
