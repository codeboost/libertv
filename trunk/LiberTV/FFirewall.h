#ifndef __FFIREWALL_H__
#define __FFIREWALL_H__


class FFirewall
{
	BSTR	GetInternalIP();
public:
	static HRESULT AddAppToWindowsFirewall(const tchar* pszFullPath, const tchar* pszName);
	bool	AddPortMapping(std::vector<long>& aPorts, const tchar* pszProtocol, const tchar* pszDescription);
	void	RemovePortMapping(); 
};
#endif //__FFIREWALL_H__