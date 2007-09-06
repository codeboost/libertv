#include "stdafx.h"
#include "FAsyncHttpDownload.h"
#include <wininet.h>
#include "atlutil.h"
#include <atlfile.h>
#include "AppSettings.h"
#include "AutoResource.h"

void CloseHInternet(HINTERNET hHandle)
{
	InternetCloseHandle(hHandle); 
}

typedef CAutoResource<HINTERNET, CloseHInternet> FHInternet; 

void FAsyncDownload::FHttpDownloadTP::Thread(void* paThis)
{
    m_pThis = (FAsyncDownload*)paThis; 
    for (;;)
    {
        FAsyncDownload::FAsyncDownData *aData = NULL; 
        bool bPopped = false; 
        int nRes = m_pThis->m_Urls.PopWait(aData, bPopped);
        if (nRes == WAIT_OBJECT_0)
        {
            if (bPopped)
            {
                HRESULT hr = ProcessDownload(aData); 
                if (!m_pThis->m_Stopping)	//Don't send notification if we are stopping
                    m_pThis->OnDownloadComplete(aData, hr); 
				else
					break; 
            }
            else
            {
                _DBGAlert("FAsyncDownload::FHttpDownloadTP: **Not Popped !\n", nRes); 
                ATLASSERT(0); 
            }
        }
        else
        {
            _DBGAlert("FAsyncDownload::FHttpDownloadTP: **WaitForSingleObject(): %d\n", nRes); 
            break; 
        }
    }
}

class FHttpConnection
{
    HINTERNET m_hReq; 
public:
    FHttpConnection(HINTERNET hReq)
    {
        m_hReq = hReq; 
    }


    FString   GetHeader(DWORD dwHeader)
    {
        char szHeader[1024];
        DWORD dwLen = 1024; 
        if (HttpQueryInfo(m_hReq, dwHeader, szHeader, &dwLen, 0))
        {
            return FString(szHeader); 
        }
        return "";
    }

    size_type GetContentLength()
    {
        FString CLen = GetHeader(HTTP_QUERY_CONTENT_LENGTH);
        return _strtoui64(CLen.GetBuffer(), NULL, 10); 
    }


    DWORD GetStatusCode()
    {
        FString StatusCode = GetHeader(HTTP_QUERY_STATUS_CODE );
        return strtoul(StatusCode, NULL, 10); 
    }
};


BOOL MatchContentType(FString& ReqContentType, FString& RealContentType)
{
	BOOL bContentTypeOk = FALSE; 

	if (ReqContentType.GetLength() > 0)
	{

		int iLast = 0; 
		int iEnd = 0; 
		for (;;)
		{
			iEnd = ReqContentType.Find(',', iLast);

			if (iEnd == -1)
				iEnd = ReqContentType.GetLength(); 

			FString CType = ReqContentType.Mid(iLast, iEnd - iLast);

			CType.Trim();

			if (RealContentType.Find(CType) != -1)
			{
				bContentTypeOk = TRUE; 
				break;
			}

			if (iEnd == ReqContentType.GetLength())
				break; 
			iLast = iEnd + 1; 
		}
	}
	else
		bContentTypeOk = TRUE; 

	return bContentTypeOk; 
}

HRESULT FAsyncDownload::FHttpDownloadTP::ProcessDownload(FAsyncDownData *pData)
{
    HRESULT hr = E_FAIL; 


    FString ReqUrl = pData->m_pUrlInfo->m_DownloadUrl;
    UrlUnescapeInPlace(ReqUrl.GetBuffer(), 0); 

    CUrl url;
    url.CrackUrl(ReqUrl);

	const tchar* pszUserAgent = "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)";
    FHInternet hIn = NULL; 
	if (g_AppSettings.m_Proxy.GetLength() > 0)
	{
		hIn = InternetOpen(pszUserAgent, INTERNET_OPEN_TYPE_PROXY, g_AppSettings.m_Proxy, g_AppSettings.m_ProxyA, 0);
	}
	else
	{
		hIn = InternetOpen(pszUserAgent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	}

     
    if (NULL == hIn)
        return E_HTTP_NET_ERROR; 

    FHInternet hCon = InternetConnect(hIn, url.GetHostName(), url.GetPortNumber(), url.GetUserName(), url.GetPassword(), INTERNET_SERVICE_HTTP, 0, 0); 

    if (NULL == hCon)
    {
        _DBGAlert("**FAsyncDownload::FHttpDownloadTP::ProcessDownload: InternetConnect() failed: %d\n", GetLastError()); 
        return E_HTTP_NET_ERROR; 
    }

	ULONG ulRecvTimeout = 15000; 
	InternetSetOption(hCon, INTERNET_OPTION_RECEIVE_TIMEOUT, &ulRecvTimeout, sizeof(ULONG));


    FString StrRes = url.GetUrlPath();
    StrRes+= url.GetExtraInfo(); 
    
    FHInternet hReq = HttpOpenRequest(hCon, "GET", StrRes, NULL, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_DONT_CACHE, 0); 

    if (NULL == hReq)
    {
        _DBGAlert("**FAsyncDownload::FHttpDownloadTP::ProcessDownload: HttpOpenRequest() failed: %d\n", GetLastError()); 
        return E_HTTP_NET_ERROR; 
    }

	size_type FileSize = 0;
	
	

	if (!(pData->m_pUrlInfo->m_dwDownloadFlags & HTTP_FLAG_NO_RESUME))
		FileSize = GetFileSize(pData->m_pUrlInfo->m_DownloadFile);

    // See if file already exists on the disk.
    if (FileSize > 0)
    {
        FString StrRange; 
        StrRange.Format("Range: bytes=%I64d-", FileSize); 
        HttpAddRequestHeaders(hReq, StrRange, StrRange.GetLength(), HTTP_ADDREQ_FLAG_ADD_IF_NEW);
    }


	FString StrVersion; 
	StrVersion.Format("LTV_VERSION: %s", g_AppSettings.m_AppVersion); 
	HttpAddRequestHeaders(hReq, StrVersion, StrVersion.GetLength(), HTTP_ADDREQ_FLAG_ADD_IF_NEW);

    if (!HttpSendRequest(hReq, NULL, 0, NULL, 0))
    {
		int err = GetLastError(); 
        _DBGAlert("**FAsyncDownload::FHttpDownloadTP::ProcessDownload: HttpSendRequest() failed: %d (0x%x)\n", err, HRESULT_FROM_WIN32(err)); 
        InternetCloseHandle(hCon);
        InternetCloseHandle(hIn); 
        return E_HTTP_NET_ERROR; 
    }

    const DWORD dwBufferSize = 8192;
    char pBuffer[dwBufferSize];

    FHttpConnection FConn = hReq;

    DWORD dwStatusCode = FConn.GetStatusCode(); 

	FString ReqContentType = pData->m_pUrlInfo->m_ContentType; 
	pData->m_pUrlInfo->m_ContentType = FConn.GetHeader(HTTP_QUERY_CONTENT_TYPE);
	pData->m_pUrlInfo->m_dwStatusCode = dwStatusCode; 

	if (!MatchContentType(ReqContentType, pData->m_pUrlInfo->m_ContentType))
	{
		_DBGAlert("**FAsyncDownload::FHttpDownloadTP::ProcessDownload: Content type mismatch: %s/%s\n", ReqContentType, pData->m_pUrlInfo->m_ContentType); 
		return E_NOINTERFACE; //E_NOINTERFACE = content type mismatch
	}

	if (dwStatusCode == 416 && FileSize > 0)
	{
		_DBGAlert("FAsyncDownload::FHttpDownloadTP::ProcessDownload: Server status code: %d. Download complete\n", dwStatusCode); 
		return S_OK; 
	}

    if (dwStatusCode < 200 || dwStatusCode > 206)
    {
        _DBGAlert("**FAsyncDownload::FHttpDownloadTP::ProcessDownload: Server status code: %d\n", dwStatusCode); 
		if (dwStatusCode == 404)
			return E_HTTP_NOTFOUND; 
		return E_HTTP_INVALID_STATUS; 
    }

    CAtlFile OutFile; 

	if (pData->m_pUrlInfo->m_dwDownloadFlags & HTTP_FLAG_NO_RESUME)
		DeleteFile(pData->m_pUrlInfo->m_DownloadFile); 

    hr = OutFile.Create(pData->m_pUrlInfo->m_DownloadFile, GENERIC_WRITE, 0, OPEN_ALWAYS);

    if (FAILED(hr))
    {
		_DBGAlert("**FAsyncDownload::FHttpDownloadTP::ProcessDownload: CreateFile failed: 0x%x, %d : %s\n", hr, GetLastError(), pData->m_pUrlInfo->m_DownloadFile); 
        return E_HTTP_WRITE_FILE; 
    }

    size_type llTotalRead = 0; 
    size_type llSizeMax = 0; 

	size_type ContentLen = FConn.GetContentLength(); 

	pData->m_pUrlInfo->m_ContentLength = ContentLen; 

    if (dwStatusCode == 206)
    {
        FString FStrRange = FConn.GetHeader(HTTP_QUERY_CONTENT_RANGE);
        
        if (FStrRange)
        {
           //Content-Range: bytes 21010-47021/47022
           const char* pszBytes = strstr(FStrRange, "bytes ");
           if (pszBytes != NULL)
           {
               pszBytes+=sizeof("bytes");
               LONGLONG llOffset = _strtoi64(pszBytes, NULL, 10); 
               hr = OutFile.Seek(llOffset, FILE_BEGIN); 
               llTotalRead = (size_type)llOffset; 
               if (FAILED(hr))
               {
                   _DBGAlert("**FAsyncDownload::FHttpDownloadTP::ProcessDownload: Seek to position %d failed: 0x%x, %d\n", hr, GetLastError()); 
               }

               const char* pszTotal = strchr(pszBytes, '/');
               if (pszTotal != NULL)
                   llSizeMax = _strtoi64(pszTotal + 1, NULL, 10); 
           }
        }
    }
	else
	{
		if (ContentLen > 0 && ContentLen == FileSize)
		{
			OutFile.Close();
			return S_OK; 
		}
	}

    if (llSizeMax == 0)
		llSizeMax = ContentLen;


    pData->pBindStatusCallback.OnProgress((ULONG)llTotalRead, (ULONG)llSizeMax, BINDSTATUS_BEGINDOWNLOADDATA, L"");

    DWORD dwBytesRead = 0; 
    for (;;)
    {
        if (!InternetReadFile(hReq, pBuffer, dwBufferSize, &dwBytesRead))
        {
            _DBGAlert("**FAsyncDownload::FHttpDownloadTP::ProcessDownload: InternetReadFile() failed: %d\n", GetLastError()); 
			OutFile.Close();
            return E_HTTP_NET_ERROR; 
        }

		if (dwBytesRead == 0)
		{
			hr = S_OK; 
			break; 
		}

        DWORD dwBytesWritten = 0; 
        hr = OutFile.Write(pBuffer, dwBytesRead, &dwBytesWritten); 

		if (FAILED(hr))
        {
            _DBGAlert("**FAsyncDownload::FHttpDownloadTP::ProcessDownload: FileWrite failed: 0x%x, %d\n", hr, GetLastError()); 
			OutFile.Close();
            return E_HTTP_WRITE_FILE; 
        }

        llTotalRead+=dwBytesRead;
		
		pData->pBindStatusCallback.OnProgress((ULONG)llTotalRead, llSizeMax > 0 ? (ULONG)llSizeMax : llTotalRead , BINDSTATUS_DOWNLOADINGDATA, L"");


        if (m_pThis->m_Stopping || pData->pBindStatusCallback.m_Abort)
        {
            _DBGAlert("**FAsyncDownload::FHttpDownloadTP::ProcessDownload: Download aborted\n", hr, GetLastError()); 
            hr = E_ABORT; 
            break; 
        }
    }

	OutFile.Close();
    return hr; 
}

void FAsyncDownload::FAsyncDownData::SendAlert(dword dwCode)
{
	if (m_pNotify != NULL)
	{
		FDownloadAlert alert; 
		alert.dwCode = dwCode; 
		alert.hr = hr; 
		alert.m_Param1 = m_pUrlInfo; 
		alert.m_DownloadHandle = m_pUrlInfo->m_Handle;
		m_pNotify->Notify(alert); 
	}
}

void FAsyncDownload::OnDownloadComplete(FAsyncDownData* pData, HRESULT hr)
{
	
	ATLASSERT(NULL != pData); 
	
	if (pData->pBindStatusCallback.m_Paused)
		return ; //Do not notify about the pause

	if (pData->pBindStatusCallback.m_Abort )
		hr = E_ABORT; 

	pData->hr = hr; 
	pData->SendAlert(ALERT_HTTP_DOWNLOAD_FINISHED); 

	//Remove the download from the list.
	SynchronizeThread(m_Sync); 

	for (size_t k = 0; k < m_aDownloads.GetCount(); k++ )
	{
		if (m_aDownloads[k] && m_aDownloads[k] == pData)
		{
			delete pData;
			m_aDownloads[k] = NULL; 
			return; 
		}
	}
}

BOOL FAsyncDownload::AddDownload(FAsyncDownData* pData)
{
	SynchronizeThread(m_Sync);
	bool bAdded = FALSE; 
	for (size_t k = 0; k < m_aDownloads.GetCount(); k++)
	{
		if (NULL == m_aDownloads[k])
		{
			m_aDownloads[k] = pData; 
			bAdded = TRUE; 
			break; 
		}
	}
	if (!bAdded)
		m_aDownloads.Add(pData); 

	if (m_aDownloads.GetCount() > m_DownloadPool.GetThreadCount())
	{
		m_DownloadPool.AddWorker(1, this); 
	}

	return m_Urls.Push(pData); 
}

void FAsyncDownload::AbortDownloads()
{
	_DBGAlert("FASyncDownload: AbortDownloads()\n"); 
	SynchronizeThread(m_Sync);
	{
		m_Stopping = TRUE; 
		m_Urls.Clear(); 
		for (size_t k = 0; k < m_aDownloads.GetCount(); k++)
		{
			if (NULL != m_aDownloads[k])
			{
				m_aDownloads[k]->pBindStatusCallback.m_Abort = TRUE; 
			}
		}
		//Notify all paused downloads that we are aborting
		for (size_t k = 0; k < m_aPaused.GetCount(); k++)
		{
			m_aPaused[k]->hr = E_ABORT; 
			m_aPaused[k]->SendAlert(ALERT_HTTP_DOWNLOAD_FINISHED); 
			delete m_aPaused[k];
		}
	}
	_DBGAlert("FASyncDownload: AbortDownloads() finished\n"); 
}

void FAsyncDownload::Stop()
{
	_DBGAlert("FAsyncDownload: Stopping\n"); 
	AbortDownloads();
	m_DownloadPool.StopAndWait(INFINITE);

	SynchronizeThread(m_Sync); 

	for (size_t k = 0; k < m_aDownloads.GetCount(); k++)
	{
		delete m_aDownloads[k];
	}
	m_aDownloads.RemoveAll(); 
	m_aPaused.RemoveAll();
	_DBGAlert("FASyncDownload: Stop() finished\n"); 
}

DHANDLE FAsyncDownload::DownloadURL(FDownloadInfo* pInfo,  IClipDownloadNotify* pvObj)
{
	if (!pInfo)
		return 0; 

	if (!PathIsURL(pInfo->m_DownloadUrl))
	{
		_DBGAlert("**FAsyncDownload: Path is not a URL: %s\n", pInfo->m_DownloadUrl); 
		return 0; 
	}

	DHANDLE hHandle = (DWORD)InterlockedIncrement((LONG*)&m_HandleID);

	FAsyncDownData* pData = new FAsyncDownData; 
	pData->m_pUrlInfo = pInfo; 
	pData->m_pNotify = pvObj; 
	pData->m_pUrlInfo->m_Handle = hHandle; 
	AddDownload(pData);
    return hHandle; 
}

BOOL FAsyncDownload::StopDownload(DHANDLE hDownload)
{
	SynchronizeThread(m_Sync); 
	for (size_t k = 0; k < m_aDownloads.GetCount(); k++){
		if (NULL != m_aDownloads[k])
		{
			if (m_aDownloads[k]->m_pUrlInfo->m_Handle == hDownload)
			{
				m_aDownloads[k]->pBindStatusCallback.m_Abort = TRUE; 
				return TRUE; 
			}
		}
	}

	return FALSE; 
}


BOOL FAsyncDownload::GetDownloadProgress(DHANDLE hDownload, FDownloadProgress& Progress)
{
    SynchronizeThread(m_Sync); 
    for (size_t k = 0; k < m_aDownloads.GetCount(); k++)
    {
        if (m_aDownloads[k] && m_aDownloads[k]->m_pUrlInfo->m_Handle == hDownload)
        {
            Progress = m_aDownloads[k]->pBindStatusCallback; 
            Progress.Recompute(); 
            return TRUE; 
        }
    }
    return FALSE; 
}

void FAsyncDownload::Init()
{
	m_Stopping = FALSE; 
    m_HandleID = 0; 
	m_Urls.SetSize(16);
	m_DownloadPool.AddWorker(1, this); 
}

BOOL FAsyncDownload::Pause(DHANDLE hDownload)
{
	SynchronizeThread(m_Sync); 
	for (size_t k = 0; k < m_aDownloads.GetCount(); k++)
	{
		if (m_aDownloads[k] && m_aDownloads[k]->m_pUrlInfo->m_Handle == hDownload){
			m_aDownloads[k]->pBindStatusCallback.m_Paused = TRUE;
			m_aDownloads[k]->pBindStatusCallback.m_Abort = TRUE;
	
			m_aPaused.Add(m_aDownloads[k]); 
			m_aDownloads[k] = NULL; 
			return TRUE; 
		}
	}
	return FALSE; 
}

BOOL FAsyncDownload::Resume(DHANDLE hDownload)
{
	SynchronizeThread(m_Sync); 
	for (size_t k = 0; k < m_aPaused.GetCount(); k++)
	{
		if (m_aPaused[k] && m_aPaused[k]->m_pUrlInfo->m_Handle == hDownload){
			m_aPaused[k]->pBindStatusCallback.m_Paused = FALSE;
			m_aPaused[k]->pBindStatusCallback.m_Abort = FALSE;
			FAsyncDownData* pData = m_aPaused[k];
			m_aPaused.RemoveAt(k); 
			return AddDownload(pData);
		}
	}
	return FALSE; 
}

BOOL FAsyncDownload::IsPaused(DHANDLE handle)
{
	SynchronizeThread(m_Sync);
	for (size_t k = 0; k < m_aPaused.GetCount(); k++)
	{
		if (m_aPaused[k] && m_aPaused[k]->m_pUrlInfo->m_Handle == handle){
			return TRUE; 
		}
	}
	return FALSE; 
}


//////////////////////////////////////////////////////////////////////////

BOOL FDownloadNotify::Init(IClipDownloadNotify* pNotify)
{
	m_Object = pNotify; 
	m_Alerts.SetSize(128); 
	if (m_AlertThread.Create(this))
		return TRUE; 
	return FALSE; 
}

void FDownloadNotify::Stop()
{
	m_Alerts.Clear(); 
	m_AlertThread.StopThread(); 
}

void FDownloadNotify::Notify(FDownloadAlert &alert)
{
	m_Alerts.Push(alert); 
}

void FDownloadNotify::FAlertThread::Thread(void *p)
{
	FDownloadNotify* pThis = (FDownloadNotify*)p; 
	_DBGAlert("Notify thread started\n"); 
	CoInitializeEx(NULL, 0); 
	for (;;)
	{
		bool bPopped; 
		FDownloadAlert alert; 
		int nRes = pThis->m_Alerts.PopWait(alert, bPopped);
		if (nRes == WAIT_OBJECT_0)
		{
			if (bPopped)
				pThis->m_Object->Notify(alert); 
		}
		else
			break; 
	}
	_DBGAlert("Notify thread terminating\n"); 
}









