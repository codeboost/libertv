#pragma once

class FCollectionMenu
{
protected:
	FArray<FString> m_zeLabels; 
	HMENU	CreateLabelsSubmenu(FLabels& Labels, int nStartID);
	FString ShowRenameDlg(HWND hWndParent, const tchar* pszName); 
public:
	int				ShowContextMenu(HWND hWndParent, const FArray<FString> &aVids); 
	static FString	ShowAddLabelDlg(HWND hWndParent, const tchar* pszLabel); 
	
};