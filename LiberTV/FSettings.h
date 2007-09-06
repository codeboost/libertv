#ifndef __FSETTINGS_H__
#define __FSETTINGS_H__
#include "FFunctions.h"

class FSettings 
{
public:
	FString GetValueStr(const tchar* ValueName, const tchar* Section = "");
	int		GetValueInt(const tchar* ValueName, const tchar* Section = "");
	bool	SetValueStr(const tchar* ValueName, const tchar* ValueValue, const tchar* Section = "");
	int		SetValueInt(const tchar* ValueName, const tchar* Section, int ValueValue);

	bool	LoadConfig(const tchar* FileName);
	bool	SaveConfig(const tchar* FileName);

};

#endif //__FSETTINGS_H__