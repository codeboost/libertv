#include "stdafx.h"
#include "FCmdLine.h"
#include "winutils.h"

void FCmdLine::Set(const char* cmdLine)
{
	m_argv = CommandLineToArgvA((PCHAR)cmdLine, &m_argc); 
}

BOOL FCmdLine::HasParam(const char* paramName)
{
	for (int k = 0; k < m_argc; k++)
	{
		if (strstr(m_argv[k], paramName))
			return true; 
	}
	return false; 
}

FCmdLine::~FCmdLine()
{
	//GlobalFree(m_argv);
}

const char* FCmdLine::GetAt(int k)
{
	if (k < m_argc)
		return m_argv[k];
	return ""; 
}

FString FCmdLine::operator[]( int k )
{
	return GetAt(k);
}

int FCmdLine::GetArgc()
{
	return m_argc;
}

FCmdLine::FCmdLine()
{
	m_argv = NULL; 
	m_argc = 0;
}