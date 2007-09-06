// Implementation of the CNotifyIconData class and the CTrayIconImpl template.
#pragma once

#include <atlmisc.h>

// Wrapper class for the Win32 NOTIFYICONDATA structure
class CNotifyIconData : public NOTIFYICONDATA
{
public:	
	CNotifyIconData()
	{
		memset(this, 0, sizeof(NOTIFYICONDATA));
		cbSize = sizeof(NOTIFYICONDATA);
	}
	UINT m_ID; 
};

// Template used to support adding an icon to the taskbar.
// This class will maintain a taskbar icon and associated context menu.
template <class T>
class CTrayIconImpl
{
protected:
	
	CNotifyIconData m_nid;
	bool m_bInstalled;
	UINT m_nDefault;
	CMenu	m_TrayMenu; 
public:	
	UINT WM_TRAYICON;
	CTrayIconImpl() : m_bInstalled(false), m_nDefault(0)
	{
		WM_TRAYICON = ::RegisterWindowMessage(_T("WM_TRAYICON"));
	}
	
	~CTrayIconImpl()
	{
		// Remove the icon
		RemoveIcon();
	}

	// Install a taskbar icon
	// 	lpszToolTip 	- The tooltip to display
	//	hIcon 		- The icon to display
	// 	nID		- The resource ID of the context menu
	/// returns true on success
	bool InstallIcon(LPCTSTR lpszToolTip, HICON hIcon, UINT nID)
	{
		T* pT = static_cast<T*>(this);
		// Fill in the data		
		m_nid.cbSize = sizeof(NOTIFYICONDATA);
		m_nid.hWnd = pT->m_hWnd;
		m_nid.uID = 1;
		m_nid.hIcon = hIcon;
		m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP ;
		m_nid.uCallbackMessage = WM_TRAYICON;
		m_nid.m_ID = nID; 
		_tcscpy(m_nid.szTip, lpszToolTip);
		m_TrayMenu.LoadMenu(nID);

		// Install
		m_bInstalled = Shell_NotifyIcon(NIM_ADD, &m_nid) ? true : false;
		// Done
		return m_bInstalled;
	}

	bool ModifyIcon(LPCTSTR lpszToolTip, HICON hIcon, UINT nID)
	{
		T* pT = static_cast<T*>(this);
		// Fill in the data		
		m_nid.cbSize =  sizeof(NOTIFYICONDATA);
		m_nid.hWnd = pT->m_hWnd;
		m_nid.uID = 1;
		m_nid.hIcon = hIcon;
		m_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP ;
		m_nid.uCallbackMessage = WM_TRAYICON;
		m_nid.m_ID = nID; 
		_tcscpy(m_nid.szTip, lpszToolTip);

		// Install
		m_bInstalled = Shell_NotifyIcon(NIM_MODIFY, &m_nid) ? true : false;
		// Done
		return m_bInstalled;
	}
	bool SetTrayIconTooltip(LPCSTR lpszToolTip)
	{
		T* pT = static_cast<T*>(this);
		// Fill in the data		
		m_nid.cbSize = sizeof(NOTIFYICONDATA); 
		m_nid.hWnd = pT->m_hWnd;
		m_nid.uID = 1;
		m_nid.uFlags = NIF_TIP ;
		m_nid.uCallbackMessage = WM_TRAYICON;
		_tcscpy(m_nid.szTip, lpszToolTip);
		// Install
		m_bInstalled = Shell_NotifyIcon(NIM_MODIFY, &m_nid) ? true : false;
		// Done
		return m_bInstalled;
	}

	// Remove taskbar icon
	// returns true on success
	bool RemoveIcon()
	{
		// Remove
		m_nid.uFlags = 0;
		m_TrayMenu.DestroyMenu();
		m_TrayMenu.m_hMenu = NULL; 
		return Shell_NotifyIcon(NIM_DELETE, &m_nid) ? true : false;
	}

	// Set the icon tooltip text
	// returns true on success
	bool SetTooltipText(LPCTSTR pszTooltipText)
	{
		if (pszTooltipText == NULL)
			return FALSE;
		// Fill the structure
		m_nid.uFlags = NIF_TIP;
		_tcscpy(m_nid.szTip, pszTooltipText);
		// Set
		return Shell_NotifyIcon(NIM_MODIFY, &m_nid) ? true : false;
	}

	// Set the default menu item ID
	inline void SetDefaultItem(UINT nID) { m_nDefault = nID; }

	BEGIN_MSG_MAP(CTrayIconImpl)
		MESSAGE_HANDLER(WM_TRAYICON, OnTrayIcon)
	END_MSG_MAP()

	virtual BOOL OpenTrayMenu()
	{
		T* pT = static_cast<T*>(this);
		// Load the menu
		CMenu &oMenu = m_TrayMenu; 

		// Get the sub-menu
		CMenuHandle oPopup(oMenu.GetSubMenu(0));
		// Prepare
		pT->PrepareMenu(oPopup);
		// Get the menu position
		WTL::CPoint pos;
		GetCursorPos(&pos);
		// Make app the foreground
		SetForegroundWindow(pT->m_hWnd);
		// Set the default menu item
		if (m_nDefault == 0)
			oPopup.SetMenuDefaultItem(0, TRUE);
		else
			oPopup.SetMenuDefaultItem(m_nDefault);
		// Track
		oPopup.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, pT->m_hWnd);
		// BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
		pT->PostMessage(WM_NULL);
		return TRUE; 
	}

	LRESULT OnTrayIcon(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		// Is this the ID we want?
		if (wParam != m_nid.m_ID)
			return 0;
		T* pT = static_cast<T*>(this);
		// Was the right-button clicked?
		if (LOWORD(lParam) == WM_RBUTTONUP)
		{
		}
		return 0;
	}

	// Allow the menu items to be enabled/checked/etc.
	virtual void PrepareMenu(HMENU hMenu)
	{
		// Stub
	}

	BOOL ShowBalloon(const char* title, const char * szMsg, const UINT timeout,  const DWORD flags)
	{
		T* pT = static_cast<T*>(this);
		m_nid.uFlags = NIF_INFO;
		m_nid.uTimeout = timeout;
		m_nid.dwInfoFlags = flags;
		m_nid.hWnd = pT->m_hWnd; 
		m_nid.uID = 1; 
		strncpy(m_nid.szInfo,szMsg, 256 );
		strncpy(m_nid.szInfoTitle,title, 64);
		return Shell_NotifyIcon(NIM_MODIFY, &m_nid);		
	}
};
