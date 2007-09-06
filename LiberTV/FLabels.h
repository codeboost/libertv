#pragma once
#include "Utils.h"
#include <list>

/*
FLabels
Manages a list of labels 
*/
struct FLabelItem
{
	FString m_StrLabel; 
	FLabelItem(){}
	FLabelItem(const tchar* pszLabel)
	{
		m_StrLabel = pszLabel; 
	}
	bool operator == (const FLabelItem& item)
	{
		return  this == &item ? true: m_StrLabel == item.m_StrLabel;
	}
};

class FLabels
{
	FArray<FLabelItem>	  m_Labels;
	FString				  m_LabelStr; 
	void BuildLabelStr(); 
public:
	FLabels();
	~FLabels(); 
	inline FString& GetLabelStr()
	{
		return m_LabelStr; 
	}

	inline size_t GetItemCount()
	{
		return m_Labels.GetCount();
	}
	void ParseLabelStr(const tchar* pszLabelString, BOOL AddToList = FALSE); 
	BOOL AddLabel(const tchar* pszLabel);
	BOOL HasLabel(const tchar* pszLabel); 
	BOOL HasSubstring(const tchar* pszLabel); 
	void RemoveLabel(const tchar* pszLabel); 
	const FString &GetLabelAt(size_t index);
	void Clear(); 
	bool operator += (const tchar* pszLabel); 
	bool operator -= (const tchar* pszLabel); 
	bool operator+=(const FLabels& src); 

};


