#ifndef __FMTSTATUSSTRING_H__
#define __FMTSTATUSSTRING_H__

#include "Utils.h"

const int bufferGrowSize = 8192; 
const int maxItemLength  = 1024; 
class FMTItemStatusString
{
	struct StatusItem
	{
		FString m_StrName; 
		FString m_StrValue; 
	};
public:
	CAtlArray<StatusItem> m_Items;
	FString				  m_VideoID; 



	void AddValue(const char* pValueName, const char* pFmt, ...)
	{
		va_list paramList;
		va_start(paramList, pFmt);
		StatusItem aItem; m_Items.Add(aItem);
		StatusItem& pItem = m_Items.GetAt(m_Items.GetCount()-1);
		pItem.m_StrName = pValueName; 
		pItem.m_StrValue.FormatV(pFmt, paramList); 
	}

	//Output a Javascript struct
	//{field: 'value', field: 'value', field: 'value'}
	size_t BuildJavaScriptStruct(char* aOut)
	{
        char* aOrg = aOut; 
		strcat(aOut, "{");
		FString aItem;
		FString aValue;
        
		for (size_t k = 0; k < m_Items.GetCount(); k++)
		{
			
			StatusItem& pItem = m_Items[k];

			const char* pFmtString = ", %s: '%s'";	  
			if (k == 0)
				pFmtString = "%s: '%s'";

			EscapeJString(pItem.m_StrValue, aValue); 
			aItem.Format(pFmtString, pItem.m_StrName, aValue);
			strcat(aOut, aItem); 
            if ((aOut - aOrg) + aItem.GetLength() < maxItemLength - 1)
                aOut+=aItem.GetLength(); 
            else
            {
                _DBGAlert("***BuildJavaScriptStruct buffer overflow!\n"); 
                break; 
            }
		}
		strcat(aOut, "}");
		return aOut - aOrg; 
	}

	void SetVideoID(const char* videoID)
	{
		m_VideoID.Format("<video id=\"%s\">\n", videoID); 
	}
};


class FMTStatusStringXML
{
	CAtlArray<FMTItemStatusString *>  m_aVideos; 
	CAtlString						  m_OutBuffer; 
    size_t							  m_totalLength; 
public:

	FMTStatusStringXML()
	{
        m_totalLength = 0; 
		m_OutBuffer.GetBufferSetLength(bufferGrowSize); 
		*m_OutBuffer.GetBuffer() = 0; 
	}
	~FMTStatusStringXML()
	{
		Clear(); 
	}
	FMTItemStatusString* AddVideo(const char* videoID)
	{
		FMTItemStatusString* p = new FMTItemStatusString; 
		p->SetVideoID(videoID); 
		m_aVideos.Add(p); 
		return p; 
	}

    void ResizeBuffer()
    {
        int allocLen = m_OutBuffer.GetAllocLength(); 
        if (m_OutBuffer.GetAllocLength() - m_totalLength < maxItemLength)
        {
            FString fTemp = m_OutBuffer; 
            m_OutBuffer.GetBufferSetLength(m_OutBuffer.GetAllocLength() + bufferGrowSize); 
            strcpy(m_OutBuffer.GetBuffer(), fTemp);
        }
    }

    int BuildJavaScriptArray()
	{

        ResizeBuffer(); 
		tchar* pBuf = m_OutBuffer.GetBuffer(); 
		memset(pBuf, 0, m_OutBuffer.GetAllocLength()); 

		*pBuf = 0;
		strcpy(pBuf, "[");
		for (size_t k = 0; k < m_aVideos.GetCount(); k++)
		{
            ResizeBuffer(); 
            pBuf = m_OutBuffer.GetBuffer(); //may change
			size_t len = m_aVideos[k]->BuildJavaScriptStruct(pBuf); 
			pBuf+=len; 
            
			if (k < m_aVideos.GetCount() - 1)
            {
				strcat(pBuf, ",\n"); 
                len+=2; 
            }

            m_totalLength+=len; 
		}

		strcat(pBuf, "]"); 
		return m_OutBuffer.GetLength(); 
	}

	void Clear()
	{
		for (size_t k = 0; k < m_aVideos.GetCount(); k++)
		{
			delete m_aVideos[k];
			m_aVideos[k] = NULL; 
		}
		m_aVideos.RemoveAll();
        m_totalLength = 0; 
	}

	CAtlString& GetJavaScriptBuffer()
	{
		return m_OutBuffer; 
	}
};
#endif //__FMTSTATUSSTRING_H__