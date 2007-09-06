#pragma once
#include <msxml2.h>
#include "Utils.h"

class LTVUTILDLL_API FXMLUtil
{
public:
	//<tag>text</tag>
	static HRESULT GetTagText(IXMLDOMNode* pRootNode, const char* pszTagName, CComBSTR& bstrTagValue);
	//<tag name="elementValue">
	static HRESULT GetTagAttributeValue(IXMLDOMNode* pRootNode, const char* pszTagName, const char* pszPropName, CComVariant& vtAttVal); 
	static HRESULT GetNextElement(IXMLDOMNodeList* pList, CComPtr<IXMLDOMElement>& pEle); 
	static HRESULT GetNodeAttributeValue(IXMLDOMNode* pNode, const char* pszAttName, CComVariant& pAttVal); 

	static FString GetTagText(IXMLDOMNode* pRootNode, const char* pszTagName);
	static FString GetTagAttributeValue(IXMLDOMNode* pRootNode, const char* pszTagName, const char* pszPropName);
	static FString GetNodeAttributeValue(IXMLDOMNode* pNode, const char* pszAttName); 
};

struct LTVUTILDLL_API FXMLVal
{
	CComVariant m_Val; 

	FXMLVal(const char* pszVal);
	FXMLVal(const FXMLVal& rhs);
	FXMLVal(const CComVariant &vt);
	FXMLVal();
	FXMLVal& operator = (const FXMLVal& rhs);
	FString ToString();
	ULONG	ToULong(int radix = 10);
	UINT64	ToUINT64(int radix = 10);
	long	ToLong(int radix=10);
};

class LTVUTILDLL_API FXMLNode 
{
protected:

public:
	CComQIPtr<IXMLDOMNode> m_pNode; 
	FXMLNode(IXMLDOMNode* pNode);
	FXMLNode(FXMLNode& pNode);
	FXMLNode(IXMLDOMElement* pElement);
	FXMLNode();

	FXMLNode& operator = (IXMLDOMNode* pNode);
	operator IXMLDOMNode*()
	{
		return m_pNode;
	}
	FXMLVal GetText();
	FXMLVal GetAttribute(const char* pszName); 
	FXMLNode GetChild(const char* pszTagName, BOOL bCreateIfNotExists = FALSE);
	FXMLNode GetChildEx(const char* pszTagName);
	HRESULT SetText(const char* pszText);
	HRESULT SetAttribute(const char* pszName, CComVariant& vtVal);
	HRESULT SetAttribute(const char* pszName, const char* pszVal);
	HRESULT SetAttribute(const char* pszName, ULONG ulVal);
	HRESULT SetAttribute(const char* pszName, UINT64 ui64Val);
	//Formats the whole tree, by adding newlines and tabs to siblings and children
	void	Format(int NestingLevel, IXMLDOMDocument* pDoc = NULL); 
};