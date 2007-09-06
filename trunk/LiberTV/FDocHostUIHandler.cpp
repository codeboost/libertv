// FDocHostUIHandler.cpp : Implementation of CFDocHostUIHandler

#include "stdafx.h"
#include "FDocHostUIHandler.h"


// CFDocHostUIHandler


STDMETHODIMP CFDocHostUIHandler::RSSDownloadItem(BSTR bstrItemGuid)
{
	// TODO: Add your implementation code here

	return S_OK;
}

STDMETHODIMP CFDocHostUIHandler::Exec (const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut)
{

	return E_NOTIMPL; 
}

STDMETHODIMP CFDocHostUIHandler::QueryStatus(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD *prgCmds, OLECMDTEXT *pCmdText)
{
	return E_NOTIMPL; 
}
