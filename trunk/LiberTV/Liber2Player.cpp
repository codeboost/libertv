// Liber2Player.cpp : Implementation of WinMain

#define WIN32_LEAN_AND_MEAN
#include "stdafx.h"
#include "resource.h"
#include "Utils.h"
#include "GlobalObjects.h"		//Metatorrent handler
#include "FTrayWnd.h"
#include "FWaitDlg.h"
#include "vfw.h"
#include "FIniFile.h"
#include "FDownloadMgr.h"
#include "FDocHostUIHandler.h"
//////////////////////////////////////////////////////////////////////////
#include "FHtmlEventDispatcher.h"
#include "Liber2Player.h"
#include "TestWindow.h"
#include "AppCheck.h"
#include "FDownloadIndex.h"
#include "FFirewall.h"

HINSTANCE g_Instance;

class CLiber2PlayerModule : public CAtlExeModuleT< CLiber2PlayerModule >
{
public :
	DECLARE_LIBID(LIBID_Liber2PlayerLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_LIBER2PLAYER, "{E48FDF0E-71F8-4E3C-915D-5DB13BACB6F6}")
};

BEGIN_OBJECT_MAP(ObjectMap)
	OBJECT_ENTRY(CLSID_FHtmlEventHandler, CFHtmlEventHandler)
	OBJECT_ENTRY(CLSID_FHtmlEventDispatcher, CFHtmlEventDispatcher)
	OBJECT_ENTRY(CLSID_FDocHostUIHandler, CFDocHostUIHandler)
END_OBJECT_MAP()

//The most global globals
CMyServerAppModule			_Module; 
GlobalObjects				g_Objects; 
AppSettings					g_AppSettings; 
FFirewall					g_Firewall;
FAutoWindow<FTrayWindow>	g_TrayWindow; 
FAutoWindow<FMainFrame>		g_MainFrame; 
IAppManager*				g_pAppManager = NULL; 

//////////////////////////////////////////////////////////////////////////
static HANDLE AppRunning = NULL; 
static HANDLE AppMutex = NULL; 
BOOL bRestarting = FALSE; 
#define LTV_MUTEX LTV_APP_NAME"_Mutex"


HWND FindInstance()
{
	return FindWindow(LTV_CLASSNAME, 0);
}

bool RegisterInstance ()
{
	HANDLE testMutex= CreateMutex(NULL, TRUE, LTV_MUTEX);
	if (ERROR_ALREADY_EXISTS == GetLastError())
	{
		//Already running
		return false;
	}
	else
	{
		AppMutex = testMutex;
	} 
	return true;
}


BOOL UploadFile(const tchar* pszFileName, const tchar* pszUploadURL)
{
	FILE* f = fopen(pszFileName, "rb"); 
	int FileSize = 0; 
	CAutoPtr<char> aBuf;
	if (f)
	{
		fseek(f, 0, SEEK_END); 
		FileSize = ftell(f); 
		if (FileSize > 0)
		{
			fseek(f, 0, SEEK_SET); 
			aBuf.Attach(new char [FileSize + 1]);
			fread(aBuf.m_p, FileSize, 1, f); 
		}
		fclose(f); 
	}
	else
		return FALSE; 

	HINTERNET hInternet = NULL;
	hInternet = InternetOpen("Upload XML File", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!hInternet)
	{
		return FALSE;
	}
	CUrl aUrl;
	aUrl.CrackUrl(pszUploadURL);

	HINTERNET hConnect  = NULL;

	hConnect = InternetConnect(hInternet, aUrl.GetHostName(), aUrl.GetPortNumber(),	NULL, NULL, INTERNET_SERVICE_HTTP, NULL, NULL );
	if(!hConnect)
	{
		InternetCloseHandle(hInternet);
		return FALSE;
	}

	// Open the connection
	HINTERNET hRequest = HttpOpenRequest (hConnect, "POST", aUrl.GetUrlPath(), NULL, NULL, NULL,  0, 0);

	if (!hRequest)
	{
		InternetCloseHandle(hConnect);
		InternetCloseHandle(hInternet);
		return FALSE;
	}

	BOOL bSendOK = HttpSendRequest ( hRequest, NULL, 0 , (LPVOID)aBuf.m_p, FileSize);

	InternetCloseHandle(hRequest);
	InternetCloseHandle(hConnect);
	InternetCloseHandle(hInternet);
	return TRUE;
}

class QHttpNotify : public IQHttpNotify
{
public:
	void OnDownloadComplete(const FQHttpParams& pDownload, HRESULT hr)
	{
		_DBGAlert("Download complete (0x%x): %s into %s\n", hr, pDownload.m_Url, pDownload.m_FileName);
		ShellExecute(NULL, "open", pDownload.m_FileName, "", "", SW_SHOW); 
	}
};

/////////////////////////////////////////////////////////////////////////////////////

//    FUNCTION:    CopyFolder

//    DESCRIPTION: Copies a directory to a new location

//

//    RETURN:         TRUE for success, FALSE for failure

//

/////////////////////////////////////////////////////////////////////////////////////

BOOL RecursiveCopyFolder(FString csPath, FString csNewPath)

{

	BOOL bRet = TRUE;
	if(!CreateDirectory(csNewPath, NULL) && !PathIsDirectory(csNewPath))
		bRet = FALSE;
	FString csPathMask;
	FString csFullPath;
	FString csNewFullPath;
	csPath     += _T("\\");
	csNewPath  += _T("\\");
	csPathMask = csPath + _T("*.*");
	WIN32_FIND_DATA ffData;
	HANDLE hFind;
	hFind = FindFirstFile(csPathMask, &ffData);
	if (hFind == INVALID_HANDLE_VALUE){
		return FALSE;
	}
	// Copying all the files
	while (hFind && FindNextFile(hFind, &ffData)) 
	{
		csFullPath    = csPath    + ffData.cFileName;
		csNewFullPath = csNewPath + ffData.cFileName;
		if( !(ffData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) 
		{
			if( !CopyFile(csFullPath, csNewFullPath, FALSE) ) 
			{
				// Not stopping here, trying to copy the rest of the files
				bRet = FALSE;
			}
		}
		else // it is a directory -> Copying recursivly
		{ 
			if( (_tcscmp(ffData.cFileName, _T(".")) != 0) &&
				(_tcscmp(ffData.cFileName, _T("..")) != 0) ) 
			{
				if( !RecursiveCopyFolder(csFullPath, csNewFullPath) )
				{
					// Not stopping here, trying to copy the rest of the files
					bRet = FALSE;
				}
			}
		}
	}
	FindClose(hFind);
	return bRet;
}
 
int WINAPI RunWinMain(LPSTR lpCmdLine, int nCmdShow) 
{
	//UploadFile("M:\\Movies\\Test.avi.torrent", "http://testguide.libertv.ro/guide/user_upload.php?videoID=32323");
	g_AppSettings.m_CmdLine.Set(GetCommandLine()); 
	if (!RegisterInstance())
	{
		   BOOL bRestartError = FALSE; 
		   if (bRestarting)
		   {
			   //Wait on the mutex until it becomes available
			   HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, LTV_MUTEX);
			   if (hMutex != NULL)
			   {
				   int nRes = WaitForSingleObject(hMutex, 15000);

				   if (nRes == WAIT_ABANDONED || nRes == WAIT_OBJECT_0)
					   AppMutex = hMutex; 
				   else
				   {
					   bRestartError = TRUE; 
					   CloseHandle(hMutex); 
				   }
			   }

			   if (bRestartError)
			   {
					MessageBox(NULL, "Could not restart LiberTV. Please start it manually.", "Startup Error", MB_OK | MB_ICONERROR); 
					return 0; 
			   }
		   }
		   else
		   {
			   HWND hWndInstance = FindInstance(); 
			   SendMessage(hWndInstance, WM_COMMAND, MAKEWPARAM(ID_TRAY_OPENPLAYER, 0), 0); 

			   const char* pCmdLine = GetCommandLine(); 
			   if (pCmdLine != NULL)
			   {
				   COPYDATASTRUCT cds; 
				   cds.dwData = 1; 
				   cds.cbData = (DWORD)strlen(pCmdLine); 
				   cds.lpData = (void*)pCmdLine; 
				   //Send command line
				   SendMessage(hWndInstance, WM_COPYDATA, NULL, (LPARAM)&cds); 
			   }
			   return 0; 
		   }
	}

	DWORD dwNoAutoUpdate = _RegDword("NoAutoUpdate");

	if (dwNoAutoUpdate == 0 && FUpdater::IsValidUpdate(_RegStr("Version")))
	{
		FString StrCmdLine = "/VERYSILENT";
		if (g_AppSettings.m_CmdLine.HasParam("/silent"))
			StrCmdLine.Append(" /LTVCMDLINE=/silent");
		
		ShellExecute(NULL, "open", _RegStr("UpdateFile"), StrCmdLine, "", SW_SHOW); 
		FUpdater::ClearUpdateInfo(); 
		return 0; 
	}

	DWORD dwMajor, dwMinor, dwBuild; 
	dwMinor = dwMajor = dwBuild = 0; 
	HRESULT iehr = GetIEVersion(&dwMajor, &dwMinor, &dwBuild);

	if (FAILED(iehr) || dwMajor < 6)
	{
		FString StrMsg; 
		StrMsg.Format(LTV_APP_NAME" requires Internet Explorer 6 or later to be installed on your computer.\n\nVersion currently installed: %d.%d.%d", dwMajor, dwMinor, dwBuild); 
		MessageBox(NULL, StrMsg, LTV_APP_NAME": Wrong Internet Explorer version", MB_OK | MB_ICONERROR); 
		return 0; 
	}


	if (FAILED(_Module.InitObjectLibray()))
		return 0; 

	

	FString FStrCurDir = g_AppSettings.m_CmdLine.GetAt(0); 

	FStrCurDir.Trim(" \"");
	FStrCurDir.Trim("\""); 
	
	FString ExeName = FStrCurDir; 
	
	PathRemoveFileSpec(FStrCurDir.GetBuffer()); 
	///PathRemoveArgs(ExeName.GetBuffer());

	SetCurrentDirectory(FStrCurDir);
	g_AppSettings.m_AppDirectory = FStrCurDir; 
	g_AppSettings.m_ExePath.Format("\"%s\"", ExeName);
	if (!g_AppSettings.LoadSettings())
    {
        MessageBox(NULL, "Could not load settings.", "Fatal Error", MB_OK | MB_ICONERROR); 
        return -1; 
    }
	

	if (g_AppSettings.m_CmdLine.HasParam("/RemoveStorage"))
	{
		if (g_AppSettings.m_IndexPath.GetLength() > 0)
		{
			FDownload_Storage aStorage; 
			if (aStorage.DeleteStorage(TRUE))
			{
				MessageBox(NULL, "Storage removed", LTV_APP_NAME, MB_OK | MB_ICONINFORMATION);
			}
			else
			{
				MessageBox(NULL, "Could not remove storage. Please remove all files manually", LTV_APP_NAME, MB_OK | MB_ICONWARNING);
				ShellExecute(NULL, "open", g_AppSettings.m_IndexPath, "", "", SW_SHOW); 
			}
		}
		else
			MessageBox(NULL, "Couldn't determine storage path due to misconfiguration. "LTV_APP_NAME" storage was not removed.", LTV_APP_NAME, MB_OK | MB_ICONERROR); 

		return -1; 
	}


	if (g_AppSettings.m_CmdLine.HasParam("/ConvertStorage"))
	{
/*
		CComPtr<IXMLDOMDocument> m_pDoc; 
		HRESULT hr = m_pDoc.CoCreateInstance(CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER); 

		VARIANT_BOOL bSuccess; 
		m_pDoc->load(CComVariant("M:\\temp\\test.xml"), &bSuccess);

		CComQIPtr<IXMLDOMNode> pDocNodeP = m_pDoc; 
		FXMLNode pDocNode = pDocNodeP;
		pDocNode.Format(0, m_pDoc); 

		hr = m_pDoc->save(CComVariant("M:\\temp\\test1.xml"));
		return 0; 

		FDownload_Storage aStorage; 
		FArray<FDownloadEx*> pDownloads; 
		if (aStorage.LoadStorage(pDownloads))
		{
			FDownloadIndex XMLIndex; 
			HRESULT hr = XMLIndex.ConvertIndex(pDownloads, "M:\\Temp\\LiberTV Index.xml");
		}
		*/
		return -1; 
	}

	if (g_AppSettings.m_CmdLine.HasParam("/MoveStorage"))
	{
		if (g_AppSettings.m_CmdLine.GetArgc() < 4)
		{
			MessageBox(NULL, "Must specify output path", "Error", 0);
			return -1; 
		}

		
		FString NewDir = g_AppSettings.m_CmdLine.GetAt(2);

		if (!CreateDirectory(NewDir, NULL) && !PathIsDirectory(NewDir))
		{
			MessageBox(NULL, "Cannot create output dir", "Error", 0); 
			return -1; 
		}

		FString NewDataPath = g_AppSettings.m_CmdLine.GetAt(3); 


		FArray<FDownloadEx*> pDownloads;
		FDownload_Storage aStorage; 
		if (aStorage.LoadStorage(pDownloads))
		{
			//Steps:
			//Copy everything from Download->DataPath to NewDir
			//Modify DataPath = ""
			//Save Mtti in the NewDir\Index
			for (size_t k = 0; k < pDownloads.GetCount(); k++)
			{
				FDownloadEx* pDown = pDownloads[k]; 
				pDown->m_Detail.m_DataPath = PathRemoveRoot(pDown->m_Detail.m_DataPath, g_AppSettings.m_DownloadsFolder);
				pDown->m_Detail.m_DataPath = _PathCombine(NewDataPath, pDown->m_Detail.m_DataPath); 
				pDown->m_Conf.m_FileName = PathRemoveRoot(pDown->m_Conf.m_FileName, g_AppSettings.m_IndexPath); 
				pDown->m_Conf.m_FileName = _PathCombine(NewDir, pDown->m_Conf.m_FileName); 
				pDown->SaveToDisk(); 
			}
			MessageBox(NULL, "Done", "OK", 0); 
		}
		return 1; 
	}

	_DBGAlert("*$* Starting Instance. Commandline =  %s\n", lpCmdLine); 
	
	HRESULT hr = FFirewall::AddAppToWindowsFirewall(g_AppSettings.m_CmdLine.GetAt(0), LTV_APP_NAME);
	if (FAILED(hr))
	{
		_DBGAlert("*$* Could not add app to windows firewall exceptions list: 0x%8x", hr);
	}

	if (!g_Objects.Init())
	{
		MessageBox(NULL, "Could not start global objects. Please re-install.", "Fatal Error", MB_OK | MB_ICONERROR);
		return -1; 
	}
/*
	const FArray<FRSSFeed*>& aFeeds = g_Objects._RSSManager->LockChannels();
	g_Objects._RSSManager->ReleaseChannelsLock();
	g_Objects.Stop();
	
	return -1; 
*/
    if (!g_Objects._DownloadManager.Init())
    {
        MessageBox(NULL, "Could not start download manager", "Fatal Error", MB_OK | MB_ICONERROR); 
        return -1; 
    }


	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);
	_Module.Lock();
	
	
	g_TrayWindow.Create(NULL, NULL, "LTV_TrayWnd", 0); 
	g_pAppManager = g_TrayWindow.pWindow; 

	int nRet = -1; 
	if (g_TrayWindow->IsWindow())
	{
		g_TrayWindow->ShowWindow(SW_HIDE); 
		g_TrayWindow->UpdateWindow(); 

		nRet = theLoop.Run();
	}
	else
	{
		MessageBox(NULL, LTV_APP_NAME" could not initialize. Please reinstall", "Fatal error", MB_OK | MB_ICONERROR);
	}
	g_TrayWindow.DestroyWindow(); 
	_Module.RemoveMessageLoop();

	if (g_Objects.dwRestartOnClose != 0)
	{
		if (g_Objects.dwRestartOnClose == 1)
		{
			ShellExecute(NULL, "open", g_AppSettings.m_ExePath, "/Restart", g_AppSettings.m_AppDirectory, SW_SHOW); 
		}
		else
		{
			ShellExecute(NULL, "open", g_AppSettings.m_ExePath, "/silent /Restart", g_AppSettings.m_AppDirectory, SW_SHOW); 
		}
	}
	return nRet;
}

//////////////////////////////////////////////////////////////////////////
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, 
                                LPTSTR lpCmdLine, int nShowCmd)
{

	g_Instance = hInstance;
/*
	FRSSManager pManager; 
	g_AppSettings.m_IndexPath = "F:\\LiberTV Index";
	pManager.LoadIndex(g_AppSettings.m_IndexPath);
	const FArray<FRSSFeed*>& aFeeds = pManager.LockChannels();
	pManager.ReleaseChannelsLock();
	pManager.Clear();
	return 0; 

*/
//	HijackCommandLine();

    HRESULT hRes = ::CoInitialize(NULL);


    // If you are running on NT 4.0 or higher you can use the following call instead to 
    // make the EXE free threaded. This means that calls come in on a random RPC thread.
    //	HRESULT hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    ATLASSERT(SUCCEEDED(hRes));

    // this resolves ATL window thunking problem when Microsoft Layer for Unicode (MSLU) is used
    ::DefWindowProc(NULL, 0, 0, 0L);

    AtlInitCommonControls(ICC_COOL_CLASSES | ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(ObjectMap, hInstance );

    ATLASSERT(SUCCEEDED(hRes));

    AtlAxWinInit();

    int nRet = 0;
    TCHAR szTokens[] = _T("-/");
    bool bRun = true;
    bool bAutomation = false;

    LPCTSTR lpszToken = _Module.FindOneOf(::GetCommandLine(), szTokens);
    while(lpszToken != NULL)
    {
        if(lstrcmpi(lpszToken, _T("UnregServer")) == 0)
        {
            nRet = _Module.UnregisterServer();
            bRun = false;
            break;
        }
        else if(lstrcmpi(lpszToken, _T("RegServer")) == 0)
        {
            nRet = _Module.RegisterServer();
            bRun = false;
            break;
        }
        else if((lstrcmpi(lpszToken, _T("Automation")) == 0) ||
            (lstrcmpi(lpszToken, _T("Embedding")) == 0))
        {
            bAutomation = true;
            break;
        }
		else if ((lstrcmpi(lpszToken, _T("Restart")) == 0))
		{
			bRestarting = TRUE; 
			break; 
		}

        lpszToken = _Module.FindOneOf(lpszToken, szTokens);
    }

    if(bRun)
    {
        _Module.StartMonitor();
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
        ATLASSERT(SUCCEEDED(hRes));
        hRes = ::CoResumeClassObjects();
        ATLASSERT(SUCCEEDED(hRes));

        if(bAutomation)
        {
            CMessageLoop theLoop;
            nRet = theLoop.Run();
        }
        else
        {
            nRet = RunWinMain(lpCmdLine, nShowCmd);
        }

        _Module.RevokeClassObjects();
        ::Sleep(_Module.m_dwPause);
    }

	_Module.DeInitObjectLibrary();
    _Module.Term();
    ::CoUninitialize();

    return nRet;    

}

