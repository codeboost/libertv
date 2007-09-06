#pragma once

#include <string>
std::string GetQTVersion();
std::string GetVLCVersion();
HRESULT GetIEVersion(LPDWORD pdwMajor, LPDWORD
						  pdwMinor, LPDWORD pdwBuild);
