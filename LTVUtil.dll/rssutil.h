//-------------------------------------------------------------------------------------------------
// <copyright file="rssutil.h" company="Microsoft">
//    Copyright (c) Microsoft Corporation.  All rights reserved.
//
//    The use and distribution terms for this software are covered by the
//    Common Public License 1.0 (http://opensource.org/licenses/cpl.php)
//    which can be found in the file CPL.TXT at the root of this distribution.
//    By using this software in any fashion, you are agreeing to be bound by
//    the terms of this license.
//
//    You must not remove this notice, or any other, from this software.
// </copyright>
//
// <summary>
//    RSS helper funtions.
// </summary>
//-------------------------------------------------------------------------------------------------

#pragma once


#ifdef __cplusplus
extern "C" {
#endif

#define ReleaseRssChannel(p) if (p) { RssFreeChannel(p); }
#define ReleaseNullRssChannel(p) if (p) { RssFreeChannel(p); p = NULL; }


	struct RSS_UNKNOWN_ATTRIBUTE
	{
		CComBSTR wzNamespace;
		CComBSTR wzAttribute;
		CComBSTR wzValue;

		RSS_UNKNOWN_ATTRIBUTE* pNext;
		~RSS_UNKNOWN_ATTRIBUTE();
		RSS_UNKNOWN_ATTRIBUTE()
		{
			pNext = NULL; 
		}
	};

	struct RSS_UNKNOWN_ELEMENT
	{
		CComBSTR wzNamespace;
		CComBSTR wzElement;
		CComBSTR wzValue;

		RSS_UNKNOWN_ATTRIBUTE* pAttributes;
		RSS_UNKNOWN_ELEMENT* pNext;
		~RSS_UNKNOWN_ELEMENT();
		RSS_UNKNOWN_ELEMENT(){
			pAttributes = NULL; 
			pNext = NULL; 
		}
	};

	struct RSS_ENCLOSURE
	{
		CComBSTR	wzEnclosureUrl;
		__int64		i64EnclosureSize;
		CComBSTR	wzEnclosureType;
		RSS_ENCLOSURE()
		{
			i64EnclosureSize = 0; 
		}
	};

	struct RSS_ITEM
	{
		CComBSTR wzTitle;
		CComBSTR wzLink;
		CComBSTR wzDescription;

		CComBSTR wzGuid;
		FILETIME ftPublished;

		std::vector<RSS_ENCLOSURE> m_Enclosures; 

		RSS_UNKNOWN_ELEMENT* pUnknownElements;
		~RSS_ITEM();
		RSS_ITEM(){
			pUnknownElements = NULL; 
			ftPublished.dwHighDateTime = 0; 
			ftPublished.dwLowDateTime = 0; 
		}
	};

	struct RSS_CHANNEL
	{
		CComBSTR wzTitle;
		CComBSTR wzLink;
		CComBSTR wzDescription;
		CComBSTR wzImageURL;
		DWORD dwTimeToLive;

		RSS_UNKNOWN_ELEMENT* pUnknownElements;

		DWORD cItems;
		RSS_ITEM* rgItems;
		RSS_CHANNEL()
		{
			pUnknownElements = NULL; 
			rgItems = NULL; 
			dwTimeToLive = 0; 
		}
		~RSS_CHANNEL();
	};

	HRESULT DAPI RssInitialize(
		);

	void DAPI RssUninitialize(
		);

	HRESULT DAPI RssParseFromString(
		__in LPCWSTR wzRssString,
		__out RSS_CHANNEL **ppChannel
		);

	HRESULT DAPI RssParseFromFile(
		__in LPCWSTR wzRssFile,
		__out RSS_CHANNEL **ppChannel
		);

	// Adding this until we have the updated specstrings.h
#ifndef __in_xcount
#define __in_xcount(size)
#endif

	void DAPI RssFreeChannel(
		__in_xcount(pChannel->cItems) RSS_CHANNEL *pChannel
		);

#ifdef __cplusplus
}
#endif