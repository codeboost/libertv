#ifndef __FVIDEOWINDOW_H__
#define __FVIDEOWINDOW_H__
#include "IVideoPlayer.h"

class FVideoWindow : public CWindowImpl<FVideoWindow>, 
					 public IVideoPlayer
	
{
public:
	DECLARE_WND_CLASS_EX("FVW", 0, 0); 
	BEGIN_MSG_MAP(FVideoWindow)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
	END_MSG_MAP()

	LRESULT			OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled){return 1; }
	virtual BOOL	ProcessMessage(MSG* pMsg){return FALSE;}; //return TRUE if processed; FALSE if not
};

#endif //__FVIDEOWINDOW_H__