var g_MaxVolume = 140; 
var g_ThumbWidth = 11; 
var SeekMin = 0;
var SeekMax = 1;
var SeekAvail = 0;
var SeekPos = 0;
var ScrollEnable = false;
var bPlaying = 1;
var bMouseOver = false;
var g_currentVideo = new Array;
var c_videoID = 0; 
var c_episodeID = 16; 

function IsMuted()
{
	return currentX == 0;	
}

//Max volume is 0, -10000 complete silence; See IBasicAudio::SetVolume() in MSDN
function SetVolume(volume)
{

	volume = Math.min(100, Math.max(volume, 0));
	setVolumeP(volume); 
}

function GetVolume(volume)
{
	volume = getVolumeP();

	return(IsMuted() ? 0: volume);
}

function IncreaseVolume()
{
	setVolumeP(getVolumeP() + 4);
	try
	{ external.ControlBar_SetVolume(GetVolume()); } 
	catch(e){}
}

function DecreaseVolume()
{
	setVolumeP(getVolumeP() - 4);
	try
	{ external.ControlBar_SetVolume(GetVolume()); } 
	catch(e){}
}

function SetPlayState(bState)
{
	try{
		if (bState == 1) //paused
		{	
			if (bMouseOver)
				document.getElementById("button").src = "player/bt_play_hover.gif"
			else
				document.getElementById("button").src = "player/bt_play.gif";	
		}
		else 
		{
			if (bMouseOver)
				document.getElementById("button").src = "player/bt_pause_hover.gif";		
			else
				document.getElementById("button").src = "player/bt_pause.gif";		
		}
	}
	catch (e){}
	
	bPlaying = bState; 
}

function Status_SetTimer(pStrTimer)
{
	if (document.getElementById("lbl_timer") != null)
		lbl_timer.innerText = pStrTimer;  
}

function Status_SetMessage(idi, strMsg)
{
	if (document.getElementById("lbl_status") != null)
	{
		if (strMsg.length > 0)
			lbl_status.innerText = strMsg; 
		else
			lbl_status.innerText = "No movie loaded";
	}
}

///--------------------------------------

var currentX = 0;
var moving = false;
var lastX = 0;
var outsideDistance = 0;
var scrollingVideo = false;

document.ondragstart=new Function("return false");
document.onselectstart=new Function ("return false");


function rewindVideo()
{
	try
	{	
		external.SeekBar_SetPosition(0);
	} catch(e) { }
	updateScroll();
}

//return integer from stylesheet value, eg. for '20px', returns '20'.
function getStyleInt(vStyleStr)
{
	var px = vStyleStr.indexOf("px");
	if (-1 == px)
		return vStyleStr;
	return vStyleStr.substring(0, px);
}

//setVolumePercent
function setVolumeP(vPercent)
{
	if (vPercent > 100.0)	
		vPercent = 100.0;
	if (vPercent < 0.0)
		vPercent = 0.0;

	var xpos = (vPercent / 100.0) * g_MaxVolume; 
	currentX = xpos;
	var volset = document.getElementById("volume-white");
	var voltext = document.getElementById("volume-text");
	volset.style.width = xpos + "px";
	voltext.innerText = "Volume " + vPercent.toFixed(0) + "%";
}

//Get volume position in percent 
function getVolumeP()
{
    if (document.readyState != "complete")
        return ; 

	var volset = document.getElementById("volume-white");
	var voltot = document.getElementById("volume-black");
	var xpos = volset.offsetWidth / voltot.offsetWidth;
	return xpos * 100;
}

function endMotion()
{
	if (moving) 
		endMove();
	else if (scrollingVideo)
		scrollEnd();
		
	moving = false;
	scrollingVideo = false;
}

function motion(e)
{
	scrollVideo(e);	
	move(e);
}

function findPosX(obj)
{
	var curleft = 0;
	if (obj.offsetParent)
	{
		while (obj.offsetParent)
		{
			curleft += obj.offsetLeft
			obj = obj.offsetParent;
		}
	}
	else if (obj.x)
		curleft += obj.x;
	return curleft;
}

function startMove(e)
{
	moving = true;
	lastX = window.event.screenX;
	
	var elem = document.getElementById("volume-black");
	var pos = findPosX(elem);	
	var newPos = window.event.clientX - pos;
	var volume = document.getElementById("volume-white");
	volume.style.width = newPos + "px";
		
	window.event.cancelBubble = true; 
}

function move(e)
{
	if (moving)
	{			
		currentX = window.event.clientX;
		
		var elem = document.getElementById("volume-black");
		var volume = document.getElementById("volume-white");
		var oldVol = getStyleInt(volume.style.width)*1;		
		var elemPos = findPosX(elem);
		
		var delta = currentX - elemPos;				
		
		oldVol = delta*1;			
		if (oldVol < 0) oldVol = 0;
		if (oldVol > elem.offsetWidth) oldVol = elem.offsetWidth;
		volume.style.width = oldVol + "px";
		
		var vPercent = getVolumeP();
		var voltext = document.getElementById("volume-text");
		voltext.innerText = "Volume " + vPercent.toFixed(0) + "%";
			
		lastX = window.event.screenX;		
		
		try
		{
			external.ControlBar_SetVolume(GetVolume());
		}
		catch(e){}
	}	
}

function endMove()
{
	move(window.event);
	moving = false;	
}

function scrollStart(e)
{

	scrollingVideo = true;
	//call pause	
	
	window.event.cancelBubble = true;
}

function SeekBar_SetPos(nPos)
{
    if (document.readyState != "complete")
        return ; 

	if (nPos * 1.0 > SeekMax) nPos = SeekMax;
	if (nPos * 1.0 < SeekMin) nPos = SeekMin;
	SeekPos = nPos;
	updateScroll();
	refresh(); //refresh video info
}

function SeekBar_Enable(bEnable)
{
	ScrollEnable = bEnable;
	if (ScrollEnable)
	{
		//change color	
	}		
	else
	{
		//change color	
	}
}

function SeekBar_SetRange(Min, Max)
{
    if (document.readyState != "complete")
        return ; 

	Min = Min * 1.0;
	Max = Max * 1.0;

	if (SeekMin != Min || SeekMax != Max)	
	{
		SeekMin = Min;
		SeekMax = Max;
		if (SeekMax - SeekMin == 0) SeekMax = SeekMin + 1;	//prevent div by 0
		if (SeekAvail > SeekMax) SeekAvail = SeekMax;
		if (SeekPos > SeekAvail) SeekPos = SeekAvail;
		updateScroll();
	}
}

function SeekBar_SetAvail(nAvail)
{
    if (document.readyState != "complete")
        return ; 

	nAvail = nAvail * 1.0;
	if (nAvail != SeekAvail)
	{
//		external.OutputDebugString("Available set: " + nAvail);
		if (nAvail > SeekMax) nAvail = SeekMax;
		if (nAvail < SeekMin) nAvail = SeekMin;
		SeekAvail = nAvail;
		updateScroll();
	}
}

function updateScroll()
{	
    if (document.readyState != "complete")
        return ; 

	var perc = SeekPos / (SeekMax - SeekMin) * 100;	
	if (perc < 0) perc = 0;
	if (perc > 100) perc = 100;
	
	var ap = SeekAvail / (SeekMax - SeekMin) * 100;	
	if (ap < 0) ap = 0;
	if (ap > 100) ap = 100;
	
	if (isNaN(perc)) perc = 0;
	
	var divPlayed = document.getElementById("divPlayed"); 
	var divAvail = document.getElementById("divAvail"); 
	if (perc >= ap) 
	{
		divAvail.style.width = 0;
	}
	else
	{
		divAvail.style.width = ap + "%";
	}
	
	if (Math.round(perc) <= 0)
	{
		divPlayed.style.width = 0;
	}
	else
	{ 
		divPlayed.style.width = perc + "%";
	}			
	
	divTotal.style.width = "100%";
}

function scrollVideo(e)
{
	if (scrollingVideo && ScrollEnable)
	{
		var scrollX = window.event.clientX - findPosX(document.getElementById("scroll-bar"));	
		var width = document.getElementById("scroll-bar").offsetWidth;
		var perc = scrollX / width * 100;		
		if (perc < 0) perc = 0;
		if (perc > 100) perc = 100;
		
		SeekPos = perc * (SeekMax - SeekMin) / 100;	
		
		if (SeekPos > SeekAvail) SeekPos = SeekAvail;
		if (SeekPos < 0) SeekPos = 0;
	
		updateScroll();			
		try
		{
			if (SeekPos <= SeekAvail)
				external.SeekBar_SetPosition(SeekPos);
		}
		catch(e)
		{
			//...
		}
	}
}

function scrollEnd()
{
	scrollVideo(window.event);
	
	scrollingVideo = false;
	try
	{
//		if (SeekPos <= SeekAvail)
//			external.SeekBar_SetPosition(SeekPos);
	}
	catch(e)
	{
		//...
	}
	//call play
}

function setTime(t)
{
    try{
	    document.getElementById("timer").innerHTML = t;
	} catch(e){}
}

function setStatus(t)
{
    try{
	document.getElementById("status").innerHTML = t;	
	} catch(e){}
}

function buttonOnOver()
{
	bMouseOver = true;
	
	if (bPlaying == 0)
	{
		document.getElementById("button").src = "player/bt_pause_hover.gif";	
	}
	else
	{
		document.getElementById("button").src = "player/bt_play_hover.gif";	
	}	
}

function buttonOnOut()
{
	bMouseOver = false;
	if (bPlaying == 0)
	{
		document.getElementById("button").src = "player/bt_pause.gif";	
	}
	else
	{
		document.getElementById("button").src = "player/bt_play.gif";	
	}	
}

function buttonMouseDown()
{
	if (bPlaying == 1)
	{
		document.getElementById("button").src = "player/bt_play_pressed.gif";			
	}
	else
	{
		document.getElementById("button").src = "player/bt_pause_pressed.gif";	
	}		
}

function buttonClick()
{
	//external call
	try
	{
		external.ControlBar_PlayButton(bPlaying);
	}
	catch(e)
	{
		//SetPlayState(!bPlaying);
	}
}


function showNextPrev(bShow)
{
    try{
		next_prev.style.display = bShow  == 0 ? "none":"inline";
	} catch(e){}
}

function showInfoBtn(bShow)
{
    try{
	    if (bShow == 0)
	    {
		    subImage.style.display="none";
	    }
	    else
	    {
		    subImage.style.display="inline";
	    }
	} catch(e){}
}

function playNext()
{
	try{external.PlayNextClip();}catch(e){}
}
function playPrev()
{
	try{external.PlayPrevClip();}catch(e){}
}

function onStop()
{  
    try{
	    lbl_timer.innerText = "";
	    lbl_status.innerText = "";
	    SeekBar_SetRange(0, 0); 
	    SeekBar_SetAvail(0);
	} catch(e){}
}

function onBodyMouseUp()
{
	if (window.event.button == 2)
	{
		try{
			external.ShowContextMenu(10, 0); 
		}
		catch(e){
		}
	}
	else 
	endMotion(); 
}	

function toggleSubs()
{
	try{
		external.ToggleSubtitles(0);
	}
	catch(e){
	}
}

function toggleFullscreen()
{
	try{
	external.RestoreFullScreen();
	} catch(e){}
}	

function addComment()
{
	try{
		external.ExecCmd("ShowCommentsWnd", 0, 0); 
	} catch(e){}
}
function videoOptions()
{
	try{
		if (g_currentVideo.length)
			external.ShowVideoOptions(g_currentVideo[c_videoID]);
	} catch(e){}
}

function videoInfo()
{
	try{
	if (g_currentVideo.length)
		external.EpisodeDetails(g_currentVideo[c_videoID]);
	} catch(e){}
}

function onBodyClick()
{
	try{
		external.SetMainFrameFocus();
	}
	catch (e){}
}

function showBuffering(bShow){
    try{
	    if (bShow * 1)
	    {
		    if (img_buffering.src.indexOf("player/buffering_1.gif") == -1)
			    img_buffering.src = "player/buffering_1.gif";
	    }
	    else
	    {
		    if (img_buffering.src.indexOf("player/buffering_0.gif") == -1)
			    img_buffering.src = "player/buffering_0.gif";
	    }
	} catch(e){}
}

function refresh()
{
		var videoID = external.GetPlayingVideoID(); 
		if (videoID > 0)
		{
			var pArray = new Array; 
			external.GetVideoInfo(videoID, this, pArray); 
			if (pArray.length)
			  g_currentVideo = pArray[0];
		}
}	
///----------------------