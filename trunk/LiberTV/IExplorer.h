#include "Utils.h"
#include "wtlext.h"
#include <ExDispid.h>
#include <mshtml.h>
#include <comdef.h>
#include <atlstr.h>
#include "FHtmlEventHandler.h"

#define EVENTFN void __stdcall

	
class FDocument
{
public:
	CComQIPtr<IHTMLDocument2> pDocument; 

	HRESULT CallJScript(const tchar* wFunctionName, ATL::CAtlArray<ATL::CAtlString>& aParams, CComVariant* pvResult = NULL);
	HRESULT CallJScript(const tchar* wFunc, CComVariant* pvResult = NULL); 
	HRESULT CallJScript(const tchar* wFunc, const tchar* aParam1, CComVariant* pvResult = NULL); 
	HRESULT CallJScript(const tchar* wFunc, const tchar* aParam1, const tchar* aParam2, CComVariant* pvResult = NULL); 
	HRESULT CallJScript(const tchar* wFunc, const tchar* aParam1, const tchar* aParam2, const tchar* aParam3, CComVariant* pvResult = NULL); 

	FString	CallScriptStr(const tchar* wFunc, const tchar* aParam); 
	long	CallScriptLng(const tchar* wFunc, const tchar* aParam); 


	HRESULT SetElementInnerText(LPCSTR ElementName, LPCSTR InnerText);
	HRESULT SetElementInnerHTML(BSTR ElementName, BSTR InnerHTML);
	HRESULT QueryHtmlElementFormElement( LPCTSTR FormName, LPCTSTR ElementName, LPDISPATCH * ppDispatch );
	HRESULT QueryHtmlElement( LPCSTR ElementName, LPDISPATCH * ppDispatch );
	HRESULT QueryHtmlElementValueListBox( ATL::CString & retVal, LPCTSTR Form, LPCTSTR Field, BOOL bPutValue);


	HRESULT GetSetText( ATL::CString & retVal, LPCTSTR Form, LPCTSTR Field, BOOL bPutValue );
	HRESULT GetSetCheck ( BOOL & retVal, LPCTSTR Form, LPCTSTR Field, BOOL bPutValue);

	DWORD	GetDword(LPCTSTR Form, LPCTSTR Field);
	DWORD	GetDword(LPCSTR Form, LPCSTR Field, DWORD& dwValOut); 
	FString	GetText(LPCTSTR Form, LPCTSTR Field); 
	BOOL	GetCheck(LPCSTR Form, LPCSTR Field);
	

	HRESULT SetDword(LPCSTR Form, LPCTSTR Field, DWORD dwValue);
	HRESULT	SetText(LPCSTR Form, LPCSTR Field, ATL::CString& Value); 
	HRESULT	SetCheck(LPCSTR Form, LPCSTR Field, BOOL dwValue);
};

class IHttpNotify
{
public:
	~IHttpNotify(){}
	virtual void OnDocumentComplete(DWORD dwID, BOOL bMainFrame) = 0;
	virtual BOOL OnNavigateError(DWORD dwID, BOOL bMainFrame, const tchar* szUrl) = 0;
	virtual void OnProgressChange(DWORD dwID, long lCurrent, long lMax) = 0;
	virtual BOOL OnBeforeNavigate(DWORD dwID, const tchar* pszURL) = 0;
	virtual void OnNavigateComplete(DWORD dwID, const tchar* pszURL) = 0;
	virtual void OnDownloadBegin(DWORD dwID) = 0;
};

class CWTLIExplorer : public CWTLAxControl<CWTLIExplorer,IWebBrowser2>, public FDocument
{
private:
	IHttpNotify* m_pNotify; 
	DWORD		m_dwID;
public:
	CWTLIExplorer()
	{
        m_pNotify = NULL; 
		m_dwID = 0; 
	}
	virtual ~CWTLIExplorer()
	{
		CAxWindow::SetExternalDispatch((IDispatch*) NULL);
		CAxWindow::SetExternalUIHandler((IDocHostUIHandlerDispatch*) NULL);
	}

	BEGIN_SINK_MAP( CWTLIExplorer )
		SINK_ENTRY(0, DISPID_DOCUMENTCOMPLETE,  event_OnDocumentComplete)
		SINK_ENTRY(0, DISPID_NAVIGATECOMPLETE2, event_OnNavigateComplete) 
		SINK_ENTRY(0, DISPID_NAVIGATEERROR,     event_OnNavigateError)      
        SINK_ENTRY(0, DISPID_PROGRESSCHANGE,    event_OnProgressChange)
		SINK_ENTRY(0, DISPID_BEFORENAVIGATE2,	event_OnBeforeNavigate2)
		SINK_ENTRY(0, DISPID_DOWNLOADBEGIN,		event_OnDownloadBegin) 
		SINK_ENTRY(0, DISPID_NEWWINDOW2,		event_OnNewWindow2)
		SINK_ENTRY(0, DISPID_NEWWINDOW3,		event_OnNewWindow3)

	END_SINK_MAP()

    EVENTFN event_OnDocumentComplete( IDispatch* pDisp,  VARIANT* URL );
    EVENTFN	event_OnNavigateError(IDispatch* pDisp, VARIANT *URL, VARIANT *TargetFrameName, VARIANT *StatusCode, VARIANT_BOOL *Cancel);
	EVENTFN event_OnNavigateComplete( IDispatch* pDisp,  VARIANT* URL );
    EVENTFN event_OnProgressChange(long vtMax, long vtCur); 
	EVENTFN event_OnBeforeNavigate2(IDispatch *pDisp, VARIANT *URL, VARIANT *Flags, VARIANT *TargetFrameName, VARIANT *PostData, VARIANT *Headers, VARIANT_BOOL *Cancel );
	EVENTFN event_OnDownloadBegin(); 
	EVENTFN event_OnNewWindow2(IDispatch **ppDisp, VARIANT_BOOL *Cancel);
	EVENTFN event_OnNewWindow3(IDispatch **ppDisp, VARIANT_BOOL *Cancel, DWORD dwFlags,	BSTR bstrUrlContext, BSTR bstrUrl);

	HRESULT Navigate(const tchar* pszPath, const tchar* pAdditionalHeaders="", DWORD dwNavFlags = navNoHistory);
	HRESULT SetObjectHandler( INT iType, LPCTSTR objectName,  DWORD pMember, DWORD pThis);
	void	SetNotify(IHttpNotify* pNotify, DWORD dwBrowserID)
	{
		m_pNotify = pNotify; 
		m_dwID = dwBrowserID; 
	}
	inline DWORD GetId(){return m_dwID;}
	HRESULT SetBodyFocus();
};




