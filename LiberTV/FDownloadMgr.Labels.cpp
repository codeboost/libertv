#include "stdafx.h"
#include "FDownloadMgr.h"

void FDownloadMgr::SetLabel(vidtype videoID, const tchar* pszLabels)
{
	SynchronizeThread(m_Sync);
	FDownload* pDown = FindByID(videoID); 
	if (pDown)
	{
		pDown->m_Detail.m_Labels.ParseLabelStr(pszLabels); 
		SaveDownload(pDown); 
	}
}

void FDownloadMgr::SetLabel(const FArray<FString>& aItems, const tchar* pszLabels)
{
	SynchronizeThread(m_Sync);
	for (size_t k = 0; k < aItems.GetCount(); k++)
	{
		SetLabel(strtoul(aItems[k], NULL, 10), pszLabels); 
	}
}

void FDownloadMgr::AddLabel(vidtype videoID, const tchar* pszLabel)
{
	SynchronizeThread(m_Sync); 
	FDownload* pDown = FindByID(videoID); 
	if (pDown)
	{
		pDown->m_Detail.m_Labels.AddLabel(pszLabel);
		SaveDownload(pDown); 
	}
}

void FDownloadMgr::AddLabel(const FArray<FString>& aItems, const tchar* pszLabel)
{
	SynchronizeThread(m_Sync);
	for (size_t k = 0; k < aItems.GetCount(); k++)
	{
		AddLabel(strtoul(aItems[k], NULL, 10), pszLabel); 
	}
}


BOOL _RemoveLabel(FDownload* pDown, const tchar* pszLabel)
{
	if (pDown)
	{
		pDown->m_Detail.m_Labels.RemoveLabel(pszLabel); 
		return TRUE; 
	}
	return FALSE; 
}

void FDownloadMgr::RemoveLabel(vidtype videoID, const tchar* pszLabel)
{
	SynchronizeThread(m_Sync);
	FDownload * pDown = FindByID(videoID); 
	if (pDown && _RemoveLabel(pDown, pszLabel))
	{
		SaveDownload(pDown); 
	}
}
void FDownloadMgr::RemoveLabel(const FArray<FString>& aItems, const tchar* pszLabel)
{
	SynchronizeThread(m_Sync);
	for (size_t k = 0; k < aItems.GetCount(); k++)
	{
		RemoveLabel(strtoul(aItems[k], NULL, 10), pszLabel); 
	}
}

FString FDownloadMgr::GetLabel(vidtype videoID)
{
	SynchronizeThread(m_Sync);
	FDownload* pDown = FindByID(videoID); 
	if (pDown)
	{
		return pDown->m_Detail.m_Labels.GetLabelStr();
	}
	return ""; 
}


void FDownloadMgr::RemoveLabel(const tchar* pszLabel)
{
	SynchronizeThread(m_Sync); 

	for (size_t k = 0; k < m_Downloads.GetCount(); k++)
	{
		if (_RemoveLabel(m_Downloads[k], pszLabel))
		{
			SaveDownload(m_Downloads[k]);
		}
	}
}

FString FDownloadMgr::GetVideoLabels()
{
	SynchronizeThread(m_Sync); 
	FString OutStr;
	size_t itemCount = m_Downloads.GetCount(); 

	int count = 0; 
	for (size_t k = 0; k < itemCount; k++)
	{
		FDownload* pDown = m_Downloads[k];
		FString& LabelStr = pDown->m_Detail.m_Labels.GetLabelStr();
		if (LabelStr.GetLength() > 0)
		{
			if (count > 0)
				OutStr.Append(";");

			FString ItemStr;
			ItemStr.Format("%d,%s", pDown->m_Detail.m_VideoID, LabelStr);
			OutStr.Append(ItemStr); 
			count++; 
		}
	}

	return OutStr; 
}