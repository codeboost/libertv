#include "stdafx.h"
#include "FGuids.h"
#include "crc32.h"

//////////////////////////////////////////////////////////////////////////

GuidType RSSGuidFromString(const tchar* pszStr)
{
	if (NULL == pszStr)
		return 0; 
	return crc32(pszStr, strlen(pszStr), 0); 
}


void FRSSGuidMap::SetGuid( const char* pszGuid, GuidInfo& info )
{
	GuidType dwcrc32 = crc32(pszGuid, strlen(pszGuid), 0); 
	SetGuid(dwcrc32, info); 	
}

GuidInfo FRSSGuidMap::FindGuid( const tchar* pszGuid )
{
	SynchronizeThread(m_Sync); 
	GuidType dwcrc32 = crc32(pszGuid, strlen(pszGuid), 0); 
	return FindGuid(dwcrc32);
}

void FRSSGuidMap::SetGuid(GuidType dwGuidCRC, GuidInfo& info)
{
	SynchronizeThread(m_Sync); 
	m_GuidMap.SetAt(dwGuidCRC, info); 
}

GuidInfo FRSSGuidMap::FindGuid(GuidType dwGuidCRC)
{
	GuidInfo pInfo;
	if (dwGuidCRC == 0)
		return pInfo; 
	SynchronizeThread(m_Sync); 
	GuidMapPair *pPair = m_GuidMap.Lookup(dwGuidCRC); 
	if (pPair)
		pInfo =  pPair->m_value; 
	return pInfo; 
}

FString FRSSGuidMap::GetFileName()
{
	return m_FileName; 
}

BOOL FRSSGuidMap::Load(const char* pszFileName /* = NULL */)
{
	SynchronizeThread(m_Sync); 
	m_GuidMap.RemoveAll();

	m_FileName = pszFileName; 
	FILE* f = fopen(pszFileName, "rb");
	if (!f)
		return FALSE; 

	fseek(f, 0, SEEK_END); 
	DWORD size = ftell(f); 
	fseek(f, 0, SEEK_SET); 

	if (size == 0)
	{
		fclose(f); 
		return TRUE; 
	}

	DWORD nItems = size / sizeof(GuidInfo);
	GuidInfo* pInfo = new GuidInfo[nItems + 1];

	for (DWORD k = 0; k < nItems; k++)
	{
		GuidType dwGuid = 0; 
		if (1 != fread(&dwGuid, sizeof(GuidType), 1, f))
			break; 
		if (1 != fread(&pInfo[k], sizeof(GuidInfo), 1, f))
			break; 
		m_GuidMap.SetAt(dwGuid, pInfo[k]); 
	}
	delete[] pInfo; 
	fclose(f);
	return TRUE; 
}

BOOL FRSSGuidMap::Save(const char* pszFileName /* = NULL */)
{
	if (NULL == pszFileName)
		pszFileName = m_FileName; 
	
	FILE* f = fopen(pszFileName, "wb");

	if (NULL == f)
		return FALSE; 

	POSITION pos = m_GuidMap.GetStartPosition(); 

	while (pos)
	{
		GuidMapPair* pPair = m_GuidMap.GetNext(pos); 
		if (1 != fwrite(&pPair->m_key, sizeof(GuidType), 1, f))
			break; 
		if (1 != fwrite(&pPair->m_value, sizeof(GuidInfo), 1, f))
			break; 
	}
	fclose(f); 
	return TRUE; 
}

//////////////////////////////////////////////////////////////////////////








































#if 0

void FGuids::AddItem(const char* pszGuid, DWORD dwFlags)
{
	m_Items.AddTail(FGuidItem(pszGuid, dwFlags)); 
}

BOOL FGuids::FindItem(const char* pszGuid, DWORD& dwFlags)
{
	POSITION pos = m_Items.GetHeadPosition();
	while (pos != 0)
	{
		FGuidItem& pMatch = m_Items.GetNext(pos); 
		if (pMatch.m_Guid == pszGuid)
		{
			dwFlags = pMatch.m_dwFlags;
			return TRUE; 
		}
	}
	return FALSE; 
}

BOOL FGuids::SetFlags(const char* pszGuid, DWORD dwNewFlags)
{
	POSITION pos = m_Items.GetHeadPosition();
	while (pos != 0)
	{
		FGuidItem& pMatch = m_Items.GetNext(pos); 
		if (pMatch.m_Guid == pszGuid)
		{
			pMatch.m_dwFlags = dwNewFlags; 
			return TRUE; 
		}
	}
	return FALSE; 
}

size_t FGuids::GetItemCount()
{
	return m_Items.GetCount(); 
}

FString FGuids::GetFileName()
{
	return m_FileName; 
}

BOOL FGuids::Save(const char* pszFileName /* = NULL */)
{
	if (pszFileName == NULL)
		pszFileName = m_FileName; 

	FILE* f = fopen(pszFileName, "wb");

	if (f == NULL)
		return FALSE; 

	POSITION pos = m_Items.GetHeadPosition();
	while (pos != 0)
	{
		FGuidItem& pItem = m_Items.GetNext(pos); 

		DWORD dwLen = pItem.m_Guid.GetLength(); 
		if (dwLen > 4096)
			dwLen = 4096;
		if (1 != fwrite(&pItem.m_dwFlags, sizeof(DWORD), 1, f)) break; 
		if (1 != fwrite(&dwLen, sizeof(DWORD), 1, f)) break; 
		if (dwLen != fwrite(pItem.m_Guid.GetBuffer(), 1, dwLen, f))	break; 
	}
	fclose(f); 
	return TRUE; 
}

BOOL FGuids::Load(const char* pszFileName)
{
	m_FileName = pszFileName; 

	FILE* f = fopen(pszFileName, "rb");

	if (NULL == f)
		return FALSE; 

	while (!feof(f))
	{
		DWORD dwFlags; 
		DWORD dwLen; 
		char pszBuffer[4097];

		if (1 != fread(&dwFlags, sizeof(DWORD), 1, f)) break; 
		if (1 != fread(&dwLen, sizeof(DWORD), 1, f)) break; 
		if (dwLen > 4096)
			break; 
		if (dwLen != fread(pszBuffer, 1, dwLen, f)) break; 
		pszBuffer[dwLen] = 0;
		AddItem(pszBuffer, dwFlags); 
	}
	fclose(f); 
	return TRUE; 
}

void FGuids::Clear()
{
	m_Items.RemoveAll(); 
}
#endif