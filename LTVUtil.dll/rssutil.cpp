#include "stdafx.h"
#include "rssutil.h"
#include "xmlutil.h"
#include "strutil.h"
#include "memutil.h"
#include "timeutil.h"

#define ReleaseBstr(bstr) if (bstr) { ::SysFreeString(bstr); }
#define ReleaseNullBstr(bstr) if (bstr) { ::SysFreeString(bstr); bstr = NULL; }


static HRESULT ParseRssDocument(
								__in IXMLDOMDocument *pixd,
								__out RSS_CHANNEL **ppChannel
								);
static HRESULT ParseRssChannel(
							   __in IXMLDOMNode *pixnChannel,
							   __out RSS_CHANNEL **ppChannel
							   );
static HRESULT ParseRssItem(
							__in IXMLDOMNode *pixnItem,
							__in DWORD cItem,
							__in_xcount(pChannel->cItems) RSS_CHANNEL *pChannel
							);
static HRESULT ParseRssUnknownElement(
									  __in IXMLDOMNode *pNode,
									  __inout RSS_UNKNOWN_ELEMENT** ppUnknownElement
									  );
static HRESULT ParseRssUnknownAttribute(
										__in IXMLDOMNode *pNode,
										__inout RSS_UNKNOWN_ATTRIBUTE** ppUnknownAttribute
										);
static void FreeRssUnknownElementList(
									  __in_opt RSS_UNKNOWN_ELEMENT* pUnknownElement
									  );
static void FreeRssUnknownAttributeList(
										__in_opt RSS_UNKNOWN_ATTRIBUTE* pUnknownAttribute
										);


/********************************************************************
RssInitialize - Initialize RSS utilities.

*********************************************************************/
extern "C" HRESULT DAPI RssInitialize()
{
	return XmlInitialize();
}


/********************************************************************
RssUninitialize - Uninitialize RSS utilities.

*********************************************************************/
extern "C" void DAPI RssUninitialize()
{
	XmlUninitialize();
}


/********************************************************************
RssParseFromString - parses out an RSS channel from a string.

*********************************************************************/
extern "C" HRESULT DAPI RssParseFromString(
	__in LPCWSTR wzRssString,
	__out RSS_CHANNEL **ppChannel
	)
{
	Assert(wzRssString);
	Assert(ppChannel);

	HRESULT hr = S_OK;
	RSS_CHANNEL *pNewChannel = NULL;
	IXMLDOMDocument *pixdRss = NULL;

	hr = XmlLoadDocument(wzRssString, &pixdRss);
	ExitOnFailure(hr, "Failed to load RSS string as XML document.");

	hr = ParseRssDocument(pixdRss, &pNewChannel);
	ExitOnFailure(hr, "Failed to parse RSS document.");

	*ppChannel = pNewChannel;
	pNewChannel = NULL;

LExit:
	ReleaseObject(pixdRss);

	ReleaseRssChannel(pNewChannel);

	return hr;
}


/********************************************************************
RssParseFromFile - parses out an RSS channel from a file path.

*********************************************************************/
extern "C" HRESULT DAPI RssParseFromFile(
	__in LPCWSTR wzRssFile,
	__out RSS_CHANNEL **ppChannel
	)
{
	Assert(wzRssFile);
	Assert(ppChannel);

	HRESULT hr = S_OK;
	RSS_CHANNEL *pNewChannel = NULL;
	IXMLDOMDocument *pixdRss = NULL;

	hr = XmlLoadDocumentFromFile(wzRssFile, &pixdRss);
	ExitOnFailure(hr, "Failed to load RSS string as XML document.");

	hr = ParseRssDocument(pixdRss, &pNewChannel);
	ExitOnFailure(hr, "Failed to parse RSS document.");

	*ppChannel = pNewChannel;
	pNewChannel = NULL;

LExit:
	ReleaseObject(pixdRss);

	ReleaseRssChannel(pNewChannel);

	return hr;
}


/********************************************************************
RssFreeChannel - parses out an RSS channel from a string.

*********************************************************************/
RSS_UNKNOWN_ATTRIBUTE::~RSS_UNKNOWN_ATTRIBUTE()
{
	delete pNext; 
}

RSS_UNKNOWN_ELEMENT::~RSS_UNKNOWN_ELEMENT()
{
	delete pAttributes; 
	delete pNext; 
}

RSS_ITEM::~RSS_ITEM()
{
	delete pUnknownElements; 
}

RSS_CHANNEL::~RSS_CHANNEL()
{
	delete pUnknownElements; 
	delete[] rgItems;
}

extern "C" void DAPI RssFreeChannel(
									__in_xcount(pChannel->cItems) RSS_CHANNEL *pChannel
									)
{
	delete pChannel; 
}


/********************************************************************
ParseRssDocument - parses out an RSS channel from a loaded XML DOM document.

*********************************************************************/
static HRESULT ParseRssDocument(
								__in IXMLDOMDocument *pixd,
								__out RSS_CHANNEL **ppChannel
								)
{
	Assert(pixd);
	Assert (ppChannel);

	HRESULT hr = S_OK;
	IXMLDOMElement *pRssElement = NULL;
	IXMLDOMNodeList *pChannelNodes = NULL;
	IXMLDOMNode *pNode = NULL;
	BSTR bstrNodeName = NULL;

	RSS_CHANNEL *pNewChannel = NULL;

	//
	// Get the document element and start processing channels.
	//
	hr = pixd ->get_documentElement(&pRssElement);
	ExitOnFailure(hr, "failed get_documentElement in ParseRssDocument");

	hr = pRssElement->get_childNodes(&pChannelNodes);
	ExitOnFailure(hr, "Failed to get child nodes of Rss Document element.");

	while (S_OK == (hr = XmlNextElement(pChannelNodes, &pNode, &bstrNodeName)))
	{
		if (0 == lstrcmpW(bstrNodeName, L"channel"))
		{
			hr = ParseRssChannel(pNode, &pNewChannel);
			ExitOnFailure(hr, "Failed to parse RSS channel.");
		}
		else if (0 == lstrcmpW(bstrNodeName, L"link"))
		{
		}

		ReleaseNullBstr(bstrNodeName);
		ReleaseNullObject(pNode);
	}

	if (S_FALSE == hr)
	{
		hr = S_OK;
	}

	*ppChannel = pNewChannel;
	pNewChannel = NULL;

LExit:
	ReleaseBstr(bstrNodeName);
	ReleaseObject(pNode);
	ReleaseObject(pChannelNodes);
	ReleaseObject(pRssElement);

	ReleaseRssChannel(pNewChannel);

	return hr;
}


/********************************************************************
ParseRssChannel - parses out an RSS channel from a loaded XML DOM element.

*********************************************************************/
static HRESULT ParseRssChannel(
							   __in IXMLDOMNode *pixnChannel,
							   __out RSS_CHANNEL **ppChannel
							   )
{
	Assert(pixnChannel);
	Assert (ppChannel);

	HRESULT hr = S_OK;
	IXMLDOMNodeList *pNodeList = NULL;

	RSS_CHANNEL *pNewChannel = NULL;
	long cItems = 0;

	IXMLDOMNode *pNode = NULL;
	BSTR bstrNodeName = NULL;
	BSTR bstrNodeValue = NULL;

	//
	// First, calculate how many RSS items we're going to have and allocate
	// the RSS_CHANNEL structure
	//
	hr = XmlSelectNodes(pixnChannel, L"item", &pNodeList);
	ExitOnFailure(hr, "Failed to select all RSS items in an RSS channel.");

	hr = pNodeList->get_length(&cItems);
	ExitOnFailure(hr, "Failed to count the number of RSS items in RSS channel.");

	pNewChannel = new RSS_CHANNEL; 
	pNewChannel->rgItems = new RSS_ITEM[cItems];
	pNewChannel->cItems = cItems;

	ReleaseObject(pNodeList); 
	//
	// Process the elements under a channel now.
	//
	hr = pixnChannel->get_childNodes(&pNodeList);
	ExitOnFailure(hr, "Failed to get child nodes of RSS channel element.");

	cItems = 0; // reset the counter and use this to walk through the channel items
	while (S_OK == (hr = XmlNextElement(pNodeList, &pNode, &bstrNodeName)))
	{		
		CComBSTR nodeNamespace;

		hr = pNode->get_namespaceURI(&nodeNamespace);
		ExitOnFailure(hr, "Failed to get namespace of channel node");

		if (0 == lstrcmpW(bstrNodeName, L"image") && nodeNamespace == NULL)	// in default namespace
		{
			CComPtr<IXMLDOMNodeList> pChildren;
			long count; 

			hr = pNode->get_childNodes(&pChildren);
			ExitOnFailure(hr, "Failed to get child nodes for image object");

			hr = pChildren->get_length(&count);
			ExitOnFailure(hr, "Failed to get children count");

			for (long k = 0; k < count; ++k)
			{
				CComPtr<IXMLDOMNode> pItem;
				CComBSTR bstrName;

				hr = pChildren->get_item(k, &pItem);
				ExitOnFailure(hr, "Failed to get item index k from children");

				if (pItem == NULL) continue;

				hr = pItem->get_nodeName(&bstrName);
				ExitOnFailure(hr, "Failed to get node name for index k of children");

				if (bstrName == L"url" && pNewChannel->wzImageURL == NULL)
				{
					hr = XmlGetText(pItem, &pNewChannel->wzImageURL);
					ExitOnFailure(hr, "Failed to extract image from item in children.");
				}
			}
		}
		else
		if (0 == lstrcmpW(bstrNodeName, L"title") && pNewChannel->wzTitle == NULL)
		{
			hr = XmlGetText(pNode, &pNewChannel->wzTitle);
			ExitOnFailure(hr, "Failed to get RSS channel title.");
		}
		else if (0 == lstrcmpW(bstrNodeName, L"link") && pNewChannel->wzLink == NULL)
		{
			hr = XmlGetText(pNode, &pNewChannel->wzLink);
			ExitOnFailure(hr, "Failed to get RSS channel link.");
		}
		else if (0 == lstrcmpW(bstrNodeName, L"description") && pNewChannel->wzDescription == NULL)
		{
			hr = XmlGetText(pNode, &pNewChannel->wzDescription);
			ExitOnFailure(hr, "Failed to get RSS channel description.");
		}
		else if (0 == lstrcmpW(bstrNodeName, L"ttl"))
		{
			CComBSTR value; 
			hr = XmlGetText(pNode, &value);
			ExitOnFailure(hr, "Failed to get RSS channel description.");

			pNewChannel->dwTimeToLive = (DWORD)wcstoul(value, NULL, 10);
		}
		else if (0 == lstrcmpW(bstrNodeName, L"item"))
		{
			hr = ParseRssItem(pNode, cItems, pNewChannel);
			ExitOnFailure(hr, "Failed to parse RSS item.");

			cItems++;
		}
		else
		{
			hr = ParseRssUnknownElement(pNode, &pNewChannel->pUnknownElements);
			ExitOnFailure1(hr, "Failed to parse unknown RSS channel element: %S", bstrNodeName);
		}

		ReleaseNullBstr(bstrNodeValue);
		ReleaseNullBstr(bstrNodeName);
		ReleaseNullObject(pNode);
	}

	*ppChannel = pNewChannel;
	pNewChannel = NULL;

LExit:
	ReleaseBstr(bstrNodeName);
	ReleaseObject(pNode);
	ReleaseObject(pNodeList);

	ReleaseRssChannel(pNewChannel);

	return hr;
}


/********************************************************************
ParseRssItem - parses out an RSS item from a loaded XML DOM node.

*********************************************************************/
static HRESULT ParseRssItem(
							__in IXMLDOMNode *pixnItem,
							__in DWORD cItem,
							__in_xcount(pChannel->cItems) RSS_CHANNEL *pChannel
							)
{
	HRESULT hr = S_OK;

	RSS_ITEM *pItem = NULL;
	IXMLDOMNodeList *pNodeList = NULL;

	IXMLDOMNode *pNode = NULL;
	BSTR bstrNodeName = NULL;
	BSTR bstrNodeValue = NULL;

	//
	// First make sure we're dealing with a valid item.
	//
	if (pChannel->cItems <= cItem)
	{
		hr = E_UNEXPECTED;
		ExitOnFailure(hr, "Unexpected number of items parsed.");
	}

	pItem = pChannel->rgItems + cItem;

	//
	// Process the elements under an item now.
	//
	hr = pixnItem->get_childNodes(&pNodeList);
	ExitOnFailure(hr, "Failed to get child nodes of RSS item element.");
	while (S_OK == (hr = XmlNextElement(pNodeList, &pNode, &bstrNodeName)))
	{
		if (0 == lstrcmpW(bstrNodeName, L"title") && pItem->wzTitle == NULL)
		{
			hr = XmlGetText(pNode, &pItem->wzTitle);
			ExitOnFailure(hr, "Failed to get RSS channel title.");
		}
		else if (0 == lstrcmpW(bstrNodeName, L"link") && pItem->wzLink == NULL)
		{
			hr = XmlGetText(pNode, &pItem->wzLink);
			ExitOnFailure(hr, "Failed to get RSS channel link.");
		}
		else if (0 == lstrcmpW(bstrNodeName, L"description") && pItem->wzDescription == NULL)
		{
			hr = XmlGetText(pNode, &pItem->wzDescription);
			ExitOnFailure(hr, "Failed to get RSS item description.");
		}
		else if (0 == lstrcmpW(bstrNodeName, L"guid") && pItem->wzGuid == NULL)
		{
			hr = XmlGetText(pNode, &pItem->wzGuid);
			ExitOnFailure(hr, "Failed to get RSS item guid.");
		}
		else if (0 == lstrcmpW(bstrNodeName, L"pubDate"))
		{
			//we go easy on the date/time conversion and we don't fail if we can't get the item's pub date
			//some times may be in other languages or strange formats
			CComBSTR value; 
			hr = XmlGetText(pNode, &value);
			hr = TimeFromString(value, &pItem->ftPublished);
			if (FAILED(hr))
			{
				SYSTEMTIME now; 
				GetSystemTime(&now);
				SystemTimeToFileTime(&now, &pItem->ftPublished);
				hr = S_OK; 
			}
		}
		else if (0 == lstrcmpW(bstrNodeName, L"enclosure"))
		{
			RSS_ENCLOSURE Enclosure; 

			hr = XmlGetAttribute(pNode, L"url", &Enclosure.wzEnclosureUrl);
			ExitOnFailure(hr, "Failed to get RSS item enclosure url.");

			hr = XmlGetAttributeNumber64(pNode, L"length", &Enclosure.i64EnclosureSize);
			ExitOnFailure(hr, "Failed to get RSS item enclosure length.");

			hr = XmlGetAttribute(pNode, L"type", &Enclosure.wzEnclosureType);
			ExitOnFailure(hr, "Failed to get RSS item enclosure type.");
			pItem->m_Enclosures.push_back(Enclosure); 
		}
		else
		{
			hr = ParseRssUnknownElement(pNode, &pItem->pUnknownElements);
			ExitOnFailure1(hr, "Failed to parse unknown RSS item element: %S", bstrNodeName);
		}

		ReleaseNullBstr(bstrNodeValue);
		ReleaseNullBstr(bstrNodeName);
		ReleaseNullObject(pNode);
	}

LExit:
	ReleaseBstr(bstrNodeValue);
	ReleaseBstr(bstrNodeName);
	ReleaseObject(pNode);
	ReleaseObject(pNodeList);

	return hr;
}


/********************************************************************
ParseRssUnknownElement - parses out an unknown item from the RSS feed from a loaded XML DOM node.

*********************************************************************/
static HRESULT ParseRssUnknownElement(
									  __in IXMLDOMNode *pNode,
									  __inout RSS_UNKNOWN_ELEMENT** ppUnknownElement
									  )
{
	Assert(ppUnknownElement);

	HRESULT hr = S_OK;
	BSTR bstrNodeNamespace = NULL;
	BSTR bstrNodeName = NULL;
	BSTR bstrNodeValue = NULL;
	IXMLDOMNamedNodeMap* pixnnmAttributes = NULL;
	IXMLDOMNode* pixnAttribute = NULL;
	RSS_UNKNOWN_ELEMENT* pNewUnknownElement;

	pNewUnknownElement = new RSS_UNKNOWN_ELEMENT;
	ExitOnNull(pNewUnknownElement, hr, E_OUTOFMEMORY, "Failed to allocate unknown element.");

	if (pNewUnknownElement->wzNamespace)
		pNewUnknownElement->wzNamespace.Empty();

	hr = pNode->get_namespaceURI(&pNewUnknownElement->wzNamespace);
	if (S_FALSE == hr)
	{
		hr = S_OK;
	}
	ExitOnFailure(hr, "Failed to get unknown element namespace.");

	hr = pNode->get_baseName(&pNewUnknownElement->wzElement);
	ExitOnFailure(hr, "Failed to get unknown element name.");

	hr = XmlGetText(pNode, &pNewUnknownElement->wzValue);
	ExitOnFailure(hr, "Failed to get unknown element value.");

	hr = pNode->get_attributes(&pixnnmAttributes);
	ExitOnFailure(hr, "Failed get attributes on RSS unknown element.");

	while (S_OK == (hr = pixnnmAttributes->nextNode(&pixnAttribute)))
	{
		hr = ParseRssUnknownAttribute(pixnAttribute, &pNewUnknownElement->pAttributes);
		ExitOnFailure(hr, "Failed to parse attribute on RSS unknown element.");

		ReleaseNullObject(pixnAttribute);
	}

	if (S_FALSE == hr)
	{
		hr = S_OK;
	}
	ExitOnFailure(hr, "Failed to enumerate all attributes on RSS unknown element.");

	RSS_UNKNOWN_ELEMENT** ppTail = ppUnknownElement;
	while (*ppTail)
	{
		ppTail = &(*ppTail)->pNext;
	}

	*ppTail = pNewUnknownElement;
	pNewUnknownElement = NULL;

LExit:
	FreeRssUnknownElementList(pNewUnknownElement);

	ReleaseBstr(bstrNodeNamespace);
	ReleaseBstr(bstrNodeName);
	ReleaseBstr(bstrNodeValue);
	ReleaseObject(pixnnmAttributes);
	ReleaseObject(pixnAttribute);

	return hr;
}


/********************************************************************
ParseRssUnknownAttribute - parses out attribute from an unknown element

*********************************************************************/
static HRESULT ParseRssUnknownAttribute(
										__in IXMLDOMNode *pNode,
										__inout RSS_UNKNOWN_ATTRIBUTE** ppUnknownAttribute
										)
{
	Assert(ppUnknownAttribute);

	HRESULT hr = S_OK;
	BSTR bstrNodeNamespace = NULL;
	BSTR bstrNodeName = NULL;
	BSTR bstrNodeValue = NULL;
	RSS_UNKNOWN_ATTRIBUTE* pNewUnknownAttribute;

	pNewUnknownAttribute = new RSS_UNKNOWN_ATTRIBUTE; 

	hr = pNode->get_namespaceURI(&pNewUnknownAttribute->wzNamespace);
	if (S_FALSE == hr)
	{
		hr = S_OK;
	}
	ExitOnFailure(hr, "Failed to get unknown attribute namespace.");

	hr = pNode->get_baseName(&pNewUnknownAttribute->wzAttribute);
	ExitOnFailure(hr, "Failed to get unknown attribute name.");

	if (pNewUnknownAttribute->wzValue == NULL)
	{
		hr = XmlGetText(pNode, &pNewUnknownAttribute->wzValue);
		ExitOnFailure(hr, "Failed to get unknown attribute value.");
	}

	RSS_UNKNOWN_ATTRIBUTE** ppTail = ppUnknownAttribute;
	while (*ppTail)
	{
		ppTail = &(*ppTail)->pNext;
	}

	*ppTail = pNewUnknownAttribute;
	pNewUnknownAttribute = NULL;

LExit:
	FreeRssUnknownAttributeList(pNewUnknownAttribute);

	ReleaseBstr(bstrNodeNamespace);
	ReleaseBstr(bstrNodeName);
	ReleaseBstr(bstrNodeValue);

	return hr;
}


/********************************************************************
FreeRssUnknownElement - releases all of the memory used by a list of unknown elements

*********************************************************************/
static void FreeRssUnknownElementList(
									  __in_opt RSS_UNKNOWN_ELEMENT* pUnknownElement
									  )
{
	while (pUnknownElement)
	{
		RSS_UNKNOWN_ELEMENT* pFree = pUnknownElement;
		pUnknownElement = pUnknownElement->pNext;
		delete pFree; 
	}
}


/********************************************************************
FreeRssUnknownAttribute - releases all of the memory used by a list of unknown attributes

*********************************************************************/
static void FreeRssUnknownAttributeList(
										__in_opt RSS_UNKNOWN_ATTRIBUTE* pUnknownAttribute
										)
{
	while (pUnknownAttribute)
	{
		RSS_UNKNOWN_ATTRIBUTE* pFree = pUnknownAttribute;
		pUnknownAttribute = pUnknownAttribute->pNext;
		delete pFree; 
	}
}