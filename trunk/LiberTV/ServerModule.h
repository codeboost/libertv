#pragma once
using namespace ATL;


class CMyServerAppModule : public CServerAppModule 
{ 
public:
	CComPtr<IDispatch>		m_LibraryObject;
	CComPtr<IDispatch>		m_UIHandler; 

	CMyServerAppModule(){}
	HRESULT InitObjectLibray( void );
	HRESULT DeInitObjectLibrary( void );
};

extern CMyServerAppModule _Module;