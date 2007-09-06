#include "stdafx.h"
#include "FMPlayerDShow.h"
#include <InitGuid.h>
#include <dxerr8.h>
#include "IDirectVobSub.h"
#include "AppSettings.h"
#include "vfw.h"
#include "QEdit.h"
#include <math.h>

#ifndef max
#define max(a, b)  (((a) > (b)) ? (a) : (b)) 
#endif

#ifndef min
#define min(a, b)  (((a) < (b)) ? (a) : (b)) 
#endif



//GUID guid = "{64697678-0000-0010-8000-00AA00389B71}"
//DSH	DirectShow CLSID	4cc: xvid     {64697678-0000-0010-8000-00AA00389B71}


#define INITGUID
DEFINE_GUID(IID_IXvidDecoder,
			0x64697678, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71);

//AC3: DSH	DirectShow CLSID	{A753A1EC-973E-4718-AF8E-A3F554D45C44}


DEFINE_GUID(IID_IAC3Decoder, 
			0xA753A1EC, 0x973E, 0x4718, 0xAF, 0x8E, 0xA3, 0xF5, 0x54, 0xD4, 0x5C, 0x44);


//DirectVobSub:
//DSH	DirectShow CLSID	{93A22E7A-5091-45EF-BA61-6DA26156A5D0}

DEFINE_GUID(IID_IFDirectVobSub, 
			0x93A22E7A, 0x5091, 0x45ef, 0xba, 0x61, 0x6d, 0xa2, 0x61, 0x56, 0xa5, 0xd0);


//CoreAVC {09571A4B-F1FE-4C60-9760-DE6D310C7C31}
DEFINE_GUID(IID_ICoreAVC, 
			0x09571A4B, 0xF1FE, 0x4C60, 0x97, 0x60, 0xde, 0x6d, 0x31, 0x0c, 0x7c, 0x31);



//Divx4
//{78766964-0000-0010-8000-00AA00389B71}
DEFINE_GUID(IID_IDivX4, 
			0x78766964, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);



//Microsoft Audio (msadds32.ax)
//{22E24591-49D0-11D2-BB50-006008320064}
DEFINE_GUID(IID_IMSAudio, 
			0x22e24591, 0x49d0, 0x11d2, 0xbb, 0x50, 0x00, 0x60, 0x08, 0x32, 0x00, 0x64);

//Default Video Renderer
//CLSID_VideoRendererDefault {6BC1CFFA-8FC1-4261-AC22-CFB4CC38DB50}


#define VIDEO_TYPE_AVI 0
#define VIDEO_TYPE_MPG 1

BOOL g_bLoadL3Codec = TRUE; 
BOOL g_bLoadAC3Dec = TRUE; 

CAtlString GetErrorDescription(HRESULT hr)
{

	CAtlString aStr;
	aStr.Format("HRESULT=0x%x; ErrorString=%s; Description=%s", hr, DXGetErrorString8(hr), DXGetErrorDescription8(hr));
	return aStr; 
}

//extern void _DBGAlertM (const tchar* Mask, ...);
#define _DBGAlertM _DBGAlert




////// Helper function to create the filter instance without 
/////registering the filter 

HRESULT DSHelpCreateInstance( 
							 BSTR          bstrLibName,		 
							 REFCLSID              rclsid,	//XVID guid
							 LPUNKNOWN             pUnkOuter, //NULL
							 REFIID                riid,	 //IID_IBaseFilter
							 LPVOID*               ppv)		//IFilter
{ 

	// Load the library (bstrlibname should have the fullpath) 
	HINSTANCE hDLL = CoLoadLibrary(bstrLibName, TRUE); 

	if (hDLL == NULL) 
		return E_FAIL; 

	// Get the function pointer 
	typedef HRESULT (WINAPI* PFNDllGetClassObject)( 
		REFCLSID  rclsid, 
		REFIID    riid, 
		LPVOID*   ppv); 

	PFNDllGetClassObject pfnDllGetClassObject = 
		(PFNDllGetClassObject)GetProcAddress( 
		hDLL, "DllGetClassObject"); 
	if (!pfnDllGetClassObject) 
		return E_FAIL; 

	// Get the class faftory 
	CComPtr<IClassFactory> pFactory; 
	HRESULT hr = pfnDllGetClassObject(rclsid, IID_IClassFactory, 
		(LPVOID*)&pFactory); 
	if (hr != S_OK) 
		return hr; 

	// Create object instance   
	return pFactory->CreateInstance(pUnkOuter, riid, ppv);
}


HRESULT DisconnectPins(IBaseFilter *pFilter)
{
	if (NULL == pFilter)
		return E_POINTER; 

	IEnumPins *pEnum = 0;
	IPin *pPin = 0;
	HRESULT hr = pFilter->EnumPins(&pEnum);
	if (FAILED(hr))
	{
		return hr;
	}

	while (pEnum->Next(1, &pPin, NULL) == S_OK)
	{
		pPin->Disconnect();
		pPin->Release();
	}
	pEnum->Release();

	// Did not find a matching pin.
	return S_OK;
}

HRESULT GetPin( IBaseFilter * pFilter, PIN_DIRECTION dirrequired, int iNum, IPin **ppPin)
{
	CComPtr< IEnumPins > pEnum;
	*ppPin = NULL;

	if (!pFilter)
		return E_POINTER;

	HRESULT hr = pFilter->EnumPins(&pEnum);
	if(FAILED(hr)) 
		return hr;

	ULONG ulFound;
	IPin *pPin;
	hr = E_FAIL;

	while(S_OK == pEnum->Next(1, &pPin, &ulFound))
	{
		PIN_DIRECTION pindir = (PIN_DIRECTION)3;

		pPin->QueryDirection(&pindir);
		if(pindir == dirrequired)
		{
			if(iNum == 0)
			{
				*ppPin = pPin;  // Return the pin's interface
				hr = S_OK;      // Found requested pin, so clear error
				break;
			}
			iNum--;
		} 

		pPin->Release();
	} 

	return hr;
}

//
// NOTE: The GetInPin and GetOutPin methods DO NOT increment the reference count
// of the returned pin.  Use CComPtr interface pointers in your code to prevent
// memory leaks caused by reference counting problems.  The SDK samples that use
// these methods all use CComPtr<IPin> interface pointers.
// 
//     For example:  CComPtr<IPin> pPin = GetInPin(pFilter,0);
//
IPin * GetInPin( IBaseFilter * pFilter, int nPin )
{
	CComPtr<IPin> pComPin;
	GetPin(pFilter, PINDIR_INPUT, nPin, &pComPin);
	return pComPin;
}


IPin * GetOutPin( IBaseFilter * pFilter, int nPin )
{
	CComPtr<IPin> pComPin;
	GetPin(pFilter, PINDIR_OUTPUT, nPin, &pComPin);
	return pComPin;
}

HRESULT __fastcall RenderOutputPins(IGraphBuilder * pGB, IBaseFilter * pFilter)
{
	HRESULT     hr;
	IEnumPins      *pEnumPin = NULL;
	IPin           *pConnectedPin = NULL, *pPin = NULL;
	PIN_DIRECTION PinDirection;
	ULONG     ulFetched;

	// Enumerate all pins on the filter
	hr=pFilter->EnumPins(&pEnumPin);

	if( SUCCEEDED(hr) )
	{
		// Step through every pin, looking for the output pins
		while( (hr = pEnumPin->Next(1L, &pPin, &ulFetched))==S_OK )
		{
			// Is this pin connected?  We're not interested in connected pins.
			hr=pPin->ConnectedTo(&pConnectedPin);
			if( pConnectedPin )
			{
				pConnectedPin->Release();
				pConnectedPin=NULL;
			}

			// If this pin is not connected, render it.
			if( hr==VFW_E_NOT_CONNECTED )
			{
				hr = pPin->QueryDirection(&PinDirection);
				if( hr==S_OK && PinDirection==PINDIR_OUTPUT )
				{
					hr = pGB->Render(pPin);
				}
			}
			pPin->Release();

			// If there was an error, stop enumerating
			if( FAILED(hr) )
				break;
		}
	}

	// Release pin enumerator
	pEnumPin->Release();
	return hr;
}



void _DBGLogResult(HRESULT hr, const tchar* szFilterName)
{
	if (FAILED(hr))
		_DBGAlertM("FMPlayerDShow: **%s not loaded: %s\n", szFilterName, GetErrorDescription(hr));
	else
		_DBGAlertM("FMPlayerDShow: %s loaded\n", szFilterName); 
}

FMPlayerDShow::FMPlayerDShow()
{
	m_hWndVideo = NULL; 	
}

void FMPlayerDShow::ReleaseFilters()
{
	if (m_MediaControl != NULL)
		m_MediaControl->Stop(); 
}

void FMPlayerDShow::Clear()
{
	if (m_MediaControl != NULL)
		m_MediaControl->Stop(); 

	m_MediaControl.Release();
	m_MediaEvent.Release();
	m_FilterGraph.Release();

	DisconnectPins(m_xVid); 
	DisconnectPins(m_AviSplitter); 

	m_xVid.Release();
	m_FileSource.Release();
	m_AviSplitter.Release(); 

	m_MSAud.Release();
	m_AC3Filter.Release(); 
	m_DSoundFilter.Release();


	m_GraphBuilt = FALSE;
	m_VideoType = 0; 
	m_FourCC = ""; 
	m_StreamFormat = "";

	m_VideoRenderer.Release(); 

}

FMPlayerDShow::~FMPlayerDShow()
{
	Clear();
}



HRESULT FMPlayerDShow::Init(HWND hWndParent, IMediaPlayerNotify* pNotify)
{
	USES_CONVERSION;
	m_pNotify = pNotify;
	_DBGAlertM("FMPlayerDShower: In ::Init()\n"); 

	Clear();
	m_AspectRatio.SetSize(0, 0); 
	m_hWndParent = hWndParent; 
	::GetClientRect(m_hWndParent, &m_VideoRect); 

	m_Offset = 0; 
	HRESULT hr = S_OK; 

	_DBGAlertM("FMPlayerDShow: Init() Done\n"); 


	hr = m_FilterGraph.CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC); 
	m_MediaControl = m_FilterGraph; 

	return hr; 
}


FString FormatFourCC(DWORD fourcc)
{
	char s[5];
	s[4] = 0;

	s[3]=   tolower(((fourcc & 0xff000000)>>24)&0xff);
	s[2]=   tolower(((fourcc & 0xff0000)>>16)&0xff);
	s[1]=   tolower(((fourcc & 0xff00)>>8)&0xff);
	s[0]=   tolower(((fourcc & 0xff)>>0)&0xff);

	return FString(s); 
}

BOOL FMPlayerDShow::GetAVIInfo(const tchar* pFileName)
{
	AVIFileInit();

	PAVIFILE pfile;
	BOOL bOK = FALSE; 
	if(AVIFileOpen(&pfile, pFileName, OF_SHARE_DENY_NONE, 0L) == 0)
	{
		AVIFILEINFO afi;
		memset(&afi, 0, sizeof(afi));
		AVIFileInfo(pfile, &afi, sizeof(AVIFILEINFO));

		CComPtr<IAVIStream> pavi;
		if(AVIFileGetStream(pfile, &pavi, streamtypeVIDEO, 0) == AVIERR_OK)
		{
			AVISTREAMINFO si;
			AVIStreamInfo(pavi, &si, sizeof(si));
			m_FourCC = FormatFourCC(si.fccHandler);
			m_FrameRate = (double)si.dwRate / (double)si.dwScale; 

			LONG lFormat; 
			if (0 == AVIStreamFormatSize(pavi, 0, &lFormat))
			{
				char* pBuf = new char[lFormat];
				if (0 == AVIStreamReadFormat(pavi, 0, pBuf, &lFormat))
				{
					BITMAPINFOHEADER* pHeader = (BITMAPINFOHEADER*)pBuf; 
					m_StreamFormat = FormatFourCC(pHeader->biCompression);
				}
				delete[] pBuf; 
			}

			bOK = TRUE; 
		}

		AVIFileRelease(pfile);
	}

	AVIFileExit();
	return bOK; 
}

HRESULT FMPlayerDShow::InitFilters(const tchar* pFileName)
{
	USES_CONVERSION; 
	HRESULT hr = S_OK; 
	m_FrameRate = 25.0;

	if (m_VideoType == VIDEO_TYPE_MPG)
	{
		hr = m_xVid.CoCreateInstance(CLSID_CMpegVideoCodec, NULL, CLSCTX_INPROC);
		_DBGLogResult(hr, "mpg codec"); 
		m_FourCC = "mpeg1";
		//we use IMediaDet for MPEG and we determine the AVI frame rate from the AVI header.
		CComPtr<IMediaDet> iMediaDet;
		if (SUCCEEDED(iMediaDet.CoCreateInstance(CLSID_MediaDet, NULL, CLSCTX_INPROC)))
			if (SUCCEEDED(iMediaDet->put_Filename(T2OLE(pFileName))))
				iMediaDet->get_FrameRate(&m_FrameRate);
	}
	else
	{
		FString sFourCC; 
		if (GetAVIInfo(pFileName))
		{
			_DBGAlert("%s: 4CC = %s\n", pFileName, sFourCC); 
			if (m_FourCC == "x264" || m_FourCC == "h264")
			{
				hr = DSHelpCreateInstance(T2OLE(g_AppSettings.AppDir("x264.ax")), IID_ICoreAVC, NULL, IID_IBaseFilter, (void**)&m_xVid); 
				_DBGLogResult(hr, "x264.ax"); 
			}
			else
			{
				if (m_FourCC == "divx" && m_StreamFormat == "divx")
				{
					hr = DSHelpCreateInstance(T2OLE(g_AppSettings.AppDir("divxdec.ax")), IID_IDivX4, NULL, IID_IBaseFilter, (void**)&m_xVid);
					_DBGLogResult(hr, "divxdec.ax"); 	
				}
				else
				{
					hr = DSHelpCreateInstance(T2OLE(g_AppSettings.AppDir("xvid.ax")), IID_IXvidDecoder, NULL, IID_IBaseFilter, (void**)&m_xVid); 
					_DBGLogResult(hr, "xvid.ax"); 
				}
			}
		}
	}

	hr = DSHelpCreateInstance(T2OLE(g_AppSettings.AppDir("ac3filter.ax")), IID_IAC3Decoder, NULL, IID_IBaseFilter, (void**)&m_AC3Filter);
	_DBGLogResult(hr, "ac3filter.ax"); 	


	//	hr = DSHelpCreateInstance(T2OLE(g_AppSettings.AppDir("msadds32.ax")), IID_IMSAudio, NULL, IID_IBaseFilter, (void**)&m_MSAud);
	//	_DBGLogResult(hr, "msadds32.ax"); 

	hr = m_DSoundFilter.CoCreateInstance(CLSID_DSoundRender, NULL, CLSCTX_INPROC); 
	_DBGLogResult(hr, "dsoundrender"); 

	if (m_VideoType == VIDEO_TYPE_MPG)
	{
		hr = m_AviSplitter.CoCreateInstance(CLSID_MPEG1Splitter, NULL, CLSCTX_INPROC); 
		_DBGLogResult(hr, "mpg splitter"); 
	}
	else
	{
		hr = m_AviSplitter.CoCreateInstance(CLSID_AviSplitter, NULL, CLSCTX_INPROC); 
		_DBGLogResult(hr, "avisplitter"); 
	}




	//Create video renderer
	if (m_VideoRenderer == NULL)
	{
		///hr = DSHelpCreateInstance(L"quartz.dll", CLSID_VideoRendererDefault, NULL, IID_IBaseFilter, (void**)&m_VideoRenderer);
		hr = m_VideoRenderer.CoCreateInstance(CLSID_VideoRendererDefault, NULL, CLSCTX_INPROC); 
		_DBGLogResult(hr, "video renderer"); 
	}
	else
	{
		DisconnectPins(m_VideoRenderer); 
	}

	if (SUCCEEDED(hr))
	{
		CComQIPtr<IGraphBuilder> pGraphBuilder = m_FilterGraph; 

		hr |= pGraphBuilder->AddFilter(m_xVid, L"Video Decoder");
		hr |= pGraphBuilder->AddFilter(m_DSoundFilter, L"DSound Output");
		hr |= pGraphBuilder->AddFilter(m_AC3Filter, L"AC3 Audio Decoder"); 
		hr |= pGraphBuilder->AddFilter(m_AviSplitter, L"Avi Splitter");
		hr |= pGraphBuilder->AddFilter(m_VideoRenderer, L"Video Renderer"); 

		if (FAILED(hr))
		{
			_DBGAlertM("**Warning prebuilding graph: %s\n", GetErrorDescription(hr));
		}
	}
	return hr; 
}


HRESULT FMPlayerDShow::BuildTheGraph()
{
	CComQIPtr<IGraphBuilder> pGraphBuilder = m_FilterGraph; 
	HRESULT hr = E_FAIL; 
	if (pGraphBuilder)
	{
		CComPtr<IPin> Pin1 = GetOutPin(m_AviSplitter, 0); 
		CComPtr<IPin> Pin2 = GetOutPin(m_AviSplitter, 1); 
		CComPtr<IPin> PinXV = GetInPin(m_xVid, 0); 
		if (Pin1 && Pin2 && PinXV)
		{
			hr = m_FilterGraph->ConnectDirect(Pin1, PinXV, NULL); 
			if (FAILED(hr))
			{
				hr = m_FilterGraph->ConnectDirect(Pin2, PinXV, NULL); 
			}
			if (SUCCEEDED(hr))
			{
				CComPtr<IPin> PinOutXV = GetOutPin(m_xVid, 0); 
				CComPtr<IPin> PinInRenderer = GetInPin(m_VideoRenderer, 0); 
				if (PinOutXV && PinInRenderer)
				{
					CComPtr<IPin> PinConnected; 
					if (PinOutXV->ConnectedTo(&PinConnected) & VFW_E_NOT_CONNECTED)
					{
						//Disconnect renderer from any filters;
						PinInRenderer->Disconnect(); 
						hr = m_FilterGraph->ConnectDirect(PinOutXV, PinInRenderer, NULL); 
					}
					if (SUCCEEDED(hr))
					{
						hr = RenderOutputPins(pGraphBuilder, m_AviSplitter); 
						if (FAILED(hr))
							_DBGAlertM("FMediaMgr: **RenderOutputPins: %s\n", GetErrorDescription(hr)); 
						m_GraphBuilt = TRUE; 
					}
					else
						_DBGAlertM("FMediaMgr: **Connect PinOutXV to PinInRenderer: %s\n", GetErrorDescription(hr)); 
				}
				else
					_DBGAlertM("FMediaMgr: **PinOutXV or PinInRenderer is NULL\n"); 
			}
			else
				_DBGAlertM("FMediaMgr: **Connect Splitter to xvid: %s\n", GetErrorDescription(hr)); 
		}
		else
			_DBGAlertM("FMediaMgr: **Pin1 or Pin2 or PinXV is NULL.\n"); 
	}

	return hr; 
}

HRESULT FMPlayerDShow::DetermineVideoType(const tchar* pFileName)
{

	FILE* f = fopen(pFileName, "rb"); 
	if (NULL == f)
		return E_FAIL; 

	char tbuf[4];
	fseek(f, 8, SEEK_SET); 
	fread(tbuf, 4, 1, f); 
	if (tbuf[0] == 'A' && tbuf[1] == 'V' && tbuf[2] == 'I' && tbuf[3] == 0x20)
	{
		m_VideoType = VIDEO_TYPE_AVI;
		_DBGAlertM("AVI Format: %s\n", pFileName); 
	}
	else
	{
		m_VideoType = VIDEO_TYPE_MPG;
		_DBGAlertM("MPG Format : %s\n", pFileName); 
	}
	fclose(f); 
	return S_OK; 
}
//////////////////////////////////////////////////////////////////////////
HRESULT FMPlayerDShow::LoadMedia(const tchar* pFileName, IMediaOptions* pOptions)
{
	USES_CONVERSION;

	HRESULT hr = E_FAIL; 

	if (m_FourCC == "divx" && m_StreamFormat == "divx")
	{
		Init(m_hWndParent, m_pNotify); 
	}

	if (m_FilterGraph == NULL)
		return E_FAIL;

	m_MediaControl->Stop(); 


	if (m_FileSource)
	{
		m_FilterGraph->RemoveFilter(m_FileSource);
		m_FileSource.Release();
	}
	else
	{
	/*	CComQIPtr<IGraphBuilder> pGraphBuilder = m_FilterGraph; 

		if (SUCCEEDED(pGraphBuilder->RenderFile(T2OLE(pFileName), NULL)))
		{
			hr = Start();
			return hr; 
		}
	*/
		if (SUCCEEDED(DetermineVideoType(pFileName)))
		{
			InitFilters(pFileName); 
		}
		else
		{
			_DBGAlertM("FMPlayerDShow: PlayVideo: **Unable to determine file format.\n");
			return E_FAIL;
		}
	}

	_DBGAlertM("FMPlayerDShow: Starting playback: %s; offset = %I64d\n", pFileName, pOptions->rtOffset); 

	CComQIPtr<IGraphBuilder> pGraphBuilder = m_FilterGraph; 

	hr = pGraphBuilder->AddSourceFilter(T2OLE(pFileName), T2OLE(pFileName), &m_FileSource); 

	if (SUCCEEDED(hr))
	{
		CComPtr<IPin> PinSourceOut = GetOutPin(m_FileSource, 0); 
		CComPtr<IPin> PinSplitterIn = GetInPin(m_AviSplitter, 0); 

		if (PinSourceOut && PinSplitterIn)
		{
			PinSplitterIn->Disconnect(); 
			hr = m_FilterGraph->ConnectDirect(PinSourceOut, PinSplitterIn, NULL); 
			if (FAILED(hr))
				_DBGAlertM("FMediaMgr: **Connect SourceOut to SplitterIn: %s\n", GetErrorDescription(hr)); 
		}
		else
			_DBGAlertM("FMediaMgr: **PinSourceOut or PinSourceIn is NULL\n"); 

		hr = BuildTheGraph(); 

		if (SUCCEEDED(hr))
		{
			m_FileName = pFileName; 
			m_Offset = pOptions->rtOffset;

			hr = Start(); 
			if (FAILED(hr))
				_DBGAlertM("FMPlayerDShow: **Start() failed: %s\n", GetErrorDescription(hr));
			else
			{
				SetVideoRect(m_VideoRect); 
			}
		}
		else
			_DBGAlertM("FMPlayerDShow: **Cannot Start(): %s\n", GetErrorDescription(hr)); 
	}
	else
		_DBGAlertM("FMPlayerDShow: **AddSourceFilter() failed: %s\n", GetErrorDescription(hr));

	_DBGAlertM("FMPlayerDShow: StartPlayback() exiting: %s\n", GetErrorDescription(hr)); 
	return hr; 
}
long FMPlayerDShow::OnMediaEvent()
{
	HRESULT hr = E_FAIL; 
	long lEventCode = 0; 
	long lp1, lp2;
	if (m_MediaEvent)
	{
		hr = m_MediaEvent->GetEvent(&lEventCode, &lp1, &lp2, 0); 
		if (SUCCEEDED(hr))
			hr = m_MediaEvent->FreeEventParams(lEventCode, lp1, lp2);
	}
	return lEventCode;
}

HRESULT FMPlayerDShow::Stop()
{
	Clear(); 
	return S_OK; 
}


HRESULT FMPlayerDShow::Start()
{
	HRESULT hr = E_FAIL; 

	CComQIPtr<IFilterGraph> pFilterGraph = m_FilterGraph; 

	if (pFilterGraph == NULL)
		return E_FAIL; 


	m_MediaEvent = pFilterGraph; 
	m_pBV		 = pFilterGraph; 

	BOOL bIsFullScreen = IsFullScreen(); 
	SetParentWnd(m_hWndParent); 

	if (bIsFullScreen)
		SetFullScreen(TRUE); 

	if (m_MediaControl != NULL)
	{
		hr = SeekPosition(m_Offset);
		m_Offset = 0; 
	}
	return hr; 
}

HRESULT FMPlayerDShow::Play()
{
	return m_MediaControl ? m_MediaControl->Run(): E_FAIL;
}

HRESULT FMPlayerDShow::Pause()
{
	return m_MediaControl ? m_MediaControl->Pause(): E_FAIL;
}

HRESULT FMPlayerDShow::SetParentWnd(HWND hWndParent)
{
	CComQIPtr<IVideoWindow> pVideoWnd = m_FilterGraph; 

	if (!m_VideoWnd.IsEqualObject(pVideoWnd))
	{
		m_VideoWnd = pVideoWnd;
		if (m_VideoWnd)
		{
			OAHWND hWndOwner = NULL;
			m_VideoWnd->get_Owner(&hWndOwner);
			if (hWndOwner != (OAHWND)hWndParent)
			{
				m_VideoWnd->put_Owner((OAHWND)m_hWndParent); 
				m_VideoWnd->put_AutoShow(OATRUE); 
				m_VideoWnd->put_MessageDrain((OAHWND)m_hWndParent); 
				m_VideoWnd->SetWindowForeground(OAFALSE);
				m_VideoWnd->put_WindowStyle(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);

				m_hWndParent = hWndParent; 
			}
		}
	}



	return S_OK; 
}

long FMPlayerDShow::GetMediaEvent()
{
	long lEventCode = 0; 
	long lp1, lp2;

	if (m_MediaEvent != NULL){
		if (SUCCEEDED(m_MediaEvent->GetEvent(&lEventCode, &lp1, &lp2, 0)))
			m_MediaEvent->FreeEventParams(lEventCode, lp1, lp2);
	}


	return lEventCode;
}


// pass in CWnd onsize x and y and return a CRect 
// used in a call to m_pVideoWindow->SetWindowPosition(x,y,w,h)
WTL::CRect FMPlayerDShow::GetAspectRatio(long lx,long ly) 
{ 
	WTL::CRect rect(0,0,lx,ly); 
	if(!m_pBV) 
		return rect; 

	HRESULT hr = 0; 
	long lNewX = lx; 
	long lNewY = ly; 
	long lX = 0; 
	long lY = 0; 
	// work out multiplier for x,y 
	float fXvideo = 1; 
	float fYvideo = 1; 
	hr = m_pBV->get_VideoWidth(&lX); 
	hr = m_pBV->get_VideoHeight(&lY); 

	// video aspect ratio 
	if(lX > lY) 
		fYvideo = (float)lY / (float)lX; 
	else if(lX < lY) 
		fXvideo = (float)lX / (float)lY; 

	if (m_AspectRatio.cy > 0 && m_AspectRatio.cx > 0)
		fYvideo = (float)m_AspectRatio.cx / (float)m_AspectRatio.cy; 

	// screen x,y by aspect 
	if(lx > ly || lX > lY) 
	{ 
		float fXscreen = (float)ly / (float)lx; 
		// screen ratio 
		lNewX = (long)(lNewX * fXscreen);
		// video ratio 
		lNewX = (long)(lNewX / fYvideo);
		// too big, shrink 
		if(lNewX > lx) 
		{ 
			lNewX = lx;     
			lNewY = (long)(lNewX * fYvideo);
		} 
	} 
	else 
	{ 
		float fYscreen = (float)lx / (float)ly; 
		lNewY = (long)(lNewY * fYscreen);
		lNewY = (long)(lNewY / fXvideo);
		// too big, shrink 
		if(lNewY > ly) 
		{ 
			lNewY = ly;     
			lNewX = (long)(lNewY * fXvideo);
		} 
	} 

	int nOffSetX = (rect.Width() - lNewX) / 2; 
	int nOffSetY = (rect.Height() - lNewY) / 2; 

	rect.left = rect.left + nOffSetX; 
	rect.top = rect.top + nOffSetY; 
	rect.right = rect.right - nOffSetX; 
	rect.bottom = rect.bottom - nOffSetY; 

	return rect; 

}


WTL::CSize FMPlayerDShow::GetVideoSize()
{
	bool fKeepAspectRatio = true;//AfxGetAppSettings().fKeepAspectRatio;
	bool fCompMonDeskARDiff = false; //AfxGetAppSettings().fCompMonDeskARDiff;

	WTL::CSize ret(0,0);

	WTL::CSize wh(0, 0), arxy(0, 0);

	{
		m_pBV->GetVideoSize(&wh.cx, &wh.cy);

		long arx = 0, ary = 0;
		CComQIPtr<IBasicVideo2> pBV2 = m_VideoWnd;
		if(pBV2 && SUCCEEDED(pBV2->GetPreferredAspectRatio(&arx, &ary)) && arx > 0 && ary > 0)
			arxy.SetSize(arx, ary);
	}

	WTL::CSize& ar = m_AspectRatio;
	if(ar.cx && ar.cy) arxy = ar;

	if(wh.cx <= 0 || wh.cy <= 0)
		return ret;

	// with the overlay mixer IBasicVideo2 won't tell the new AR when changed dynamically

	ret = (arxy.cx <= 0 || arxy.cy <= 0)
		? wh
		: WTL::CSize(MulDiv(wh.cy, arxy.cx, arxy.cy), wh.cy);

	return ret;
}

HRESULT FMPlayerDShow::SetVideoRect(const RECT &rcPos)
{
	HRESULT hr = S_FALSE; 

	WTL::CRect wr = rcPos; 

	m_VideoRect = rcPos; 
	if (m_VideoWnd)
	{

		WTL::CSize arxy = GetVideoSize(); 
		WTL::CRect vr = WTL::CRect(0,0,0,0);
		WTL::CSize ws = wr.Size(); 

		int w = ws.cx;
		int h = ws.cy;

		double m_ZoomX = 1; 
		double m_ZoomY = 1; 
		double m_PosX = 0.5; 
		double m_PosY = 0.5; 

		BOOL bFromInside = TRUE;  

		if(bFromInside)
		{
			h = ws.cy;
			w = MulDiv(h, arxy.cx, arxy.cy);

			if(bFromInside && w > ws.cx)
			{
				w = ws.cx;
				h = MulDiv(w, arxy.cy, arxy.cx);
			}
		}

		WTL::CSize size((int)(m_ZoomX*w), (int)(m_ZoomY*h));

		WTL::CPoint pos(
			(int)(m_PosX*(wr.Width()*3 - m_ZoomX*w) - wr.Width()), 
			(int)(m_PosY*(wr.Height()*3 - m_ZoomY*h) - wr.Height()));


		vr = WTL::CRect(pos, size);

		wr |= WTL::CRect(0,0,0,0);
		vr |= WTL::CRect(0,0,0,0);

		{

			hr = m_pBV->SetDefaultSourcePosition();
			hr = m_pBV->SetDestinationPosition(vr.left, vr.top, vr.Width(), vr.Height());
			hr = m_VideoWnd->SetWindowPosition(wr.left, wr.top, wr.Width(), wr.Height());


		}
	}

	return hr; 
}

VideoState FMPlayerDShow::GetVideoState()
{
	if (m_MediaControl)
	{
		FILTER_STATE fs;
		if (SUCCEEDED(m_MediaControl->GetState(0, (OAFilterState*)&fs)))
		{
			if (fs == State_Stopped)
				return vssStopped; 
			else
				if (fs == State_Paused)
					return vssPaused; 
				else
					if (fs == State_Running)
						return vssPlaying; 
		}
	}
	return vssStopped; 
}


MEDIA_TIME	FMPlayerDShow::GetDuration()
{
	CComQIPtr<IMediaSeeking> iSeeking = m_FilterGraph; 
	MEDIA_TIME rtDur = 0; 
	if (iSeeking)
	{
		iSeeking->GetDuration(&rtDur);
	}
	return rtDur; 
}

MEDIA_TIME FMPlayerDShow::GetPosition()
{
	MEDIA_TIME rtCur = 0; 
	CComQIPtr<IMediaSeeking> iSeeking = m_FilterGraph; 
	if (iSeeking)
	{
		iSeeking->GetCurrentPosition(&rtCur); 
	}
	return rtCur; 
}

HRESULT FMPlayerDShow::SeekPosition(MEDIA_TIME rtCurrent)
{
	CComQIPtr<IMediaSeeking> iSeeking = m_FilterGraph; 

	HRESULT hr = E_FAIL;
	if (iSeeking)
	{
		hr = iSeeking->SetPositions(&rtCurrent, AM_SEEKING_AbsolutePositioning , NULL, AM_SEEKING_NoPositioning);
	}
	return hr; 
}

BOOL FMPlayerDShow::IsWaiting()
{
	return E_NOINTERFACE; 
}

HRESULT FMPlayerDShow::ShowVideoWnd(int nCmdShow)
{
	if (m_MediaEvent)
	{
		m_MediaEvent->SetNotifyWindow(nCmdShow  == SW_SHOW ? (OAHWND)m_hWndParent : NULL, WM_DSHOW_CLIP_COMPLETE, 0); 
	}

	if (m_VideoWnd)
	{
		m_VideoWnd->put_MessageDrain(nCmdShow  == SW_SHOW ? (OAHWND)m_hWndParent : NULL); 
		return m_VideoWnd->put_Visible(nCmdShow==SW_SHOW ? OATRUE : OAFALSE);
	}
	return E_FAIL; 
}
BOOL FMPlayerDShow::IsFullScreen()
{
	if (m_VideoWnd)
	{
		long lFullScreen = OAFALSE; 
		if (SUCCEEDED(m_VideoWnd->get_FullScreenMode(&lFullScreen)))
			return lFullScreen == OATRUE ? TRUE : FALSE;
	}
	return FALSE; 
}

HRESULT FMPlayerDShow::SetFullScreen(BOOL bFullScreen)
{
	if (m_VideoWnd)
	{
		return m_VideoWnd->put_FullScreenMode(bFullScreen ? OATRUE: OAFALSE); 
	}
	return E_FAIL; 
}


long FMPlayerDShow::GetVolume()
{
	long lVol = 0; 
	CComQIPtr<IBasicAudio> pBA = m_FilterGraph;
	if (pBA != NULL && SUCCEEDED(pBA->get_Volume(&lVol)))
	{
		lVol = min(lVol, 0);
		double dblVol = pow((double)10, (double)lVol / 5000 + 2);
		dblVol = max(min(dblVol, 100), 0);
		return (long)dblVol; 
	}
	return -10000; 
}

HRESULT FMPlayerDShow::SetVolume(long lVol)
{
	CComQIPtr<IBasicAudio> pBA = m_FilterGraph;
	
	if (pBA != NULL)
	{
		long lVolNew = -10000; 
		if (lVol > 0)
		{		
			
			double dblVol = (double) lVol; 
			dblVol = (log(dblVol) / log(10.0) - 2) * 5000.0;
			dblVol = max(min(dblVol, 0), -10000.0);
			lVolNew = (long)dblVol;
		}
		return pBA->put_Volume(lVolNew); 
	}
	return S_OK; 
}

