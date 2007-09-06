#pragma once

#include "Utils.h"

class LTVUTILDLL_API FCmdLine
{

public:
	FCmdLine();
	~FCmdLine();

	void Set(const char* cmdLine); 
	BOOL HasParam(const char* paramName); 
	const char* GetAt(int k); 
	FString operator[](int k);
	int GetArgc();
protected:
	PCHAR* m_argv; 
	int    m_argc; 
};