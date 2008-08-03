#include "stdafx.h"
#include "FDownloadIndex.h"
#include "mshtml.h"
HRESULT FDownloadIndex::LoadIndex(const char* pszFileName)
{
	HRESULT hr = E_FAIL; 
	hr = m_pDoc.CoCreateInstance(CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER); 
	if (SUCCEEDED(hr))
	{
		m_pDoc->put_async(VARIANT_FALSE); 
		m_pDoc->put_validateOnParse(VARIANT_FALSE); 
		VARIANT_BOOL bSuccessful; 
		hr = m_pDoc->load(CComVariant(pszFileName), &bSuccessful); 
		if (SUCCEEDED(hr) && bSuccessful)
		{
			hr = LoadDocument();
		}
	}
	return hr; 
}

HRESULT FDownloadIndex::LoadDocument()
{
	ATLASSERT(m_pDoc != NULL); 
	if (m_pDoc == NULL)
		return E_POINTER; 

	CComPtr<IXMLDOMElement> pRootElem; 
	HRESULT hr = E_FAIL; 

	hr = m_pDoc->get_documentElement(&pRootElem); 		//LiberTV

	CComPtr<IXMLDOMNodeList> pEntries; 
	hr = pRootElem->selectNodes(CComBSTR("MEDIA"), &pEntries);
	if (SUCCEEDED(hr) && pEntries)
	{
		//Loop through each entry and extract shit from them
		for (;;)
		{
			CComPtr<IXMLDOMElement> pNextElement; 
			hr = FXMLUtil::GetNextElement(pEntries, pNextElement);
			if (FAILED(hr) || pNextElement == NULL)
			{
				hr = S_OK; 
				break;
			}
			ProcessMediaEntry(pNextElement); 
		}
	}
	return hr; 
}

HRESULT FDownloadIndex::ProcessMediaEntry(IXMLDOMElement* pNode)
{
	FDownloadEx* pEx = new FDownloadEx(); 
	m_pDownloads.Add(pEx); 
	return pEx->LoadFromXML(pNode); 
}

HRESULT FDownloadIndex::ConvertIndex( FArray<FDownloadEx*> &pDownloads, const char* pszIndexPath )
{
	HRESULT hr = E_FAIL; 
	hr = m_pDoc.CoCreateInstance(CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER); 
	if (SUCCEEDED(hr))
	{
		CComPtr<IXMLDOMElement> pEle; 
		hr = m_pDoc->createElement(L LTV_APP_NAME, &pEle);
		if (pEle)
		{
			hr = m_pDoc->appendChild(pEle, NULL);
			if (SUCCEEDED(hr))
			{
				FXMLNode eleNode(pEle);
				eleNode.SetAttribute("VERSION", "1.4");
				for (size_t k = 0; k < pDownloads.GetCount(); k++)
				{
					FDownloadEx* pDown = pDownloads[k]; 
					CComPtr<IXMLDOMElement> NewElement; 
					hr = m_pDoc->createElement(L"MEDIA", &NewElement);
					FXMLNode MediaNode = NewElement; 
					if (MediaNode)
					{
						hr = pEle->appendChild(MediaNode.m_pNode, NULL);
						if (SUCCEEDED(hr))
						{
							hr = pDown->SaveToXML(MediaNode.m_pNode, TRUE);
							_DBGAlert("Convert result %d: 0x%x\n", pDown->m_Detail.m_VideoID, hr); 
						}
					}
				}
			}

			CComQIPtr<IXMLDOMNode> pqDocNode = m_pDoc; 
			FXMLNode pDocNode = pqDocNode; 
			pDocNode.Format(0, m_pDoc); 


			CComPtr<IXMLDOMProcessingInstruction> pi; 
			hr = m_pDoc->createProcessingInstruction(L"xml", L"version=\"1.0\"", &pi);
			if (pi)
			{
				CComPtr<IXMLDOMNode> pChild, pNew; 
				m_pDoc->get_firstChild(&pChild); 
				if (pChild)
				{
					CComVariant vtChild = pChild; 
					m_pDoc->insertBefore(pi, vtChild, &pNew); 
				}
			}

			hr = m_pDoc->save(CComVariant(pszIndexPath));

			/*
			CComPtr<IXMLDOMDocument> XSLDOM; 
			hr = XSLDOM.CoCreateInstance(CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER); 
			XSLDOM->put_async(VARIANT_FALSE); 
			XSLDOM->put_validateOnParse(VARIANT_FALSE); 
			VARIANT_BOOL bSuccess; 
			hr = XSLDOM->load(CComVariant("M:\\temp\\test.xsl"), &bSuccess);

			CComPtr<IXMLDOMDocument> NewDOM; 
			hr = NewDOM.CoCreateInstance(CLSID_DOMDocument30, NULL, CLSCTX_INPROC_SERVER); 
			CComVariant vtObject = NewDOM; 
			hr = m_pDoc->transformNodeToObject(XSLDOM, vtObject); 
			hr = NewDOM->save(CComVariant(pszIndexPath));
			*/


		}
	}
	return hr; 
}

