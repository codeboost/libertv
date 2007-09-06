#include "stdafx.h"
#include "FLabels.h"
#include "strutils.h"

FLabels::FLabels()
{

}

FLabels::~FLabels()
{

}

void FLabels::ParseLabelStr(const tchar* pszLabelString, BOOL bAddToList)
{
	if (!bAddToList)
	{
		m_Labels.RemoveAll(); 
		m_LabelStr = "";
	}

	FArray<FString> aArray; 
	SplitStringToArray(pszLabelString, aArray, ",");

	for (size_t k = 0; k < aArray.GetCount(); k++)
	{
		AddLabel(aArray[k]); 
	}

	BuildLabelStr(); 
}

void FLabels::BuildLabelStr()
{
	m_LabelStr = "";
	int count = 0; 
	for (size_t k = 0; k < m_Labels.GetCount(); k++)
	{
		FLabelItem& item = m_Labels[k]; 
		if (count > 0)
			m_LabelStr.Append(",");
		m_LabelStr.Append(item.m_StrLabel); 
		count++; 
	}	
}

void FLabels::Clear()
{
	m_Labels.RemoveAll();
	m_LabelStr = "";
}

BOOL FLabels::AddLabel(const tchar* pszLabel)
{
	FString NewLabel = pszLabel; 
	NewLabel.Replace('\'', ' ');
	NewLabel.Replace('\"', ' ');
	NewLabel.Replace(',', ' '); 
	NewLabel.Replace(';', ' '); 
	NewLabel.Trim(); 
	if (NewLabel.GetLength() > 0 && !HasLabel(NewLabel))
	{
		m_Labels.Add(FLabelItem(NewLabel));
		BuildLabelStr(); 
		return TRUE; 
	}
	return FALSE; 
}

void FLabels::RemoveLabel(const tchar* pszLabel)
{
	for (size_t k = 0; k < m_Labels.GetCount(); k++)
	{
		if (m_Labels[k].m_StrLabel == pszLabel)
		{
			m_Labels.RemoveAt(k, 1); 
			break; 
		}
	}

	BuildLabelStr(); 
}


BOOL FLabels::HasLabel(const tchar* pszLabel)
{
	for (size_t k = 0; k < m_Labels.GetCount(); k++)
	{
		if (m_Labels[k].m_StrLabel == pszLabel)
			return TRUE; 
	}
	return FALSE; 
}

bool FLabels::operator += (const tchar* pszLabel)
{
	return AddLabel(pszLabel) ? true: false; 
}

bool FLabels::operator -= (const tchar* pszLabel)
{
	RemoveLabel(pszLabel); 
	return true; 
}

bool FLabels::operator +=(const FLabels& rSource)
{
	if (this != &rSource)
	{
		for (size_t k = 0; k < rSource.m_Labels.GetCount(); k++)
		{
			AddLabel(rSource.m_Labels[k].m_StrLabel); 
		}

		BuildLabelStr(); 
	}
	return true; 
}

const FString& FLabels::GetLabelAt(size_t index)
{
	return m_Labels[index].m_StrLabel;
}

