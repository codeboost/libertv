#include "stdafx.h"
#include "FLabelMgr.h"

FLabelManager::FLabelManager()
{
}

FLabelManager::~FLabelManager()
{
	SynchronizeThread(m_Sync); 
	Save();
	m_Labels.RemoveAll(); 
	m_Conf.Clear();
}

int FLabelManager::FindLabel(const tchar* pszLabel)
{
	if (pszLabel == NULL)
		return - 1; 

	SynchronizeThread(m_Sync); 

	for (size_t k = 0; k < m_Labels.GetCount(); k++)
	{
		if (m_Labels[k] == pszLabel)
			return (int)k; 
	}
	return -1; 
}

void FLabelManager::AddLabel(const tchar* pszLabel)
{
	if (pszLabel == NULL)
		return; 

	SynchronizeThread(m_Sync); 
	if (FindLabel(pszLabel) == -1)
	{
		m_Labels.Add(FString(pszLabel)); 
		Save(); 
	}
}

void FLabelManager::RemoveLabel(const tchar* pszLabel)
{
	SynchronizeThread(m_Sync); 
	int kFind = FindLabel(pszLabel); 
	if (kFind != -1)
	{
		m_Labels.RemoveAt(kFind, 1); 
		Save(); 
	}
}

BOOL FLabelManager::Load(const tchar* pszFileName)
{
	SynchronizeThread(m_Sync); 
	m_Labels.RemoveAll(); 
	if (m_Conf.Load(pszFileName))
	{
		for (size_t k = 0; ; k++)
		{
			FString SectionName; 
			SectionName.Format("Label %d", k); 
			if (m_Conf.SectionExists(SectionName))
			{
				FString aLabel = m_Conf.GetValue(SectionName, "Caption"); 
				if (aLabel.GetLength() > 0)
				{
					if (aLabel.GetLength() > 20)
						aLabel = aLabel.Left(20);
					m_Labels.Add(aLabel); 
				}
			} else 
				break; 
		}
		return m_Labels.GetCount() > 0; 
	}
	return FALSE; 
}

BOOL FLabelManager::Save(const tchar* pszFileName /* = NULL */)
{
	SynchronizeThread(m_Sync); 
	m_Conf.Clear(); 
	for (size_t k = 0; k < m_Labels.GetCount(); k++)
	{
		FString SectionName; 
		SectionName.Format("Label %d", k); 
		m_Conf.ModifyValue(SectionName, "Caption", m_Labels[k], TRUE); 
	}
	return m_Conf.Save(pszFileName); 
}

FString FLabelManager::GetLabels()
{
	SynchronizeThread(m_Sync); 
	FString StrLabels; 
	for (size_t k = 0; k < m_Labels.GetCount(); k++)
	{
		if (k > 0)
			StrLabels+=",";
		StrLabels+=m_Labels[k];
	}
	return StrLabels; 
}

BOOL FLabelManager::GetLabels(FArray<FString>& aLabels)
{
	SynchronizeThread(m_Sync); 
	aLabels = m_Labels; 
	return TRUE; 
}













