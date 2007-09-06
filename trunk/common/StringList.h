#pragma once

#include "utils.h"

//Primitive string list class
//Can save/load file

class LTVUTILDLL_API IStringList{
public:
	virtual void AddString(const tchar* pszString) = 0; 
	virtual void RemoveString(const tchar* pszString, BOOL bCaseSensitive = TRUE) = 0; 
	virtual BOOL FindString(const tchar* pszString, BOOL bCaseSensitive = TRUE) = 0; 
	virtual BOOL Load(const tchar* pszFileName) = 0; 
	virtual BOOL Save(const tchar* pszFileName = NULL) = 0; 
	virtual void Clear() = 0; 
	virtual size_t GetItemCount() = 0; 
	virtual const tchar* GetFileName() = 0; 
	virtual ~IStringList() {}
};

class LTVUTILDLL_API FStringList : public IStringList
{
	CAtlList<FString>	m_Strings; 
	FString				m_FileName; 
public:
	FStringList(); 
	~FStringList(); 
	void AddString(const tchar* pszString); 
	void RemoveString(const tchar* pszString, BOOL bCaseSensitive = TRUE);
	BOOL FindString(const tchar* pszString, BOOL bCaseSensitive = TRUE);
	BOOL Load(const tchar* pszFileName); 
	BOOL Save(const tchar* pszFileName = NULL); 
	void Clear(); 
	size_t GetItemCount(); 
	const tchar* GetFileName();
};
