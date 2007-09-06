// FHtmlEventHandler.h : Declaration of the CFHtmlEventHandler

#pragma once
#include "resource.h"       // main symbols

#include "Liber2Player.h"


// CFHtmlEventHandler

class ATL_NO_VTABLE CFHtmlEventHandler : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFHtmlEventHandler, &CLSID_FHtmlEventHandler>,
	public IDispatchImpl<IFHtmlEventHandler, &IID_IFHtmlEventHandler, &LIBID_Liber2PlayerLib, /*wMajor =*/ 1, /*wMinor =*/ 0>
{

    DWORD m_pThis;
    DWORD m_pMember;
public:
	CFHtmlEventHandler()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FHTMLEVENTHANDLER)


BEGIN_COM_MAP(CFHtmlEventHandler)
	COM_INTERFACE_ENTRY(IFHtmlEventHandler)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

public:

    STDMETHOD(SetHandler)(LONG pThis, LONG pMember);
    STDMETHOD(CallHandler)(void);

};

//OBJECT_ENTRY_AUTO(__uuidof(FHtmlEventHandler), CFHtmlEventHandler)
