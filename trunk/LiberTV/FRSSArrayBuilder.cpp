#include "stdafx.h"
#include "FRSSArrayBuilder.h"
#include "FRSSManager.h"
#include "GlobalObjects.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

void AddRSSItem(FJScriptArrayBuilder& jsArray, DWORD dwChannelID, const FRSSItem* pItem)
{
	jsArray.NewElement(",");
	jsArray.AddValueUINT("", dwChannelID); 
	jsArray.AddValueUINT("", pItem->m_Guid); //Add as string
	jsArray.AddValueStr("", pItem->m_Title); 
	jsArray.AddValueStr("", pItem->m_Link); 
	jsArray.AddValueUINT("", (UINT)FileTimeToUnixTime((LARGE_INTEGER*)&pItem->m_ftPublished));
	jsArray.AddValueDouble("", (double)pItem->m_EnclosureSize);
	jsArray.AddValueStr("", pItem->m_EnclosureType);
	jsArray.AddValueStr("", pItem->m_EnclosureURL);
	jsArray.AddValueUINT("", pItem->m_videoID); 
	jsArray.AddValueUINT("", pItem->m_dwFlags); 
	jsArray.AddValueStr("", pItem->m_Description);
	jsArray.AddValueStr("", pItem->m_ItemImage); 
}

struct SortItem{
	DWORD	  dwChannelID; 
	FRSSItem* pItem; 
	SortItem(DWORD ChannelID, FRSSItem* Item){
		dwChannelID = ChannelID; 
		pItem = Item; 
	}
};
bool SortFunc(const FRSSItem* a1, const FRSSItem* a2){
	return CompareFileTime(&a1->m_ftPublished, &a2->m_ftPublished) > 0; 
}


size_t RSSFilterChannel(FRSSFeed* pFeed, std::vector<FRSSItem*>& pItemArray, const tchar* pszFilter, DWORD dwMatchFlag = 0)
{
	if (pFeed == NULL || pFeed->m_pChannel == NULL)
		return 0; 

	FRSSChannel* pChannel = pFeed->m_pChannel; 

	size_t totalCount = pChannel->m_Items.GetCount(); 
	size_t itemIndex = 0; 
	size_t filterLen = strlen(pszFilter);

	for (size_t k = 0; k < totalCount; k++)
	{
		FRSSItem* pItem = pChannel->m_Items[k];
		pItem->m_dwChannelId = pFeed->m_dwChannelID; 

		if (dwMatchFlag > 0 && !(pItem->m_dwFlags & dwMatchFlag))
			continue; 

		if (filterLen > 0)
		{
			
			if (stristr(pItem->m_Title, pszFilter) != NULL ||
				stristr(pItem->m_Description, pszFilter) != NULL)
			{
				pItemArray.push_back(pItem); 
			}
		}
		else
			pItemArray.push_back(pItem); 
	}
	return pItemArray.size(); 
}

//returns filtered items for a channel
HRESULT FRSSArrayBuilder::RSSGetChannelItems(IDispatch* pDispScript, IDispatch* pDispOutArray, IDispatch* pDispOptions, CountStruct& counters)
{
	FJScriptArrayBuilder jsArray; 
	jsArray.SetDispatch(pDispScript, pDispOutArray); 
	jsArray.New();

	FDispObject pOptions;
	pOptions.SetDispatch(pDispOptions);

	FString StrFilter = pOptions.GetPropertyStr("filter");
	UINT channelID = pOptions.GetPropertyUINT("channelId");
	UINT uiTotalItems = pOptions.GetPropertyUINT("maxItems");
	UINT uiFrom = pOptions.GetPropertyUINT("offset");

	if (uiTotalItems == 0)
		uiTotalItems = 20; 

	FRSSFeed* pFeed = g_Objects._RSSManager->LockChannel(channelID);
	if (pFeed && pFeed->m_pChannel)
	{
		std::vector<FRSSItem*> pItems; 
		pItems.reserve(pFeed->m_pChannel->m_Items.GetCount());

		

		UINT TotalCount = (UINT)RSSFilterChannel(pFeed, pItems, StrFilter); 

		counters.dwTotalItems = TotalCount;

		UINT count = min(TotalCount, uiTotalItems); 

		if (uiFrom > TotalCount)
			uiFrom = TotalCount - count; 
		else
			if (uiFrom + count > TotalCount)
				count = TotalCount - uiFrom; 

		counters.dwShownItems = count; 
		for (UINT k = uiFrom; k < count + uiFrom; k++)
		{
			AddRSSItem(jsArray, channelID, pItems[k]); 
			if (pItems[k]->m_dwFlags & FEED_ITEM_NEW)
				counters.dwNewItems++; 
		}
	}

	FDispObject pOutArray; 
	pOutArray.SetDispatch(pDispOutArray); 
	pOutArray.AddProperty("TotalItems", &CComVariant(counters.dwTotalItems));
	pOutArray.AddProperty("NewItems", &CComVariant(counters.dwNewItems));

	jsArray.End();
	g_Objects._RSSManager->ReleaseChannelsLock();
	return S_OK; 
}

HRESULT FRSSArrayBuilder::RSSGetItems(IDispatch* pDispScript, IDispatch* pDispOutArray, IDispatch* pDispOptions, CountStruct& counters)
{
	USES_CONVERSION; 
	FJScriptArrayBuilder jsArray; 
	jsArray.SetDispatch(pDispScript, pDispOutArray); 
	jsArray.New();

	FDispObject pOptions;
	pOptions.SetDispatch(pDispOptions);

	FString StrFilter = pOptions.GetPropertyStr("filter");
	BOOL bShowNewItems = StrFilter.CompareNoCase("new") == 0;
	UINT channelID = pOptions.GetPropertyUINT("channelId");
	UINT uiTotalItems = pOptions.GetPropertyUINT("maxItems");
	UINT uiFrom = pOptions.GetPropertyUINT("offset");
	if (uiTotalItems == 0)
		uiTotalItems = 20; 

	DWORD dwTotalItems = 0; 
	DWORD dwNewItems = 0; 
	DWORD dwHidden = 0; 
	DWORD dwShownItems = 0; 
	GuidType ChannelGuid = 0;

	if (channelID > 2)	//not a special channel
	{
		RSSGetChannelItems(pDispScript, pDispOutArray, pDispOptions, counters);
		return S_OK; 
	}
	else
	if (channelID == 0 || channelID == 1)	//new items = 0, all items = 1, downloads = 2
	{
		DWORD dwMatchFlag  = channelID == 0 ? FEED_ITEM_NEW : 0;

		const FArray<FRSSFeed*>& aFeeds = g_Objects._RSSManager->LockChannels();
		//1. compute total number of items, allocate the pointer array
		//2. filter the items by matching strings
		//3. sort the array 
		

		size_t totalCount = 0; 
		for (size_t k = 0; k < aFeeds.GetCount(); k++)
			totalCount+=aFeeds[k]->m_pChannel ? aFeeds[k]->m_pChannel->m_Items.GetCount() : 0;

		

		std::vector<FRSSItem*> pItems; 
		pItems.reserve(totalCount);
		
		//2. filter items by matching strings
		for (size_t k = 0; k < aFeeds.GetCount(); k++)
			RSSFilterChannel(aFeeds[k], pItems, StrFilter, dwMatchFlag); 

		//3. sort the array
		sort(pItems.begin(), pItems.end(), SortFunc);

		UINT TotalCount = (UINT)pItems.size(); 

		counters.dwTotalItems = TotalCount; 
		//4. start from offset and return maxcount elements
		UINT count = min(TotalCount, uiTotalItems); 

		if (uiFrom > TotalCount)
			uiFrom = TotalCount - count; 
		else
		if (uiFrom + count > TotalCount)
			count = TotalCount - uiFrom; 

		counters.dwShownItems = count; 

		//5. add the items to the array
		for (UINT k = uiFrom; k < uiFrom + count; k++)
		{
			AddRSSItem(jsArray, pItems[k]->m_dwChannelId, pItems[k]); 
		}
		g_Objects._RSSManager->ReleaseChannelsLock();

		FDispObject pOutArray; 
		pOutArray.SetDispatch(pDispOutArray); 
		pOutArray.AddProperty("TotalItems", &CComVariant(counters.dwTotalItems));
	}
	

	jsArray.End();
	
	return S_OK;
}