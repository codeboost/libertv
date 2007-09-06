#ifndef __FDLGSAVEVIDEO_H__
#define __FDLGSAVEVIDEO_H__

#include "resource.h"
#include "FDownload.h"
#include "AppSettings.h"
#include "IAppManager.h"


class FRegStrArray{
public:
	std::vector<FString> m_RegArray; 
	size_t				 m_MaxCount; 

	FRegStrArray(): m_MaxCount(0){}

	size_t LoadStrings(HKEY hKeyRoot, const tchar* pszKeyName, const tchar* pszValuePrefix, size_t nMaxValues = 10)
	{
		for (size_t k = 0; k < nMaxValues; k++)
		{
			FString StrValueName; 
			StrValueName.Format("%s_%d", pszValuePrefix, k); 
			FString Str = _RegStr(StrValueName, pszKeyName, hKeyRoot); 
			if (Str.GetLength() > 0)
			{
				m_RegArray.push_back(Str);
			}
		}
		m_MaxCount = nMaxValues; 
		return m_RegArray.size(); 
	}

	BOOL SaveStrings(HKEY hKeyRoot, const tchar* pszKeyName, const tchar* pszValuePrefix)
	{
		for (size_t k = 0; k < m_RegArray.size(); k++)
		{
			FString StrValueName; 
			StrValueName.Format("%s_%d", pszValuePrefix, k); 
			_RegSetStr(StrValueName, m_RegArray[k], pszKeyName, hKeyRoot); 
		}
		return TRUE; 
	}
};

class FDlgSaveVideo : public CSimpleDialog<IDD_SAVE_VIDEO>, public CDialogResize<FDlgSaveVideo>
{                 
public:           
	enum {IDD=IDD_SAVE_VIDEO};
	BEGIN_MSG_MAP(FDlgSaveVideo)
		COMMAND_ID_HANDLER(IDOK, OnOk)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BROWSE, OnBrowse)
		COMMAND_ID_HANDLER(IDC_STORAGE_FOLDER, OnStorageFolder);
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		CHAIN_MSG_MAP(CDialogResize<FDlgSaveVideo>)
	END_MSG_MAP();

	/*	BEGIN_DLGRESIZE_MAP(FDlgSaveVideo)
			DLGRESIZE_CONTROL(IDC_SESSION, DLSZ_SIZE_X)
			DLGRESIZE_CONTROL(IDC_PEERS, DLSZ_SIZE_X | DLSZ_SIZE_Y)
			DLGRESIZE_CONTROL(IDOK, DLSZ_MOVE_X | DLSZ_MOVE_Y)
			DLGRESIZE_CONTROL(IDC_VIDEO_NAME, DLSZ_SIZE_X)
		END_DLGRESIZE_MAP()*/
		struct SaveDlgOpenParams{
			FString m_StrMessage;			//Message to set 
			AppSettings*	m_pSettings; 
			SaveDlgOpenParams(AppSettings* pSettings): m_pSettings(pSettings){
			}
			SaveDlgOpenParams(){
				m_pSettings = NULL; 
			}
		};
		WTL::CComboBox	m_StorageFolder; 
		FDownloadEx		m_Download; 
		FRegStrArray	m_aStrings;
		int				m_Result; 
		FString			m_StrStorage; 
		SaveDlgOpenParams m_OpenParams; 
		
		
		void	UpdateFreeSpace(); 
		LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

		LRESULT OnCancel(WORD wNotifyCode, WORD wID, HWND hWndCtrl, BOOL& bHandled); 
		LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnBrowse(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
		LRESULT OnStorageFolder(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled); 
		int Open(HWND hWnd, SaveDlgOpenParams& pParams);
};


class FDlgInfoDownload : public CSimpleDialog<IDD_DOWNLOAD_STARTED>
{
public:
	enum{IDD=IDD_DOWNLOAD_STARTED};
	BEGIN_MSG_MAP(FDlgInfoDownload)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOk)
	END_MSG_MAP();

	LRESULT OnOk(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

};

class FDlgSelectIndexFolder : public CSimpleDialog<IDD_COLLECTION_FOLDER>
{
	enum{IDD=IDD_COLLECTION_FOLDER};
	BEGIN_MSG_MAP(FDlgInfoDownload)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnButtonClick)
		COMMAND_ID_HANDLER(IDCANCEL, OnButtonClick)
		COMMAND_ID_HANDLER(ID_CHANGE_FOLDER, OnButtonClick)
	END_MSG_MAP();
	
	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnButtonClick(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
};

#endif //__FDLGSAVEVIDEO_H__