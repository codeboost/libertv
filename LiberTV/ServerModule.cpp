#include "stdafx.h"
#include "ServerModule.h"

HRESULT CMyServerAppModule::InitObjectLibray( void )
{
	HRESULT hr = S_OK;

	hr = m_LibraryObject.CoCreateInstance( _bstr_t("Liber2Player.FHtmlEventDispatcher.1") );

	if ( FAILED( hr ) )
	{
		FString StrErr; 
		StrErr.Format("Unable to initialize internal COM library.\n\nPlease re-install.\n\nError Code: 0x%x", hr); 
		MessageBox( 0, StrErr,"Critital", MB_ICONERROR);
		return hr; 
	}

	hr = m_UIHandler.CoCreateInstance(_bstr_t("Liber2Player.FDocHostUIHandler.1"));
	if (FAILED(hr))
	{
		FString StrErr; 
		StrErr.Format("Unable to create DocHostUIHandler\n\nPlease re-install.\n\nError Code: 0x%x", hr);
		MessageBox(0, StrErr, "Critical", MB_ICONERROR);
	}
	return hr;
}

HRESULT CMyServerAppModule::DeInitObjectLibrary( void )
{
	if ( m_LibraryObject )
		m_LibraryObject.Release();
	if (m_UIHandler)
		m_UIHandler.Release(); 
	return S_OK;
}