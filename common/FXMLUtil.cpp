#include "stdafx.h"
#include "FXMLUtil.h"


HRESULT FXMLUtil::GetNextElement(IXMLDOMNodeList* pList, CComPtr<IXMLDOMElement>& pEle)
{
	HRESULT hr = S_OK; 
	CComPtr<IXMLDOMNode> pNextEle; 
	while (hr == S_OK)
	{
		hr = pList->nextNode(&pNextEle); 
		if (SUCCEEDED(hr) && pNextEle)
		{
			DOMNodeType nodeType; 
			pNextEle->get_nodeType(&nodeType);
			if (nodeType == NODE_ELEMENT)
				break; 
			pNextEle.Release(); 
		}
	}

	pEle = pNextEle; 
	return hr; 
}

HRESULT FXMLUtil::GetTagText(IXMLDOMNode* pRootNode, const char* pszTagName, CComBSTR& bstrTagValue)
{
	HRESULT hr = E_FAIL; 
	CComPtr<IXMLDOMNode> pNode; 
	hr = pRootNode->selectSingleNode(CComBSTR(pszTagName), &pNode); 
	if (SUCCEEDED(hr) && pNode != NULL)
	{
		hr = pNode->get_text(&bstrTagValue); 
	}
	return hr; 
}

HRESULT FXMLUtil::GetTagAttributeValue( IXMLDOMNode* pRootNode, const char* pszTagName, const char* pszPropName, CComVariant& vtAttVal )
{
	HRESULT hr = E_FAIL ; 
	CComPtr<IXMLDOMNode> pNode; 
	hr = pRootNode->selectSingleNode(CComBSTR(pszTagName), &pNode); 

	if (SUCCEEDED(hr) && pNode != NULL)
	{
		CComPtr<IXMLDOMNamedNodeMap> attMap; 
		hr = pNode->get_attributes(&attMap);
		if (SUCCEEDED(hr) && attMap != NULL)
		{
			CComPtr<IXMLDOMNode> pNamedItem; 
			hr = attMap->getNamedItem(CComBSTR(pszPropName), &pNamedItem); 
			if (SUCCEEDED(hr) && pNamedItem != NULL)
			{
				hr = pNamedItem->get_nodeValue(&vtAttVal); 
			}
		}
	}
	return hr; 
}

FString FXMLUtil::GetTagText(IXMLDOMNode* pRootNode, const char* pszTagName)
{
	CComBSTR bstrText; 
	FString Text;
	if (SUCCEEDED(GetTagText(pRootNode, pszTagName, bstrText)))
	{
		USES_CONVERSION; 
		Text = OLE2T(bstrText);
	}
	return Text; 
}

FString FXMLUtil::GetTagAttributeValue(IXMLDOMNode* pRootNode, const char* pszTagName, const char* pszPropName)
{
	FString AttVal; 
	CComVariant vtAttVal; 
	if (SUCCEEDED(GetTagAttributeValue(pRootNode, pszTagName, pszPropName, vtAttVal)))
	{
		if (vtAttVal.vt == VT_BSTR)
		{
			USES_CONVERSION; 
			AttVal = OLE2T(vtAttVal.bstrVal);
		}
	}
	return AttVal; 
}

HRESULT FXMLUtil::GetNodeAttributeValue(IXMLDOMNode* pNode, const char* pszAttName, CComVariant& pAttVal)
{
	CComPtr<IXMLDOMNamedNodeMap> attMap; 
	HRESULT hr = pNode->get_attributes(&attMap);
	if (SUCCEEDED(hr) && attMap != NULL)
	{
		CComPtr<IXMLDOMNode> pNamedItem; 
		hr = attMap->getNamedItem(CComBSTR(pszAttName), &pNamedItem); 
		if (SUCCEEDED(hr) && pNamedItem != NULL)
		{
			hr = pNamedItem->get_nodeValue(&pAttVal); 
		}
	}
	return hr; 
}

FString FXMLUtil::GetNodeAttributeValue(IXMLDOMNode* pNode, const char* pszAttName)
{
	CComVariant vt; 
	FString AttVal ; 
	if (SUCCEEDED(GetNodeAttributeValue(pNode, pszAttName, vt)))
	{
		USES_CONVERSION; 
		if (vt.vt == VT_BSTR)
			AttVal = OLE2T(vt.bstrVal); 
	}
	return AttVal; 
}


//////////////////////////////////////////////////////////////////////////

FString FXMLVal::ToString()
{
	USES_CONVERSION; 
	if (SUCCEEDED(m_Val.ChangeType(VT_BSTR)))
		return OLE2T(m_Val.bstrVal);

	return "";
}

ULONG FXMLVal::ToULong( int radix /*= 10*/ )
{
	//return strtoul(m_Val, NULL, radix);
	if (SUCCEEDED(m_Val.ChangeType(VT_UINT)))
	{
		return m_Val.uiVal;
	}
	return 0; 
}

LONG FXMLVal::ToLong(int radix/* =10 */)
{
	if (SUCCEEDED(m_Val.ChangeType(VT_I4)))
		return m_Val.lVal;
	return 0; 
}

UINT64 FXMLVal::ToUINT64( int radix /*= 10*/ )
{
	if (SUCCEEDED(m_Val.ChangeType(VT_BSTR)))
	{
		USES_CONVERSION; 
		return _strtoui64(OLE2T(m_Val.bstrVal), NULL, radix);
	}
	return 0; 
}

FXMLVal::FXMLVal( const char* pszVal )
{
	m_Val = pszVal; 
}

FXMLVal::FXMLVal( const FXMLVal& rhs )
{
	m_Val = rhs.m_Val;
}

FXMLVal::FXMLVal()
{

}

FXMLVal::FXMLVal( const CComVariant &vt )
{
	m_Val = vt;
}
FXMLVal& FXMLVal::operator=( const FXMLVal& rhs )
{
	if (this != &rhs)
		m_Val = rhs.m_Val; 
	return *this;
}

FXMLVal FXMLNode::GetText()
{
	if (m_pNode)
	{
		CComBSTR bstrText; 
		if (SUCCEEDED(m_pNode->get_text(&bstrText)))
		{
			return bstrText;
		}
	}
	return ""; 
}

FXMLVal FXMLNode::GetAttribute(const char* pszName)
{
	USES_CONVERSION; 
	CComQIPtr<IXMLDOMElement> pEle = m_pNode; 
	CComVariant vt; 
	if (pEle)
	{
		pEle->getAttribute(T2OLE(pszName), &vt);
	}
	return vt;
}

FXMLNode::FXMLNode( IXMLDOMNode* pNode )
{
	m_pNode = pNode;
}

FXMLNode::FXMLNode( FXMLNode& pNode )
{
	m_pNode = pNode.m_pNode;
}

FXMLNode::FXMLNode( IXMLDOMElement* pElement )
{
	m_pNode = pElement;
}

FXMLNode::FXMLNode()
{
	
}

int GetNestingCount(IXMLDOMNode* pNode, IXMLDOMDocument* pDoc)
{
	CComPtr<IXMLDOMNode> thisNode = pNode; 
	CComPtr<IXMLDOMNode> parentNode; 

	int nestCount = 0; 
	while (thisNode)
	{
		HRESULT hr = thisNode->get_parentNode(&parentNode); 
		if (hr == S_OK)
			nestCount++; 
		else
			break; 
		thisNode = parentNode; 
		parentNode.Release(); 
	}
	return nestCount; 
}

FXMLNode FXMLNode::GetChild( const char* pszTagName, BOOL bCreateIfNotExists)
{
	if (m_pNode)
	{
		HRESULT hr = E_FAIL; 
		CComPtr<IXMLDOMNode> pNode; 
		hr = m_pNode->selectSingleNode(CComBSTR(pszTagName), &pNode); 
		if (SUCCEEDED(hr) && pNode != NULL)
		{
			return FXMLNode(pNode); 
		}
		else
		{
			CComPtr<IXMLDOMDocument> pDoc; 
			m_pNode->get_ownerDocument(&pDoc); 
			if (pDoc)
			{
				USES_CONVERSION; 
				CComPtr<IXMLDOMElement> pNewElement; 
				pDoc->createElement(T2OLE(pszTagName), &pNewElement); 
				if (pNewElement && SUCCEEDED(m_pNode->appendChild(pNewElement, NULL)))
				{
					/*
					//Format the node
					int nestCount = GetNestingCount(pNewElement, pDoc); 

					FString Fmt; 
					FString Tabs; 
					
					Fmt.Append("\n");
					for (int i = 0; i < nestCount; i++)
					{
						Tabs.Append("\t");
					}

					Fmt.Append(Tabs);
					
					CComPtr<IXMLDOMText> pws;
					CComPtr<IXMLDOMText> pAfter; 
					pDoc->createTextNode(CComBSTR(Fmt), &pws); 
					Fmt = Tabs; 
					Fmt.Append("\n");
					pDoc->createTextNode(CComBSTR(Fmt), &pAfter); 
					if (pws && pAfter)
					{
						CComPtr<IXMLDOMNode> pNew;
						CComVariant vtBefore = pNewElement; 
						m_pNode->insertBefore(pws, vtBefore, &pNew); 
						pNew.Release(); 
						VARIANT_BOOL bHasChildren; 
						m_pNode->hasChildNodes(&bHasChildren);
						if (bHasChildren == VARIANT_FALSE)
							m_pNode->appendChild(pAfter, &pNew);
					}
					*/
					return FXMLNode(pNewElement);
				}
			}
		}
	}
	return FXMLNode(); 
}

FXMLNode& FXMLNode::operator=( IXMLDOMNode* pNode )
{
	m_pNode = pNode; 
	return *this;
}

HRESULT FXMLNode::SetText( const char* pszText )
{
	if (m_pNode)
	{
		USES_CONVERSION; 
		return m_pNode->put_text(T2OLE(pszText));
	}
	return E_FAIL;
}

HRESULT FXMLNode::SetAttribute( const char* pszName, CComVariant& vtVal )
{
	USES_CONVERSION; 
	CComQIPtr<IXMLDOMElement> pElement = m_pNode; 
	HRESULT hr = E_FAIL; 
	if (pElement)
		hr = pElement->setAttribute(T2OLE(pszName), vtVal); 
	return hr;
}

HRESULT FXMLNode::SetAttribute( const char* pszName, const char* pszVal )
{
	return SetAttribute(pszName, CComVariant(pszVal));
}

HRESULT FXMLNode::SetAttribute( const char* pszName, ULONG ulVal )
{
	return SetAttribute(pszName, CComVariant(ulVal));
}

HRESULT FXMLNode::SetAttribute( const char* pszName, UINT64 ui64Val )
{
	return SetAttribute(pszName, CComVariant(ui64Val));
}

FXMLNode FXMLNode::GetChildEx( const char* pszTagName )
{
	return GetChild(pszTagName, TRUE);
}

void FXMLNode::Format(int NestingLevel, IXMLDOMDocument* pDoc)
{
	CComPtr<IXMLDOMNodeList> pChildren;
	
	if (NULL == pDoc)
		m_pNode->get_ownerDocument(&pDoc); 

	if (NULL == pDoc)
		return; 

	HRESULT hr = m_pNode->get_childNodes(&pChildren);
	if (pChildren)
	{
		long lCount; 
		if (SUCCEEDED(pChildren->get_length(&lCount)))
		{
			for (long lItem = 0; lItem < lCount; lItem++)
			{
				FXMLNode pChild; 
				if (SUCCEEDED(pChildren->get_item(lItem, &pChild.m_pNode) && pChild))
				{
					pChild.Format(NestingLevel + 1, pDoc);
					pChild.m_pNode.Release(); 
				}
			}
		}
	}

	CComBSTR nodeName; 
	m_pNode->get_nodeName(&nodeName); 

	CComPtr<IXMLDOMNode> pChild; 
	m_pNode->get_firstChild(&pChild); 
	if (pChild)
	{
		FString Fmt = "\n";
		for (int i = 0; i < NestingLevel; i++)
			Fmt.Append("\t");

		CComPtr<IXMLDOMText> pText; 
		CComPtr<IXMLDOMNode> pNewNode; 
		pDoc->createTextNode(CComBSTR(Fmt), &pText); 
		
		CComVariant vtNode = pChild; 
		hr = m_pNode->insertBefore(pText, vtNode, &pNewNode); 

		if (pNewNode)
			pNewNode.Release(); 
		
		if (NestingLevel > 0)
		{
			FString Fmt = "\n";
			for (int i = 0; i < NestingLevel - 1; i++)
				Fmt.Append("\t");

			pChild.Release(); 
			vtNode = pChild; 
			pText.Release(); 
			pDoc->createTextNode(CComBSTR(Fmt), &pText); 
			hr = m_pNode->insertBefore(pText, vtNode, &pNewNode); 
		}
	} 
}