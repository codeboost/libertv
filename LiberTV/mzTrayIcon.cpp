#include "stdafx.h"
#include "mzTrayIcon.h"
#include <Windows.h>
#include <shellapi.h>


void mzTrayIcon::manage_Icon(uint iconID, const char* tool_Tip, ulong msg){

    NOTIFYICONDATA nid; 
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.uFlags =    (iconID > 0 ? NIF_ICON : 0) | 
					(tool_Tip   ? NIF_TIP  : 0) |
					(msg == NIM_ADD ? NIF_MESSAGE : 0);
    
    if (NULL != tool_Tip)
        lstrcpy(nid.szTip, tool_Tip); 
    
    nid.hIcon = iconID ? (HICON)LoadIcon(m_hInstance, MAKEINTRESOURCE(iconID)):(HICON)NULL; 
    nid.uCallbackMessage = (msg == NIM_ADD) ? TRAY_NOTIFY_MESSAGE : 0;
    nid.hWnd = m_hWnd; 
	nid.uID = m_uid; 

    if (iconID > 0)
        m_activeIcon = iconID; 

    Shell_NotifyIcon(msg, &nid); 

    DestroyIcon(nid.hIcon); 

    
}
//-------------------------------------------------------------------------------------------------

void mzTrayIcon::add_Icon(uint uid, uint icon_ID, const char* tool_Tip){

	m_uid = uid; 
    manage_Icon(icon_ID, tool_Tip, NIM_ADD);
    
}
//-------------------------------------------------------------------------------------------------

void mzTrayIcon::modify_Icon(uint icon_ID, const char* tool_Tip){

    manage_Icon(icon_ID, tool_Tip, NIM_MODIFY);
}
//-------------------------------------------------------------------------------------------------

void mzTrayIcon::delete_Icon(){

    manage_Icon(0, NULL, NIM_DELETE); 
    m_activeIcon = 0; 
}
//-------------------------------------------------------------------------------------------------

void mzTrayIcon::set_ToolTip(const char* tool_Tip){

    manage_Icon(0, tool_Tip, NIM_MODIFY); 
}
//-------------------------------------------------------------------------------------------------

uint mzTrayIcon::get_IconID(){

    return m_activeIcon; 
}

//-------------------------------------------------------------------------------------------------

mzTrayIcon::~mzTrayIcon(){
	delete_Icon(); 
}
//-------------------------------------------------------------------------------------------------

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

void mzTrayIcon::show_Balloon(const char* title, const char * szMsg, const uint timeout,  const ulong flags){

	
	NOTIFYICONDATA m_nid; 
	m_nid.cbSize=sizeof(NOTIFYICONDATA);
	m_nid.uFlags = NIF_INFO;
	m_nid.uTimeout = timeout;
	m_nid.dwInfoFlags = flags;
	m_nid.uID = m_uid; 
	m_nid.hWnd = m_hWnd; 
	strncpy(m_nid.szInfo,szMsg, 256 );
	strncpy(m_nid.szInfoTitle,title, 64);
	Shell_NotifyIcon(NIM_MODIFY, &m_nid);
}