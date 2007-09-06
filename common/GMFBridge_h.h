

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Mon Jan 23 01:26:07 2006
 */
/* Compiler settings for .\GMFBridge.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __GMFBridge_h_h__
#define __GMFBridge_h_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IGMFBridgeController_FWD_DEFINED__
#define __IGMFBridgeController_FWD_DEFINED__
typedef interface IGMFBridgeController IGMFBridgeController;
#endif 	/* __IGMFBridgeController_FWD_DEFINED__ */


#ifndef __GMFBridgeController_FWD_DEFINED__
#define __GMFBridgeController_FWD_DEFINED__

#ifdef __cplusplus
typedef class GMFBridgeController GMFBridgeController;
#else
typedef struct GMFBridgeController GMFBridgeController;
#endif /* __cplusplus */

#endif 	/* __GMFBridgeController_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

/* interface __MIDL_itf_GMFBridge_0000 */
/* [local] */ 

typedef /* [public][public] */ 
enum __MIDL___MIDL_itf_GMFBridge_0000_0001
    {	eUncompressed	= 0,
	eMuxInputs	= eUncompressed + 1,
	eAny	= eMuxInputs + 1
    } 	eFormatType;



extern RPC_IF_HANDLE __MIDL_itf_GMFBridge_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_GMFBridge_0000_v0_0_s_ifspec;

#ifndef __IGMFBridgeController_INTERFACE_DEFINED__
#define __IGMFBridgeController_INTERFACE_DEFINED__

/* interface IGMFBridgeController */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IGMFBridgeController;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("8C4D8054-FCBA-4783-865A-7E8B3C814011")
    IGMFBridgeController : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE AddStream( 
            BOOL bVideo,
            eFormatType AllowedTypes,
            BOOL bDiscardUnconnected) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE InsertSinkFilter( 
            /* [in] */ IUnknown *pGraph,
            /* [retval][out] */ IUnknown **ppFilter) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE InsertSourceFilter( 
            /* [in] */ IUnknown *pUnkSourceGraphSinkFilter,
            /* [in] */ IUnknown *pRenderGraph,
            /* [retval][out] */ IUnknown **ppFilter) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateSourceGraph( 
            /* [in] */ BSTR strFile,
            /* [in] */ IUnknown *pGraph,
            /* [out] */ IUnknown **pSinkFilter) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE CreateRenderGraph( 
            /* [in] */ IUnknown *pSourceGraphSinkFilter,
            /* [in] */ IUnknown *pRenderGraph,
            /* [out] */ IUnknown **pRenderGraphSourceFilter) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE BridgeGraphs( 
            /* [in] */ IUnknown *pSourceGraphSinkFilter,
            /* [in] */ IUnknown *pRenderGraphSourceFilter) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetNotify( 
            /* [in] */ LONG_PTR hwnd,
            /* [in] */ long msg) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetBufferMinimum( 
            /* [in] */ long nMillisecs) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSegmentTime( 
            /* [retval][out] */ double *pdSeconds) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE NoMoreSegments( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSegmentOffset( 
            /* [retval][out] */ double *pdOffset) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IGMFBridgeControllerVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IGMFBridgeController * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IGMFBridgeController * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IGMFBridgeController * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IGMFBridgeController * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IGMFBridgeController * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IGMFBridgeController * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IGMFBridgeController * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *AddStream )( 
            IGMFBridgeController * This,
            BOOL bVideo,
            eFormatType AllowedTypes,
            BOOL bDiscardUnconnected);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *InsertSinkFilter )( 
            IGMFBridgeController * This,
            /* [in] */ IUnknown *pGraph,
            /* [retval][out] */ IUnknown **ppFilter);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *InsertSourceFilter )( 
            IGMFBridgeController * This,
            /* [in] */ IUnknown *pUnkSourceGraphSinkFilter,
            /* [in] */ IUnknown *pRenderGraph,
            /* [retval][out] */ IUnknown **ppFilter);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CreateSourceGraph )( 
            IGMFBridgeController * This,
            /* [in] */ BSTR strFile,
            /* [in] */ IUnknown *pGraph,
            /* [out] */ IUnknown **pSinkFilter);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *CreateRenderGraph )( 
            IGMFBridgeController * This,
            /* [in] */ IUnknown *pSourceGraphSinkFilter,
            /* [in] */ IUnknown *pRenderGraph,
            /* [out] */ IUnknown **pRenderGraphSourceFilter);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *BridgeGraphs )( 
            IGMFBridgeController * This,
            /* [in] */ IUnknown *pSourceGraphSinkFilter,
            /* [in] */ IUnknown *pRenderGraphSourceFilter);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetNotify )( 
            IGMFBridgeController * This,
            /* [in] */ LONG_PTR hwnd,
            /* [in] */ long msg);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetBufferMinimum )( 
            IGMFBridgeController * This,
            /* [in] */ long nMillisecs);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSegmentTime )( 
            IGMFBridgeController * This,
            /* [retval][out] */ double *pdSeconds);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *NoMoreSegments )( 
            IGMFBridgeController * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSegmentOffset )( 
            IGMFBridgeController * This,
            /* [retval][out] */ double *pdOffset);
        
        END_INTERFACE
    } IGMFBridgeControllerVtbl;

    interface IGMFBridgeController
    {
        CONST_VTBL struct IGMFBridgeControllerVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IGMFBridgeController_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IGMFBridgeController_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IGMFBridgeController_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IGMFBridgeController_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IGMFBridgeController_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IGMFBridgeController_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IGMFBridgeController_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IGMFBridgeController_AddStream(This,bVideo,AllowedTypes,bDiscardUnconnected)	\
    (This)->lpVtbl -> AddStream(This,bVideo,AllowedTypes,bDiscardUnconnected)

#define IGMFBridgeController_InsertSinkFilter(This,pGraph,ppFilter)	\
    (This)->lpVtbl -> InsertSinkFilter(This,pGraph,ppFilter)

#define IGMFBridgeController_InsertSourceFilter(This,pUnkSourceGraphSinkFilter,pRenderGraph,ppFilter)	\
    (This)->lpVtbl -> InsertSourceFilter(This,pUnkSourceGraphSinkFilter,pRenderGraph,ppFilter)

#define IGMFBridgeController_CreateSourceGraph(This,strFile,pGraph,pSinkFilter)	\
    (This)->lpVtbl -> CreateSourceGraph(This,strFile,pGraph,pSinkFilter)

#define IGMFBridgeController_CreateRenderGraph(This,pSourceGraphSinkFilter,pRenderGraph,pRenderGraphSourceFilter)	\
    (This)->lpVtbl -> CreateRenderGraph(This,pSourceGraphSinkFilter,pRenderGraph,pRenderGraphSourceFilter)

#define IGMFBridgeController_BridgeGraphs(This,pSourceGraphSinkFilter,pRenderGraphSourceFilter)	\
    (This)->lpVtbl -> BridgeGraphs(This,pSourceGraphSinkFilter,pRenderGraphSourceFilter)

#define IGMFBridgeController_SetNotify(This,hwnd,msg)	\
    (This)->lpVtbl -> SetNotify(This,hwnd,msg)

#define IGMFBridgeController_SetBufferMinimum(This,nMillisecs)	\
    (This)->lpVtbl -> SetBufferMinimum(This,nMillisecs)

#define IGMFBridgeController_GetSegmentTime(This,pdSeconds)	\
    (This)->lpVtbl -> GetSegmentTime(This,pdSeconds)

#define IGMFBridgeController_NoMoreSegments(This)	\
    (This)->lpVtbl -> NoMoreSegments(This)

#define IGMFBridgeController_GetSegmentOffset(This,pdOffset)	\
    (This)->lpVtbl -> GetSegmentOffset(This,pdOffset)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGMFBridgeController_AddStream_Proxy( 
    IGMFBridgeController * This,
    BOOL bVideo,
    eFormatType AllowedTypes,
    BOOL bDiscardUnconnected);


void __RPC_STUB IGMFBridgeController_AddStream_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGMFBridgeController_InsertSinkFilter_Proxy( 
    IGMFBridgeController * This,
    /* [in] */ IUnknown *pGraph,
    /* [retval][out] */ IUnknown **ppFilter);


void __RPC_STUB IGMFBridgeController_InsertSinkFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGMFBridgeController_InsertSourceFilter_Proxy( 
    IGMFBridgeController * This,
    /* [in] */ IUnknown *pUnkSourceGraphSinkFilter,
    /* [in] */ IUnknown *pRenderGraph,
    /* [retval][out] */ IUnknown **ppFilter);


void __RPC_STUB IGMFBridgeController_InsertSourceFilter_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGMFBridgeController_CreateSourceGraph_Proxy( 
    IGMFBridgeController * This,
    /* [in] */ BSTR strFile,
    /* [in] */ IUnknown *pGraph,
    /* [out] */ IUnknown **pSinkFilter);


void __RPC_STUB IGMFBridgeController_CreateSourceGraph_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGMFBridgeController_CreateRenderGraph_Proxy( 
    IGMFBridgeController * This,
    /* [in] */ IUnknown *pSourceGraphSinkFilter,
    /* [in] */ IUnknown *pRenderGraph,
    /* [out] */ IUnknown **pRenderGraphSourceFilter);


void __RPC_STUB IGMFBridgeController_CreateRenderGraph_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGMFBridgeController_BridgeGraphs_Proxy( 
    IGMFBridgeController * This,
    /* [in] */ IUnknown *pSourceGraphSinkFilter,
    /* [in] */ IUnknown *pRenderGraphSourceFilter);


void __RPC_STUB IGMFBridgeController_BridgeGraphs_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGMFBridgeController_SetNotify_Proxy( 
    IGMFBridgeController * This,
    /* [in] */ LONG_PTR hwnd,
    /* [in] */ long msg);


void __RPC_STUB IGMFBridgeController_SetNotify_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGMFBridgeController_SetBufferMinimum_Proxy( 
    IGMFBridgeController * This,
    /* [in] */ long nMillisecs);


void __RPC_STUB IGMFBridgeController_SetBufferMinimum_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGMFBridgeController_GetSegmentTime_Proxy( 
    IGMFBridgeController * This,
    /* [retval][out] */ double *pdSeconds);


void __RPC_STUB IGMFBridgeController_GetSegmentTime_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGMFBridgeController_NoMoreSegments_Proxy( 
    IGMFBridgeController * This);


void __RPC_STUB IGMFBridgeController_NoMoreSegments_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IGMFBridgeController_GetSegmentOffset_Proxy( 
    IGMFBridgeController * This,
    /* [retval][out] */ double *pdOffset);


void __RPC_STUB IGMFBridgeController_GetSegmentOffset_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IGMFBridgeController_INTERFACE_DEFINED__ */



#ifndef __GMFBridgeLib_LIBRARY_DEFINED__
#define __GMFBridgeLib_LIBRARY_DEFINED__

/* library GMFBridgeLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_GMFBridgeLib;

EXTERN_C const CLSID CLSID_GMFBridgeController;

#ifdef __cplusplus

class DECLSPEC_UUID("08E3287F-3A5C-47e9-8179-A9E9221A5CDE")
GMFBridgeController;
#endif
#endif /* __GMFBridgeLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


