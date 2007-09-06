#include "stdafx.h"
#include "FDlgSaveVideo.h"
#include "Utils.h"
#include "GlobalObjects.h"
#include "FDownload.h"
#include "FDlgSelectFolder.h"

int FDlgSaveVideo::Open(HWND hWnd, SaveDlgOpenParams& pParams)
{
	m_OpenParams = pParams; 
	DoModal(hWnd);
	return m_Result; 
}

LRESULT FDlgSaveVideo::OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	m_Result = IDCANCEL; 
	OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
	return 0; 
}

LRESULT FDlgSaveVideo::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	FString aSaveFolderOrg;
	FString aSaveFolder; 
	if (GetDlgItemText(IDC_STORAGE_FOLDER, aSaveFolderOrg) > 0)
	{
		if (!PathIsDirectory(aSaveFolderOrg))
		{
			CreateDirectory(aSaveFolderOrg, NULL);
		}


		if (ERROR_SUCCESS != IsPathWriteable(aSaveFolderOrg))
		{
			::SetFocus(GetDlgItem(IDC_STORAGE_FOLDER)); 
			MessageBox("Cannot write to selected path. Path is read-only or does not exist. Please choose a different folder.", "LiberTV: Invalid path", MB_OK | MB_ICONERROR);
			bHandled = TRUE; 
			return 1; 
		}
		
		aSaveFolder.GetBufferSetLength(MAX_PATH); 
		PathCanonicalize(aSaveFolder.GetBuffer(), aSaveFolderOrg); 
		PathMakePretty(aSaveFolder.GetBuffer()); 
		

		std::vector<FString>& aStrings = m_aStrings.m_RegArray;
		
		for (size_t k = 0; k < aStrings.size(); k++)
		{
			FString& aStr = aStrings[k];
			if (aSaveFolder.CompareNoCase(aStr) == 0)
			{
				aStrings.erase(aStrings.begin() + k, aStrings.begin() + k + 1); 
			}
		}

		aStrings.insert(aStrings.begin(), aSaveFolder); 
		m_aStrings.SaveStrings(LTV_REG_ROOT, LTV_REG_KEY"\\LastFolders", "Folder");
	}
	else
	{
		::SetFocus(GetDlgItem(IDC_STORAGE_FOLDER)); 
		MessageBox("The entered path is invalid. Please choose a valid folder.", "LiberTV: Invalid input", MB_OK | MB_ICONWARNING);
		bHandled = TRUE; 
		return 1; 
	}

	m_StrStorage = aSaveFolder;
	m_Result = IDOK; 

	m_OpenParams.m_pSettings->m_DownloadsFolder = aSaveFolder; 
	OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
	return 0;
}

LRESULT FDlgSaveVideo::OnStorageFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	UpdateFreeSpace();
	return 0; 
}

void FDlgSaveVideo::UpdateFreeSpace()
{
	FString aSaveFolderOrg;
	if (GetDlgItemText(IDC_STORAGE_FOLDER, aSaveFolderOrg) > 0)
	{
		int nDriveNumber = PathGetDriveNumber(aSaveFolderOrg);
		if (nDriveNumber != -1)
		{
			ULARGE_INTEGER ulFreeCaller, ulTotal, ulFree;
			if (SHGetDiskFreeSpace(aSaveFolderOrg, &ulFreeCaller, &ulTotal, &ulFree))
			{
				FString SpaceFree;
				SpaceFree.Format("Free space: %s", SizeString(ulFreeCaller.QuadPart));
				SetDlgItemText(IDC_FREE_SPACE, SpaceFree); 
			}
		}
	}
}

LRESULT FDlgSaveVideo::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	//Get list of download folders from registry and fill the combo-box
	//Set combo-box value = Last save folder
	//Get video details, update the video details the controls

	if (m_OpenParams.m_StrMessage.GetLength() > 0)
		SetDlgItemText(IDC_MESSAGE, m_OpenParams.m_StrMessage); 
	
	m_StorageFolder.Attach(GetDlgItem(IDC_STORAGE_FOLDER));
	
	//Fill in the folders combo
	

	size_t ItemCount = m_aStrings.LoadStrings(LTV_REG_ROOT, LTV_REG_KEY"\\LastFolders", "Folder", 10);
	
	for (size_t k = 0; k < m_aStrings.m_RegArray.size(); k++)
	{
		if (m_aStrings.m_RegArray[k] != g_AppSettings.m_DownloadsFolder)
			m_StorageFolder.InsertString((int)k, m_aStrings.m_RegArray[k]); 
	}

	if (g_AppSettings.m_DownloadsFolder.GetLength() == 0)
		m_StorageFolder.SetWindowText(GetWinUserDirectory("LiberTV Downloads"));
	else
		m_StorageFolder.SetWindowText(g_AppSettings.m_DownloadsFolder); 

	UpdateFreeSpace();
	CenterWindow(GetParent()); 
	return 0; 
}

LRESULT FDlgSaveVideo::OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	FSelectFolder aDlgFolder; 
	FString aOutFolder; 
	fsize_type minSize = m_Download.m_Detail.m_TotalSize;
	if (minSize == 0) minSize = 350 * MEGABYTE; 

	if (IDOK == aDlgFolder.SelectFolder(aOutFolder, m_hWnd, "Select storage folder", minSize))
	{
		SetDlgItemText(IDC_STORAGE_FOLDER, aOutFolder); 
	}
	return 0; 
}

//////////////////////////////////////////////////////////////////////////

LRESULT FDlgInfoDownload::OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	CenterWindow(GetParent()); 
	return 0; 
}

LRESULT FDlgInfoDownload::OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled)
{
	if (IsDlgButtonChecked(IDC_DONT_SHOW_AGAIN))
	{
		g_AppSettings.m_Flags |= PFLAG_DONT_SHOW_INFO_DLG;
		g_AppSettings.SaveSettings();
	}
	OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
	return 0; 
}

LRESULT FDlgSelectIndexFolder::OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	SetDlgItemText(IDC_FOLDER, g_AppSettings.m_IndexPath);
	CenterWindow(GetParent()); 
	return 0; 
}

LRESULT FDlgSelectIndexFolder::OnButtonClick( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled )
{
	switch(wID)
	{
	case IDOK:
		{
			FString NewFolder;
			if (GetDlgItemText(IDC_FOLDER, NewFolder) && g_AppSettings.m_IndexPath != NewFolder)
			{
				MessageBox("LiberTV must be restarted for the changes to take effect.");
				g_AppSettings.m_IndexPath = NewFolder; 
				g_AppSettings.SaveSettings();
				OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
			} 
			else
			{
				OnCloseCmd(wNotifyCode, IDCANCEL, hWndCtl, bHandled);
			}
		}
		break; 
	case IDCANCEL:
		OnCloseCmd(wNotifyCode, wID, hWndCtl, bHandled);
		break; 
	case ID_CHANGE_FOLDER:
		{
			FSelectFolder SelFolder; 
			FString StrOut = g_AppSettings.m_IndexPath; 
			if (IDOK == SelFolder.SelectFolder(StrOut, m_hWnd, "Choose LiberTV Collection Path", 0))
			{
				SetDlgItemText(IDC_FOLDER, StrOut); 
			}
		}
		break; 
	}
	return 0; 
}