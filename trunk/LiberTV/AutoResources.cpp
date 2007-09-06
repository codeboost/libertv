#include "stdafx.h"
#include "AutoResources.h"

void DisposeHInternet(HINTERNET handle )
{
	InternetCloseHandle(handle); 
}

void DisposeArray(void* pObj)
{
	delete[] pObj; 
}

void DisposeHandle(HANDLE handle )
{
	CloseHandle(handle); 
}