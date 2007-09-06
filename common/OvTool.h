/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Mon Oct 17 14:29:25 2005
 */
/* Compiler settings for C:\Source\GDCL\Tools\OvTool\OvTool.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifndef __OvTool_h__
#define __OvTool_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IOvMgr_FWD_DEFINED__
#define __IOvMgr_FWD_DEFINED__
typedef interface IOvMgr IOvMgr;
#endif 	/* __IOvMgr_FWD_DEFINED__ */


#ifndef __IOvMgr2_FWD_DEFINED__
#define __IOvMgr2_FWD_DEFINED__
typedef interface IOvMgr2 IOvMgr2;
#endif 	/* __IOvMgr2_FWD_DEFINED__ */


#ifndef __OvMgr_FWD_DEFINED__
#define __OvMgr_FWD_DEFINED__

#ifdef __cplusplus
typedef class OvMgr OvMgr;
#else
typedef struct OvMgr OvMgr;
#endif /* __cplusplus */

#endif 	/* __OvMgr_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __IOvMgr_INTERFACE_DEFINED__
#define __IOvMgr_INTERFACE_DEFINED__

/* interface IOvMgr */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IOvMgr;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("C4396010-9285-47C9-B8EA-D61481F2D7EC")
    IOvMgr : public IDispatch
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetGraph( 
            /* [in] */ IUnknown __RPC_FAR *pGraph) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetHDC( 
            /* [retval][out] */ long __RPC_FAR *pHDC) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE ReleaseDC( 
            long hDC) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_ColourKey( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSize( 
            /* [out] */ long __RPC_FAR *plWidth,
            /* [out] */ long __RPC_FAR *plHeight) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOvMgrVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IOvMgr __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IOvMgr __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IOvMgr __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IOvMgr __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IOvMgr __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IOvMgr __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IOvMgr __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetGraph )( 
            IOvMgr __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pGraph);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHDC )( 
            IOvMgr __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pHDC);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseDC )( 
            IOvMgr __RPC_FAR * This,
            long hDC);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ColourKey )( 
            IOvMgr __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            IOvMgr __RPC_FAR * This,
            /* [out] */ long __RPC_FAR *plWidth,
            /* [out] */ long __RPC_FAR *plHeight);
        
        END_INTERFACE
    } IOvMgrVtbl;

    interface IOvMgr
    {
        CONST_VTBL struct IOvMgrVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOvMgr_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOvMgr_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOvMgr_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOvMgr_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IOvMgr_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IOvMgr_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IOvMgr_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IOvMgr_SetGraph(This,pGraph)	\
    (This)->lpVtbl -> SetGraph(This,pGraph)

#define IOvMgr_GetHDC(This,pHDC)	\
    (This)->lpVtbl -> GetHDC(This,pHDC)

#define IOvMgr_ReleaseDC(This,hDC)	\
    (This)->lpVtbl -> ReleaseDC(This,hDC)

#define IOvMgr_get_ColourKey(This,pVal)	\
    (This)->lpVtbl -> get_ColourKey(This,pVal)

#define IOvMgr_GetSize(This,plWidth,plHeight)	\
    (This)->lpVtbl -> GetSize(This,plWidth,plHeight)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IOvMgr_SetGraph_Proxy( 
    IOvMgr __RPC_FAR * This,
    /* [in] */ IUnknown __RPC_FAR *pGraph);


void __RPC_STUB IOvMgr_SetGraph_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IOvMgr_GetHDC_Proxy( 
    IOvMgr __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pHDC);


void __RPC_STUB IOvMgr_GetHDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IOvMgr_ReleaseDC_Proxy( 
    IOvMgr __RPC_FAR * This,
    long hDC);


void __RPC_STUB IOvMgr_ReleaseDC_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IOvMgr_get_ColourKey_Proxy( 
    IOvMgr __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IOvMgr_get_ColourKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IOvMgr_GetSize_Proxy( 
    IOvMgr __RPC_FAR * This,
    /* [out] */ long __RPC_FAR *plWidth,
    /* [out] */ long __RPC_FAR *plHeight);


void __RPC_STUB IOvMgr_GetSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOvMgr_INTERFACE_DEFINED__ */


#ifndef __IOvMgr2_INTERFACE_DEFINED__
#define __IOvMgr2_INTERFACE_DEFINED__

/* interface IOvMgr2 */
/* [unique][helpstring][uuid][dual][object] */ 


EXTERN_C const IID IID_IOvMgr2;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("38D31DC3-DA77-4752-B169-1B4521062EA7")
    IOvMgr2 : public IOvMgr
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetOverlayImageSize( 
            /* [in] */ long Width,
            /* [in] */ long Height) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetOverlayImageSize( 
            /* [out] */ long __RPC_FAR *Width,
            /* [out] */ long __RPC_FAR *Height) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_AlphaBlending( 
            /* [in] */ float Alpha) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_AlphaBlending( 
            /* [retval][out] */ float __RPC_FAR *Alpha) = 0;
        
        virtual /* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE put_VMRColourKey( 
            /* [in] */ long newVal) = 0;
        
        virtual /* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE get_VMRColourKey( 
            /* [retval][out] */ long __RPC_FAR *pVal) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IOvMgr2Vtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IOvMgr2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IOvMgr2 __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IOvMgr2 __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IOvMgr2 __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IOvMgr2 __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IOvMgr2 __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IOvMgr2 __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetGraph )( 
            IOvMgr2 __RPC_FAR * This,
            /* [in] */ IUnknown __RPC_FAR *pGraph);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetHDC )( 
            IOvMgr2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pHDC);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *ReleaseDC )( 
            IOvMgr2 __RPC_FAR * This,
            long hDC);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_ColourKey )( 
            IOvMgr2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetSize )( 
            IOvMgr2 __RPC_FAR * This,
            /* [out] */ long __RPC_FAR *plWidth,
            /* [out] */ long __RPC_FAR *plHeight);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *SetOverlayImageSize )( 
            IOvMgr2 __RPC_FAR * This,
            /* [in] */ long Width,
            /* [in] */ long Height);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetOverlayImageSize )( 
            IOvMgr2 __RPC_FAR * This,
            /* [out] */ long __RPC_FAR *Width,
            /* [out] */ long __RPC_FAR *Height);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_AlphaBlending )( 
            IOvMgr2 __RPC_FAR * This,
            /* [in] */ float Alpha);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_AlphaBlending )( 
            IOvMgr2 __RPC_FAR * This,
            /* [retval][out] */ float __RPC_FAR *Alpha);
        
        /* [helpstring][id][propput] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *put_VMRColourKey )( 
            IOvMgr2 __RPC_FAR * This,
            /* [in] */ long newVal);
        
        /* [helpstring][id][propget] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *get_VMRColourKey )( 
            IOvMgr2 __RPC_FAR * This,
            /* [retval][out] */ long __RPC_FAR *pVal);
        
        END_INTERFACE
    } IOvMgr2Vtbl;

    interface IOvMgr2
    {
        CONST_VTBL struct IOvMgr2Vtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IOvMgr2_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IOvMgr2_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IOvMgr2_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IOvMgr2_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IOvMgr2_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IOvMgr2_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IOvMgr2_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#define IOvMgr2_SetGraph(This,pGraph)	\
    (This)->lpVtbl -> SetGraph(This,pGraph)

#define IOvMgr2_GetHDC(This,pHDC)	\
    (This)->lpVtbl -> GetHDC(This,pHDC)

#define IOvMgr2_ReleaseDC(This,hDC)	\
    (This)->lpVtbl -> ReleaseDC(This,hDC)

#define IOvMgr2_get_ColourKey(This,pVal)	\
    (This)->lpVtbl -> get_ColourKey(This,pVal)

#define IOvMgr2_GetSize(This,plWidth,plHeight)	\
    (This)->lpVtbl -> GetSize(This,plWidth,plHeight)


#define IOvMgr2_SetOverlayImageSize(This,Width,Height)	\
    (This)->lpVtbl -> SetOverlayImageSize(This,Width,Height)

#define IOvMgr2_GetOverlayImageSize(This,Width,Height)	\
    (This)->lpVtbl -> GetOverlayImageSize(This,Width,Height)

#define IOvMgr2_put_AlphaBlending(This,Alpha)	\
    (This)->lpVtbl -> put_AlphaBlending(This,Alpha)

#define IOvMgr2_get_AlphaBlending(This,Alpha)	\
    (This)->lpVtbl -> get_AlphaBlending(This,Alpha)

#define IOvMgr2_put_VMRColourKey(This,newVal)	\
    (This)->lpVtbl -> put_VMRColourKey(This,newVal)

#define IOvMgr2_get_VMRColourKey(This,pVal)	\
    (This)->lpVtbl -> get_VMRColourKey(This,pVal)

#endif /* COBJMACROS */


#endif 	/* C style interface */



/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IOvMgr2_SetOverlayImageSize_Proxy( 
    IOvMgr2 __RPC_FAR * This,
    /* [in] */ long Width,
    /* [in] */ long Height);


void __RPC_STUB IOvMgr2_SetOverlayImageSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id] */ HRESULT STDMETHODCALLTYPE IOvMgr2_GetOverlayImageSize_Proxy( 
    IOvMgr2 __RPC_FAR * This,
    /* [out] */ long __RPC_FAR *Width,
    /* [out] */ long __RPC_FAR *Height);


void __RPC_STUB IOvMgr2_GetOverlayImageSize_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IOvMgr2_put_AlphaBlending_Proxy( 
    IOvMgr2 __RPC_FAR * This,
    /* [in] */ float Alpha);


void __RPC_STUB IOvMgr2_put_AlphaBlending_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IOvMgr2_get_AlphaBlending_Proxy( 
    IOvMgr2 __RPC_FAR * This,
    /* [retval][out] */ float __RPC_FAR *Alpha);


void __RPC_STUB IOvMgr2_get_AlphaBlending_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propput] */ HRESULT STDMETHODCALLTYPE IOvMgr2_put_VMRColourKey_Proxy( 
    IOvMgr2 __RPC_FAR * This,
    /* [in] */ long newVal);


void __RPC_STUB IOvMgr2_put_VMRColourKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


/* [helpstring][id][propget] */ HRESULT STDMETHODCALLTYPE IOvMgr2_get_VMRColourKey_Proxy( 
    IOvMgr2 __RPC_FAR * This,
    /* [retval][out] */ long __RPC_FAR *pVal);


void __RPC_STUB IOvMgr2_get_VMRColourKey_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IOvMgr2_INTERFACE_DEFINED__ */



#ifndef __OVTOOLLib_LIBRARY_DEFINED__
#define __OVTOOLLib_LIBRARY_DEFINED__

/* library OVTOOLLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_OVTOOLLib;

EXTERN_C const CLSID CLSID_OvMgr;

#ifdef __cplusplus

class DECLSPEC_UUID("DEFC92B5-D996-4EF0-82AC-5C1110F67BCD")
OvMgr;
#endif
#endif /* __OVTOOLLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
