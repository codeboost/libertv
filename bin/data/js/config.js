var g_ChannelGuide     = "http://guide2.libertv.ro/guide/";

function ShowEpisodeDetails(id_episode)
{
	try { external.NavigateGuide(g_ChannelGuide + "?id_episode=" + id_episode, 0); } catch (e) { alert('cannot navigate guide'); }	
}

function currentTime()
{
	var d = new Date();
	return d.getMinutes() * 60 * 1000 + d.getSeconds() * 1000 + d.getMilliseconds();
}
//Video array indices
var c_videoID             = 0;
var c_videoName           = 1;
var c_videoStatusStr      = 2;
var c_videoTotalSize      = 3;
var c_videoDownloaded     = 4;
var c_videoTotalDuration  = 5;
var c_videoAvailDuration  = 6;
var c_videoTimeAdded      = 7;
var c_videoTimeCompleted  = 8;
var c_videoNumClips       = 9;
var c_videoTotalUpSpeed   = 10;
var c_videoTotalDownSpeed = 11;
var c_videoIsLoaded       = 12;
var c_videoIsPlaying      = 13;
var c_videoFlags          = 14;
var c_videoWatched        = 15;
var c_episodeID           = 16;
var c_showID              = 17;
var c_videoCanPlay        = 18;
var c_curClipID           = 19;
var c_curClipSize         = 20;
var c_curClipDownloaded   = 21;
var c_curClipDownSpeed    = 22;
var c_curClipUpSpeed      = 23;
var c_curClipSeeds        = 24;
var c_curClipPeers        = 25;
var c_curClipComplete     = 26;
var c_curClipIncomplete   = 27;
var c_curClipState        = 28;
var c_videoRSSFlags		  = 29;