#include "stdafx.h"
#include "IControlBar.h"
#include "GlobalObjects.h"

extern DVD_HMSF_TIMECODE RT2HMSF(REFERENCE_TIME rt, double fps = 0);
extern REFERENCE_TIME HMSF2RT(DVD_HMSF_TIMECODE hmsf, double fps = 0);

inline DVD_HMSF_TIMECODE RT2DVDT(REFERENCE_TIME rtTime)
{
	DVD_HMSF_TIMECODE a;
	rtTime = rtTime / 10000000;	//seconds
	a.bHours = (uint)(rtTime / 3600);    
	rtTime%=3600;
	a.bMinutes= (uint)(rtTime / 60);      
	rtTime%=60;
	a.bSeconds = (uint)rtTime;
	return a; 
}

void IControlBar::SaveSettings()
{
	g_AppSettings.m_Volume = GetVolume(); 
	g_AppSettings.SaveSettings(); 
}

void IControlBar::LoadSettings()
{
	SetVolume(g_AppSettings.m_Volume); 
}

void IFStatusBar::FormatStatusTimer(FString& posstr, FString& durstr, REFERENCE_TIME rtNow, REFERENCE_TIME rtDur, int nDisplayMode)
{
	if(nDisplayMode == 0)
	{
		DVD_HMSF_TIMECODE tcNow = RT2DVDT(rtNow);
		DVD_HMSF_TIMECODE tcDur = RT2DVDT(rtDur);

		if(tcDur.bHours > 0 || (rtNow >= rtDur && tcNow.bHours > 0)) 
			posstr.Format(_T("%02d:%02d:%02d"), tcNow.bHours, tcNow.bMinutes, tcNow.bSeconds);
		else 
			posstr.Format(_T("%02d:%02d"), tcNow.bMinutes, tcNow.bSeconds);

		if(tcDur.bHours > 0)
			durstr.Format(_T("%02d:%02d:%02d"), tcDur.bHours, tcDur.bMinutes, tcDur.bSeconds);
		else
			durstr.Format(_T("%02d:%02d"), tcDur.bMinutes, tcDur.bSeconds);
	}
	else 
	{
		posstr.Format(_T("%I64d"), rtNow);
		durstr.Format(_T("%I64d"), rtDur);
	}
}

void IFStatusBar::Status_SetTimer(REFERENCE_TIME rtNow, REFERENCE_TIME rtDur, bool fHighPrecision, int nDisplayMode)
{
	FString str;
	FString durstr, posstr; 

	FormatStatusTimer(posstr, durstr, rtNow, rtDur, nDisplayMode); 
	if (nDisplayMode == 0)
	{
		if(fHighPrecision)
		{
			str.Format(_T("%s.%03d"), posstr, (rtNow/10000)%1000);
			posstr = str;
			str.Format(_T("%s.%03d"), durstr, (rtDur/10000)%1000);
			durstr = str;
			str.Empty();
		}
	}
	str = (/*start <= 0 &&*/ rtDur <= 0) ? posstr : posstr + _T(" / ") + durstr;

	Status_SetTimer(str);
}

void IFStatusBar::Status_FormatMessage(int idi, const char* Fmt, ...)
{
	va_list paramList;
	va_start(paramList, Fmt);
	FString aString; 
	aString.FormatV(Fmt, paramList);
	Status_SetMessage(idi, aString); 
	va_end(paramList);
}

