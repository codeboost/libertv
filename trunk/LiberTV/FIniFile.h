#pragma once

#include <atlcoll.h>
#include "Utils.h"


#define INI_LINE_BLANK      0x01
#define INI_LINE_COMMENT    0x02
#define INI_LINE_DATA       0x04

#define INI_LINE_TYPE       0x07

#define FLAG_GETVAL			0x00
#define FLAG_PUTVAL			0x01
#define FLAG_ADDIFNEW		0x02

class FIniConfig
{
public:
    struct IniLine
    {
        FString m_ValName;   
        FString m_Value; 
        dword   m_dwFlags; 
        IniLine()
        {
            m_dwFlags = 0; 
        }
        void SetLine(const tchar* pszLine, size_t nLen); 
        void Escape(); 
//		IniLine& operator = (const IniLine& aNew); 
    };    

    struct IniSection
    {
        FString            m_Name;
        CAtlArray<IniLine> m_Lines; 
        dword              m_dwFlags; 
        IniSection()
        {
            m_dwFlags = 0; 
        }
		IniSection(const IniSection& aNew); 
		IniSection& operator = (const IniSection& aNew); 
        void Clear()
        {
            m_Lines.RemoveAll();
            m_dwFlags = 0; 
        }
        IniLine* FindLine(const tchar* pszValue);
        void AddLine(const tchar* pszLine); 
        void SetSection(const tchar* pszSection); 
        void ModifyValue(const tchar* pszValueName, const tchar* pszValue, BOOL bAdd = TRUE);   
        FString GetValue(const tchar* pszValue); 
        DWORD   GetValueDWORD(const tchar* pszValue, int radix = 10); 
        BOOL Load(const tchar* pszFileName);
        BOOL Save(const tchar* pszFileName); 
    };

protected:
    
	CAtlArray<IniSection>  m_Sections; 
	size_t                 m_CurSection;
	BOOL				   m_IsEncrypted; 

	void ParseLine(const tchar* pszLine, size_t len); 
	IniSection* FindSection(const tchar* pszName); 
	BOOL ParseBuffer(tchar* pszBuffer, DWORD size); 
	int SaveToBuffer(char*& aBuffer, BOOL bIgnoreCrypt); 
public:
    FString   m_FileName; 
	virtual ~FIniConfig(){
		Clear(); 
	}
    void Clear(); 

	CAtlArray<IniSection>& GetSections(){return m_Sections;}
    void AddSection(const tchar* pszSection); 
	void AddSection(IniSection& aSection); 
    BOOL ModifyValue(const tchar* pszSection, const tchar* pszValueName, const tchar* pszValue, BOOL bAdd = TRUE); 
    BOOL ModifyValueDWORD(const tchar* pszSection, const tchar* pszValueName, DWORD dwValue, int radix = 10, BOOL bAdd = TRUE);
	BOOL ModifyValueUINT64(const tchar* pszSection, const tchar* pszValueName, UINT64 uiValue, int radix = 10, BOOL bAdd = TRUE); 
    FString GetValue(const tchar* pszSection, const tchar* pszValue); 
    DWORD   GetValueDWORD(const tchar* pszSection, const tchar* pszValue, int radix = 10); 
	BOOL	ValueExists(const tchar* pszsection, const tchar* pszValueName); 
	void	ExchangeValueStr(const tchar* pszSection, const tchar* pszValue, FString& pStr, DWORD dwFlags);
	void	ExchangeValueDWORD(const tchar* pszSection, const tchar* pszValue, DWORD& dwVal, int radix, DWORD dwFlags);
	void	ExchangeValueUINT64(const tchar* pszSection, const tchar* pszValue, unsigned __int64 &ui64Val, int radix, DWORD dwFlags);
	void	RemoveSection(const tchar* pszSectionName); 

	unsigned __int64 GetValueUINT64(const tchar* pszSection, const tchar* pszValue, int radix = 10); 
	BOOL	SectionExists(const tchar* pszSection); 
    BOOL Load(const tchar* pszFileName);
	BOOL LoadFromBuffer(char* pBuffer, DWORD Len); 
    BOOL Save(const tchar* pszFileName = NULL, BOOL bIgnoreEncrypt = FALSE); 
	BOOL IsLoaded()
	{
		return (m_FileName.GetLength() > 0 && m_Sections.GetCount() > 1);
	}

	
	//Copy section from source
	void CopySection(FIniConfig& aConfSource, const tchar* szSectionSource); 
	FIniConfig (const FIniConfig& aConfig); 
	FIniConfig();
	FIniConfig& operator = (const FIniConfig& aConf);

};

typedef FIniConfig FIniFile;
