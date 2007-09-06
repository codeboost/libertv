var g_DeveloperMode = 0; 
var g_Months = null;

function _DBGAlert(str)
{
	try{external.OutputDebugString(str);}catch(e){}
}

function setDeveloperMode(bSet)
{
	//g_DeveloperMode = (bSet == "1") ? true : false;
}

function formatNumber(num, decimalNum)
{
	return num.toFixed(decimalNum); 
}
  
function formatSize(val)
{
	if (val > 1073741824)
	{
		val /= 1073741824;
		return val.toFixed(2) + "GB";
	}
	else
	if (val > 1048576)
	{
		val /= 1048576;
		return val.toFixed(2) + "MB";
	}
	else
	if (val > 1024)
	{
		val /= 1024;
		return val.toFixed(2) + "KB";
	}
	return val.toFixed(2) + "B";
}

function formatSpeed(val)
{

	val = val * 1;
	if (val > 1048576)
	{
		val /= 1048576;
		return val.toFixed(2) + "MB/s";
	}
	else
	if (val > 1024)
	{
		val /= 1024;
		return val.toFixed(2) + "KB/s";
	}
	return val.toFixed(2) + "B/s";
}

function formatTime(val)
{
	val = val * 1;
	var hours = Math.floor((val % (3600 * 24)) / 3600) + "";
	var minutes = Math.floor((val % 3600) / 60) + "";
	var seconds = Math.floor(val % 60) + "";
	var days = Math.floor(val / (3600 * 24));
			
	if (hours.length == 1) hours = '0' + hours;
	if (minutes.length == 1) minutes = '0' + minutes;
	if (seconds.length == 1) seconds = '0' + seconds;
	
	if (days > 0)
		return days + "d " + hours + ":" + minutes + ":" + seconds;
	else
		return hours + ":" + minutes + ":" + seconds;		
}	

function getWindowHeight()
{
	if (self.innerWidth)
	{
		return self.innerHeight;
	}
	else if (document.documentElement && document.documentElement.clientHeight)
	{
		return document.documentElement.clientHeight;
	}
	else if (document.body)
	{
		return document.body.clientHeight;
	}	
	return 0;
}

function formatDateTime(dateval)
{
	var d = new Date(dateval * 1000);  
	return formatDate(dateval) + " " + d.toLocaleTimeString(); 	
}

function formatDate(dateval)
{
	if (g_Months == null)
   		g_Months = new Array("January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December");

	var d = new Date(dateval * 1000);  
	return g_Months[d.getMonth()] + " " + d.getDate() + " " + d.getFullYear();                           
}

String.prototype.trim = function() { return this.replace(/^\s+|\s+$/g, ""); };

function parseTitle(itemTitle)
{
	var re = /\.*(\d+)x(\d+)/;
	var re1 = /\.*[s|S](\d+)[e|E](\d+)/;
	var res = re.exec(itemTitle);

	if (res == null || res.length != 3 || res.index == null)
	  	res = re1.exec(itemTitle); 
	
	if (res != null && res.length == 3 && res.index)
	{
		itemTitle = itemTitle.substring(0, res.index).trim().replace(/\./g, " ") + " - Season " + res[1] + " Episode " + res[2];
	}	

	return itemTitle; 
}
