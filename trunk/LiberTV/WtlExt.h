// WtlExt.h : WTL Extensions/Helpers
// Copyright: Jesús Salas 2002 ( jesuspsalas@hotmail.com )
// License  : none, feel free to use it.
/////////////////////////////////////////////////////////////////////////////


//http://support.microsoft.com/default.aspx?scid=kb;en-us;274202

#pragma once


template <class T,class Interface>
class CWTLDispEventHelper : public IDispEventImpl<0,T>
{
	public:
		CComPtr<IUnknown> m_pUnk;
		HRESULT EasyAdvise(IUnknown* pUnk) 
		{  
			m_pUnk = pUnk;
			AtlGetObjectSourceInterface(pUnk,&m_libid, &m_iid, &m_wMajorVerNum, &m_wMinorVerNum);
			return DispEventAdvise(pUnk, &m_iid);
		}
		HRESULT EasyUnadvise() 
		{ 
			HRESULT hr = S_OK; 
			if (m_pUnk)
			{
				AtlGetObjectSourceInterface(m_pUnk,&m_libid, &m_iid, &m_wMajorVerNum, &m_wMinorVerNum);
				hr =  DispEventUnadvise(m_pUnk, &m_iid);
				m_pUnk.Release(); 
			}
			return hr; 
	  }

};


template <class T, class Interface>
class CWTLAxControl :	public CComPtr<Interface>,
						public CWindowImpl<CWTLAxControl<T, Interface>,CAxWindow>, 
						public CWTLDispEventHelper<T,Interface>					
{
	public:

      
	BEGIN_MSG_MAP(CWTLAxControl)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
	END_MSG_MAP()

	virtual ~CWTLAxControl(){
	}

    CComQIPtr<IWebBrowser2> m_spWB; 
	LRESULT OnCreate( UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL & bHandled )
	{
		LRESULT lRet;
		// We must call DefWindowProc before we can attach to the control.
		lRet = DefWindowProc( uMsg, wParam,lParam );
		// Get the Pointer to the control with Events (true)
		AttachControl(true);

		//Hide context menu
		CComPtr<IUnknown> spUnk;					
		AtlAxGetHost(m_hWnd, &spUnk);				
		CComQIPtr<IAxWinAmbientDispatch> spWinAmb(spUnk);		
		if (spWinAmb)
		{
			VARIANT_BOOL bAllowInteraction = VARIANT_FALSE; 
			
			spWinAmb->put_AllowContextMenu(VARIANT_TRUE);	
			spWinAmb->put_AllowShowUI(bAllowInteraction); 
		}
		return lRet;
	}
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam , LPARAM lParam, BOOL & bHandled)
    {
        EasyUnadvise();
        return 0; 
    }


	HRESULT AttachControl( BOOL bWithEvents = false ) 
	{
		HRESULT hr = S_OK;
		CComPtr<IUnknown> spUnk;
		// Get the IUnknown interface of control
		hr |= AtlAxGetControl( m_hWnd, &spUnk);

		if (SUCCEEDED(hr))

            if (spUnk)
            {
                m_spWB = spUnk;
				// Query our interface
			    hr |= spUnk->QueryInterface( __uuidof(Interface), (void**) (CComPtr<Interface>*)this);
			    hr|= EasyAdvise( spUnk );
            }

		return hr;
	}

};


