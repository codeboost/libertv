#ifndef IVideoPlayerDotH
#define IVideoPlayerDotH
#include "IFVideo.h"
#include "IStatusBar.h"


class IVideoPlayer
{
public:
	virtual HRESULT 	SetVolume(long lVol) = 0;
	virtual HRESULT 	SetPosition(MEDIA_TIME lPos) = 0;
	virtual HRESULT 	Play() = 0;
	virtual HRESULT 	Pause() = 0;
	virtual HRESULT 	Stop() = 0;
	virtual BOOL 		IsPlaying(){return FALSE;}
	virtual HRESULT 	PlayNext(BOOL bCycle) = 0;
	virtual HRESULT		PlayPrev(BOOL bCycle) = 0;
	virtual ulong		GetVideoID() = 0;
	virtual int			GetCurrentClipId() = 0; 
	virtual BOOL		PlayClip(int nClipId) = 0; 
	virtual HRESULT		PlayMT(ulong videoID, BOOL bPaused = FALSE) = 0; 
	virtual HRESULT		SelectSubtitle(const tchar* pszLang) = 0;
	virtual HRESULT		GetSubtitleFilename(char* pszSubFilename, DWORD* dwPtrLen) = 0; 
	virtual HRESULT		GetNotify(IMediaPlayerNotify** pNotify) = 0;
	virtual void		SetStatusBar(IStatusBar* pStatusBar) = 0; 
	//virtual HRESULT		Play(IFPlaylist* pPlaylist); 	
};




#endif