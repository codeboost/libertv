#include "stdafx.h"
#include "FMPlayer_QT.h"



void CToPstr(char *theString)
{
	char	tempString[256];

	tempString[0] = strlen (theString);
	tempString[1] = '\0';

	strcat ( tempString, theString );
	strcpy	( theString, tempString );
}


LRESULT FQTWnd::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
//	MessageBox("The Owl", "He", MB_OK | MB_ICONQUESTION); 
	return 0; 
}

LRESULT FQTWnd::OnEraseBackground( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	bHandled = TRUE; 
	return 1;
}

LRESULT FQTWnd::OnPaint( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	WTL::CBrush aBrush; 
	aBrush.CreateSolidBrush(0); 
	WTL::CRect r; 
	GetClientRect(&r);
	CPaintDC dc(m_hWnd); 
	dc.FillRect(&r, aBrush); 
	bHandled = TRUE; 
	return 1;
}


void FMediaPlayerQT::CloseMovie()
{
}

HRESULT FMediaPlayerQT::Init(HWND hWndParent, IMediaPlayerNotify* pNotify)
{
	HRESULT hr = E_FAIL;  
	m_pNotify = pNotify; 

	WTL::CRect r; 
	::GetWindowRect(hWndParent, &r);
	m_wndView.Create(hWndParent, r, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0);


	hr = m_wndView.QueryHost(&spHost);
	if (SUCCEEDED(hr))
	{

		LPOLESTR                wszQTControlCLSID = NULL;
		StringFromCLSID(__uuidof(QTControl), &wszQTControlCLSID);
		hr = spHost->CreateControl(wszQTControlCLSID, m_wndView, 0);
		if (SUCCEEDED(hr))
		{
			hr = m_wndView.QueryControl(&m_QTControl);
		}
		if (SUCCEEDED(hr))
		{
			//m_QTControl->QuickTimeInitialize()
		}
	}

	return hr; 
}

HRESULT FMediaPlayerQT::LoadMedia(const tchar* pFileName, MEDIA_TIME rtOffset /* = 0 */, MEDIA_TIME rtMaxDuration /* = 0 */)
{
	USES_CONVERSION;
	HRESULT hr = E_FAIL; 

	if (m_QTControl)
	{
		FString StrQTFileName = "file://";
		StrQTFileName.Append(pFileName);
		StrQTFileName.Replace("\\", "/"); 
		hr = m_QTControl->put_URL(T2OLE(pFileName)); 
		m_QTControl->put_BackColor(0); 
		m_QTControl->put_Sizing(qtMovieFitsControlMaintainAspectRatio);
		hr = m_QTControl->get_Movie(&m_QTMovie);
	}

	return hr; 
}


HRESULT FMediaPlayerQT::Play()
{
	HRESULT hr = E_FAIL; 

	if (m_QTMovie)
	{
		hr = m_QTMovie->Play(_variant_t(1.0f));
	}
	return hr; 
}

HRESULT FMediaPlayerQT::Pause()
{
	HRESULT hr = E_FAIL; 
	return hr; 
}

HRESULT FMediaPlayerQT::Stop()
{
	HRESULT hr = E_FAIL; 
	return hr; 
}

HRESULT FMediaPlayerQT::SeekPosition(MEDIA_TIME rtCurrent)
{
	HRESULT hr = E_FAIL; 
	return hr; 
}

MEDIA_TIME FMediaPlayerQT::GetPosition()
{
	MEDIA_TIME rtTime = 0; 
	return rtTime; 
}

MEDIA_TIME FMediaPlayerQT::GetDuration()
{
	MEDIA_TIME rtDur = 0; 
	return rtDur; 
}

MEDIA_TIME FMediaPlayerQT::GetAvailDuration()
{
	return GetDuration(); //TODO: Fix it somehow ?
}

HRESULT FMediaPlayerQT::SetVideoRect(const RECT &r)
{
	HRESULT hr = E_FAIL; 

	if (m_QTMovie)
	{
		QTRECT qtRect; 
		qtRect.left = r.left; 
		qtRect.top = r.top; 
		qtRect.right = r.right; 
		qtRect.bottom = r.bottom; 
		hr = m_QTMovie->put_Rectangle(&qtRect);

		QTPOINT val; 
		val.x = r.right; 
		val.y = r.bottom; 
		m_QTMovie->PutDimensions(&val);
	}
	return hr; 
}

HRESULT FMediaPlayerQT::ShowVideoWnd(int nShow)
{
	HRESULT hr = E_FAIL; 
	return hr;
}

long FMediaPlayerQT::GetVolume()
{
	long lVolume = 0; 
	return lVolume; 
}

HRESULT FMediaPlayerQT::SetVolume(long lVol)
{
	//lVol = min(lVol, 0);	

	//double dblVol = pow((double)10, (double)lVol / 5000 + 2);
	//dblVol = max(min(dblVol, 100), 0);
	//LONG lVolPercent = 0; 
	//lVolPercent = ((lVol / 10000) + 1) * 100;
//		if (SUCCEEDED(hr))
//			hr = pAudio->put_volume((long)dblVol); 
	return E_NOTIMPL; 
}

HRESULT FMediaPlayerQT::SetFullScreen(BOOL bFullScreen)
{
	HRESULT hr = E_FAIL; 
	return hr; 
}

BOOL FMediaPlayerQT::IsFullScreen()
{
	BOOL bFullScreen = FALSE; 
	return bFullScreen; 
}

HRESULT FMediaPlayerQT::GetBufferingProgress(long* lProgress)
{
	return E_NOTIMPL;
}

double FMediaPlayerQT::GetFrameRate()
{
	double dblRate = 0.0; 
	return dblRate; 
}

void FMediaPlayerQT::SetAspectRatio(int cx, int cy)
{

}



VideoState FMediaPlayerQT::GetVideoState()
{
	VideoState State = vssUndefined; 

	return State; 
}

void FMediaPlayerQT::Clear()
{
}

long FMediaPlayerQT::GetMediaEvent()
{
	return 1; 
}