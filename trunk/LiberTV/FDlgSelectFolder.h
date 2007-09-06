#pragma once


class FSelectFolder
{
public:
	//Shows Select Folder dialog. StrOutFolder contains initial folder and receives the selected folder
	//if user presses OK. 
	int SelectFolder(FString& StrOutFolder, HWND hWndParent, const tchar* pszText, fsize_type uMinSize);
};