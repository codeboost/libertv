// FHtmlEventHandler.cpp : Implementation of CFHtmlEventHandler

#include "stdafx.h"
#include "FHtmlEventHandler.h"
#include ".\fhtmleventhandler.h"


// CFHtmlEventHandler

STDMETHODIMP CFHtmlEventHandler::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID* arr[] = 
    {
        &IID_IFHtmlEventHandler
    };

    for (int i=0; i < sizeof(arr) / sizeof(arr[0]); i++)
    {
        if (::InlineIsEqualGUID(*arr[i],riid))
            return S_OK;
    }
    return S_FALSE;
}

STDMETHODIMP CFHtmlEventHandler::SetHandler(LONG pThis, LONG pMember)
{
    m_pThis   = pThis;
    m_pMember = pMember;
    return S_OK;
}

STDMETHODIMP CFHtmlEventHandler::CallHandler(void)
{
    // a simple __thiscall asm implementation
    DWORD pMember;
    DWORD pThis;
    pThis   = m_pThis;
    pMember = m_pMember;
    BOOL bHandled=false;
    _asm
    {
        lea eax,[bHandled]
        push eax
        mov ecx,[pThis]
        call pMember
    }
    return S_OK;
}
