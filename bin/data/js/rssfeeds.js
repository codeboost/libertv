
var g_LabelsDivWidth = 225;

var ITEM_CHANNELID = 0;
var ITEM_GUID = 1;
var ITEM_TITLE = 2;
var ITEM_LINK = 3;
var ITEM_DATE = 4;
var ITEM_SIZE = 5;
var ITEM_TYPE = 6;
var ITEM_URL = 7;
var ITEM_VIDEOID = 8;
var ITEM_FLAGS = 9;
var ITEM_DESCRIPTION = 10;
var ITEM_IMAGE = 11;

var CHANNEL_ID = 0;
var CHANNEL_NAME = 1;
var CHANNEL_LINK = 2;
var CHANNEL_NUMNEW = 3;
var CHANNEL_IMAGE = 4;
var CHANNEL_DESCRIPTION = 5;
var CHANNEL_FLAGS = 6;
var CHANNEL_MAX_DOWNLOADS = 7; 
var CHANNEL_EPISODE = 8;
var CHANNEL_FILTER_CONTAINS = 9;
var CHANNEL_FILTER_NOT_CONTAINS = 10; 

var CHAN_SCROLL_MAX = -25;

var FEED_FLAG_REFRESHED = 0x08;

var MAX_SPLITTER_WIDTH = 500; 
var MIN_SPLITTER_WIDTH = 225; 

var ITEMS_PER_PAGE = 5;

window.onresize = setCollectionSize;
document.ondragstart = new Function ("return false")

var g_CurrentChannel = -1;
var g_Channel = null;

var g_Images = new Array();
var g_strFilter = "";
var g_Timer = 0;
var g_TimerInterval = 5000;
var g_ElementCache = new Array();
var g_ItemImageQueue = new Array();
var g_Resizing = false;

var g_FilterAuto = false;
var g_ChannelsInfo = new Array();
var g_ChannelList = null;
var g_Scrolling = false;
var g_ScrollStart = 0;
var g_ScrollThumbStart = 0;
var g_CurrentUpdate = 1;
var g_LoadImagesAsync = true;

var g_StreamableTypes = new Array(
"video/quicktime", 
"video/mp4", 
"video/m4v", 
"video/vnd.objectvideo", 
"video/ms-wmv", 
"video/mov", 
"video/x-m4v", 
"video/mpeg", 
"video/x-mp4", 
"video/x-flv", 
"video/flv", 
"audio/mp4", 
"audio/x-m4v", 
"video/asf", 
"video/wmv", 
"video/divx",
"video/xvid", 
"video/x-ms-wmv");

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

var ttime = 0;

//Called by player
function getSelectedChannelId()
{
	return g_CurrentChannel;
}

function setLastChannelId()
{
	try {
		external.ExecCmd("SetSelectedChannelId", g_CurrentChannel, 0);
	} catch (e) { 
	}	
}

function delayActivated()
{
	var aChannel  = external.ExecCmd("GetSelectedChannelId", 0, 0) * 1; 
	refreshChannels();	
	
	if (g_CurrentChannel != aChannel)
	{
		goToChannelById(aChannel);	
	}
	
	g_Timer = setInterval("changeChannel(g_Channel)", g_TimerInterval);		
}

function onActivated()
{
	if (document.readyState != "complete")
	{
		return; 
	}
	
	try{		
		ITEMS_PER_PAGE = external.ExecCmd("GetRSSItemsPerPage", 0, 0) * 1;
	} catch (e) 
	{ 
		ITEMS_PER_PAGE = 5;
	}
	
	try{
		var splitterWidth = external.ExecCmd("GetSplitterPosition", 0, 0) * 1;
		doSplitterResize(splitterWidth);
	} catch (e) { }
	
	setTimeout("delayActivated()", 50);
}

function onDeactivated()
{
	clearInterval(g_Timer);	
}

function channelSetAutoDownload()
{
	try
	{
		external.ExecCmd("SetFeedAutoDownload", g_CurrentChannel, 1);
		g_Channel.isAuto = g_Channel.isAuto ? false : true;
				
		refreshChannels();
	} catch (e) { }
}

function markAllAsViewed()
{
	try 
	{
		external.ExecCmd("SetFeedAsRead", g_CurrentChannel, 0);
 		refreshChannels();
	} catch (e) { }	
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

function filterAutoChannels()
{
	g_FilterAuto = g_FilterAuto ? false : true;	
	if (g_FilterAuto)
		btAutoFilter.src = "rssfeeds/filter_bt_1.gif";
	else
		btAutoFilter.src = "rssfeeds/filter_bt_0.gif";
	
	channelsScroller.style.top = CHAN_SCROLL_MAX + "px";	
	
	refreshChannels();
}

function searchStringChange()
{
	g_strFilter = search_box.value.toLowerCase();
	var channelTable = document.getElementById("channelTable");
	var items = channelTable.tBodies[0].rows;
	
	var current_unsel = false;	
	var new_ch = null;
	var new_idx = -1;
	
	channelsScroller.style.top = CHAN_SCROLL_MAX + "px";	
		
//	debugger; 

	for (var i = 0; i < items.length; ++i)
	{
		if (items[i].idType != "channel") continue;
		
		var ch_title = items[i].all.channelName.innerText.toLowerCase();
								
		if (ch_title.indexOf(g_strFilter) < 0)
		{
			items[i].style.display = "none";
			if (g_CurrentChannel == items[i].channelID)
				current_unsel = true;
		}
		else
		{				
			if (g_CurrentChannel == items[i].channelID)
			{
				new_ch = items[i];
				new_idx = i;
			}
			else if (new_ch == null) 
			{
				new_ch = items[i];
				new_idx = i;
			}
							
			items[i].style.display = "block";
		}
	}
			
	if (current_unsel)
	{
		if (new_ch == null)
		{
			channelDescription.innerText = "";
			channelTitle.innerText = "";
			unselectChannel(items["channelNew"], false, null);			
			unselectChannel(items["channelDummy"], false, null);
			g_CurrentChannel = null;
			g_Channel = null;
			clearItems();
		}
		else
		{
			goToChannelById(new_ch.channelID);
		}
	}
	else
	{
		if (new_ch) goToChannelById(new_ch.channelID)
	}
		
	linkChannels();
    checkScrollButtons();		
}

function requireScrollButtons()
{
	return channelsScroller.offsetHeight - 40 > channelsContainer.offsetHeight && channelsContainer.offsetHeight > 0;
}

function checkScrollButtons()
{
    if (requireScrollButtons())
	{
//		channelScrollUp.style.display = "block";
//		channelScrollDown.style.display = "block";		
	}
	else
	{
//		channelScrollUp.style.display = "none";
//		channelScrollDown.style.display = "none";
	}	
}

function setCollectionSize()
{
	var height = document.documentElement.clientHeight;
	var width = document.documentElement.clientWidth; 
	if (height > 0)
	{
	    channels_div.style.height = height; //Math.max(collection_div.offsetHeight+30, height); 
	    collection_div.style.height = height - topDiv.offsetHeight;
	    splitter_div.style.height = height;
	    scroller_div.style.height = height;	    
	    scrollBarTotal.style.height = Math.max(height - 150, 0);
	    var nh = (height - channelsHeader.offsetHeight);
	    if (nh < 0) nh = 0;
	    channelsContainer.style.height = nh + "px";
	}
	if (width > 0)
	{
		collection_div.style.width = width - channels_div.offsetWidth - splitter_div.offsetWidth - scroller_div.offsetWidth; 
	   	leftHeader.style.width = channels_div.offsetWidth + splitter_div.offsetWidth / 2 + scroller_div.offsetWidth-5;
	    channelTitle.style.width = Math.max(collection_div.offsetWidth - 520, 0); //add up the widths of the top bar elements and you get 520!
    }
    
    checkScrollButtons();
	resizeScrollBarThumb();
}

function addFeed()
{
	var obj = new Object; 
	var	newChannelID = external.RSSAddFeed(obj);
	if (newChannelID != 0)
	{
		item_filter.value = "";
		g_strFilter = "";
		refreshChannels(newChannelID);
	}
}

function playItem(videoID)
{
	external.PlayVideo(videoID + "");
}

function errorItemImage(img)
{
	if (img.src != "rssfeeds/episode_image.gif")
		img.src = "rssfeeds/episode_image.gif";	
}

function currentTime()
{
	var d = new Date();
	return d.getMinutes() * 60 * 1000 + d.getSeconds() * 1000 + d.getMilliseconds();
}

function setChannelDetails(channelID, aRow, channel, prevChannelID)
{
	if (aRow.crc32 && aRow.crc32 == channel.crc32) 
	{
		return;
	}
	channelID = channelID * 1; 
	if (!g_ChannelsInfo[channelID]) g_ChannelsInfo[channelID] = new Object();
	
	g_ChannelsInfo[channelID].name = decode_entities(channel[CHANNEL_NAME]);
	g_ChannelsInfo[channelID].img = channel[CHANNEL_IMAGE];
	g_ChannelsInfo[channelID].numNew = channel[CHANNEL_NUMNEW]*1;
	g_ChannelsInfo[channelID].maxDownloads = channel[CHANNEL_MAX_DOWNLOADS];
	g_ChannelsInfo[channelID].url = channel[CHANNEL_LINK];
	g_ChannelsInfo[channelID].flags = channel[CHANNEL_FLAGS]*1;
	g_ChannelsInfo[channelID].prevChannelID = prevChannelID;
	g_ChannelsInfo[channelID].id_episode = channel[CHANNEL_EPISODE];
	g_ChannelsInfo[channelID].filterContains = channel[CHANNEL_FILTER_CONTAINS];
	g_ChannelsInfo[channelID].filterNotContains = channel[CHANNEL_FILTER_NOT_CONTAINS];
	
	aRow.style.display = "block";	
	aRow.id = "channel" + channelID;
	aRow.idType = "channel";
	aRow.isAuto = channel[CHANNEL_FLAGS] & 1 >= 0 ? true : false;
	aRow.crc32 = channel.crc32;
	
	var elems = aRow.all;
	var titleDiv = elems.channelName;
	var ch_title = channel[CHANNEL_NAME];
//	if (ch_title.length > 20) ch_title = ch_title.substr(0, 20);
	if (channel[CHANNEL_NUMNEW]*1 > 0) ch_title += "  (" + channel[CHANNEL_NUMNEW] + ")";
	titleDiv.innerText = decode_entities(ch_title);

	if (channel[CHANNEL_FLAGS]*1 == 1)
		elems.rssIcon.src = "rssfeeds/rss_icon_2.gif";
	
	aRow.channelID = channel[CHANNEL_ID]*1;
}

function decode_entities(str)
{
	if (str.indexOf("&") >= 0)
		str = str.replace("&apos;", "'").replace("&quot;", "\"").replace("&lt;", "<").replace("&gt;", ">");	
	return str;
}

function linkChannels()
{
	var rows = channelTable.tBodies[0].rows;
	var chrows = new Array();
	for (var i = 0; i < rows.length; ++i)
	{
		if (!rows[i].channelID) continue;	
		if (rows[i].style.display == "none") continue;	
		chrows.push(rows[i]);
	}	
	var prevID = chrows.length > 1 ? chrows[chrows.length - 2].channelID : 0;
	for (var i = chrows.length - 1; i >= 0; --i)
	{
		chrows[i].prevChannelID = prevID;
		prevID = chrows[i].channelID;
	}
}

function refreshChannels(channelID)
{	
	clearChannels();
	g_ChannelList = new Array;
	try
	{
		external.RSSGetChannels(this, "", g_ChannelList);
	} catch (e) { return; }
		
	var channels = g_ChannelList;
	
	var channelTable = document.getElementById("channelTable");
	var templateRow = channelTable.tBodies[0].rows["channelRow"];
	var dummyRow = channelTable.tBodies[0].rows["channelDummy"];

	var rows = channelTable.tBodies[0].rows;
	
	var firstChannel = rows['channelNew'];
	var newsChannel = channelTable.tBodies[0].rows['channelNew'];
	
	if (!g_ChannelsInfo[0]) g_ChannelsInfo[0] = new Object();
	if (!g_ChannelsInfo[1]) g_ChannelsInfo[1] = new Object();
	if (!g_ChannelsInfo[2]) g_ChannelsInfo[2] = new Object();
		
	if (channels.length == 0) 
	{
		channelContainer.style.display = "none";
//		channelScrollUp.style.display = "none";
//		channelScrollDown.style.display = "none";
		infoText.style.display = "block";
		g_ChannelsInfo[0].numNew = 0;		
		newsChannel.all.channelName.innerText = "NEW ITEMS";		
		changeChannel(firstChannel);
		return;
	}
	channelContainer.style.display = "block";
	infoText.style.display = "none";
	
	var totalNew = 0;	
	var prevChannelID = 0;
	
	for (var i = channels.length - 1; i >= 0; --i) 
	{
		var channel = channels[i];
		
		if (channel.length == 0)
			continue; 
			
		if (channel[CHANNEL_NAME].toLowerCase().indexOf(g_strFilter) < 0)
			continue;
		var flags = channel[CHANNEL_FLAGS]*1;
		if (g_FilterAuto && !(flags & 1))  
			continue;
				
		var thisChannelID = channel[CHANNEL_ID];
		
		totalNew += channel[CHANNEL_NUMNEW]*1;
				
		var isNew = false;
		var aRow = null;
		for (var j = 0; j < rows.length; ++j)
		{
			if (rows[j].channelID == thisChannelID)
			{
				aRow = rows[j];
			}
		}
		
		if (!aRow) 
		{
			aRow = templateRow.cloneNode(true);
			isNew = true;
		}
		
		setChannelDetails(thisChannelID, aRow, channel, channel.prevChannelID);
		
		if (channelID)
		{	
			if (aRow.channelID == channelID && aRow.style.display != "none")
			{
				firstChannel = aRow;
			}
		}
		else
		{	
			if (aRow.channelID == g_CurrentChannel && aRow.style.display != "none")
			{
				firstChannel = aRow;
			}
		}	
		
		if (isNew)
			channelTable.tBodies[0].insertBefore(aRow, dummyRow);
	}	
		
	linkChannels();
	
	newsChannel.all.channelName.innerText = "NEW ITEMS  (" + totalNew + ")";
	g_ChannelsInfo[0].numNew = totalNew;
	
    checkScrollButtons();
	resizeScrollBarThumb();
	
	if (firstChannel != null)
		changeChannel(firstChannel);		
}

function refreshChannelData()
{
	var channels = new Array; 
	try
	{
		external.RSSGetChannels(this, "", channels);
	} catch (e) { return; }
	
	var rows = channelTable.tBodies[0].rows;
	var prevChannelID = 0;
	for (var i = channels.length - 1; i >= 0; --i) 
	{
		var channel = channels[i];

		var aRow = null;		
		for (var j = 0; j < rows.length; ++j)
		{
			if (rows[j].channelID == channel[CHANNEL_ID])
			{
				aRow = rows[j];
			}
		}
		
		if (aRow)
		{
			setChannelDetails(channel[CHANNEL_ID], aRow, channel, prevChannelID);
			prevChannelID = channel[CHANNEL_ID];
		}
	}
}


function clearChannels()
{
	var channelTable = document.getElementById("channelTable");
	var rows = channelTable.tBodies[0].rows;
	var delRows = new Array();
	var idx=0;
	for (var i = 0; i < rows.length; ++i)
	{
		rows[i].all.lblRight.style.backgroundImage = "";
		if (rows[i].idType == "channel")
			delRows[idx++] = rows[i];
	}
	
	for (var i = 0; i < delRows.length; ++i)
	{
		channelTable.tBodies[0].removeChild(delRows[i]);
	}
}

function onLoad()
{
	onActivated();
	channelsScroller.style.top = CHAN_SCROLL_MAX + "px";	
	g_Timer = setInterval("changeChannel(g_Channel)", g_TimerInterval);
}

function findPrevChannel(elems, idx)
{
	var prev = null;
	var pc_id = idx - 1;
	
	while (pc_id >= 0 && !isVisible(elems[pc_id])) pc_id--;
	if (pc_id >= 0) prev = elems[pc_id];
		
	return prev;
}

function findNextChannel(elems, idx)
{
	var next = null;
	var nc_id = idx + 1;
	
	while (nc_id < elems.length && !isVisible(elems[nc_id])) nc_id++;
	if (nc_id < elems.length) next = elems[nc_id];
	
	return next;
}

function isVisible(e) 
{	
	return e.style.display.toLowerCase() != 'none' && e.style.visibility.toLowerCase() != 'hidden';
}

function goToChannelById(channelID)
{
	changeChannelById(channelID);
	var scHeight = channelsScroller.style.top.replace("px", "") * 1;
	
	if (g_Channel.offsetTop - scHeight > channelsContainer.offsetHeight)
	{
		var ny = -g_Channel.offsetTop + 5;
		var mins = channelsContainer.offsetHeight - channelsScroller.offsetHeight - CHAN_SCROLL_MAX;
		if (mins > CHAN_SCROLL_MAX) mins = CHAN_SCROLL_MAX;
		
		if (ny < mins) ny = mins;
		if (ny > CHAN_SCROLL_MAX) ny = CHAN_SCROLL_MAX;
		
		channelsScroller.style.top = ny + "px";				
	}		
}
function selectChannelById(channelID)
{
	var channelTable = document.getElementById("channelTable");
	var rows = channelTable.tBodies[0].rows;
	
	for (var i = 0; i < rows.length; ++i)
	{
		if (rows[i].channelID == channelID)
		{
			var prev = findPrevChannel(rows, i);
			var next = findNextChannel(rows, i);

			var pt = prev.all.channelName ? prev.all.channelName.innerText : "-";
			var nt = next.all.channelName ? next.all.channelName.innerText : "-";
							
			selectChannel(rows[i], prev, next);					
		}	
		else
		{
			var next = findNextChannel(rows, i);
			var prev = findPrevChannel(rows, i);			
			var prevSel = false;
			if (prev) prevSel = g_CurrentChannel == prev.channelID;
			
			unselectChannel(rows[i], prevSel, prev);
		}
	}
}

function selectChannel(elem, prev, next)
{	
	if (elem.id == "channelNew" || elem.id == "channelAll" || elem.id == "channelDownloads")
	{
		elem.className = "label_sel";
		elem.all.feedItem.className = "feed_item_sel";
		elem.all.ctrls.className = "label_sel";		
		elem.all.rssIcon.src = "rssfeeds/tab_star_sel.gif";		
	}
	else
	{
		elem.className = "label_sel";		
		elem.all.feedItem.className = "feed_item_sel";	
		elem.all.del.style.display = "block";
		elem.all.ctrls.className = "label_sel";		
		if (g_ChannelsInfo[elem.channelID].isRefreshing)
		{
			elem.all.rssIcon.src = "rssfeeds/refreshing_1.gif";
		}
		else
		{
			elem.all.rssIcon.src = "rssfeeds/rss_icon_1.gif";
		}
	}
			
	elem.all.lblLeft.style.backgroundImage = "url(rssfeeds/lbl_sel.gif)";
	elem.all.lblRight.style.backgroundImage = "url(rssfeeds/lbl_sel_right.gif)";
	
	if (prev) 
	{
		prev.all.lblRight.style.backgroundImage="url(rssfeeds/lbl_sel_prev.gif)";
		prev.all.lblRight.style.backgroundPosition = "bottom";
		prev.all.feedItem.style.borderBottom = "";
	}
	if (next) 
	{
		next.all.lblRight.style.backgroundImage="url(rssfeeds/lbl_sel_next.gif)";
		next.all.lblRight.style.backgroundPosition = "top";
	}						
}

function unselectChannel(elem, prevSel, prev)
{
	elem.className = "label_nosel";		
	if (elem.idType == "channel") 
	{
		elem.all.feedItem.className = "feed_item_nosel";
		elem.all.del.style.display = "none";
		elem.all.ctrls.className = "label_no_sel";
				
		if (g_ChannelsInfo[elem.channelID].isRefreshing)
		{
			elem.all.rssIcon.src = "rssfeeds/refreshing.gif";
		}
		else if (elem.isAuto)
		{
			elem.all.rssIcon.src = "rssfeeds/rss_icon_2.gif";
		}
		else
		{
			elem.all.rssIcon.src = "rssfeeds/rss_icon.gif";
		}
	}
	
	if (elem.id == "channelNew" || elem.id == "channelAll" || elem.id == "channelDownloads")
	{
		elem.all.feedItem.className = "feed_item_nosel";
		elem.all.ctrls.className = "label_no_sel";		
		elem.all.rssIcon.src = "rssfeeds/tab_star.gif";		
	}
	
	elem.all.lblLeft.style.backgroundImage = "";
	elem.all.lblRight.style.backgroundImage = "";
					
	if (prevSel)
	{
		elem.all.lblRight.style.backgroundImage = "url(rssfeeds/lbl_sel_next.gif)";
		if (prev) prev.all.feedItem.style.borderBottom = "";
	}	
	else
	{
		if (prev && prev.channelID != g_CurrentChannel) prev.all.feedItem.style.borderBottom = "1px solid #353232";			
	}	
}

function deleteChannel(channel)
{
	try {
		external.RSSDeleteChannel(channel.channelID);
	} catch (e) { }
	
	goToChannelById(channel.prevChannelID);
	refreshChannels();
}

function refreshCurrentChannel()
{
	try {
		external.RSSRefreshChannel(g_CurrentChannel * 1);
	} catch (e) { }		
	clearItems();
//	txtRefresh.style.display = "block";
}

function updateChannelNewItems(channelID, numNew)
{
	if (!g_ChannelsInfo[channelID]) return;
	var prevNewItems = g_ChannelsInfo[channelID].numNew;
	if (numNew != prevNewItems)
	{
		var prevTotalNew = g_ChannelsInfo[0].numNew * 1; //New items
		var newTotalNew = prevTotalNew - prevNewItems + numNew;
		
		if (newTotalNew < 0)
			newTotalNew = 0; 
			
		var rows = channelTable.tBodies[0].rows;
		for (var i = 0; i < rows.length; ++i)
			if (rows[i].channelID == channelID)
			{
				rows[i].all.channelName.innerText = g_ChannelsInfo[channelID].name + "  (" + numNew + ")";	
				break;
			}
			
		var newItemsChannel = channelTable.tBodies[0].rows['channelNew'];
		newItemsChannel.all.channelName.innerText = "NEW ITEMS  (" + newTotalNew + ")";		
	}	
}

function onChannelRefresh(channelID, state, numNew)
{
	if (document.readyState != "complete")
		return; 
	channelID *= 1;
	state = state * 1;
	
	refreshChannelData();			
	
	if (state == 0)
	{
		if (g_ChannelsInfo[channelID]) 
		{		
			g_ChannelsInfo[channelID].isRefreshing = false;
			g_ChannelsInfo[channelID].flags |= FEED_FLAG_REFRESHED;
		}
					
		selectChannelById(g_CurrentChannel);
		
		if (g_CurrentChannel == channelID)		
		{
			txtRefresh.style.display = "none";
			changeChannel(g_Channel);
		}
			
		updateChannelNewItems(channelID, numNew * 1);
		setChannelInfo(true);
	}
	else
	{
		//start refresh of channel
		if (g_ChannelsInfo[channelID]) 	g_ChannelsInfo[channelID].isRefreshing = true;
		selectChannelById(g_CurrentChannel);
				
		if (g_CurrentChannel == channelID)
		{
			//g_Channel.all.rssIcon.src = "rssfeeds/refreshing_1.gif";		
			txtRefresh.style.display = "block";		
			clearItems();
		}		
	}
}

function setChannelInfo(fullRefresh)
{
	var channels = new Array; 
	try
	{
		external.RSSGetChannels(this, "", channels);
	} catch (e) { return; }
		
	var channelDescription = document.getElementById("channelDescription");
	var channelTitle = document.getElementById("channelTitle");
	var channelAutoDownload = document.getElementById("imgAutoDownload");
	var channelAutoDownloadText = document.getElementById("imgAutoDownloadText");
	var channelRefreshButton = document.getElementById("imgChannelRefresh");
	var channelInfoButton = document.getElementById("imgInfoButton");
	
	if (g_CurrentChannel == 0 || g_CurrentChannel == 1)
	{		
		channelAutoDownload.style.display = "none";
		channelAutoDownloadText.style.display = "none";
		channelRefreshButton.style.display = "none";
		channelInfoButton.style.display = "none";
		
		if (fullRefresh)
		{
			channelDescription.style.display = "none";
			if (g_CurrentChannel == 0)
				channelTitle.innerText = "New Episodes";
			else
				channelTitle.innerText = "All Episodes";
				
		}
	}	
	else
	{
		for (var i = 0; i < channels.length; ++i)
		{
			var channel_info = channels[i];
			if (channel_info[CHANNEL_ID] == g_CurrentChannel)
			{				
				channelAutoDownload.style.display = "block";
				channelAutoDownload.src = channel_info[CHANNEL_FLAGS] & 1 ? "rssfeeds/download_on.gif":"rssfeeds/download_off.gif";
				channelAutoDownloadText.style.display = "block";
				channelRefreshButton.style.display = "block";
				channelInfoButton.style.display = channel_info[CHANNEL_EPISODE] > 0 ? "block" : "none";
				
				if (fullRefresh)
				{
					channelDescription.style.display = "block";
					channelDescription.innerHTML = channel_info[CHANNEL_DESCRIPTION];
					channelTitle.innerText = decode_entities(channel_info[CHANNEL_NAME]);
					
				}
								
				break;
			}
		}
	}	
}

function findChannelRow(channelID)
{
	var rows = channelTable.tBodies[0].rows;
	for (var i = 0; i < rows.length; ++i)
	{
		if (rows[i].channelID == channelID)
		{
			return rows[i];
		}
	}
	
	return null;
}

function changeChannelById(channelID)
{
	//find or die trying...
	var pRow = findChannelRow(channelID);
	if (pRow)
	{
		changeChannel(pRow);
	}
	else
	{
		refreshChannels(); 
		pRow = findChannelRow(channelID);
		if (pRow == null)
			return; 
		changeChannel(pRow);
	}
}

function setChannelSelection(channel)
{
	selectChannelById(channel.channelID);
}

function findItem(guid)
{
	var rows = itemTable.rows;
	var aRow = null; 
	for (var j = 0; j < rows.length; ++j)
	{
		if (rows[j].itemGUID && rows[j].itemGUID == guid)
		{
			aRow = rows[j];
			break;
		}
	}
	return aRow; 
}


//aRow - the row, item - data array, newElem -> boolean (just created or exists)
function setItemDetails(i, aRow, item, newElem)
{
	aRow.currentUpdate = g_CurrentUpdate;
	
	if (aRow.crc32 && aRow.crc32 == item.crc32) return;
	
	var elems = aRow.all;		
							
	if (newElem)
	{
		aRow.style.display = "block";
		aRow.idType = "item";
		aRow.id = "i" + i;
		aRow.itemGUID = item[ITEM_GUID];
		aRow.crc32 = item.crc32;
		elems.itemName.innerText = parseTitle(item[ITEM_TITLE]);
		elems.itemSize.innerText = formatSize(item[ITEM_SIZE]);
		elems.itemType.innerText = item[ITEM_TYPE];
		elems.itemDate.innerText = formatDate(item[ITEM_DATE]);
		elems.itemDescription.innerHTML = item[ITEM_DESCRIPTION];
		
		if (g_CurrentChannel == 0 || g_CurrentChannel == 1 )
		{
			elems.itemChannelLink.style.display = "block";
			elems.itemChannelLink.innerText = g_ChannelsInfo[item[ITEM_CHANNELID]].name;				
			elems.itemChannelLink.channelID = item[ITEM_CHANNELID];
		}
		
		var elemItem = elems.itemImage;
		if (item[ITEM_IMAGE].length > 0)
		{
			var originalURL = item[ITEM_IMAGE];
			
			var img;
			if (!elemItem.loaded)	
				img = external.ExecCmd("GetItemImageURL", originalURL, 0);
			
			if (g_LoadImagesAsync)
			{
				elemItem.temp_src = img;
				g_ItemImageQueue.unshift(elemItem);
			}
			else
			{
				elemItem.src = img;
				elemItem.loaded = true;			
			}
		}
		else
		{
			var originalURL = g_ChannelsInfo[item[ITEM_CHANNELID]].img;	
			var img;
			
			if (!elemItem.loaded)
				img = external.ExecCmd("GetItemImageURL", originalURL, 0);
			
			if (g_LoadImagesAsync)
			{
				elemItem.temp_src = img;
				if (img.length > 0)
					g_ItemImageQueue.unshift(elemItem);							
			}
			else
			{
				elemItem.src = img;
				elemItem.loaded = true;
			}
		}						
	}
			
	if (item[ITEM_FLAGS] & 1 > 0)
	{
		elems.itemNewIcon.style.display = "block";
	}
	else
	{
		elems.itemNewIcon.style.display = "none";
	}
	
	elems.btDownload.itemGUID = item[ITEM_GUID]; 
	elems.btDownload.parentRow = aRow;
	elems.btDownload.channelID = item[ITEM_CHANNELID];
	elems.btDownload.videoID = item[ITEM_VIDEOID]*1;
	elems.btPlay.itemVideoID = item[ITEM_VIDEOID]*1;
	elems.btStream.itemGUID = item[ITEM_GUID];
	
	if (item[ITEM_TYPE] == "xml/rss")
	{
		var chanID = -1;
		for (var i = 0; i < g_ChannelList.length; ++i)
			if (g_ChannelList[i][CHANNEL_LINK] == unescape(item[ITEM_URL]))
				chanID = g_ChannelList[i][CHANNEL_ID];
				
		if (chanID > 0)
		{
			setItemButton(elems, "subscribed");
			elems.btGoToChannel.channelID = chanID;
		}
		else
		{
			setItemButton(elems, "subscribe");
			elems.btSubscribe.url = item[ITEM_URL];
			elems.btSubscribe.name = item[ITEM_TITLE];
			elems.btSubscribe.image = item[ITEM_IMAGE];
			elems.btSubscribe.itemLink = item[ITEM_LINK];
		}
	}
	else
	{
		if (item[ITEM_VIDEOID]*1 != 0)
		{			
			setVideoInfo(aRow, item[ITEM_VIDEOID]*1);
		}
		else
		{
			var stream = false;
			for (var k = 0; k < g_StreamableTypes.length; ++k)
				if (g_StreamableTypes[k] == item[ITEM_TYPE])
					stream = true;
			
			if (stream)
				setItemButton(elems, "stream");
			else
				setItemButton(elems, "download");
		}
	}
}

function setItemButton(elems, state)
{
	elems.btGoToChannel.style.display = state == "subscribed" ? "block" : "none";
	elems.btSubscribe.style.display = state == "subscribe" ? "block" : "none";
	elems.btPlay.style.display = state == "play" ? "block" : "none";
	elems.btDownload.style.display = state == "download" || state == "stream" ? "block" : "none";	
	elems.btStatus.style.display = state == "downloading" ? "block" : "none";		
	elems.btStream.style.display = state == "stream" ? "block" : "none";
}

function discardOldItems(itemTable)
{
	var delRows = new Array();
	var rows = itemTable.tBodies[0].rows;
	for (var i = 0; i < rows.length; ++i)
	{
		if (rows[i].currentUpdate < g_CurrentUpdate)
			delRows.push(rows[i]);
	}
	for (var i = 0; i < delRows.length; ++i)
	{
		itemTable.tBodies[0].removeChild(delRows[i]);
	}
}

function prevPage()
{
	if (page.selectedIndex > 0)
		page.selectedIndex = page.selectedIndex - 1;
	page.onchange.apply();
}

function nextPage()
{
	if (page.selectedIndex < page.options.length - 1)
		page.selectedIndex = page.selectedIndex + 1;
	page.onchange.apply();
}

function updatePageSelector(numItems)
{
	var sItem = 0;
	var pages = document.getElementById("page");	
	var optIdx = 1;
	
	for (var i = 0; i < pages.options.length; ++i)
		pages.options[i] = null;
	pages.options.length = 0;
		
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
	}
	else
	{
		page_selection.style.display = "block";
	}
	
	
	pages.selectedIndex = 0;
}

var g_ForcePageRefresh = false;

function updateItems(channel)
{
	var itemTable = document.getElementById("itemTable");
	var templateRow = itemTable.tBodies[0].rows["itemRow"];

	var items = new Array; 
	var offset = page.value;
	try {
		var filter = item_filter.value;
		
		var options = new Object; 
		options.filter = filter; 
		options.channelId = channel.channelID;
		options.offset = offset;
		options.maxItems = ITEMS_PER_PAGE;
		external.RSSGetItems(this, items, options);	
	} catch (e) { return; }
		
	infoNoItems.style.display = items.length == 0 ? "block" : "none";		
	
	g_ChannelsInfo[g_CurrentChannel].prevNumItems = items.length;
	
	g_CurrentUpdate++;

	if (g_ForcePageRefresh || items.TotalItems != g_ChannelsInfo[channel.channelID].TotalItems)
	{
		updatePageSelector(items.TotalItems);
		g_ChannelsInfo[channel.channelID].TotalItems = items.TotalItems;
		g_ForcePageRefresh = false;
	}
	
	
	for (var i = 0; i < items.length; ++i)
	{
		if (items[i].length == 0) continue;
		
		var item = items[i];
				
		var aRow = findItem(item[ITEM_GUID]);
		var newElem = false;

		if (aRow == null)
		{
			aRow = createElement(templateRow);
			newElem = true;
		}
		
		//Load data into the row		
		setItemDetails(i, aRow, item, newElem); 
		
		if (newElem)
			itemTable.tBodies[0].insertBefore(aRow, templateRow);			
	}		
	discardOldItems(itemTable);
}

var g_ChannelTimeout = 0; 
var g_ImagesTimeout = 0; 

function changeChannel(channel)
{	
	clearTimeout(g_ImagesTimeout); 
	clearTimeout(g_ChannelTimeout); 
	
	if (channel == null) return;
		
	var oldChannel = g_CurrentChannel; 
	
	if (channel.channelID != g_CurrentChannel)
    {
		clearItems();
		g_ForcePageRefresh = true;
    	document.getElementById("item_filter").value = "";
	}
	    	    	        
    var oldChannelID = g_CurrentChannel;
	
	g_CurrentChannel = channel.channelID;
	g_Channel = channel;

	if (g_ChannelsInfo[g_CurrentChannel].isRefreshing) 
		txtRefresh.style.display = "block";
	else
		txtRefresh.style.display = "none";
		
	if (channel.channelID == 0 || channel.channelID == 1)
	{
		toggleSettings();
		btnToggleSettings.style.display = "none";
	}
	else
	{
		btnToggleSettings.style.display = "block";				
	}
	
	infoNoItems.style.display  = g_ChannelsInfo[g_CurrentChannel].prevNumItems == 0 ? "block" : "none";
	
	setChannelSelection(channel);

	setChannelInfo(g_CurrentChannel != oldChannel);
	
	if (g_CurrentChannel != oldChannelID)
		refreshChannelSettings();

	if (g_ChannelsInfo[g_CurrentChannel].isRefreshing) 
		return;			
	if (g_CurrentChannel != 0 && g_CurrentChannel != 1 && !(g_ChannelsInfo[g_CurrentChannel].flags & FEED_FLAG_REFRESHED))
	{
		return;
	}
		
	g_ChannelTimeout = setTimeout("updateChannel()", 100);
}

function updateChannel()
{
	updateItems(g_Channel); 
	setCollectionSize();		
	if (g_LoadImagesAsync)
		g_ImagesTimeout = setTimeout("setItemImagesDelayed()", 250);
}

function setItemImagesDelayed()
{
	if (g_ItemImageQueue.length > 0)
	{
		var elem = g_ItemImageQueue.pop();
		elem.src = elem.temp_src;
		g_ImagesTimeout = setTimeout("setItemImagesDelayed()", 10);
	}
}

function addImage(id, url)
{
	g_Images[id] = new Image();
	g_Images[id].store_src = url;
}

function setVideoInfo(obj, videoID)
{
	obj.videoID = videoID;
	
	var elems = obj.all;
	var vi = new Array; 
	external.GetVideoInfo(videoID, this, vi);
	var vinfo = vi[0];
	
	if (vinfo[c_videoStatusStr] == "Finished" || vinfo[c_videoStatusStr].indexOf("Stream") != -1)
	{
		setItemButton(elems, "play");
	}
	else
	{
		setItemButton(elems, "downloading");
	}
}

function downloadItem(obj)
{
	try {
		external.RSSDownloadItem(obj.itemGUID);	
		setItemButton(obj.parentRow.all, "downloading");
		if (g_CurrentChannel == 0)
			goToChannelById(obj.channelID);
	} catch (e) {} 	
}

function viewItem(videoID)
{
	var sectionMyCollection = 1; 	
	try{external.MF_SetActiveSection(sectionMyCollection);}catch(e){}
}

function createElement(source_elem)
{
	if (g_ElementCache.length > 0) return g_ElementCache.pop();	
	return source_elem.cloneNode(true);
}

function cacheElement(elem)
{
	if (numElems >= 100) return;
	elem.loaded = false;
	g_ElementCache.push(elem);
}

function clearItems()
{
	g_ItemImageQueue.length = 0;
	var rows = itemTable.tBodies[0].rows;
	var delRows = new Array();
	var idx=0;
	for (var i = 0; i < rows.length; ++i)
	{
		if (rows[i].id != "itemRow") 
		{
			delRows[idx++] = rows[i];
		}
	}
	for (var i = 0; i < idx; ++i)
	{
		//cacheElement(delRows[i]);
		itemTable.tBodies[0].removeChild(delRows[i]);
	}
}


function itemsFilterChange()
{	
	g_ForcePageRefresh = true;
	changeChannel(g_Channel);
	
/*	var filterStr = document.getElementById("item_filter").value.toLowerCase();
	
	var items = document.getElementById("itemTable").tBodies[0].rows;	
	for (var i = 0; i < items.length; ++i)
	{
		var itemName = items[i].all.itemName.innerText;
		var itemDesc = items[i].all.itemDescription.innerText;
		
		if (itemName.toLowerCase().indexOf(filterStr) < 0 && itemDesc.toLowerCase().indexOf(filterStr) < 0)
			items[i].style.display = "none";
		else
			if (items[i].idType == "item")
				items[i].style.display = "block";
	}	*/
}

function onDownloadStarted(itemGUID, videoID)
{
	var rows = itemTable.tBodies[0].rows;
	for (var i = 0; i < rows.length; ++i)
	{
		if (rows[i].itemGUID == itemGUID)
		{
			videoID = videoID * 1;
			var elems = rows[i].all;
			elems.btDownload.itemGUID = itemGUID; 
			elems.btDownload.videoID = videoID;
			elems.btPlay.itemVideoID = videoID;
			if (videoID != 0)
			{			
				setVideoInfo(rows[i], videoID);
			}				
		}	
	}
}

var g_ScrollTimer = 0;
var g_ScrollSpeed = 0;

function onScrollEnter(dir)
{
	g_ScrollTimer = setInterval(dir == "up" ? "chanScrollUp()" : "chanScrollDown()", 10);
}

function onScrollMove(dir)
{
	if (dir == "up")
	{
		var pos = findPos(document.getElementById("channelScrollUp"));
		g_ScrollSpeed = (29 - (window.event.clientY - pos[1])) / 29;			
	}
	else
	{
		var pos = findPos(document.getElementById("channelScrollDown"));
		g_ScrollSpeed = (window.event.clientY - pos[1]) / 29;
	}
	g_ScrollSpeed = Math.pow(g_ScrollSpeed, 2) * 5;
	if (g_ScrollSpeed < 1) g_ScrollSpeed = 1;
}

function onScrollLeave()
{
	clearInterval(g_ScrollTimer);	
}

function chanScrollDown()
{
	var ny = channelsScroller.style.top.replace("px", "") * 1;
	ny -= g_ScrollSpeed;
	var mins = channelsContainer.offsetHeight - channelsScroller.offsetHeight - CHAN_SCROLL_MAX;
	if (mins > CHAN_SCROLL_MAX) mins = CHAN_SCROLL_MAX;
	if (ny < mins) ny = mins;
	channelsScroller.style.top = ny + "px";	
}

function chanScrollUp()
{
	var ny = channelsScroller.style.top.replace("px", "") * 1;
	ny += g_ScrollSpeed;
	if (ny > CHAN_SCROLL_MAX) ny = CHAN_SCROLL_MAX;
	channelsScroller.style.top = ny + "px";		
}

var g_ChannelScroll = false;

function onScrollWheel()
{
	if (g_ChannelScroll)
	{
		var move = window.event.wheelDelta / 100 * 20;	
		
		var ny = channelsScroller.style.top.replace("px", "") * 1;
		var mins = channelsContainer.offsetHeight - channelsScroller.offsetHeight - CHAN_SCROLL_MAX;
		if (mins > CHAN_SCROLL_MAX) mins = CHAN_SCROLL_MAX;

		ny += move;
		
		if (ny < mins) ny = mins;
		if (ny > CHAN_SCROLL_MAX) ny = CHAN_SCROLL_MAX;
		
		channelsScroller.style.top = ny + "px";		
		window.event.returnValue = false;
		window.event.cancelBubble = true;
		
		resizeScrollBarThumb();
	}
}

function setChannelScroll(value)
{
	g_ChannelScroll = value;
}

function toggleSettings()
{	
	if (g_CurrentChannel == 0 || g_CurrentChannel == 1 || settingsBox.style.display == "block")
	{
		settingsBox.style.display = "none";
		btnToggleSettings.src = "rssfeeds/toggle_options_0.gif";
	}
	else
	{
		settingsBox.style.display = "block";
		btnToggleSettings.src = "rssfeeds/toggle_options_1.gif";
		refreshChannelSettings();
	}
}

function refreshChannelSettings()
{
	max_downloads_per_channel.value = g_ChannelsInfo[g_CurrentChannel].maxDownloads;	
	set_channel_url.value = g_ChannelsInfo[g_CurrentChannel].url;
	set_channel_name.value = g_ChannelsInfo[g_CurrentChannel].name; 
	filterContains.value = g_ChannelsInfo[g_CurrentChannel].filterContains; 
	filterNotContains.value = g_ChannelsInfo[g_CurrentChannel].filterNotContains; 
}

function saveChannelSettings()
{
	try{
		var maxDownloads = parseInt(max_downloads_per_channel.value);
		if (!isNaN(maxDownloads))
		{
			var pSettings = new Object; 
			pSettings.maxDownloads = maxDownloads;
			pSettings.channelName = set_channel_name.value; 
			var bMustRefresh = false; 
			if (g_ChannelsInfo[g_CurrentChannel].filterContains != filterContains.value ||
			    g_ChannelsInfo[g_CurrentChannel].filterNotContains != filterNotContains.value)
			{
				bMustRefresh = true; 
			}
				
			pSettings.filterContains = filterContains.value; 
			pSettings.filterNotContains = filterNotContains.value; 
			external.ExecCmd("RSSSaveChannelSettings", g_CurrentChannel, pSettings);
			g_ChannelsInfo[g_CurrentChannel].maxDownloads = pSettings.maxDownloads;
			if (pSettings.channelName.length > 0 && pSettings.channelName != g_ChannelsInfo[g_CurrentChannel].name)
			{
				g_ChannelsInfo[g_CurrentChannel].name = pSettings.channelName; 
				setChannelInfo(true);
				refreshChannels(g_CurrentChannel);
				btnSaveChanges.style.backgroundImage = "url(rssfeeds/ok_1.gif)";
				btnSaveChanges.blur();
			}
			if (bMustRefresh)
				changeChannel(g_Channel);
		}
///		toggleSettings();
	}catch(e){}
}

function settingsChange()
{
	btnSaveChanges.style.backgroundImage = "url(rssfeeds/ok.gif)";	
}

function beginResize()
{
	g_Resizing = true;		
	collection_div.style.cursor = "w-resize";
	channels_div.style.cursor = "w-resize";
}

function globalMouseUp()
{
	if (g_Resizing)
	{
		g_Resizing = false;	
		collection_div.style.cursor = "";
		channels_div.style.cursor = "";
	}
	
	if (g_Scrolling)
	{
		g_Scrolling = false;
	}
		bodyDiv.releaseCapture();
}

function doSplitterResize(width)
{
	var sz = document.documentElement.clientWidth;

	var splitSz = width ? width : window.event.clientX + scroller_div.offsetWidth;
	if (splitSz < MIN_SPLITTER_WIDTH) splitSz = MIN_SPLITTER_WIDTH;
	if (sz - splitSz < MAX_SPLITTER_WIDTH) splitSz = sz - MAX_SPLITTER_WIDTH;	
	channels_div.style.width = splitSz + "px";
   	leftHeader.style.width = channels_div.offsetWidth + splitter_div.offsetWidth / 2 + scroller_div.offsetWidth-5;
	collection_div.style.width = (sz - splitSz - splitter_div.offsetWidth) + "px";	

	var rows = channelTable.tBodies[0].rows;
	for (var i = 0; i < rows.length; ++i)
	{		
		if (rows[i].all.channelName)
			rows[i].all.channelName.style.width = (splitSz - 80) + "px";
	}
}

function globalMouseMove(width)
{
	if (g_Resizing)
	{
		if (!width && !g_Resizing) return;	

		doSplitterResize(width);		
	}
	
	if (g_Scrolling)
	{
		var scrollBarMaxY = scrollBarTotal.offsetHeight;
		var scrollBarPosY = findPos(scrollBarTotal)[1];
		var thumbHeight = scrollBarThumb.offsetHeight;
		
		var mouseDelta = window.event.clientY - g_ScrollStart;
		var scroll = mouseDelta + g_ScrollThumbStart;
		if (scroll > scrollBarMaxY - thumbHeight) scroll = scrollBarMaxY - thumbHeight;
		if (scroll < 0) scroll = 0;
		
		scrollBarThumb.style.top = (scroll) + "px";		
		
		var ny = Math.floor(-scroll / scrollBarMaxY * channelsScroller.offsetHeight);		
		var mins = channelsContainer.offsetHeight - channelsScroller.offsetHeight - CHAN_SCROLL_MAX;
		if (mins > CHAN_SCROLL_MAX) mins = CHAN_SCROLL_MAX;
		
		if (ny < mins) ny = mins;		
		if (ny > CHAN_SCROLL_MAX) ny = CHAN_SCROLL_MAX;
		
		channelsScroller.style.top = ny + "px";
	}
}

function resizeScrollBarThumb()
{
	var totalHeight = channelsScroller.offsetHeight;
	var clipHeight = channelsContainer.offsetHeight;
	var scrollHeight = scrollBarTotal.offsetHeight;
	
	if (clipHeight >= totalHeight)
	{
		scroller_div.style.display = "none";
	}
	else
	{
		scroller_div.style.display = "block";
		
		var thumbHeight = Math.floor(clipHeight / totalHeight * scrollHeight);
		scrollBarThumb.style.height = thumbHeight + "px";
		
		var ny = channelsScroller.style.top.replace("px", "") * 1 - 2*CHAN_SCROLL_MAX;
		var pos = -ny / (totalHeight - clipHeight) * scrollHeight;
		if (pos < 0) pos = 0;
		if (pos + thumbHeight > scrollHeight) pos = scrollHeight - thumbHeight;
		scrollBarThumb.style.top = pos + "px";
	}
}


function getSplitterPosition()
{
	return channels_div.style.width.replace("px", "") * 1;
}

function make_words(str)
{
	var words = str.split(" ");
	var newwords = new Array();
	
	for (var i = 0; i < words.length; ++i)
	{
		while (words[i].length > 15)
		{
			newwords.push(words[i].substring(0, 15));
			words[i] = words[i].substr(15);
		}
		newwords.push(words[i]);		
	}	
	
	return newwords.join(" ");	
}

function addFeedFromItem(btn)
{
	var obj = new Object();
	obj.URL = unescape(btn.url);
	obj.Name = btn.name;
	obj.defImage = btn.image;
	obj.DetailsURL = btn.itemLink;
	
	var chanID = external.RSSAddFeed(obj);
	refreshChannels();
	changeChannelById(chanID);
}

function streamItem(guid)
{
	try {
		external.ExecCmd("RSSStreamItem", guid, 0);		
	} catch (e) {} 		
}

function viewChannelInfo()
{
	//alert("Change line 828 for the button to hide/show correctly.\nThe episode ID should be in a new index CHANNEL_EPISODE in the channel info array.\nID Call external.func(g_ChannelsInfo[g_CurrentChannel].id_episode)");
	try{
		external.ExecCmd("RSSChannelDetails", g_CurrentChannel, 0); 
	} catch(e) {}
}

function beginScroll()
{
	g_Scrolling = true;				
	g_ScrollStart = window.event.clientY;
	g_ScrollThumbStart = scrollBarThumb.style.top.replace("px", "") * 1;
	window.event.cancelBubble = true;
	bodyDiv.setCapture();
}

function setScroll()
{
	
}

function globalMouseLeave()
{
	g_Scrolling = false;	
}


document.onmousewheel= onScrollWheel;
//document.onselectstart=new Function ("if (g_Resizing) return false")
