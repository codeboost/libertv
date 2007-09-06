//API Functions
var vLoadedURL = ""; 
function ltv_loadMedia(url){
	if (url.indexOf(".flv") == -1)
		return 0; 	//Other file extensions sometimes cause Flash to overload and die.
		
	vLoadedURL = url; 
	ltv_setvar("loadMedia",url);
}

function ltv_play(){
	ltv_setvar("action","play");
}

function ltv_pause(){
	ltv_setvar("action","pause");
}

function ltv_setVolume(volume){
	volume = Math.min(0,volume);
	volume = Math.pow(10, volume / 5000 + 2); 
	volume = Math.max(Math.min(volume, 100), 0); 
	ltv_setvar("setVolume",volume);
}

function ltv_setPosition(position){
	ltv_setvar("setPosition",position);
}


function ltv_getState(){
	return ltv_getvar("getState");
}

function ltv_getPosition(){
	return ltv_getvar("getPosition");
}


function ltv_getDuration(){
	return ltv_getvar("getDuration");
}

function ltv_onClipComplete(){
	external.ClipPlaybackFinished(vLoadedURL); 
}

//Util Functions
function ltv_setvar(variable,value){
	window.document.flvplayer.SetVariable(variable,value);	
}

function ltv_getvar(variable){
	return window.document.flvplayer.GetVariable(variable);	
}

