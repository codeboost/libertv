
var g_Videos = null;
var g_strFilter = "";	  //Current filter
var g_currentLabel = "all";  //Currently selected label
var g_SortDir = "desc";		//Sort direction
var g_CurSortMode = "date";	//Current sort mode
var g_showInProgress = true;
var g_showQueued = false;
var g_TimerID = 0;
var g_VideoElementCache = new Array;
var g_VideoElementTemplate = null;
var g_LabelsDivWidth = 225;
var g_MouseSelection = 0;
var g_SelectionBox_Start_X = 0;
var g_SelectionBox_Start_Y = 0;
var g_Labels = new Array;
var g_LastSelected = null;
var g_MouseDown = false;
var g_DraggingVideos = new Array;
var g_DraggingLabel = null;
var g_PreviouslySelected = new Array;
var g_VideoElementsPosition = new Array;
var g_LastSelStepWidth = 0;
var g_LastSelStepHeight = 0;
var g_SelectionStep = 30;
var g_ShowLabels = false;
var g_ForceRefresh = false;
var g_RefreshPages = false;
var g_ItemImageQueue = new Array;
var g_LoadImagesAsync = true;
var g_VideoRemoved = false;

var g_ScrollIncrement = 20;

var statusColorQueued = "cyan";
var statusColorDownloading = "white";
var statusColorDefault = "#908d8d";

var ITEMS_PER_PAGE = 10;
var STATUS_FLAG_FIELD_DESCRIPTION = 0x100;
var RSS_FLAG_FROM_AUTODOWNLOAD = 1;  //Item has been automatically downloaded
var RSS_FLAG_FROM_RSS = 2; //Item comes from RSS Feed

//Constants
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
//FLAG_MIN_DETAILS
var c_curClipDownSpeed    = 22;
var c_curClipUpSpeed      = 23;
var c_curClipSeeds        = 24;
var c_curClipPeers        = 25;
var c_curClipComplete     = 26;
var c_curClipIncomplete   = 27;
var c_curClipState        = 28;
var c_rssFlags			  = 29; 
var c_videoDescription    = 30;
var g_CurrentView		  = 0; 

var STATUS_FLAG_HIDE_QUEUED = 1;
var STATUS_FLAG_HIDE_IN_PROGRESS = 2;
var STATUS_FLAG_MIN_DETAILS = 3; 
var STATUS_FLAG_COLLECTION = 0x1000;

window.onresize = setCollectionSize;
document.ondragstart=new Function("return false")
document.onselectstart=new Function ("return false")

function setTimer()
{
	if (g_TimerID == 0)	g_TimerID = setInterval("refresh()", 2000);
}

function clearTimer()
{
	clearInterval(g_TimerID);
	g_TimerID = 0;	
}

function onActivated()
{
//	debugger; 
	setCollectionSize();
	g_CurrentView = external.ExecCmd("CycleCollectionView", 3, 0);	//get view	
	if (g_CurrentView == 0)
	{
		g_RefreshPages = true;
		refresh();
		setTimer();
	}
	else
	{
		setListView();	
	}
}

function onDeactivated()
{
	clearTimer();	
}

function searchStringChange()
{
	g_strFilter = search_box.value;
	page.numItems = 0; //make sure the page combo is reset
	try {
		external.ExecCmd("SetCollectionSearchString", search_box.value, 0);
	} catch (e) { 
	}
	
	g_RefreshPages = true;
	refresh();
}

function updateVideos()
{
	var dwFlags = STATUS_FLAG_COLLECTION; 	//call is coming from collection
	if (!g_showInProgress)
		dwFlags |= STATUS_FLAG_HIDE_IN_PROGRESS;
	if (!g_showQueued)
		dwFlags |= STATUS_FLAG_HIDE_QUEUED;
		
	dwFlags |= STATUS_FLAG_FIELD_DESCRIPTION;
			
	g_Videos = new Array; 
	
	var params = new Object;
	params.filter = g_strFilter;
	params.label = g_currentLabel;
	params.sortBy = g_CurSortMode;
	params.sortDir = g_SortDir;
	params.flags = dwFlags;
	params.startIndex = page.value;
	params.itemCount = ITEMS_PER_PAGE;
//	params.

	external.GetDownloadStatus(this, g_Videos, params);
	if (g_RefreshPages)
	{
		updatePageSelector(g_Videos.totalCount);
		g_RefreshPages = false;
	}	
	
	infoNoItems.style.display = g_Videos.length == 0 ? "block" : "none";
	infoNoItems.innerText = search_box.value.length > 0 ? "There are no matching videos" : "There are no items in this view";
}

function getVideoElement()
{
	if (g_VideoElementCache.length > 0)
	{
		return g_VideoElementCache.pop();
	}
	else
	{
		return g_VideoElementTemplate.cloneNode(true);
	}
}

function refresh()
{
	updateVideos();
	removeDeletedVideos();
	refreshVideos();	
	
	updateLabels();
	removeDeletedLabels();
	refreshLabels();	
	
	g_VideoRemoved = false;	
}

function refreshVideo(pVid, idx)
{
	var elem = document.getElementById("video_" + pVid[c_videoID]);	
	if (!g_ForceRefresh && elem && elem.crc32 && elem.crc32 == pVid.crc32 && elem.labels.crc32 == pVid.labels.crc32) return;
	
	var isNew = false;
	if (!elem)	
	{
		elem = getVideoElement();
		isNew = true;
	}
	
	elem.crc32 = pVid.crc32;
	elem.labels = pVid.labels;
	elem.labels.crc32 = pVid.labels.crc32;
	
	elem.id = "video_" + pVid[c_videoID];
	elem.style.display = "block";
	elem.all.videoTitle.innerHTML = (pVid[c_rssFlags] & RSS_FLAG_FROM_RSS) ? parseTitle(pVid[c_videoName]): pVid[c_videoName];
	elem.videoID = pVid[c_videoID];
	
	if ((pVid[c_videoStatusStr] == "Downloading" || pVid[c_videoStatusStr] == "Paused") )
	{
		var bytesDownloaded = pVid[c_videoDownloaded] * 1;	
		if (pVid[c_curClipID] * 1 < pVid[c_videoNumClips] * 1) bytesDownloaded += pVid[c_curClipDownloaded] * 1;
	
		var percent = "0%";
		if (bytesDownloaded > pVid[c_videoTotalSize]) 
			percent = "100%";
		else 
			if (pVid[c_videoTotalSize] > 0) percent = formatNumber(100 * (bytesDownloaded / pVid[c_videoTotalSize]), 1) + "%";
			
		if (pVid[c_videoStatusStr] == "Downloading")
		{
			elem.all.videoProgressBar.className = "dwn_progress_0";
			elem.all.videoProgressComplete.className = "dwn_progress_1";
			elem.all.videoProgressBar.title = "Click to Pause";
		}
		else if (pVid[c_videoStatusStr] == "Paused")
		{
			elem.all.videoProgressBar.className = "paused_progress_0";
			elem.all.videoProgressComplete.className = "paused_progress_1";
			elem.all.videoProgressBar.title = "Click to Resume";
		}							
		elem.all.videoProgressBar.style.display = "block";
		elem.all.videoProgressComplete.style.width = percent;			
	}
	else
	{
		elem.all.videoProgressBar.style.display = "none";			
	}
	
	if (pVid[c_videoStatusStr] == "Downloading")
	{
		elem.all.dldSpeedDelimiter.style.display = "block";
		elem.all.upSpeedDelimiter.style.display = "block";
		elem.all.videoDownloadSpeed.style.display = "block";
		elem.all.videoUploadSpeed.style.display = "block";		
	}
	else
	{
		elem.all.dldSpeedDelimiter.style.display = "none";
		elem.all.upSpeedDelimiter.style.display = "none";
		elem.all.videoDownloadSpeed.style.display = "none";
		elem.all.videoUploadSpeed.style.display = "none";
	}
		
	
	elem.isPaused = pVid[c_videoStatusStr] == "Paused";
	
	elem.all.videoDelete.videoElement = elem;
	elem.all.videoImage.videoElement = elem;
	elem.all.videoImage.videoID = pVid[c_videoID];
	elem.all.btnRSS.videoID = pVid[c_videoID];
	
	elem.all.videoStatus.innerText = pVid[c_videoStatusStr];
	if (pVid[c_videoStatusStr] == "Queued")
		elem.all.videoStatus.style.color = statusColorQueued;
	else if (pVid[c_videoStatusStr] == "Downloading")
		elem.all.videoStatus.style.color = statusColorDownloading;
	else
		elem.all.videoStatus.style.color = statusColorDefault;
	
	elem.all.videoProgressBar.videoElement = elem;
	elem.all.videoTitle.videoElement = elem;
	elem.all.videoTitle.videoID = pVid[c_videoID];
	elem.all.videoTitle.id_episode = pVid[c_episodeID];
	elem.all.videoDate.innerText = formatDateTime(pVid[c_videoTimeAdded]);
	elem.all.videoSize.innerText = formatSize(pVid[c_videoTotalSize]);
	elem.all.videoDownloadSpeed.innerText = formatSpeed(pVid[c_videoTotalDownSpeed]);
	elem.all.videoUploadSpeed.innerText = formatSpeed(pVid[c_videoTotalUpSpeed]);
	elem.all.videoDescription.innerHTML = pVid[c_videoDescription];
	
	elem.rowIndex = idx;
	
	//var imgsrc = "http://guide2.libertv.ro/guide/data/content/episode/" + pVid[c_episodeID] + "/image.jpg";
	var imgsrc = external.ExecCmd("GetItemImageURL", pVid[c_videoID], 0);
	if (g_LoadImagesAsync)
	{
//		if (elem.all.videoImage.temp_src != imgsrc)
//		{
			if (imgsrc == "") imgsrc = "images/collection_rev/episode_image.gif";				
			elem.all.videoImage.temp_src = imgsrc;
			g_ItemImageQueue.push(elem.all.videoImage);
//		}
	}
	else
	{	
		if (elem.all.videoImage.src != imgsrc)
			elem.all.videoImage.src = imgsrc;
	}
	
	if (pVid[c_videoCanPlay] == 1)
	{
		elem.all.btPlay.style.display = "block";
		elem.all.btPlay.videoID = pVid[c_videoID];
	}
	else
	{
		elem.all.btPlay.style.display = "none";
	}
	
	var btnRSS = elem.all.btnRSS;
		
	btnRSS.style.display = pVid[c_rssFlags] & RSS_FLAG_FROM_RSS ? "block" : "none";
	if (pVid[c_rssFlags] & RSS_FLAG_FROM_AUTODOWNLOAD)
	{
		if (btnRSS.src != "images/collection_rev/rss_icon2.gif")
			btnRSS.src = "images/collection_rev/rss_icon2.gif";			 
	}
	else
	{
		if (btnRSS.src != "images/collection_rev/rss_icon.gif")
			btnRSS.src = "images/collection_rev/rss_icon.gif";			 			
	}
			
	showVideoLabels(elem);
	
	if (isNew)
	{
/*		if (g_VideoRemoved)
		{			
			videoList.appendChild(elem);	
		}
		else
		{*/
			var firstChild = videoList.children[idx];
			if (firstChild)			
				videoList.insertBefore(elem, firstChild);			
			else
				videoList.insertBefore(elem);
//		}
	}
}

function setItemImagesDelayed()
{
	if (g_ItemImageQueue.length > 0)
	{
		var elem = g_ItemImageQueue.pop();
		if (elem.src != elem.temp_src)
			elem.src = elem.temp_src;
		g_ImagesTimeout = setTimeout("setItemImagesDelayed()", 10);
	}
}


function refreshVideos()
{
	var idx = 0;
	
//	for (var i = g_Videos.length - 1; i >= 0; --i)
	for (var i = 0; i < g_Videos.length; ++i)
	{
		refreshVideo(g_Videos[i], idx);	
		idx++;
	}	
	
	setTimeout("setItemImagesDelayed()", 100);
}

function removeDeletedVideos()
{
	var children = videoList.children;
	var delElems = new Array;
	for (var i = 0; i < children.length; ++i)
	{
		var videoID = children[i].id.replace("video_", "");
		
		var found = false;
		for (var j = 0; j < g_Videos.length; ++j)
			if (g_Videos[j][c_videoID] == videoID)
			{
				found = true;
				break;
			}
		
		if (!found)	
			delElems.push(children[i]);
	}	
	
	for (var i = 0; i < delElems.length; ++i)
	{
		videoList.removeChild(delElems[i]);
	}
}

function clearVideos()
{
	while (videoList.children.length)
		videoList.removeChild(videoList.children[0]);
}

function setCollectionSize()
{
	var height = document.documentElement.clientHeight;
	var width = document.documentElement.clientWidth; 
	if (height > 0)
	{
	    labels_div.style.height = height - topMainDiv.offsetHeight;	
    	collection_div.style.height = height - topMainDiv.offsetHeight; 
    }
    if (width - g_LabelsDivWidth > 0)
    {
	    labels_div.style.width = g_LabelsDivWidth;
	    collection_div.style.width = width - g_LabelsDivWidth; 
    }	
    
    calculatePositionCache();
}

function updatePageSelector(numItems)
{
	var sItem = 0;
	var pages = document.getElementById("page");	
	var optIdx = 1;
	
	if (pages.numItems && pages.numItems == numItems)
	    return false; 
	
	for (var i = 0; i < pages.options.length; ++i)
		pages.options[i] = null;
	pages.options.length = 0;

    pages.numItems = numItems; 
	while (sItem < numItems)
	{
		lItem = sItem + ITEMS_PER_PAGE;
		if (lItem > numItems) lItem = numItems;
		pages.options.add(new Option("Page " + optIdx, sItem));
		sItem = lItem;
		optIdx++;
	}
			
	if (pages.options.length <= 1)
	{
		page_selection.style.display = "none";
		return false; 
	}
	else
	{
		page_selection.style.display = "block";
	}		
	return true; 
}

function imageError(obj)
{
	var imgsrc = "images/collection_rev/episode_image.gif";
	if (obj.src != imgsrc)
		obj.src = imgsrc;	
}

function playVideo(videoID)
{
//	try{
		external.PlayVideo(videoID + "");
//	} catch(e){}
}

function pauseVideo(obj)
{
	var videoElement = obj.videoElement;
	try{
		external.PauseResume(videoElement.videoID); 
	} catch(e)
	{
		return;
	}
	
	if (videoElement.isPaused)
	{
		videoElement.all.videoProgressBar.className = "dwn_progress_0";
		videoElement.all.videoProgressComplete.className = "dwn_progress_1";
	}
	else
	{
		videoElement.all.videoProgressBar.className = "paused_progress_0";
		videoElement.all.videoProgressComplete.className = "paused_progress_1";
	}
	videoElement.isPaused = videoElement.isPaused ? false : true;
	event.returnValue = false; 
	window.event.cancelBubble = true;
}

function removeVideo(videoElement)
{
	event.returnValue = false; 
	window.event.cancelBubble = true;
	
	try	{
		var res = external.RemoveVideo(videoElement.videoID * 1); 		
		if (res == 0)return;			
	} catch(e){}
	
	g_VideoRemoved = true;
	
	videoElement.filters.item(0).Apply();
	videoElement.style.visibility = "hidden";
	videoElement.filters.item(0).play();
	
	clearTimer();
	
	setTimeout("afterDelete()", 1000);
}

function afterDelete()
{
	setTimer();
	refresh();	
}

function toggleInProgress()
{
	var dwFlags = STATUS_FLAG_COLLECTION; 
	g_showInProgress = !g_showInProgress;
	imgShowInProgress.src = "images/collection_rev/controls/" + (!g_showInProgress ? "in_progress_1.gif" : "filter_inprogress_0.gif");
	
	if (!g_showInProgress) dwFlags |= STATUS_FLAG_HIDE_IN_PROGRESS;
	if (!g_showQueued) dwFlags |= STATUS_FLAG_HIDE_QUEUED;
	try {
		external.ExecCmd("SetCollectionFlags", dwFlags, 0);
	} catch (e) { 
	}
	if (g_CurrentView == 0)
	{
		g_RefreshPages = true;
//		clearVideos();
		refresh();
	}
}

function toggleQueued()
{
	var dwFlags = STATUS_FLAG_COLLECTION;

	g_showQueued = !g_showQueued ;	
	imgShowQueued.src = "images/collection_rev/controls/" + (!g_showQueued ? "queue_1.gif" : "view_queued_0.gif");
	
	if (!g_showInProgress) dwFlags |= STATUS_FLAG_HIDE_IN_PROGRESS;
	if (!g_showQueued) dwFlags |= STATUS_FLAG_HIDE_QUEUED;
	
	try {
		external.ExecCmd("SetCollectionFlags", dwFlags, 0);
	} catch (e) { 
	}
	//debugger;
	if (g_CurrentView == 0)
	{
		g_ForceRefresh = true;
//		clearVideos();
		refresh();
	}
}

function updatePauseResumeButton()
{
	var isAllPaused = external.ExecCmd("IsAllPaused", "", "") * 1; 
	var imgsrc = "images/collection_rev/controls/" + (isAllPaused ? "pause_1.gif" : "pause_all_0.gif");
	if (imgPauseAll.src != imgsrc)
		imgPauseAll.src = imgsrc;
}

function PauseResumeAll()
{
	external.ExecCmd("PauseResumeAll", "", "");
	updatePauseResumeButton();
}

function onSearchMouseDown()
{
	window.event.cancelBubble = true;	
}

function onSearchSelect()
{
	window.event.cancelBubble = true;	
	window.event.returnValue = true;
	return true;	
}

function selectLabel(lblObj)
{
	var elems = lblObj.all;
	lblObj.className = "label_sel labsel_curves bg_right";
	elems.lblContainer.className = "labsel_main";
	elems.lblBody.className = "labsel_left";
	elems.lblArrow.style.display = "none";
}

function unselectLabel(lblObj, nextSel, prevSel)
{
	var elems = lblObj.all;
	lblObj.className = "label_unsel" + (prevSel ? " label_first_unsel" : "") + (nextSel ? " label_next_sel" : "");
	elems.lblContainer.className = "";
	elems.lblBody.className = "";	
	elems.lblArrow.style.display = "inline";
}

function setCurrentLabel(td)
{
	if (td == null)
		g_currentLabel = "all";
	else
		g_currentLabel = td.label;	
		
	imgShowInProgress.style.display = g_currentLabel == "all" ? "block" : "none";	
	imgShowQueued.style.display = g_currentLabel == "all" ? "block" : "none";	
	
	unselectAll();
			
	search_box.value = "";
	g_strFilter = "";
	
	_DBGAlert("currentLabel = " + g_currentLabel + "<br>"); 
		
	var table = document.getElementById("labelTable");
	var n = table.tBodies[0].rows.length - 2;
		
	for (var i = 0; i < table.tBodies[0].rows.length; ++i)
	{		
		var elem = table.tBodies[0].rows[i];
		if (elem.label == g_currentLabel)
		{
 			if (elem.idType == "fixed_labelRow")				
 				elem.all.lbl.className = "label_sticky_sel";
 			else if (elem.idType == "std_labelRow")
 				elem.all.lbl.className = "label_sel";
				
			elem.all.lblLeft.style.backgroundImage = "url(images/collection_rev/lbl_sel.gif)";
			var prev = table.tBodies[0].rows[i-1];
			if (prev)
			{
 				prev.all.lblRight.style.backgroundImage = "url(images/collection_rev/lbl_sel_prev.gif)";
 				prev.all.lblRight.style.backgroundPosition = "bottom";
			}
			var next = table.tBodies[0].rows[i+1];
			if (next && (next.idType == "std_labelRow" || next.idType == "fixed_labelRow" || next.idType == "noskin"))
			{
				next.all.lblRight.style.backgroundImage = "url(images/collection_rev/lbl_sel_next.gif)";
				next.all.lblRight.style.backgroundPosition = "top";
			}
			elem.all.lblRight.style.backgroundImage = "url(images/collection_rev/lbl_sel_right.gif)";
			elem.all.ctrls.className = "label_ctrls_sel";
			
			if (elem.idType == "std_labelRow")
				elem.all.del.style.display = "inline";
				
			if (elem.idType == "std_labelRow" || elem.idType == "fixed_labelRow")
			{
				elem.all.img.style.display = "none";
			}
		}
		else
		{
			var prevSel = i > 0 ? table.tBodies[0].rows[i-1].label==g_currentLabel : false;
			var nextSel = i < n ? table.tBodies[0].rows[i+1].label==g_currentLabel : false;
		
			if (elem.idType == "fixed_labelRow")
			{
				if (nextSel)
					elem.all.lbl.className = "label_sticky_b";
				else
					elem.all.lbl.className = "label_sticky";
			}
			else if (elem.idType == "std_labelRow")
			{
				if (nextSel)
					elem.all.lbl.className = "label_nosel_b";
				else
					elem.all.lbl.className = "label_nosel";
			}				
				
			elem.all.lblLeft.style.backgroundImage = "";
			elem.all.ctrls.className = "label_nosel";
				
			if (!prevSel)			
				elem.all.lblRight.style.backgroundImage = "";
							
			if (elem.idType == "std_labelRow")
				elem.all.del.style.display = "none";
				
			if (elem.idType == "std_labelRow" || elem.idType == "fixed_labelRow")
			{
				elem.all.img.style.display = "inline";
			}
		}
	}	
	
	try {
		external.ExecCmd("SetLastLabel", g_currentLabel, 0);
	} catch (e) { 
	}
	
	clearVideos();		
	g_RefreshPages = true;
	refresh();
	unselectAll();		
}

function deleteLabel(lbl)
{
	external.RemoveGlobalLabel(lbl.parentNode.parentNode.label);
	setCurrentLabel(null);
	refresh();
	window.event.cancelBubble = true;
	return false;
}
function showSelectionBox()
{
	var sbox = selection_box;
	g_SelectionBox_Start_X = window.event.clientX;
	//selection start Y is in div coords
	g_SelectionBox_Start_Y = window.event.clientY + collection_div.scrollTop;
	sbox.style.left = g_SelectionBox_Start_X + "px";
	sbox.style.top = g_SelectionBox_Start_Y + "px";
	sbox.style.width = "0px";
	sbox.style.height = "0px";
	sbox.style.display = "block";	
		
	clearTimer();
	calculatePositionCache();
	bodyDiv.setCapture();
}

var g_PrevMouseY = 0;
var g_PrevMouseX = 0;
var g_ScrollTimer = 0; 
var g_ScrollArea = 48; 

function resizeSelectionBox()
{
	var sbox = selection_box;
	
	var currentX = g_PrevMouseX;
	var currentY = g_PrevMouseY;	
	
	var startX = g_SelectionBox_Start_X;
	var startY = g_SelectionBox_Start_Y;
	
	var tmp = 0;
	if (currentX < startX) {
		tmp = startX; startX = currentX; currentX = tmp;
	}
	if (currentY < startY) {
		tmp = startY; startY = currentY; currentY = tmp;
	}
		
	sbox.style.left = startX + "px";
	sbox.style.top = (startY - collection_div.scrollTop) + "px";
	sbox.style.width = (currentX - startX) + "px";
	sbox.style.height = (currentY - startY) + "px";	
}

function scrollWindow()
{
	if (!g_MouseSelection)
		return; 
	
	var dv = collection_div; 
	var oldTop = dv.scrollTop;
	
	//translate this into window coords
	g_PrevMouseY -= oldTop;
	
	if (g_PrevMouseY > document.documentElement.clientHeight - g_ScrollArea)
	{
		dv.scrollTop+=g_ScrollIncrement; 
	}
	else
	if (g_PrevMouseY < g_ScrollArea)
	{
		if (dv.scrollTop > g_ScrollIncrement)
			dv.scrollTop -= g_ScrollIncrement; 
		else
			dv.scrollTop = 0;
	}
	else
	{
		_DBGAlert("timer reset "+g_PrevMouseY+"<br>");
		clearInterval(g_ScrollTimer); 
		g_ScrollTimer = 0; 
	}
	
	//translate it back into the div's coords
	g_PrevMouseY += dv.scrollTop;
	
	resizeSelectionBox();
	findSelectedItems();
}

function updateSelectionBox()
{
	if (g_MouseSelection == 0) return;
	
	var sbox = selection_box;
	var currentX = window.event.clientX;
	//first we get y in window coords
	var currentY = window.event.clientY;			

	if (currentY == -1) 
	{
		if (g_PrevMouseY < g_SelectionBox_Start_Y) 
			currentY = 0;
		else
			currentY = document.documentElement.clientHeight;
	}
	
	if (currentY < g_ScrollArea || (currentY > document.documentElement.clientHeight - g_ScrollArea))
	{
		if (g_ScrollTimer == 0)
			g_ScrollTimer = setInterval("scrollWindow()", 50); 
	}
	
	//now we transform y into the div's coords		
	currentY += collection_div.scrollTop;
	
	//save these raw values for the resize box function
	g_PrevMouseX = currentX;
	g_PrevMouseY = currentY;

	resizeSelectionBox();
	findSelectedItems();	
}

function hideSelectionBox()
{
	selection_box.style.display = "none";
	if (g_CurrentView == 0)
	{
	    setTimer();
	}
	bodyDiv.releaseCapture();
}

function findPos(obj) {
	var curleft = curtop = 0;
	if (obj.offsetParent) {
		curleft = obj.offsetLeft
		curtop = obj.offsetTop
		while (obj = obj.offsetParent) {
			curleft += obj.offsetLeft
			curtop += obj.offsetTop
		}
	}
	return [curleft,curtop];
}

function globalMouseUp()
{
	var dragElemID = g_DraggingLabel == null ? "dragvideo" : "draglabel";
	var dv = document.getElementById(dragElemID);
	if (dv)
	{
		if (dv.style.display != "none")
		{
			dv.style.display = "none";
			dv.style.width = "auto";
		}
	}	

	g_MouseDown = false;	
	g_DraggingLabel = null;
	g_DraggingVideos.length = 0; 
	
	if (window.event && !window.event.shiftKey && !window.event.ctrlKey)
	{
		updateSelectionBox();
	}
		
	g_MouseSelection = 0;
	if (g_ScrollTimer > 0)
	{
		clearInterval(g_ScrollTimer); 
		g_ScrollTimer = 0;
	}

	hideSelectionBox();	
}

function globalMouseMove()
{
	if (g_MouseDown)
	{
		var dragElemID = g_DraggingLabel ? "draglabel" : "dragvideo";
		var dv = document.getElementById(dragElemID);
		if (dv)
		{
			if (dv.style.display != "block") 
			{
				dv.style.display = "block";
				g_DragSize = dv.offsetWidth;
			}
			var posX = window.event.clientX + g_MouseRelX;
			var posY = window.event.clientY + g_MouseRelY + document.documentElement.scrollTop;
			var sizeX = g_DragSize; 
			if (posX + sizeX > document.body.offsetWidth)
				sizeX = document.body.offsetWidth - posX;
			if (sizeX < 0) sizeX = 0;
			dv.style.width = sizeX + "px";
			dv.style.left = posX;
			dv.style.top = posY;
		}		
	}
	
	if (g_MouseSelection)
	{
		updateSelectionBox();						
	}
}

function collectionMouseDown()
{
	if (window.event.button == 1)
	{
		//test if we're not clicking on the scrollbar
		var collection_div = document.getElementById("collection_div");
		var divpos = findPos(collection_div);
		if (window.event.clientX > divpos[0] + collection_div.offsetWidth - 16)
			return;			
		g_MouseSelection = 1;	
		showSelectionBox();		
	}
}

function updateLabels()
{
//	try {					
		g_Labels.length = 0; 
		external.GetGlobalLabels(g_Labels);
//	} catch (e) { 
//	 	if (g_DeveloperMode)
//			throw e;
//		return;
//	}
	removeDeletedLabels();	
}

function refreshLabels()
{
	for (var k = 0; k < g_Labels.length; ++k)
		refreshLabel(g_Labels[k]);		
}

function refreshLabel(label)
{
	var table = document.getElementById("labelTable");
	var myLabel = null;
	var rows = table.tBodies[0].rows;
	for (var i = 0; i < rows.length; ++i)
	{
		var row = rows[i];
		if (row.label == label) myLabel = row;		
	}
		
	if (!myLabel)
	{
		var tpl = table.tBodies[0].rows["tpl_labelRow"];
		var insIdx = tpl.rowIndex-1;
		myLabel = tpl.cloneNode(true);
		myLabel.idType = "std_labelRow";
		myLabel.id = "label_" + tpl.rowIndex; 
		myLabel.label = label;
		myLabel.all.labelTitle.innerText = label;
		table.tBodies[0].insertBefore(myLabel, rows[insIdx]);
	}
			
	if (myLabel)
	{
		myLabel.style.display = "block";
	}	
}

function removeDeletedLabels()
{
	for (var k = 0; k < labelTable.tBodies[0].rows.length; ++k)
	{
		var elem = labelTable.tBodies[0].rows[k];
		var found = false;
		if (elem.idType == "std_labelRow")
		{
			for (var i = 0; i < g_Labels.length; ++i)
			{
				if (g_Labels[i] == elem.label)
					found = true;
			}
			if (!found)
			{
				labelTable.tBodies[0].removeChild(elem);
				k--;
			}
		}
	}	
}

function showContextMenu(videoElement)
{
	if (window.event.button == 2)
	{
		var argStr = "";
		var n = g_PreviouslySelected.length;
		
		if (!videoElement.isSelected)	//not set or not selected
		{
			selectVideo(videoElement);
			argStr = videoElement.videoID;
		}
		else
		{
			for (var k = 0; k < n; k++)
			{
				argStr += g_PreviouslySelected[k].videoID + ",";
			}
		}
		
		if (argStr.length > 0)
		{
//			try{
				var res = external.ShowContextMenu(1, argStr);
				if (res > 0)
				{
					if (res == 3) 
					{
						g_VideoRemoved = true;	
						unselectAll();
					}
					refresh();
				}
				window.event.cancelBubble = true;
//			}
//			catch(e){}
		}
	}
	else
	{
		selectVideo(videoElement);	
	}
}

function findVideoElement(child)
{
	if (child && child.id && child.id.indexOf("video_") > 0) return child;
	if (child.parentNode)	return findVideoElement(child.parentNode);
	return null;
}

function calculatePositionCache()
{
	var videos = videoList.children;
	var n = videos.length;
	
	g_VideoElementsPosition.length = 0;
	
	for (var k = 0; k < n; ++k)
	{
		var obj = new Object;
		var pos = findPos(videos[k]);
		obj.x1 = pos[0];
		obj.y1 = pos[1];
		obj.x2 = obj.x1 + videos[k].offsetWidth;
		obj.y2 = obj.y1 + videos[k].offsetHeight;
		obj.videoElement = videos[k];
		g_VideoElementsPosition.push(obj);
	}	
}

function findSelectedItems()
{
	//process the y values to compute selected items
	var currentY = g_PrevMouseY;
	var startY = g_SelectionBox_Start_Y;
	var currentX = g_PrevMouseX;
	var startX = g_SelectionBox_Start_X;
			
	var tmp;
	if (currentY < startY) {
		tmp = startY; startY = currentY; currentY = tmp;
	}
	if (currentX < startX) {
		tmp = startX; startX = currentX; currentX = tmp;
	}

	var currentWidth = currentX - startX;
	var currentHeight = currentY - startY;
	
	if (Math.abs(g_LastSelStepWidth - currentWidth) < g_SelectionStep && Math.abs(g_LastSelStepHeight - currentHeight) < g_SelectionStep)
		return;

	var elems = g_VideoElementsPosition;
	var n = elems.length;	//unselect all
	
	var elementHeight = 100;
	
	for (var k = 0; k < g_PreviouslySelected.length; ++k)
	{
		unselectVideoElement(g_PreviouslySelected[k]);
	}
	
	g_PreviouslySelected.length = 0;
	
	var selStart = Math.max(Math.floor(startY / elementHeight) - 2, 0);
	var selEnd = Math.min(Math.floor(currentY / elementHeight), videoList.children.length);
	
	for (var k = selStart; k < selEnd; k++)
	{
		pElem = elems[k];
		if (pElem.videoElement && pElem.videoElement.videoID)
		{			
			//startY and currentY are in the div's coords
			if (pElem.y2 > startY && pElem.y1 < currentY && pElem.x1 < currentX && pElem.x2 > startX)
			{
				selectVideoElement(pElem.videoElement);
				g_PreviouslySelected.push(pElem.videoElement);
			}
			else
			{
				unselectVideoElement(pElem.videoElement);
			}
		}
	}			
		
	g_LastSelStepWidth = currentWidth;
	g_LastSelStepHeight = currentHeight;	
}

var g_SelectMaxTime = 0;

function selectVideoElement(videoElement)
{
	videoElement.isSelected = true;	
	videoElement.all.imageBackground.className = "eptem_photo_border_sel";
	videoElement.all.videoTitle.className = "video_title_sel";
}

function unselectVideoElement(videoElement)
{
	videoElement.isSelected = false;	
	videoElement.all.imageBackground.className = "eptem_photo_border";
	videoElement.all.videoTitle.className = "video_title";
}

function selectVideo(videoElement)
{		
	var videos = videoList.children;
	var n = videos.length;
	
	if (window.event && window.event.shiftKey)
	{
		var lastIndex = g_LastSelected != null ? g_LastSelected.rowIndex : videoElement.rowIndex;		
		var indexMin = lastIndex > videoElement.rowIndex ? videoElement.rowIndex : lastIndex;
		var indexMax = lastIndex < videoElement.rowIndex ? videoElement.rowIndex : lastIndex;
				
		g_PreviouslySelected.length = 0;
		for (var k = 0; k < n; k++)
		{
			var pVid = videos[k];
			if (pVid && pVid.videoID)
			{
				if (pVid.rowIndex >= indexMin && pVid.rowIndex <= indexMax)
				{
					selectVideoElement(pVid);
					g_PreviouslySelected.push(pVid);
				}
				else
				{
					unselectVideoElement(pVid);
				}
			}
		}
		window.event.cancelBubble = true;	
	}	
	else if (window.event && window.event.ctrlKey)
	{
		if (videoElement.isSelected)
		{
			unselectVideoElement(videoElement);
		}
		else
		{
			selectVideoElement(videoElement);
			var found = false;
			for (var k = 0; k < g_PreviouslySelected.length; ++k)
				if (g_PreviouslySelected[k] == videoElement)
					found = true;
			if (!found) g_PreviouslySelected.push(videoElement);
			g_LastSelected = videoElement;
		}
		window.event.cancelBubble = true;	
	}
	else
	{
		for (var k = 0; k < g_PreviouslySelected.length; k++)
		{
			var pVid = g_PreviouslySelected[k];
			if (pVid && pVid.videoID && pVid != videoElement)
			{
				unselectVideoElement(pVid);
			}
		}
		g_PreviouslySelected.length = 0;
		g_PreviouslySelected.push(videoElement);
		selectVideoElement(videoElement);		
		g_LastSelected = videoElement;		
	}	
}

function setCurrentLabelStr(strLabel)
{
	var table = document.getElementById("labelTable");
	var n = table.tBodies[0].rows.length - 2;
	for (var i = 0; i < table.tBodies[0].rows.length; ++i)
	{		
		var elem = table.tBodies[0].rows[i];
		if (elem.label == strLabel)
		{
			setCurrentLabel(elem);
			return; 
		}
	}
	setCurrentLabel(null);
}

function addLabelMouseUp()
{
	if (g_MouseDown && g_DraggingVideos.length > 0)
	{
		var strVideos = g_DraggingVideos.join(",");
		var newLabel = external.ShowAddLabelDlg("");
		if (newLabel && newLabel != "")
		{
			external.AddGlobalLabel(newLabel + "");
			external.AddLabel(strVideos, newLabel);
			setCurrentLabelStr(newLabel);
		}		
	}	
}

function showAddLabel()
{
	var newLabel = external.ShowAddLabelDlg("");
	if (newLabel && newLabel != "")
	{
		external.AddGlobalLabel(newLabel + "");
		refresh();
		setCurrentLabelStr(g_currentLabel);
	}
}

function beginDrag(videoElement)
{
	if (window.event.button == 1)
	{
		g_MouseDown = true;
		
		var videos = videoList.children;
		g_DraggingVideos.length = 0; 
		
		if (videoElement.isSelected == false)
		{
			selectVideo(videoElement);
			g_DraggingVideos.push(videoElement.videoID);
		}
		else
		{
			for (var k = 0 ; k < videos.length; k++)
			{
				if (videos[k].isSelected)
				{
					g_DraggingVideos.push(videos[k].videoID);
				}
			}
		}
				
		var dv = document.getElementById("dragvideo");
		if (dv)
		{
			var pos = findPos(videoElement.all.videoTitle);
			g_MouseRelX = 10; //pos[0] - window.event.clientX + 180;
			g_MouseRelY = 0;
			if (g_DraggingVideos.length <= 1)
			{
				dv.all.videoTitle.innerText = videoElement.all.videoTitle.innerText;
				dv.all.videoImage.src = videoElement.all.videoImage.src;				
			}
			else
			{
				dv.all.videoTitle.innerText = "(Multiple items)";
				dv.all.videoImage.src = "images/collection_rev/episode_more_items.gif";				
			}
		}				
		
		window.event.cancelBubble = true;
	}
}

function labelMouseUp(lbl)
{
	if (g_MouseDown && g_DraggingVideos.length > 0)
	{
//		try{
			var strDragging = g_DraggingVideos.join(","); 
			external.AddLabel(strDragging, lbl.label);
//		}
//		catch(e){}
		labelHideFeedback(lbl);
		dragvideo.style.display = "none";
		refresh(); 
	}
}

function labelHideFeedback(lblObj)
{
	if (g_MouseDown && g_DraggingVideos.length > 0)
		if (lblObj.className == "label_feedback")
			lblObj.className = "label_unsel";	
}

function labelFeedBack(lblObj)
{
	if (g_MouseDown && g_DraggingVideos.length > 0)
	{
		if (lblObj.className == "label_unsel")
		{
			lblObj.className = "label_feedback";
		}
	}
}

function beginLabelDrag(lbl)
{
	if (g_CurrentView != 0) return;
	
	g_MouseDown = true;
	g_DraggingLabel = lbl;
	var dv = document.getElementById("draglabel");
	if (dv)
	{
		var pos = findPos(lbl.children['lbl']);
		g_MouseRelX = pos[0] - window.event.clientX + 10;
		g_MouseRelY = -16;
		dv.innerText = lbl.label;
	}
}

function videoMouseUp(pTR)
{
	if (g_MouseDown && g_DraggingLabel != null)
	{
//		try{
			external.AddLabel(pTR.videoID, g_DraggingLabel.label);
//		} catch(e){}
		draglabel.style.display = "none";
		g_DraggingLabel = null;
		if (!g_ShowLabels) toggleLabels();
		refresh(); 
	}
}

function unselectAll()
{
	var videos = g_PreviouslySelected;
	for (var k = 0; k < videos.length; ++k)
		unselectVideoElement(videos[k]);	
}

function showVideoLabels(videoElement, pVid)
{
	var objects = videoElement.all;
	if (g_ShowLabels)
	{	
		var labeled = true;
		if (videoElement.labels != null && videoElement.labels.length > 0)
		{
			objects.videoLabels.style.display = "inline";
			var labels = videoElement.labels;
			
			var mlt = objects.videoLabelList;
			while (mlt.childNodes[0]) mlt.removeChild(mlt.childNodes[0]);			
			for (var i = 0; i < labels.length; ++i)
			{
				if (labels[i].length == 0) continue;
				var spanEl = document.createElement("span");
				spanEl.innerText = labels[i];
				spanEl.videoID = videoElement.videoID;
				spanEl.label = labels[i];
				spanEl.title = "Right click to remove";
				spanEl.onmousedown = function()
				{
					if (window.event.button == 2)
					{
						removeLabelFromVideo(this.videoID, this.label);
					}
					else
						setCurrentLabelStr(this.label);
				}
				spanEl.className = "text_small";	//change this
				if (i < labels.length - 1)	spanEl.style.paddingRight = "5px";
				spanEl.style.textDecoration = "none";
				spanEl.style.cursor = "pointer";
				mlt.appendChild(spanEl);
			}
		}
		else
		{
			objects.videoLabels.style.display = "none";
		}
	}
	else
	{
		objects.videoLabels.style.display = "none";
	}
}

function removeLabelFromVideo(videoID, lbl)
{
	external.RemoveLabel(videoID*1, lbl+""); 
	g_ForceRefresh = true;
	refresh();
	g_ForceRefresh = false;
	window.event.cancelBubble = true;
	return false;
}

function toggleLabels()
{
	g_ShowLabels = !g_ShowLabels;
	if (g_ShowLabels)
		label_toggle.src = "images/collection_rev/bt_hide_labels.gif";				
	else
		label_toggle.src = "images/collection_rev/bt_view_labels.gif";				


	g_ForceRefresh = true; 
	refresh(); 
	g_ForceRefresh = false; 
}

function prevPageDown()
{
	genericButtonMouseDown(prevPageBtn);
	prevPageBtn.src = "rssfeeds/previous_page_1.gif";
}

function prevPage()
{
	prevPageBtn.src = "rssfeeds/previous_page.gif";
	
	if (prevPageBtn.mouseInside)
	{
		if (page.selectedIndex > 0)
			page.selectedIndex = page.selectedIndex - 1;
		page.onchange.apply();
	}
	
	prevPageBtn.releaseCapture(); 
}

function nextPageDown()
{	
	genericButtonMouseDown(nextPageBtn);
	nextPageBtn.src = "rssfeeds/next_page_1.gif";
}

function nextPage()
{
	nextPageBtn.src = "rssfeeds/next_page.gif";
	nextPageBtn.releaseCapture();
	
	if (nextPageBtn.mouseInside)
	{
		if (page.selectedIndex < page.options.length - 1)
			page.selectedIndex = page.selectedIndex + 1;
		page.onchange.apply();
	}
}

function videoImageError(obj)
{
	if (obj.src != "images/collection_rev/episode_image.gif")
		obj.src = "images/collection_rev/episode_image.gif";
}

function setSortBy(str)
{
	var oldSortMode = g_CurSortMode;
	

	if (str == "name")
	{
		if (g_CurSortMode != "name")
		{
			g_CurSortMode = "name";		
			g_SortDir = "desc";
		}
		else
		{
			g_SortDir = g_SortDir == "desc" ? "asc" : "desc";					
		}
		btnSortName.src = "images/collection_rev/" + (g_SortDir == "desc" ? "sb_name_1_up.gif" : "sb_name_1_down.gif");	
		btnSortDate.src = "images/collection_rev/sb_date_0.gif";
		btnSortSize.src = "images/collection_rev/sb_size_0.gif";
	}
	else if (str == "date")
	{
		if (g_CurSortMode != "date")
		{
			g_CurSortMode = "date";
			g_SortDir = "desc";		
		}		
		else
		{
			g_SortDir = g_SortDir == "desc" ? "asc" : "desc";	
		}
		
		btnSortName.src = "images/collection_rev/sb_name_0.gif";	
		btnSortDate.src = "images/collection_rev/" + (g_SortDir == "asc" ? "sb_date_1_up.gif" : "sb_date_1_down.gif");
		btnSortSize.src = "images/collection_rev/sb_size_0.gif";		
	} 
	else
	{
		if (g_CurSortMode != "status")
		{
			g_CurSortMode = "status";
			g_SortDir = "desc";		
		}		
		else
		{
			g_SortDir = g_SortDir == "desc" ? "asc" : "desc";	
		}
		
		btnSortName.src = "images/collection_rev/sb_name_0.gif";	
		btnSortDate.src = "images/collection_rev/sb_date_0.gif";
		btnSortSize.src = "images/collection_rev/" + (g_SortDir == "desc" ? "sb_size_1_up.gif" : "sb_size_1_down.gif");
	}	
	
	clearVideos();
	g_ForceRefresh = true;
	refresh();
	g_ForceRefresh = false;
}

function onViewChanged(nView)
{
	if (nView == 1)
	{
		clearTimer(); 
		collection_div.className = "collectionDivListView";
		collectionHeader.style.display = "none";
		videoList.style.display = "none";
	}
	else
	{
		collection_div.className = "collectionDiv";
		collectionHeader.style.display = "block";
		videoList.style.display = "block";		
		setTimer(); 
	}
	
	btnChangeView.src = nView == 0 ? "images/collection_rev/controls/list_view.gif" : "images/collection_rev/controls/expanded_view.gif";
	btnChangeView.title = nView == 0 ? "Switch to list view" : "Switch to thumbnail view";	
	g_CurrentView = nView; 
}

function goToVideo(obj)
{
	try { external.EpisodeDetails(obj.videoID); } catch (e) { }	
}

function genericButtonMouseDown(obj)
{
	obj.setCapture();
	window.event.cancelBubble = true;
}

function genericButtonMouseEnter(obj)
{
	if (obj.id == window.event.srcElement.id)
	{
		window.event.cancelBubble=true; 
		obj.mouseInside = true;
	}	
}

function genericButtonMouseLeave(obj)
{
	if (obj.id == window.event.srcElement.id)
	{
		window.event.cancelBubble=true; 
		obj.mouseInside = false;
	}	
}

function setListView()
{
	external.ExecCmd("CycleCollectionView", 1, 0);		
}

function changeView()
{
	var view = external.ExecCmd("CycleCollectionView", 3, 0);	//get view	
	external.ExecCmd("CycleCollectionView", view == 0 ? 1 : 0, 0);	
//	btnChangeView.src = view == 1 ? "images/collection_rev/controls/list_view.gif" : "images/collection_rev/controls/expanded_view.gif";
//	btnChangeView.title = view == 1 ? "Switch to list view" : "Switch to detailed view";
}

function gotoRSS(obj)
{
	try{
		external.ExecCmd("GotoRSS", obj.videoID, ""); 
	} catch(e){}
	window.event.cancelBubble = true;
}