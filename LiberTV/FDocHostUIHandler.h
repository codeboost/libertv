// FDocHostUIHandler.h : Declaration of the CFDocHostUIHandler

#pragma once
#include "resource.h"       // main symbols

#include "Liber2Player.h"
#include "FDocHost.h"


// CFDocHostUIHandler

class ATL_NO_VTABLE CFDocHostUIHandler : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CFDocHostUIHandler, &CLSID_FDocHostUIHandler>,
	public IDispatchImpl<IFDocHostUIHandler, &IID_IFDocHostUIHandler, &LIBID_Liber2PlayerLib, /*wMajor =*/ 1, /*wMinor =*/ 0>, 
	public IDispatchImpl<CDocHostUIHandler, &IID_IDocHostUIHandlerDispatch, &LIBID_ATLLib>, 
	public IOleCommandTarget
{
public:
	CFDocHostUIHandler()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_FDOCHOSTUIHANDLER)


BEGIN_COM_MAP(CFDocHostUIHandler)
	COM_INTERFACE_ENTRY2(IDispatch, IFDocHostUIHandler)
	COM_INTERFACE_ENTRY(IFDocHostUIHandler)
	COM_INTERFACE_ENTRY(IDocHostUIHandlerDispatch)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
	}

public:

	//IOleCommandTarget
	STDMETHOD(QueryStatus)(const GUID *pguidCmdGroup,// Pointer to command group
		ULONG cCmds,          // Number of commands in prgCmds array
		OLECMD *prgCmds,      // Array of commands
		OLECMDTEXT *pCmdText  // Pointer to name or status of command
		);
	STDMETHOD(Exec)( 
		const GUID *pguidCmdGroup,  // Pointer to command group
		DWORD nCmdID,               // Identifier of command to execute
		DWORD nCmdExecOpt,          // Options for executing the command
		VARIANTARG *pvaIn,          // Pointer to input arguments
		VARIANTARG *pvaOut          // Pointer to command output
		);
public:
	STDMETHOD(RSSDownloadItem)(BSTR bstrItemGuid);
};

OBJECT_ENTRY_AUTO(__uuidof(FDocHostUIHandler), CFDocHostUIHandler)
