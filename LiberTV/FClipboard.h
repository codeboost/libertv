#ifndef __FCLIPBOARD_H__
#define __FCLIPBOARD_H__

class CClipboard
{
public:
	BOOL m_bOpened;

	CClipboard() : m_bOpened(FALSE)
	{
	}
	CClipboard(HWND hWnd) : m_bOpened(FALSE)
	{
		Open(hWnd);
	}
	~CClipboard()
	{
		Close();
	}
	BOOL Open(HWND hWnd)
	{
		ATLASSERT(!m_bOpened);
		ATLASSERT(hWnd==NULL || ::IsWindow(hWnd));  // See Q92530
		BOOL bRes = ::OpenClipboard(hWnd);
		m_bOpened = bRes;
		return bRes;
	}
	void Close()
	{
		if( !m_bOpened ) return;
		::CloseClipboard();
		m_bOpened = FALSE;
	}
	BOOL Empty()
	{
		ATLASSERT(m_bOpened);
		return ::EmptyClipboard();
	}
	int GetFormatCount() const
	{
		return ::CountClipboardFormats();
	}
	UINT EnumFormats(UINT uFormat)
	{
		ATLASSERT(m_bOpened);
		return ::EnumClipboardFormats(uFormat);
	}
	void EnumFormats(CSimpleValArray<UINT>& aFormats)
	{
		ATLASSERT(m_bOpened);
		UINT uFormat = 0;
		while( true ) {
			uFormat = ::EnumClipboardFormats(uFormat);
			if( uFormat == 0 ) break;
			aFormats.Add(uFormat);
		}
	}
	BOOL IsFormatAvailable(UINT uFormat) const
	{
		return ::IsClipboardFormatAvailable(uFormat);
	}
	int GetFormatName(UINT uFormat, LPTSTR pstrName, int cchMax) const
	{
		// NOTE: Doesn't return names of predefined cf!
		return ::GetClipboardFormatName(uFormat, pstrName, cchMax);
	}
	HANDLE GetData(UINT uFormat) const
	{
		ATLASSERT(m_bOpened);
		return ::GetClipboardData(uFormat);
	}
#if defined(_WTL_USE_CSTRING) || defined(__ATLSTR_H__)
	BOOL GetTextData(_CSTRING_NS::CString& sText) const
	{
		ATLASSERT(m_bOpened);
		// Look for UNICODE version first because there's a better
		// chance to convert to the correct locale.
		HGLOBAL hMem = ::GetClipboardData(CF_UNICODETEXT);
		if( hMem != NULL ) {
			sText = (LPCWSTR) GlobalLock(hMem);
			return GlobalUnlock(hMem);
		}
		hMem = ::GetClipboardData(CF_TEXT);
		if( hMem != NULL ) {
			sText = (LPCSTR) GlobalLock(hMem);
			return GlobalUnlock(hMem);
		}
		return FALSE;
	}
	BOOL GetFormatName(UINT uFormat, _CSTRING_NS::CString& sName) const
	{
		for( int nSize = 256; ; nSize *= 2 ) {
			int nLen = ::GetClipboardFormatName(uFormat, sName.GetBufferSetLength(nSize), nSize);
			sName.ReleaseBuffer();
			if( nLen < nSize - 1 ) return TRUE;
		}
		return TRUE;  // Hmm, unreachable!
	}
#endif
	HANDLE SetData(UINT uFormat, HANDLE hMem)
	{
		ATLASSERT(m_bOpened);
		ATLASSERT(hMem!=NULL);   // No support for WM_RENDERFORMAT here! 
		// Enjoy the ASSERT for NULL!
		return ::SetClipboardData(uFormat, hMem);
	}
	HANDLE SetData(UINT uFormat, LPCVOID pData, int iSize)
	{
		HGLOBAL hGlobal = ::GlobalAlloc(GMEM_MOVEABLE, (SIZE_T) iSize);
		if( hGlobal == NULL ) return NULL;
		memcpy(GlobalLock(hGlobal), pData, (size_t) iSize);
		GlobalUnlock(hGlobal);
		HANDLE hHandle = ::SetClipboardData(uFormat, hGlobal);
		if( hHandle == NULL ) GlobalFree(hGlobal);
		return hHandle;
	}
	BOOL SetTextData(LPCSTR pstrText)
	{
		return SetData(CF_TEXT, pstrText, lstrlenA(pstrText) + 1) != NULL;
	}
	BOOL SetUnicodeTextData(LPCWSTR pstrText)
	{
		return SetData(CF_UNICODETEXT, pstrText, (lstrlenW(pstrText) + 1) * sizeof(WCHAR)) != NULL;
	}
	static HWND GetOwner()
	{
		return ::GetClipboardOwner();
	}
#if (WINVER >= 0x0500)
	static DWORD GetSequenceNumber()
	{
		return ::GetClipboardSequenceNumber();
	}
#endif
	static UINT RegisterFormat(LPCTSTR pstrFormat)
	{
		return ::RegisterClipboardFormat(pstrFormat);
	}
};

#endif //__FCLIPBOARD_H__