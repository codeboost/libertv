#ifndef __MZTRAYICON_H__
#define __MZTRAYICON_H__

typedef unsigned int uint; 
typedef unsigned long ulong; 

#define TRAY_NOTIFY_MESSAGE		(WM_USER + 2001)

class mzTrayIcon 
{
protected:
	uint			m_activeIcon; 
	uint			m_uid; 
	HINSTANCE		m_hInstance; 
	HWND			m_hWnd; 

protected:
	virtual void	manage_Icon (uint iconID, const char* tool_Tip, ulong msg); 
public:
	mzTrayIcon (HINSTANCE hInst, HWND hWnd): m_uid(0), m_hInstance(hInst), m_hWnd(hWnd)
	{
	}
			~mzTrayIcon ();
	void	add_Icon (uint uid, uint icon_ID, const char* tool_Tip); 
	void	modify_Icon (uint icon_ID, const char* tool_Tip); 
	void	delete_Icon (); 
	void	set_ToolTip (const char* tool_Tip); 
	uint	get_IconID (); 
	void	show_Balloon (const char* title, const char* text, const uint timeout = 2000, const ulong flags = NIIF_INFO);
};


#endif //__MZTRAYICON_H__