// AppCheck.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include "AppCheck.h"

std::string GetQTVersion()
{
	DWORD dwValue;
	DWORD valueType, valueSize = sizeof(dwValue);
	DWORD valueDefault = 0;
	LONG res = SHRegGetUSValue("SOFTWARE\\Apple Computer, Inc.\\QuickTime\\ActiveX", "QTVersion", 
		&valueType, &dwValue, &valueSize, TRUE, &valueDefault, sizeof(DWORD));

	if (ERROR_SUCCESS == res && 0 != dwValue)
	{
		char ver[255];
		itoa(dwValue, ver, 16);
		return std::string(ver);
	}
	return "";
}

std::string GetVLCVersion()
{
	char szValue[255];
	DWORD valueType;
	DWORD szSize = 255;
	DWORD valueDefault = 0;
	LONG res = SHRegGetUSValue("SOFTWARE\\VideoLAN\\VLC", "Version", &valueType, &szValue, &szSize, TRUE,
		&valueDefault, sizeof(DWORD));

	if (ERROR_SUCCESS == res && 0 != szValue)
		return szValue;
	return "";
}

HRESULT GetIEVersion(LPDWORD pdwMajor, LPDWORD
						  pdwMinor, LPDWORD pdwBuild)
{
	HINSTANCE   hBrowser;

	if(IsBadWritePtr(pdwMajor, sizeof(DWORD))
		|| IsBadWritePtr(pdwMinor, sizeof(DWORD))
		|| IsBadWritePtr(pdwBuild, sizeof(DWORD)))
		return E_INVALIDARG;

	*pdwMajor = 0;
	*pdwMinor = 0;
	*pdwBuild = 0;

	//Load the DLL.
	hBrowser = LoadLibrary(TEXT("shdocvw.dll"));

	if(hBrowser) 
	{

		HRESULT  hr = S_OK;
		DLLGETVERSIONPROC pDllGetVersion;
		pDllGetVersion =
			(DLLGETVERSIONPROC)GetProcAddress(hBrowser,
			TEXT("DllGetVersion"));

		if(pDllGetVersion) 
		{

			DLLVERSIONINFO    dvi;
			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);
			hr = (*pDllGetVersion)(&dvi);

			if(SUCCEEDED(hr)) 
			{
				*pdwMajor = dvi.dwMajorVersion;
				*pdwMinor = dvi.dwMinorVersion;
				*pdwBuild = dvi.dwBuildNumber;
			}

		} 
		else 
		{
			//If GetProcAddress failed, there is a problem 
			// with the DLL.

			hr = E_FAIL;
		}
		FreeLibrary(hBrowser);
		return hr;
	}
	return E_FAIL;
}