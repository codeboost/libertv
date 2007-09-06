#pragma once

#include "FJScriptArrayBuilderA.h"
struct CountStruct{
	DWORD dwNewItems; 
	DWORD dwTotalItems; 
	DWORD dwShownItems; 
	CountStruct()
	{
		dwNewItems = dwTotalItems = dwShownItems = 0; 
	}
};

class FRSSArrayBuilder
{
public:

	HRESULT RSSGetItems(IDispatch* pDispScript, IDispatch* pDispOutArray, IDispatch* pDispOptions, CountStruct& Counts);
	HRESULT RSSGetChannelItems(IDispatch* pDispScript, IDispatch* pDispOutArray, IDispatch* pDispOptions, CountStruct& Counts);
};

