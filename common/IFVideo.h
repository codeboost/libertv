#pragma once
/*
	Interfaces for the video objects used by LiberTV.
	(c) 2006 TorrentMedia SRL
*/

#define WM_DSHOW_CLIP_COMPLETE WM_USER + 0x108	//Directshow IMediaEvent notification

#define E_FAIL_SHOW_ERROR 0x80000009			//Plugin init Failed. Show plugin window for error message.

//Notification codes used in IMediaNotify
#define PluginInitComplete		1	//Posted when plugin has finished initializing
#define PluginMediaReady		2	//Posted when plugin has loaded the media and is ready to play it
#define PluginPlaybackFinished	3	//Posted when media has finished playing
#define PluginMediaPlaying		4	//Optional; Posted when media has started playing. If received, player will reactivate the window (prevent DivX from stealing it; used by WMPlayer)
#define PluginMediaBuffering	5	//Optional; Posted when plugin is streaming data. 
#define PluginDownloadQT		6	//Optional; Posted by movplayer.html; Player opens the browser to apple.com/quicktime


#define PluginNavBarLoaded		100	//Send by navbar when it has been loaded

//////////////////////////////////////////////////////////////////////////
//Video State Constants
enum VideoState
{
	vssUndefined		= 0,
	vssStopped			= vssUndefined + 1,
	vssPaused			= vssStopped + 1,
	vssPlaying			= vssPaused + 1,
	vssScanForward		= vssPlaying + 1,
	vssScanReverse		= vssScanForward + 1,
	vssBuffering		= vssScanReverse + 1,
	vssWaiting			= vssBuffering + 1,
	vssMediaEnded		= vssWaiting + 1,
	vssTransitioning	= vssMediaEnded + 1,
	vssReady			= vssTransitioning + 1,
	vssReconnecting		= vssReady + 1,
	vssLast				= vssReconnecting + 1
};

#define MEDIA_FLAG_CAN_REUSE_PLUGIN 0x01	//Plugin can be used with another media file
#define MEDIA_FLAG_HAS_OWN_SUBTITLES 0x02	//Plugin has detected / downloaded subtitles


typedef LONGLONG MEDIA_TIME; 

//////////////////////////////////////////////////////////////////////////
//Video Control Interface
struct IFMediaControl
{
	virtual HRESULT			Play() = 0;							//Runs the graph
	virtual HRESULT			Stop() = 0;							//Stops and destroys the graph
	virtual HRESULT			Pause() = 0;						//Pauses the graph
	virtual VideoState		GetVideoState() = 0;				//----
	virtual HRESULT			SeekPosition(MEDIA_TIME rtPos) = 0;		//Seek to rtPos
	virtual MEDIA_TIME		GetPosition() = 0;			//
	virtual MEDIA_TIME		GetDuration() = 0;					//total duration
	virtual MEDIA_TIME		GetAvailDuration() = 0;
	virtual HRESULT			GetBufferingProgress(long* lProgress) = 0; 
	virtual ~IFMediaControl(){}

};

struct IFMediaPlayer;

struct IFMediaType
{
	virtual BOOL			SupportsMediaType(const char* pszMediaType) = 0; 
	virtual HRESULT			SetMediaType(const char* pszMediaType) = 0; 
	virtual HRESULT			GetMediaType(char* pszMediaType) = 0; 
	virtual BOOL			IsMediaType(const char* pszMediaType) = 0; 
	virtual ~IFMediaType(){}
};

class IMediaPlayerNotify
{
public:
	virtual void NotifyPlayer(DWORD dwCode, HRESULT hr) = 0;
	virtual ~IMediaPlayerNotify(){}
};


struct IMediaOptions
{
	FString		m_SubPath;	//Subtitle path
	FString		m_Vout; 
	MEDIA_TIME	rtOffset;	//Start offset
	MEDIA_TIME	rtMaxDuration;	//Max duration ?
};

//////////////////////////////////////////////////////////////////////////
//Video Player Interface
struct IFMediaPlayer : IFMediaControl, IFMediaType
{
	virtual HRESULT			Init(HWND hWndParent, IMediaPlayerNotify* pNotify) = 0;			//Init
	virtual HRESULT			LoadMedia(const char* pFileName, IMediaOptions* pOptions) = 0;
	virtual void			Clear() = 0; 

	virtual HRESULT			SetVideoRect(const RECT& rcV) = 0;	//---
	virtual HRESULT			ShowVideoWnd(int nShow) = 0; 
	virtual HRESULT			SetFullScreen(BOOL bFullScreen) = 0;
	virtual BOOL			IsFullScreen() = 0; 
	virtual double			GetFrameRate() = 0; 
	virtual void			SetAspectRatio(int cx, int cy) =0;
	virtual long			GetMediaEvent() = 0;			//IMediaEvent::GetEvent()

	virtual long			GetVolume() = 0;			//0..100 percent
	virtual HRESULT			SetVolume(long lVol) = 0;	//0..100 percent
	virtual DWORD			GetMediaFlags() = 0; 
	virtual void			CycleSubtitles() = 0; 
	virtual HRESULT			SendCommand(const char* pszCmdName, void* pCmdData) = 0; //Send a command to the video renderer

	virtual ~IFMediaPlayer(){}
};

