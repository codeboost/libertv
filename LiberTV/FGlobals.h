#pragma once

#define VIEW_BROWSER	0
#define VIEW_STATUS		1
#define VIEW_PLAYER		2
#define VIEW_FEEDS		3

//Messages
#define WM_DOCUMENT_COMPLETE    WM_USER + 100
#define WM_FRAME_LOAD_ERROR     WM_USER + 101			
#define WM_PLAYER_MAX           WM_USER + 103 //???
#define WM_LTV_VERSION_UPDATE   WM_USER + 104 //New version is available
#define WM_SUBTITLE_FINISHED    WM_USER + 105 //Subtitles have been downloaded
#define WM_LTV_CONNECTABLE		WM_USER + 106 //Posted when the connectable thread has finished checking connectability
#define WM_FEED_REFRESH			WM_USER + 107 //Posted when a channel has been reloaded
#define WM_DOWNLOAD_LOADED		WM_USER + 108 //Posted when download manager has loaded a download
#define WM_MTTI_DOWNLOAD		WM_USER + 109 //Posted when a MTTI is about to be downloaded or when it has been downloaded; wParam = (0 - starting, 1 - finished); lParam = operation result (HRESULT)
#define WM_MAINFRAME_CLOSED		WM_USER + 110 //Sent to FTrayWindow by MainFrame