#pragma once

#include "mz_synchro.h"
#include "utils.h"

struct GuidInfo{
	DWORD m_dwValue; 
	GuidInfo(){
		m_dwValue = 0; 
	}
	GuidInfo(DWORD dwValue)
	{
		m_dwValue = dwValue; 
	}
};

typedef DWORD GuidType;

LTVUTILDLL_API GuidType RSSGuidFromString(const tchar* pszStr);

class LTVUTILDLL_API FRSSGuidMap 
{
	CAtlMap<GuidType, GuidInfo> m_GuidMap; 
	typedef CAtlMap<DWORD, GuidInfo>::CPair GuidMapPair; 
	mz_Sync m_Sync; 
	FString	m_FileName; 
public:
	void		SetGuid(const char* pszGuid, GuidInfo& info);
	GuidInfo	FindGuid(const char* pszGuid);
	GuidInfo	FindGuid(GuidType dwGuidCRC); 
	void		SetGuid(GuidType dwGuidCRC, GuidInfo& info); 
	BOOL		Save(const char* pszFileName = NULL); 
	BOOL		Load(const char* pszFileName = NULL); 
	FString		GetFileName(); 
};


#if 0
class FGuids
{
	struct FGuidItem{
		FString m_Guid; 
		DWORD	m_dwFlags; 
		FGuidItem(const char* pszGuid, DWORD dwFlags){
			m_Guid = pszGuid; 
			m_dwFlags = dwFlags; 
		}
	};
	FString m_FileName; 
	CAtlList<FGuidItem> m_Items; 
public:
	void	AddItem(const char* pszGuid, DWORD dwFlags);
	BOOL	FindItem(const char* pszGuid, DWORD& dwFlags); 
	BOOL	SetFlags(const char* pszGuid, DWORD dwNewFlags); 
	size_t	GetItemCount(); 
	BOOL	Save(const char* pszFileName = NULL); 
	BOOL	Load(const char* pszFileName);
	void	Clear(); 
	FString	GetFileName(); 

};
#endif