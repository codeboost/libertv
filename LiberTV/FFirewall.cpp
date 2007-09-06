#include "stdafx.h"
#include "FFirewall.h"
#include "Utils.h"
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x500

#include <objbase.h>
#include <oleauto.h>
#include <netfw.h>
#include <Natupnp.h>
#include <Iphlpapi.h>


HRESULT WindowsFirewallInitialize(OUT INetFwProfile** fwProfile)
{
	HRESULT hr = S_OK;
	INetFwMgr* fwMgr = NULL;
	INetFwPolicy* fwPolicy = NULL;

	_ASSERT(fwProfile != NULL);

	*fwProfile = NULL;

	// Create an instance of the firewall settings manager.
	hr = CoCreateInstance(
		__uuidof(NetFwMgr),
		NULL,
		CLSCTX_INPROC_SERVER,
		__uuidof(INetFwMgr),
		(void**)&fwMgr
		);
	if (FAILED(hr))
	{
		_DBGAlert("CoCreateInstance failed: 0x%08lx\n", hr);
		goto error;
	}

	// Retrieve the local firewall policy.
	hr = fwMgr->get_LocalPolicy(&fwPolicy);
	if (FAILED(hr))
	{
		_DBGAlert("get_LocalPolicy failed: 0x%08lx\n", hr);
		goto error;
	}

	// Retrieve the firewall profile currently in effect.
	hr = fwPolicy->get_CurrentProfile(fwProfile);
	if (FAILED(hr))
	{
		_DBGAlert("get_CurrentProfile failed: 0x%08lx\n", hr);
		goto error;
	}

error:

	// Release the local firewall policy.
	if (fwPolicy != NULL)
	{
		fwPolicy->Release();
	}

	// Release the firewall settings manager.
	if (fwMgr != NULL)
	{
		fwMgr->Release();
	}

	return hr;
}

void WindowsFirewallCleanup(IN INetFwProfile* fwProfile)
{
	// Release the firewall profile.
	if (fwProfile != NULL)
	{
		fwProfile->Release();
	}
}

HRESULT WindowsFirewallAppIsEnabled(
									IN INetFwProfile* fwProfile,
									IN const wchar_t* fwProcessImageFileName,
									OUT BOOL* fwAppEnabled
									)
{
	HRESULT hr = S_OK;
	BSTR fwBstrProcessImageFileName = NULL;
	VARIANT_BOOL fwEnabled;
	INetFwAuthorizedApplication* fwApp = NULL;
	INetFwAuthorizedApplications* fwApps = NULL;

	_ASSERT(fwProfile != NULL);
	_ASSERT(fwProcessImageFileName != NULL);
	_ASSERT(fwAppEnabled != NULL);

	*fwAppEnabled = FALSE;

	// Retrieve the authorized application collection.
	hr = fwProfile->get_AuthorizedApplications(&fwApps);
	if (FAILED(hr))
	{
		_DBGAlert("get_AuthorizedApplications failed: 0x%08lx\n", hr);
		goto error;
	}

	// Allocate a BSTR for the process image file name.
	fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
	if (SysStringLen(fwBstrProcessImageFileName) == 0)
	{
		hr = E_OUTOFMEMORY;
		_DBGAlert("SysAllocString failed: 0x%08lx\n", hr);
		goto error;
	}

	// Attempt to retrieve the authorized application.
	hr = fwApps->Item(fwBstrProcessImageFileName, &fwApp);
	if (SUCCEEDED(hr))
	{
		// Find out if the authorized application is enabled.
		hr = fwApp->get_Enabled(&fwEnabled);
		if (FAILED(hr))
		{
			_DBGAlert("get_Enabled failed: 0x%08lx\n", hr);
			goto error;
		}

		if (fwEnabled != VARIANT_FALSE)
		{
			// The authorized application is enabled.
			*fwAppEnabled = TRUE;

			_DBGAlert(
				"Authorized application %lS is enabled in the firewall.\n",
				fwProcessImageFileName
				);
		}
		else
		{
			_DBGAlert(
				"Authorized application %lS is disabled in the firewall.\n",
				fwProcessImageFileName
				);
		}
	}
	else
	{
		// The authorized application was not in the collection.
		hr = S_OK;

		_DBGAlert(
			"Authorized application %lS is disabled in the firewall.\n",
			fwProcessImageFileName
			);
	}

error:

	// Free the BSTR.
	SysFreeString(fwBstrProcessImageFileName);

	// Release the authorized application instance.
	if (fwApp != NULL)
	{
		fwApp->Release();
	}

	// Release the authorized application collection.
	if (fwApps != NULL)
	{
		fwApps->Release();
	}

	return hr;
}


HRESULT WindowsFirewallAddApp(
							  IN INetFwProfile* fwProfile,
							  IN const wchar_t* fwProcessImageFileName,
							  IN const wchar_t* fwName
							  )
{
	HRESULT hr = S_OK;
	BOOL fwAppEnabled;
	BSTR fwBstrName = NULL;
	BSTR fwBstrProcessImageFileName = NULL;
	INetFwAuthorizedApplication* fwApp = NULL;
	INetFwAuthorizedApplications* fwApps = NULL;

	_ASSERT(fwProfile != NULL);
	_ASSERT(fwProcessImageFileName != NULL);
	_ASSERT(fwName != NULL);

	// First check to see if the application is already authorized.
	hr = WindowsFirewallAppIsEnabled(
		fwProfile,
		fwProcessImageFileName,
		&fwAppEnabled
		);
	if (FAILED(hr))
	{
		_DBGAlert("WindowsFirewallAppIsEnabled failed: 0x%08lx\n", hr);
		goto error;
	}

	// Only add the application if it isn't already authorized.
	if (!fwAppEnabled)
	{
		// Retrieve the authorized application collection.
		hr = fwProfile->get_AuthorizedApplications(&fwApps);
		if (FAILED(hr))
		{
			_DBGAlert("get_AuthorizedApplications failed: 0x%08lx\n", hr);
			goto error;
		}

		// Create an instance of an authorized application.
		hr = CoCreateInstance(
			__uuidof(NetFwAuthorizedApplication),
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(INetFwAuthorizedApplication),
			(void**)&fwApp
			);
		if (FAILED(hr))
		{
			_DBGAlert("CoCreateInstance failed: 0x%08lx\n", hr);
			goto error;
		}

		// Allocate a BSTR for the process image file name.
		fwBstrProcessImageFileName = SysAllocString(fwProcessImageFileName);
		if (SysStringLen(fwBstrProcessImageFileName) == 0)
		{
			hr = E_OUTOFMEMORY;
			_DBGAlert("SysAllocString failed: 0x%08lx\n", hr);
			goto error;
		}

		// Set the process image file name.
		hr = fwApp->put_ProcessImageFileName(fwBstrProcessImageFileName);
		if (FAILED(hr))
		{
			_DBGAlert("put_ProcessImageFileName failed: 0x%08lx\n", hr);
			goto error;
		}

		// Allocate a BSTR for the application friendly name.
		fwBstrName = SysAllocString(fwName);
		if (SysStringLen(fwBstrName) == 0)
		{
			hr = E_OUTOFMEMORY;
			_DBGAlert("SysAllocString failed: 0x%08lx\n", hr);
			goto error;
		}

		// Set the application friendly name.
		hr = fwApp->put_Name(fwBstrName);
		if (FAILED(hr))
		{
			_DBGAlert("put_Name failed: 0x%08lx\n", hr);
			goto error;
		}

		// Add the application to the collection.
		hr = fwApps->Add(fwApp);
		if (FAILED(hr))
		{
			_DBGAlert("Add failed: 0x%08lx\n", hr);
			goto error;
		}

		_DBGAlert(
			"Authorized application %lS is now enabled in the firewall.\n",
			fwProcessImageFileName
			);
	}

error:

	// Free the BSTRs.
	SysFreeString(fwBstrName);
	SysFreeString(fwBstrProcessImageFileName);

	// Release the authorized application instance.
	if (fwApp != NULL)
	{
		fwApp->Release();
	}

	// Release the authorized application collection.
	if (fwApps != NULL)
	{
		fwApps->Release();
	}

	return hr;
}

HRESULT FFirewall::AddAppToWindowsFirewall(const tchar* pszFullPath, const tchar* pszName)
{
	USES_CONVERSION; 
	BOOL couldEnable = false;
	HRESULT hr = S_OK;
	HRESULT comInit = E_FAIL;
	INetFwProfile* fwProfile = NULL;

	// Retrieve the firewall profile currently in effect.
	hr = WindowsFirewallInitialize(&fwProfile);
	if (FAILED(hr)) {
		_DBGAlert("WindowsFirewallInitialize failed: 0x%08lx\n", hr );
		goto error;
	}

	// Add the application to the authorized application collection.
	hr = WindowsFirewallAddApp(fwProfile, T2OLE(pszFullPath),	T2OLE(pszName));
	if (FAILED(hr)) {
		_DBGAlert("WindowsFirewallAddApp failed: 0x%08lx\n", hr );
		goto error;
	}

error:

	WindowsFirewallCleanup(fwProfile);

	if (SUCCEEDED(comInit)) {
		CoUninitialize();
	}

	return hr;
}

BSTR FFirewall::GetInternalIP() {

	USES_CONVERSION;

	BSTR retStr = NULL;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	IP_ADAPTER_INFO* pAdapterInfo = (IP_ADAPTER_INFO*) malloc(sizeof(IP_ADAPTER_INFO));
	memset(pAdapterInfo, 0, sizeof(IP_ADAPTER_INFO));

	DWORD ret = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);

	if (ret == ERROR_BUFFER_OVERFLOW) {
		pAdapterInfo = (IP_ADAPTER_INFO*) realloc(pAdapterInfo, ulOutBufLen);
		ret = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	}

	if (ret == NO_ERROR)
		retStr = SysAllocString(A2W(pAdapterInfo->IpAddressList.IpAddress.String));

	free(pAdapterInfo);
	return retStr;
}

bool FFirewall::AddPortMapping(std::vector<long>& aPorts, const tchar* pszPort, const tchar* pszDescription)
{
	USES_CONVERSION;
	bool bRet = false;
	int tries = 10;
	unsigned int i = 0;

	IUPnPNAT* pNat = NULL;
	IStaticPortMappingCollection *pMappings = NULL;
	IStaticPortMapping* pMapping = NULL;

	BSTR strClient = GetInternalIP();

	if (strClient == NULL)
		goto cleanup;

	if (FAILED(CoCreateInstance(__uuidof(UPnPNAT), NULL, CLSCTX_ALL, __uuidof(IUPnPNAT), (void**) &pNat)))
		goto cleanup;

	while ((pMappings == NULL) && (tries-- > 0)) {

		if (FAILED(pNat->get_StaticPortMappingCollection(&pMappings)) || !pMappings)
		{
			if (WAIT_IO_COMPLETION == SleepEx(2000, TRUE))
				goto cleanup; 
		}
	}

	if (pMappings == NULL)
		goto cleanup;

	for (i = 0; i < aPorts.size(); i++) {

		long mappedPort = aPorts[i];

		if (SUCCEEDED(pMappings->Add(mappedPort, T2OLE(pszPort), mappedPort, strClient, VARIANT_TRUE, T2OLE(pszDescription), &pMapping)))
			bRet = true;

		if (pMapping) {
			pMapping->Release();
			pMapping = NULL;
		}
	}
cleanup:

	if (pMappings)
		pMappings->Release();

	if (pNat)
		pNat->Release();

	SysFreeString(strClient);

	return bRet;
}

void FFirewall::RemovePortMapping() {

	IUPnPNAT* pNat = NULL;
	IStaticPortMappingCollection *pMappings = NULL;

	if (FAILED(CoCreateInstance(__uuidof(UPnPNAT), NULL, CLSCTX_ALL, __uuidof(IUPnPNAT), (void**) &pNat)) || pNat == NULL)
		return;

	if (FAILED(pNat->get_StaticPortMappingCollection(&pMappings)) || pMappings == NULL) {
		pNat->Release();
		return;
	}

	BSTR strProtocol = SysAllocString(L"TCP");

	pMappings->Remove(80, strProtocol);
	pMappings->Remove(443, strProtocol);

	SysFreeString(strProtocol);

	pMappings->Release();
	pNat->Release();
}