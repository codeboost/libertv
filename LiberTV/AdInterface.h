#ifndef AdInterfaceDotH
#define AdInterfaceDotH

//Messages received from javascript
#define WM_ADWINDOW_BASE	WM_USER + 0x100
#define WM_ADWINDOW_CLOSE	WM_ADWINDOW_BASE + 1	//wParam = ad window Id

#define MAX_AD_DURATION		120 * 1000		//Maximum ad duration in milliseconds
#define DEFAULT_AD_DURATION 15 * 1000		//Default ad duration in milliseconds
#define MIN_AD_REDISPLAY_TIMEOUT 60 * 1000	//Time before the ad can be displayed again (if user scrolls into it's position)

//Flags used in AdCriteria
#define FLAG_FIRST_PLAYBACK		0x01		//Playing video for the first time
#define FLAG_PLAYBACK_START		0x100		//Playback is about to start
#define FLAG_PLAYBACK_ENDED		0x200		//Playback has ended
#define FLAG_PLAYBACK_RUNNING	0x400		//Playback is running
#define FLAG_PLAYBACK_PAUSED	0x800		//Playback is paused
#define FLAG_PLAYBACK_MASK		0xf00


//Ad Types
enum AdTypes
{
	AdNone = 0,		
	AdFullscreen,	//Full screen
	AdHBottom,		//Horizontal-bottom
	AdHTop,			//Horizontal-top
	AdVLeft,		//Vertical-left
	AdVRight,		//Vertical-right
	AdLast = AdVRight
};

//Flags used in FAdItem::dwFlags
#define AD_FLAG_NO_CLOSE		0x01	//User cannot close the ad.
#define AD_FLAG_PERSISTENT		0x02	//The ad is shown during the entire movie
#define AD_FLAG_AT_START		0x04	//Will be shown before starting playback
#define AD_FLAG_AT_END			0x08
#define AD_FLAG_NORESIZE		0x2000	//Do not resize video window, put ad on top of the video

struct FAdItem
{
	AdTypes		  m_Type;			//Display type 
	duration_type m_TimeMS;			//Position in movie, in milliseconds
	duration_type m_TimeEndMS;		//Position in movie, in milliseconds when the ad must be hidden
	float		  m_TimePercent;	//Position in movie, as percent of total duration(if TimeMS is 0).
	float		  m_TimeEndPercent;	//Position in movie, as percent, when the ad must be hidden
	dword		  m_TimeoutMS;		//Max time time to display ad (milliseconds); 
	dword		  m_Width;			//Preferred ad width
	dword		  m_Height;			//Preferred ad height
	FString		  m_URL;			//Ad url
	dword		  m_dwFlags;		//Flags: AD_FLAG_*
	dword		  m_MaxDisplays;	//Number of times to display the ad. If MaxDisplays is 0, the ad will be shown each time
	dword		  m_dwAdID;			//Internal counter
	dword		  m_LastDisplay;	//Last time the ad has been displayed
	dword		  m_MSPlayed;		//Number of milliseconds played before displaying the ad
	FAdItem(){
		m_TimeMS = 0; 
		m_Type = AdNone; 
		m_TimeoutMS = 0; 
		m_Width = 0; 
		m_Height = 0; 
		m_dwFlags = 0; 
		m_TimePercent = 0.0f;
		m_TimeEndMS = 0; 
		m_TimeEndPercent = 0; 
		m_MaxDisplays = 0;
		m_dwAdID = 0; 
		m_LastDisplay = 0; 
		m_MSPlayed = 0; 
	}
};

//Ad manager - loads and manages per-video ad information 
class IVideoAdManager 
{
public:
	virtual ~IVideoAdManager(){}
	virtual BOOL LoadAds(FIniConfig& aConf, BOOL bAddToList) = 0;
	virtual BOOL GetAds(AdTypes aType, std::vector<FAdItem> &outAds) = 0;
	virtual void Clear() = 0; 
};

class IAdNotify : public IHttpNotify
{
public:
	virtual void OnAdFinished(DWORD dwID) = 0; 
};


//Used in IAdWindow::Init()
struct IAdWindowInit
{
	HWND				m_hWndParent; 
	DWORD				m_dwWinID;			//AD Window ID
	IAdNotify*			m_pNotify;			//Object which will receive notifications (ad complete, error, etc)
	IVideoAdManager*	m_pAdManager;		//Ad manager
	AdTypes				m_AdType;			//Type of ads
	IAdWindowInit(){
		m_dwWinID = 0; 
		m_pNotify = NULL; 
		m_pAdManager = NULL; 
		m_AdType = AdNone; 
		m_hWndParent = NULL; 
	}
};

//Used in IAdWindow::UpdatePosition()
struct IAdPosition
{
	duration_type m_PlaybackPosMS;			//Current playback position, in milliseconds
	duration_type m_TotalDurationMS;		//Total video duration, in milliseconds
	DWORD		  m_dwFlags;				//Flags (playback start, pause, playback end, etc)
	duration_type m_MSPlayed; 
	IAdPosition(){
		m_PlaybackPosMS = 0; 
		m_PlaybackPosMS = 0; 
		m_dwFlags = 0; 
		m_MSPlayed = 0; 
	}
};

//Ad window - the window in which the ad is actually displayed
class IAdWindow
{
public:
	virtual ~IAdWindow(){}
	virtual BOOL Init(IAdWindowInit& pInitData) = 0;
	virtual void Clear() = 0;						//Hide all ads, clear the array and prepare for a new video
	virtual long GetPreferedHeight() = 0;			//returns Ad Height (for horizontal banners)
	virtual long GetPreferedWidth() = 0;			//returns Ad Width  (for vertical banners)
	virtual BOOL IsAdShowing() = 0;					//returns TRUE if an ad is being shown and is not timed out
	virtual void ShowWindow(int nShow) = 0;			//Shows/Hides the ad window.
	virtual void MoveWindow(LPRECT prc) = 0;		//Moves or resizes the ad window
	virtual BOOL UpdatePosition(IAdPosition& pPos) = 0; //Finds and loads ads matching the position, hides ads, handles pause, stop, etc.
	virtual BOOL SetZPosition()	= 0;				//returns TRUE if video window should be resized (made smaller) when displaying the ad, otherwise the ad is positioned on top of the video
													//calls SetWindowPos(HWND_TOP) before returning TRUE
	virtual BOOL IsAdLoaded() = 0; 
};

#endif 