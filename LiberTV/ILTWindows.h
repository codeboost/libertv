#pragma once


class IIEWindowI
{
public:
	virtual ~IIEWindowI() {}
	virtual HRESULT Navigate(const tchar* pszPath, const tchar* pTargetFrameName="", DWORD dwNavFlags = navNoHistory) = 0;
	virtual BOOL	IsViewActive() = 0;					//asks if this window is currently the active view
	virtual void	OnActivated(BOOL bActivated) = 0;		//called when view is activated or deactivated
	virtual void	SetNotifyWnd(HWND hWndNotify) = 0;		//Window to receive notifications
	virtual void	SetErrorPage(const char* pStrErrUrl) = 0; 

};

class IIEWindow : public IIEWindowI
{

};

class IWCollectionDisplay : public IIEWindow
{
};

class IWStatusDisplay : public IIEWindow
{

};