#include "stdafx.h"
#include "FMenu.h"

void _InsertMenuItem(HMENU hmenu, int nPos, int nID, const TCHAR *szItemText)
{
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask =  MIIM_STRING | MIIM_ID;
	mii.fType = 0; 
	mii.wID = nID; 
	mii.dwTypeData = (LPTSTR) szItemText;
	mii.cch = (UINT)_tcslen(szItemText);
	InsertMenuItem(hmenu, nPos, TRUE, &mii);
}

void _InsertMenuSeparator(HMENU hmenu, int nPos)
{
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_TYPE;
	mii.fType = MFT_SEPARATOR;
	mii.fState = MFS_ENABLED;
	InsertMenuItem(hmenu, nPos, TRUE, &mii);
}

void _InsertSubMenu(HMENU hmenu, int nPos, const TCHAR *szItemText, HMENU hSub)
{
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask =  MIIM_SUBMENU | MIIM_STRING;
	mii.fType =  MFT_MENUBARBREAK;
	mii.hSubMenu = hSub; 
	mii.dwTypeData = (LPTSTR) szItemText;
	mii.cch = (UINT)_tcslen(szItemText);

	InsertMenuItem(hmenu, nPos, TRUE, &mii);
}

void _InsertCheckedMenuItem(HMENU hmenu, size_t nPos, int nID, BOOL bChecked, const TCHAR* szItemText)
{
	MENUITEMINFO mii;
	memset(&mii, 0, sizeof(mii));
	mii.cbSize = sizeof(mii);
	mii.fMask = MIIM_STATE | MIIM_STRING | MIIM_ID;
	mii.wID = (UINT)nID; 
	mii.fType = 0;
	mii.dwTypeData = (LPTSTR) szItemText;
	mii.cch = (UINT)_tcslen(szItemText);

	if (bChecked)
		mii.fState = MFS_ENABLED | MFS_CHECKED; 
	else
		mii.fState = MFS_ENABLED | MFS_UNCHECKED; 

	InsertMenuItem(hmenu, (UINT)nPos, TRUE, &mii);
}