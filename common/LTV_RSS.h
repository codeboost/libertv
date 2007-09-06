#pragma once
#include "utils.h"
#include "FGuids.h"


struct LTVUTILDLL_API FRSSFilter
{
	FString m_Contains; 
	FString m_NotContains; 
	bool operator != (const FRSSFilter& rhs);
	bool operator == (const FRSSFilter& rhs);
};

struct LTVUTILDLL_API FRSSAttribute{
	FString m_Namespace; 
	FString m_Attribute; 
	FString m_Value; 
};

struct LTVUTILDLL_API FRSSUnknownElement{
	FString					m_Namespace;
	FString					m_Element;
	FString					m_Value;
	FArray<FRSSAttribute*>	m_Attributes; 
	virtual ~FRSSUnknownElement();
};

struct LTVUTILDLL_API FRSSItem{

	FString 	m_Title; 
	FString 	m_Link; 
	FString 	m_Description; 
	DWORD		m_Guid; 
	FILETIME	m_ftPublished;
	FString		m_EnclosureURL; 
	LONGLONG	m_EnclosureSize; 
	FString		m_EnclosureType; 
	FString		m_ItemImage; 
	DWORD		m_videoID; 
	DWORD		m_dwFlags;
	DWORD		m_dwChannelId; 
	FArray<FRSSUnknownElement*> 
				m_UnknownElements; 
	FRSSItem();
	virtual ~FRSSItem(); 
};

struct LTVUTILDLL_API FRSSChannel{
	FString		m_Title;
	FString		m_Link;
	FString		m_Description;
	FString		m_ChannelImage; 
	FString		m_Author; 
	DWORD		m_dwTimeToLive;
	FArray<FRSSUnknownElement*> m_UnknownElements; 
	FArray<FRSSItem*> m_Items; 
	~FRSSChannel(); 
	FRSSChannel(); 
};
struct RSS_UNKNOWN_ATTRIBUTE;
struct RSS_UNKNOWN_ELEMENT;
struct RSS_ITEM; 
struct RSS_CHANNEL;


class LTVUTILDLL_API FRSSReader{

protected:
	DWORD m_ChannelID; 
	FRSSFilter* m_pFilter; 
public:
	FRSSReader(); 
	virtual ~FRSSReader();
	HRESULT ParseRSS(const tchar* pszFileName, FRSSChannel** pChannel, FRSSFilter* pFilter = NULL); 
	void	ExtractImages(FRSSChannel* pChannel);
	void	FreeChannel(FRSSChannel* pChannel); 
	void	Clear(); 
protected:
	BOOL	ConvertAttribute(RSS_UNKNOWN_ATTRIBUTE* pAttribute, FRSSAttribute* pRSSAttr);
	BOOL	ConvertElement(RSS_UNKNOWN_ELEMENT* pElement, FRSSUnknownElement* pRSSEle);
	void	ConvertElements(RSS_UNKNOWN_ELEMENT* pEle, FArray<FRSSUnknownElement*>& aElements);
	BOOL	ConvertItem(RSS_ITEM* pItem, FRSSItem* pRSSItem);
	BOOL	ConvertChannel(RSS_CHANNEL* pChannel, FRSSChannel* pRSSChannel);
};

