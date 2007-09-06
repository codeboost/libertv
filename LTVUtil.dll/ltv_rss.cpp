#include "stdafx.h"
#include "LTV_RSS.h"
#include "rssutil.h"
#include "xmlutil.h"
#include "crc32.h"

int SplitStringToArray(const tchar* pszStr, FArray<FString> &aRes, const tchar* pSep )
{

	if (NULL == pszStr)
		return 0; 

	size_t len = strlen(pszStr); 

	if (len == 0)
		return 0; 

	ATLASSERT(pSep != NULL); 

	const tchar* pszCur = pszStr; 
	const tchar* pszLast = pszStr; 
	const tchar* pszEnd = pszStr + len; 

	for (;;)
	{
		while (pszLast < pszEnd && (*pszLast == ' ' || *pszLast == '\t' || *pszLast == '\n'))
			pszLast++;

		if (pszLast >= pszEnd)
			break; 

		pszCur = strstr(pszLast, pSep);

		if (NULL == pszCur)
			pszCur = pszEnd; 

		if (pszCur - pszLast > 1)
		{
			FString Str(pszLast,(int)(pszCur - pszLast));
			Str.Trim(); 
			aRes.Add(Str); 
		}
		pszLast = pszCur + 1; 
	}
	return (int)aRes.GetCount(); 
}

FString GetAttribute(FRSSUnknownElement* pElement, FString attrName)
{
	for (size_t j = 0; j < pElement->m_Attributes.GetCount(); ++j)
	{
		if (pElement->m_Attributes[j]->m_Attribute.CompareNoCase(attrName) == 0)
		{
			return pElement->m_Attributes[j]->m_Value;
		}
	}
	return "";
}

FRSSItem::FRSSItem()
{
	m_videoID = 0; 
	m_dwFlags = 0; 
}

FRSSItem::~FRSSItem()
{
	for (size_t k = 0; k < m_UnknownElements.GetCount(); k++)
	{
		delete m_UnknownElements[k];
	}
	m_UnknownElements.RemoveAll();
}

FRSSChannel::FRSSChannel()
{
}

FRSSUnknownElement::~FRSSUnknownElement()
{
	for (size_t k = 0; k < m_Attributes.GetCount(); k++)
	{
		delete m_Attributes[k];
	}
	m_Attributes.RemoveAll(); 
}

FRSSChannel::~FRSSChannel()
{
	for (size_t k = 0; k < m_Items.GetCount(); k++)
	{
		delete m_Items[k];
	}
	m_Items.RemoveAll(); 

	for (size_t k = 0; k < m_UnknownElements.GetCount(); k++)
	{
		delete m_UnknownElements[k];
	}
	m_UnknownElements.RemoveAll(); 
}

FRSSReader::FRSSReader()
{
	XmlInitialize();
	m_ChannelID = 1024; 
	m_pFilter = NULL; 
}

FRSSReader::~FRSSReader()
{
	XmlUninitialize();
}

void FRSSReader::Clear()
{
	XmlUninitialize();
}

BOOL FRSSReader::ConvertAttribute(RSS_UNKNOWN_ATTRIBUTE* pAttribute, FRSSAttribute* pRSSAttr)
{
	USES_CONVERSION; 
	pRSSAttr->m_Attribute = OLE2T(pAttribute->wzAttribute);
	pRSSAttr->m_Namespace = OLE2T(pAttribute->wzNamespace); 
	pRSSAttr->m_Value = OLE2T(pAttribute->wzValue); 
	return TRUE; 
}

BOOL FRSSReader::ConvertElement(RSS_UNKNOWN_ELEMENT* pElement, FRSSUnknownElement* pRSSEle)
{
	USES_CONVERSION; 
	pRSSEle->m_Element = OLE2T(pElement->wzElement);
	pRSSEle->m_Namespace = OLE2T(pElement->wzNamespace); 
	pRSSEle->m_Value = OLE2T(pElement->wzValue); 
	if (pElement->pAttributes != NULL)
	{
		RSS_UNKNOWN_ATTRIBUTE* pAttr = pElement->pAttributes; 
		while(pAttr != NULL)
		{
			CAutoPtr<FRSSAttribute> pRSSAtrr(new FRSSAttribute);
			if (ConvertAttribute(pAttr, pRSSAtrr))
			{
				pRSSEle->m_Attributes.Add(pRSSAtrr.Detach()); 
			}
			pAttr = pAttr->pNext; 
		}
	}
	return TRUE; 
}

void FRSSReader::ConvertElements(RSS_UNKNOWN_ELEMENT* pEle, FArray<FRSSUnknownElement*>& aElements)
{
	while (pEle != NULL)
	{
		CAutoPtr<FRSSUnknownElement> pRSSElement(new FRSSUnknownElement);
		if (ConvertElement(pEle, pRSSElement))
		{
			aElements.Add(pRSSElement.Detach()); 
		}
		pEle = pEle->pNext;
	}
}

void EscapeBadChars(char* pStr)
{
	size_t len = strlen(pStr);
	while (*pStr)
	{
		if (*pStr < ' ' || *pStr > 126)
			*pStr = ' ';

		pStr++;
	}
}

//Get the first enclosure which is not an image.
int GetEnclosure(RSS_ITEM* pItem)
{
	for (size_t k = 0; k < pItem->m_Enclosures.size(); k++)
	{
		if (pItem->m_Enclosures[k].wzEnclosureType == NULL)
			return (int)k; 
		if (wcsstr(pItem->m_Enclosures[k].wzEnclosureType, L"image/") == NULL)
			return (int)k; 
	}
	return -1; 
}

FString FindImageInEnclosures(RSS_ITEM* pItem)
{
	USES_CONVERSION; 
	for (size_t k = 0; k < pItem->m_Enclosures.size(); k++)
	{
		if (pItem->m_Enclosures[k].wzEnclosureType != NULL)
		{
			if (wcsstr(pItem->m_Enclosures[k].wzEnclosureType, L"image/") != NULL)
				return OLE2T(pItem->m_Enclosures[k].wzEnclosureUrl);
		}
	}
	return ""; 
}

BOOL FRSSReader::ConvertItem(RSS_ITEM* pItem, FRSSItem* pRSSItem)
{
	USES_CONVERSION; 
	pRSSItem->m_Title = OLE2T(pItem->wzTitle); 

	if (m_pFilter != NULL)
	{
			FArray<FString> aContains; 
			SplitStringToArray(m_pFilter->m_Contains, aContains, ","); 
			for (size_t k = 0; k < aContains.GetCount(); k++)
			{
				if (NULL == stristr(pRSSItem->m_Title, aContains[k]))
					return FALSE; 
			}
			FArray<FString> aNotContains; 
			SplitStringToArray(m_pFilter->m_NotContains, aNotContains, ",");
			for (size_t k = 0; k < aNotContains.GetCount(); k++)
			{
				if (NULL != stristr(pRSSItem->m_Title, aNotContains[k]))
					return FALSE; 
			}
	}

	int iIndex = GetEnclosure(pItem); 
	if (iIndex == -1)
		return FALSE; 

	pRSSItem->m_Link = OLE2T(pItem->wzLink); 
	pRSSItem->m_Description = OLE2T(pItem->wzDescription);
	StripHTMLTags(pRSSItem->m_Description.GetBuffer()); 
	pRSSItem->m_Description.Replace("\"", "'");
	EscapeBadChars(pRSSItem->m_Description.GetBuffer());
	EscapeBadChars(pRSSItem->m_Title.GetBuffer()); 

	pRSSItem->m_Description.Trim();

	FString StrGuid = OLE2T(pItem->wzGuid); 
	pRSSItem->m_ftPublished = pItem->ftPublished; 


	if (iIndex >= 0)
	{
		pRSSItem->m_EnclosureURL = OLE2T(pItem->m_Enclosures[iIndex].wzEnclosureUrl);
		pRSSItem->m_EnclosureSize = pItem->m_Enclosures[iIndex].i64EnclosureSize;
		pRSSItem->m_EnclosureType = OLE2T(pItem->m_Enclosures[iIndex].wzEnclosureType);
	}
	else
		return FALSE; 
	

	if (pRSSItem->m_EnclosureURL.GetLength() == 0)
		pRSSItem->m_EnclosureURL = pRSSItem->m_Link; 


	FString EnclosureImage = FindImageInEnclosures(pItem);
	if (PathIsURL(EnclosureImage))
		pRSSItem->m_ItemImage = EnclosureImage; 

	
	if (StrGuid.GetLength() == 0)
		StrGuid = pRSSItem->m_EnclosureURL;
	pRSSItem->m_Guid = crc32(StrGuid, StrGuid.GetLength(), 0); 

	if (pItem->pUnknownElements != NULL)
	{
		ConvertElements(pItem->pUnknownElements, pRSSItem->m_UnknownElements);
	}

	//See if we can find <media:content tag, which identifies the actual media file URL

	for (size_t k = 0; k < pRSSItem->m_UnknownElements.GetCount(); k++)
	{
		FRSSUnknownElement* pEle = pRSSItem->m_UnknownElements[k];
		//we ignore the namespace and hope for the best
		if (pEle->m_Element.CompareNoCase("content") == 0)
		{
			FString Url = GetAttribute(pEle, "url");
			if (PathIsURL(Url))
			{
				pRSSItem->m_EnclosureURL = Url; 
				pRSSItem->m_EnclosureType = GetAttribute(pEle, "type");
			}
			break; 
		}
	}

	return TRUE; 
}

BOOL FRSSReader::ConvertChannel(RSS_CHANNEL* pChannel, FRSSChannel* pRSSChannel)
{
	USES_CONVERSION; 
	pRSSChannel->m_Title = OLE2T(pChannel->wzTitle);
	pRSSChannel->m_Description = OLE2T(pChannel->wzDescription); 
	pRSSChannel->m_Link = OLE2T(pChannel->wzLink);
	pRSSChannel->m_dwTimeToLive = pChannel->dwTimeToLive;
	pRSSChannel->m_ChannelImage = OLE2T(pChannel->wzImageURL);

	EscapeBadChars(pRSSChannel->m_Title.GetBuffer());
	EscapeBadChars(pRSSChannel->m_Description.GetBuffer()); 

	if (pChannel->pUnknownElements != NULL)
	{
		ConvertElements(pChannel->pUnknownElements, pRSSChannel->m_UnknownElements);
	}

	//Items
	for (DWORD k = 0; k < pChannel->cItems; k++)
	{
		RSS_ITEM* pItem = &pChannel->rgItems[k]; 
		CAutoPtr<FRSSItem> pRSSItem (new FRSSItem); 
		
		if (ConvertItem(pItem, pRSSItem))
		{
			pRSSChannel->m_Items.Add(pRSSItem.Detach()); 
		}
	}

	return TRUE; 
}



void FRSSReader::ExtractImages(FRSSChannel* pChannel)
{
	//tags like blip:picture are recorded as: name='picture', namespace=blip
	for (size_t j = 0; j < pChannel->m_UnknownElements.GetCount(); ++j)
	{
		FRSSUnknownElement* testElement = pChannel->m_UnknownElements[j];
		if (testElement->m_Element.CompareNoCase("image") == 0 && testElement->m_Namespace.Find("itunes") >= 0)	{			//itunes:image
			pChannel->m_ChannelImage = GetAttribute(testElement, "href");
		} else if (testElement->m_Element.CompareNoCase("picture") == 0 && testElement->m_Namespace.Find("blip.tv") >= 0) {	//blip:picture
			pChannel->m_ChannelImage = testElement->m_Value;	
		} 
	}

	for (size_t i = 0; i < pChannel->m_Items.GetCount(); ++i)
	{
		FRSSItem* pItem = pChannel->m_Items[i];

		if (pItem->m_ItemImage.GetLength() > 0)
			continue; 

		int spos = pItem->m_Description.Find("<img");	//locate start of tag
		if (spos >= 0)
		{
			int epos = pItem->m_Description.Find(">", spos);	//locate end of tag
			int srcpos = pItem->m_Description.Find("src=", spos);	//locate src attribute
			int linkpos = srcpos + 5;	//skip the src=" string
			int endlinkpos = pItem->m_Description.Find("\"", linkpos);
			pItem->m_ItemImage = pItem->m_Description.Mid(linkpos, endlinkpos - linkpos);
			pItem->m_Description.Delete(spos, epos-spos+1);
		}
		else
		{
			bool setImage = false;
			for (size_t j = 0; j < pItem->m_UnknownElements.GetCount(); ++j)
			{
				FRSSUnknownElement* testElement = pItem->m_UnknownElements[j];
				if (testElement->m_Element.CompareNoCase("image") == 0 && testElement->m_Namespace.Find("itunes") >= 0)	{			//itunes:image
					pItem->m_ItemImage = GetAttribute(testElement, "href");
				} else if (testElement->m_Element.CompareNoCase("picture") == 0 && testElement->m_Namespace.Find("blip.tv") >= 0) {	//blip:picture
					pItem->m_ItemImage = testElement->m_Value;	
				} else if (testElement->m_Element.CompareNoCase("thumbnail") == 0) {
					pItem->m_ItemImage = GetAttribute(testElement, "url");
				} else if (testElement->m_Element.CompareNoCase("image") == 0 && testElement->m_Namespace == "")
				{
					pItem->m_ItemImage = testElement->m_Value;
				} 
			}
		}
	}
}

HRESULT FRSSReader::ParseRSS(const tchar* pszFileName, FRSSChannel** ppRSSChannel, FRSSFilter* pFilter)
{

	USES_CONVERSION; 
	RSS_CHANNEL* pChannel = NULL; 

	m_pFilter = pFilter; 
	LPCWSTR lpwFileName = T2OLE(pszFileName);

	HRESULT hr = RssParseFromFile(lpwFileName, &pChannel); 

	if (SUCCEEDED(hr) && pChannel != NULL)
	{
		CAutoPtr<FRSSChannel> pRSSChannel(new FRSSChannel); 
		if (ConvertChannel(pChannel, pRSSChannel))
		{			
			*ppRSSChannel = pRSSChannel.Detach(); 
			ExtractImages(*ppRSSChannel);
			hr = S_OK; 
		}
		else
		{
			hr = E_FAIL; 
		}
	}

	if (pChannel)
		RssFreeChannel(pChannel); 
	return hr; 
}


void FRSSReader::FreeChannel(FRSSChannel* pChannel)
{
	delete pChannel; 
}

bool FRSSFilter::operator != (const FRSSFilter& rhs)
{
	return !operator==(rhs); 	
}
bool FRSSFilter::operator == (const FRSSFilter& rhs)
{
	return m_Contains == rhs.m_Contains && 
		m_NotContains == rhs.m_NotContains; 
}