#pragma once

#include "Utils.h"

void _InsertMenuItem(HMENU hmenu, int nPos, int nID, const TCHAR *szItemText);
void _InsertMenuSeparator(HMENU hmenu, int nPos);
void _InsertSubMenu(HMENU hmenu, int nPos, const TCHAR *szItemText, HMENU hSub);
void _InsertCheckedMenuItem(HMENU hmenu, size_t nPos, int nID, BOOL bChecked, const TCHAR* szItemText);
