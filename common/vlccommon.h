#pragma once
enum VLCStates{
	vlcStopped = 0, 
	vlcLoading = 1,
	vlcBuffering = 2,
	vlcPlaying = 3, 
	vlcPaused = 4
};
#define VLC_RT_DIVIDER 10000 