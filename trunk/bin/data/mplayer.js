//Notification codes used in IMediaNotify
/*
#define PluginInitComplete		1	//Posted when plugin has finished initializing
#define PluginMediaReady		2	//Posted when plugin has loaded the media and is ready to play it
#define PluginPlaybackFinished	3	//Posted when media has finished playing
#define PluginMediaPlaying		4	//Optional; Posted when media has started playing. If received, player will reactivate the window (prevent DivX from stealing it; used by WMPlayer)
#define PluginMediaBuffering	5	//Optional; Posted by a streaming plugin when it is buffering data. 
*/

var LTV_PluginInitComplete 		= 1;
var LTV_PluginMediaReady  		= 2;
var LTV_PluginPlaybackFinished  = 3;
var LTV_PluginMediaPlaying		= 4;
var LTV_PluginMediaBuffering	= 5; 
var LTV_PluginDownloadQT		= 6;

var LTV_WM_Notify = 1; 
var S_OK = 0x0;
var S_FALSE = 0x01;
var E_FAIL = 0x80000008;
var E_FAIL_SHOW_ERROR = 0x80000009;
