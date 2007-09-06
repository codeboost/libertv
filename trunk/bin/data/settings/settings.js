
function OnCancel()
{
	try
	{
		external.Conf_OnCancel(0);
	}
	catch(e){}
}

function OnSave()
{
	try
	{
		external.Conf_OnSave();
	}
	catch(e){}
}

function OnLoad(lSection)
{
	try
	{
		external.Conf_OnLoad(lSection);
	}
	catch(e){}
}

function OnUnload(lSection)
{
	try
	{
		external.Conf_OnUnload(lSection); 
	}
	catch(e){}
}

function errorElement(elem)
{
	elem.style.background = "#FF5500";	
}

function okElement(elem)
{
	elem.style.background = "#FFFFFF";		
}

function validateSpeed(val)
{
	if (val == "") return true;
	
	if (isNaN(val * 1)) return false;
	
	val = val * 1;
	if (val < 0) return false;
	if (val > 102400) return false;
	return true;
}

function validateNumber(val, minim, maxim)
{
	if (val == "") return true;
	if (isNaN(val * 1)) return false;
	val = val * 1;
	if (val < minim) return false;
	if (val > maxim) return false;
	return true;
}

function blurMe(elem, def)
{
	if (elem.value == "")
		elem.value = def;	
}

function check_uncheck(ctrl)
{
	var pctrl = document.getElementById(ctrl);
	if (pctrl != null)
	{
		pctrl.checked=!pctrl.checked;
	}
	event.returnValue=false;
}



//document.onselectstart=new Function ("return false")
document.ondragstart=new Function("return false");

