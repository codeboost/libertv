#include "stdafx.h"
#include "FVideoHistory.h"


void FStringListSync::AddString(const tchar* pszString)
{
	SynchronizeThread(m_Sync); 
	m_List.AddString(pszString); 
}

void FStringListSync::Clear()
{
	SynchronizeThread(m_Sync); 
	m_List.Clear(); 
}

BOOL FStringListSync::FindString(const tchar* pszString, BOOL bCaseSensitive /* = TRUE */)
{
	SynchronizeThread(m_Sync); 
	return m_List.FindString(pszString, bCaseSensitive); 
}

BOOL FStringListSync::Load(const tchar* pszFileName)
{
	SynchronizeThread(m_Sync);
	return m_List.Load(pszFileName); 
}

BOOL FStringListSync::Save(const tchar* pszFileName /* = NULL */)
{
	SynchronizeThread(m_Sync); 
	return m_List.Save(pszFileName); 
}

void FStringListSync::RemoveString(const tchar* pszString, BOOL bCaseSensitive /* = TRUE */)
{	
	SynchronizeThread(m_Sync); 
	m_List.RemoveString(pszString, bCaseSensitive); 
}

size_t FStringListSync::GetItemCount()
{
	SynchronizeThread(m_Sync); 
	return m_List.GetItemCount(); 
}

const char* FStringListSync::GetFileName()
{
	return m_List.GetFileName(); 
}

//////////////////////////////////////////////////////////////////////////


