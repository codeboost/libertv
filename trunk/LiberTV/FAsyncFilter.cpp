#include "stdafx.h"
#include "FAsyncFilter.h"

HRESULT CMemFileStream::AddFile(LPCSTR szFileName, LONGLONG lFileSize, DWORD dwFlags)
{
	CAutoLock lck(&m_csLock);
	//No synchronization - this should only be called once during initialization, BEFORE the filter is started
	CFileEntry EFile; 
	EFile.m_FileName = szFileName; 
	EFile.m_FileSize = lFileSize; 
	EFile.m_dwFlags = dwFlags; 
	if (EFile.m_FileSize > 0)
	{
		m_Files.Add(EFile);
		m_llLength+=EFile.m_FileSize; 
		return S_OK; 
	}
	return S_FALSE; 
}

void CMemFileStream::Clear()
{
	CAutoLock lck(&m_csLock);
	m_Files.RemoveAll();
	m_llPosition = 0; 
	m_llLength = 0; 
}

HRESULT CMemFileStream::SetFileFlag(LPCSTR szFileName, DWORD dwFlags)
{
	CAutoLock lck(&m_csLock);	
	for (size_t k = 0; k < m_Files.GetCount(); k++)
	{
		if (m_Files[k].m_FileName == szFileName)
		{
			m_Files[k].m_dwFlags = dwFlags; 
			return S_OK; 
		}
	}
	return S_FALSE; 
}

HRESULT CMemFileStream::SetPointer(LONGLONG llPos)
{
	if (llPos < 0 || llPos > m_llLength) {
		return S_FALSE;
	} else {
		m_llPosition = llPos;
		return S_OK;
	}
}

BOOL CMemFileStream::CopyMemFromFile(LPCSTR szFileName, PVOID ppOut, LONG Offset, DWORD dwBytesToRead)
{
	//CopyMemory((PVOID)(pbBuffer + lBufOffs), (PVOID)(m_Memory[k].pData + lOffset), nBytesToRead);
	HANDLE hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);

	BOOL bResult = FALSE; 
	if(hFile != INVALID_HANDLE_VALUE)
	{
		LONG llNewOffs = 0; 
		DWORD dwBytesRead = 0; 
		SetFilePointer(hFile, Offset, &llNewOffs, FILE_BEGIN);
		bResult = ReadFile(hFile, ppOut, dwBytesToRead, &dwBytesRead, NULL); 
		CloseHandle(hFile); 
	}
	return bResult; 
}

HRESULT CMemFileStream::Read(PBYTE pbBuffer, DWORD dwBytesToRead, BOOL bAlign, LPDWORD pdwBytesRead)
{
	CAutoLock lck(&m_csLock);
	DWORD dwReadLength;

	_DBGAlert("BTR=%d\n", dwBytesToRead); 

	if (m_llPosition + dwBytesToRead > m_llLength) {
		dwReadLength = (DWORD)(m_llLength - m_llPosition);
	} else {
		dwReadLength = dwBytesToRead;
	}

	LONGLONG lTotal = 0; 
	LONGLONG lOffset = 0; 

	//Determine which buffer we should address
	size_t k = 0; 
	for (k = 0; k < m_Files.GetCount(); k++)
	{
		if (lTotal + m_Files[k].m_FileSize > m_llPosition)
		{
			lOffset = m_llPosition - lTotal; 
			break;
		}
		lTotal+=m_Files[k].m_FileSize; 
	}

	DWORD lBufOffs = 0; 

	HRESULT hr = S_OK; 

	for (; k < m_Files.GetCount(); k++)
	{
		DWORD nBytesToRead = min(dwBytesToRead - lBufOffs, (DWORD)(m_Files[k].m_FileSize - lOffset));

		if (nBytesToRead == 0)
			break; 

		//See if file is available, if it's not, DO SOMETHING!
		if (m_Files[k].m_dwFlags & FASYNC_AVAILABLE)
		{
			CopyMemFromFile(m_Files[k].m_FileName, (PVOID)(pbBuffer + lBufOffs), (DWORD)lOffset, (DWORD)nBytesToRead); 
		}																											   
		else
		{
			_DBGAlert("%d requested\n", k); 

			if (NULL != m_pRenderer)
				m_pRenderer->OnFileUnavailable(); 

			hr = S_FALSE; 
			break; 
			//DO SOMETHING: WaitForSingleObject(m_hCompleted); 
		}
		

		lBufOffs+=nBytesToRead; 
		lOffset = 0; 
	}

	m_llPosition += lBufOffs;
	*pdwBytesRead = lBufOffs;
	return hr;
}