// LTVUtil.Test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "LTV_RSS.h"
#include <conio.h>
#include "StringList.h"
#include "Utils.h"
#include "FXMLUtil.h"

void TestRSS()
{
	FRSSReader aReader;

	const tchar* pFileName = "F:\\LiberTV Index\\1132379798.rss";
	FRSSChannel* pChannel = NULL;
	if (SUCCEEDED(aReader.ParseRSS(pFileName, &pChannel)))
	{
		printf("Parsed RSS: %s\n===================================\n\n", pFileName);
		printf("Title: %s\n", pChannel->m_Title);
		printf("URL: %s\n", pChannel->m_Link);
		printf("Description: %s\n", pChannel->m_Description); 
		printf("Items: %d\n", pChannel->m_Items.GetCount()); 
		for (size_t k = 0; k < pChannel->m_Items.GetCount(); k++)
		{
			printf("\n\n---- Item %d\n", k); 
			FRSSItem* pItem = pChannel->m_Items[k];
			printf("Title: %s\n", pItem->m_Title); 
			printf("Link: %s\n", pItem->m_Link); 
			printf("Description: %s\n", pItem->m_Description); 
			printf("Image: %s\n", pItem->m_ItemImage);
			printf("Guid: %d\n", pItem->m_Guid); 
		}
		printf("\n\nPress any key to continue.\n"); 
		getch(); 
		//aReader.FreeChannel(pChannel); 
	}
	else
	{
		printf("Unable to parse: %s\n", pFileName); 
	}
}

class XMLTest
{
	CComPtr<IXMLDOMDocument> m_pDoc; 
public:
	HRESULT LoadFile(const char* pszFileName)
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
				if (SUCCEEDED(hr))
				{
					hr = m_pDoc->save(CComVariant("E:\\Temp\\out.xml")); 
				}
			}
		}
		return hr; 
	}

	HRESULT LoadDocument()
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
				ModifyTheValues(pNextElement);
			}
		}

		return hr; 
	}

	HRESULT ProcessMediaEntry(IXMLDOMElement* pEle)
	{
		FXMLNode pNode = pEle; 
		ULONG m_EpisodeID =			pNode.GetAttribute("EPISODEID").ToULong();
		FString VideoName =			pNode.GetChild("TITLE").GetText().ToString(); 
		FString m_DetailURL =		pNode.GetChild("MOREINFO").GetAttribute("HREF").ToString(); 
		UINT64 m_TotalDurationMS =	pNode.GetChild("DURATION").GetAttribute("VALUE").ToUINT64(); 
		FString m_DataPath	=		pNode.GetChild("STORAGE").GetAttribute("PATH").ToString(); 
		return S_OK;
	}

	void   ModifyTheValues(IXMLDOMElement* pEle)
	{
		FXMLNode pNode = pEle; 
		if (pNode)
		{
			pNode.GetChild("TITLE").SetText("The title"); 
			pNode.GetChild("DURATION").SetAttribute("VALUE", (UINT64)230942834); 
			pNode.SetAttribute("EPISODEID", (ULONG)9999999);
			pNode.SetAttribute("VIDEO_ID", (ULONG)3242342); 
			pNode.GetChildEx("THECHILD").SetAttribute("TheName", (ULONG)3242342);
		}
	}
};

int _tmain(int argc, _TCHAR* argv[])
{
	//TestRSS();

	CoInitialize(NULL);
	
	XMLTest xmlTest; 
	xmlTest.LoadFile("E:\\Temp\\video_index.xml");

	return 0;
}

