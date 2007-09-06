#pragma once
#include "StringList.h"

class FStringListSync : public IStringList
{
	FStringList m_List;
	mz_Sync		m_Sync; 
public:
	void AddString(const tchar* pszString); 
	void RemoveString(const tchar* pszString, BOOL bCaseSensitive = TRUE);
	BOOL FindString(const tchar* pszString, BOOL bCaseSensitive = TRUE);
	BOOL Load(const tchar* pszFileName); 
	BOOL Save(const tchar* pszFileName = NULL); 
	void Clear(); 
	size_t GetItemCount();
	const char* GetFileName();

};

