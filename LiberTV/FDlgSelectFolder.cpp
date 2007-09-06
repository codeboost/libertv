#include "stdafx.h"
#include "FDlgSelectFolder.h"
#include "Utils.h"


int FSelectFolder::SelectFolder(FString& StrOutFolder, HWND hWndParent, const tchar* pszText, fsize_type uMinSize)
{
	int nResult = -1; 
	for (;;)
	{
		CFolderDialog dlgFolder(NULL, pszText, BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE);

		FString StrOut = StrOutFolder;
		dlgFolder.SetInitialFolder(StrOut); 


		BOOL bOverwrite = FALSE; 

		nResult = (int)dlgFolder.DoModal(); 

		if (nResult == 1)   //User selects OK
		{
			StrOut = dlgFolder.m_szFolderPath;

			//Test if it path is read only
			int err = IsPathWriteable(StrOut);

			if (ERROR_SUCCESS != err)
			{
				FString Msg; 
				Msg.Format("Could not write to selected path (%d). Please select a different path.", err); 

				//If it's read only, force user to select a writable path
				MessageBox(NULL, Msg, "Write Error", MB_OK | MB_ICONERROR); 
				continue; 
			}
			//Test if there is enough space
			ULARGE_INTEGER ulAvail; 
			::GetDiskFreeSpaceEx(dlgFolder.m_szFolderPath, &ulAvail, NULL, NULL);
			fsize_type fs = ulAvail.HighPart; 
			fsize_type SpaceLeft = (fs << 32) | ulAvail.LowPart;

			if (SpaceLeft < uMinSize)
			{
				//force user to select a path with more free space
				MessageBox(NULL, "Not enough free space in selected folder. Please choose a folder on a drive with more free space.", "Not enough space on drive", MB_OK | MB_ICONWARNING);
				continue;
			}
			StrOutFolder = dlgFolder.m_szFolderPath;

			break; 
		}
		else 
			break; 
	}
	return nResult;
}