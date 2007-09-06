#include "stdafx.h"
#include "FIEControlBar.h"

const tchar* _i64Fmt = "%I64d";


LRESULT FIEControlBar::OnDestroy(UINT, WPARAM, LPARAM, BOOL&)
{
	SaveSettings(); 
	return 0; 
}

void FIEControlBar::OnDocumentComplete(DWORD dwID, BOOL bMainFrame)
{
	ShowWindow(SW_SHOW); 
	FIEWindow::OnDocumentComplete(dwID, bMainFrame); 
	Init(); 
}

void FIEControlBar::Init()
{
	LoadSettings();
}

int FIEControlBar::GetVolume()
{
	CComVariant lvRes; 
	m_pBrowser.CallJScript("GetVolume", &lvRes); 
	if (lvRes.vt == VT_R8)
		return (int)lvRes.dblVal;
	else if (lvRes.vt == VT_I4)
		return lvRes.lVal; 
	else
		return 0; 
}

void FIEControlBar::SetVolume(int Volume)
{
	FString SVol; 
	SVol.Format("%d", Volume); 	
	m_pBrowser.CallJScript("SetVolume", SVol);
}

void FIEControlBar::IncreaseVolume()
{
	m_pBrowser.CallJScript("IncreaseVolume"); 
}

void FIEControlBar::DecreaseVolume()
{
	m_pBrowser.CallJScript("DecreaseVolume"); 
}

BOOL FIEControlBar::IsMuted()
{
	CComVariant lvRes; 
	m_pBrowser.CallJScript("IsMuted", &lvRes); 
	
	return (lvRes.boolVal == VARIANT_TRUE)?TRUE:FALSE; 
}

REFERENCE_TIME FIEControlBar::SeekBar_GetPos()
{
	CComVariant lvRes; 
	m_pBrowser.CallJScript("SeekBar_GetPos", &lvRes); 
	return 0;
}

void FIEControlBar::SeekBar_SetPos(__int64 nPos)
{
	FString StrPos; 
	StrPos.Format(_i64Fmt, nPos);
	m_pBrowser.CallJScript("SeekBar_SetPos", StrPos); 
}

void FIEControlBar::SeekBar_Enable(bool bEnable)
{
	m_pBrowser.CallJScript("SeekBar_Enable", bEnable?"1":"0"); 
}


void FIEControlBar::SeekBar_SetRange(__int64 Min, __int64 Max)
{

	FString aRangeMin; 
	aRangeMin.Format(_i64Fmt, Min); 
	FString aRangeMax; 
	aRangeMax.Format(_i64Fmt, Max); 
	m_pBrowser.CallJScript("SeekBar_SetRange", aRangeMin, aRangeMax); 
}

void FIEControlBar::SeekBar_SetAvail(__int64 nAvail)
{
	FString aAvail; 
	aAvail.Format(_i64Fmt, nAvail); 
	m_pBrowser.CallJScript("SeekBar_SetAvail", aAvail); 
}


//Statusbar
void FIEControlBar::Status_SetTimer(const tchar* pStrTimer)
{
	m_pBrowser.CallJScript("Status_SetTimer", pStrTimer); 
}
void FIEControlBar::Status_SetMessage(int idi, const tchar* pStrMsg)
{
	FString AStr; 
	AStr.Format("%d", idi); 
	m_pBrowser.CallJScript("Status_SetMessage", AStr, pStrMsg); 
}

void FIEControlBar::SetPlayState(BOOL bPlaying)
{
	m_pBrowser.CallJScript("SetPlayState", bPlaying?"1":"0"); 
}

void FIEControlBar::OnStop()
{
    m_pBrowser.CallJScript("onStop", ""); 
}

void FIEControlBar::ShowBuffering(BOOL bShow)
{
	m_pBrowser.CallJScript("showBuffering", bShow ? "1" : "0");
}

void FIEControlBar::ShowNextPrev(BOOL bShow)
{
	m_pBrowser.CallJScript("showNextPrev", bShow ? "1" : "0");
}
