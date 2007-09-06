#pragma once

class IStatusBar {
public:
	virtual void SetStatusIcon(int nPane, HICON hIcon) = 0; 
	virtual void SetStatusText(int nPane, const char* pszText, int nObject) = 0; 
	virtual ~IStatusBar(){}
};