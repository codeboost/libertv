#pragma once
#include "FXMLUtil.h"
#include "FDownload.h"

class FMediaEntry
{

};

class FDownloadIndex
{
	CComPtr<IXMLDOMDocument> m_pDoc;	
	FArray<FDownloadEx*>	 m_pDownloads; 
public:

	HRESULT LoadIndex(const char* pszFileName); 
	HRESULT LoadDocument(); 
	HRESULT ProcessMediaEntry(IXMLDOMElement* pNode); 

	HRESULT	ConvertIndex(FArray<FDownloadEx*> &pDownloads, const char* pszIndexPath);
	BOOL	GetIndex(FArray<FDownloadEx*>& pIndex)
	{
		pIndex = m_pDownloads; 
		return TRUE; 
	}
};