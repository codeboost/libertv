#ifndef __FLABELMGR_H__
#define __FLABELMGR_H__

#include "Utils.h"
#include "FIniFile.h"

class FLabelManager 
{

	FIniConfig		m_Conf; 
	mz_Sync			m_Sync; 
	FArray<FString> m_Labels; 
	int		FindLabel(const tchar* pszLabel); 
public:

	FLabelManager();
	virtual ~FLabelManager();
	FString GetLabels();
	void	RemoveLabel(const tchar* pszLabel);
	void	AddLabel(const tchar* pszLabel);
	BOOL	Load(const tchar* pszFileName); 
	BOOL	Save(const tchar* pszFileName = NULL); 
	BOOL	GetLabels(FArray<FString>& aLabels); 

};


#endif //__FLABELMGR_H__