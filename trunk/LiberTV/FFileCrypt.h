#pragma once

#include "Utils.h"
#include "mz_Synchro.h"


struct FCryptItem{
	FString m_FileName;
	HANDLE	m_FileHandle; 
	FCryptItem()
	{
		m_FileHandle = INVALID_HANDLE_VALUE; 
	}
};

HRESULT HijackCommandLine(); 

class FFileCrypt
{
public:
	FArray<FCryptItem> m_Items;
	mz_Sync			   m_CryptSync; 
	FFileCrypt()
	{
		//Init(); 
	}
	HRESULT Init(); 
	HRESULT AddCryptItem(FCryptItem& pItem); 

};

