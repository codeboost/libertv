#include "stdafx.h"
#include "dbghelp.h"

#define MakePtr(cast, base, offset) (cast)((DWORD_PTR)(base) + (DWORD_PTR)(offset))

/*
void _DebugAlert(const char *Mask, ...)
{
    va_list paramList;
    va_start(paramList, Mask);
    char Temp[4096];
    vsprintf(Temp, Mask, paramList); 
    OutputDebugString(Temp); 
    va_end(paramList);
}
*/

HRESULT WriteProtectedMemory(LPVOID pDest, LPCVOID pSrc, DWORD dwSize)
{
    // Make it writable
    DWORD dwOldProtect = 0;
    if (VirtualProtect(pDest, dwSize, PAGE_READWRITE, &dwOldProtect))
    {
        MoveMemory(pDest, pSrc, dwSize);
        // Restore protection
        VirtualProtect(pDest, dwSize, dwOldProtect, &dwOldProtect);
        return S_OK;
    }
    return HRESULT_FROM_WIN32(GetLastError());
}


HRESULT mz_DetourFn(HMODULE hModule, LPSTR szEntry, LPVOID pHijacker, LPVOID *ppOrig)
{
    // Check args
    if ((!IS_INTRESOURCE(szEntry) && IsBadStringPtrA(szEntry, -1)) || IsBadCodePtr(FARPROC(pHijacker)))
        return E_INVALIDARG;
    
    PIMAGE_DOS_HEADER pDosHeader = PIMAGE_DOS_HEADER(hModule);
    
    if (IsBadReadPtr(pDosHeader, sizeof(IMAGE_DOS_HEADER)) || IMAGE_DOS_SIGNATURE != pDosHeader->e_magic)
        return E_INVALIDARG;
    
    PIMAGE_NT_HEADERS pNTHeaders = MakePtr(PIMAGE_NT_HEADERS, hModule, pDosHeader->e_lfanew);
    
    if (IsBadReadPtr(pNTHeaders, sizeof(IMAGE_NT_HEADERS)) || IMAGE_NT_SIGNATURE != pNTHeaders->Signature)
        return E_INVALIDARG;
    
    HRESULT hr = E_UNEXPECTED;
    
    IMAGE_DATA_DIRECTORY& expDir =  pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
    PIMAGE_EXPORT_DIRECTORY pExpDir = MakePtr(PIMAGE_EXPORT_DIRECTORY, hModule, expDir.VirtualAddress);
    
    LPDWORD pdwAddrs = MakePtr(LPDWORD, hModule, pExpDir->AddressOfFunctions);
    LPWORD pdwOrd = MakePtr(LPWORD, hModule, pExpDir->AddressOfNameOrdinals);
    DWORD dwAddrIndex = -1;
    
    if (IS_INTRESOURCE(szEntry))    //By ordinal
    {
        dwAddrIndex = WORD(szEntry) - pExpDir->Base;
        hr = S_OK;
    }
    else
    {
        // By name
        LPDWORD pdwNames = MakePtr(LPDWORD, hModule, pExpDir->AddressOfNames);
        for (DWORD iName = 0; iName < pExpDir->NumberOfNames; iName++)
        {
            LPSTR szName = MakePtr(LPSTR,hModule, pdwNames[iName]);
            if (0 == lstrcmpiA(szName, szEntry))
            {
                dwAddrIndex = pdwOrd[iName];
                hr = S_OK;
                break;
            }
        }
    }
    
    if (SUCCEEDED(hr))
    {
        if (pdwAddrs[dwAddrIndex] >= expDir.VirtualAddress && pdwAddrs[dwAddrIndex] < expDir.VirtualAddress + expDir.Size)
        {
            // We have a redirection
            LPSTR Redir = MakePtr(LPSTR, hModule, pdwAddrs[dwAddrIndex]);
            //ATLASSERT(!IsBadStringPtrA(azRedir, -1));
            
            LPSTR Dot = strchr(Redir, '.');
            int nLen = Dot - Redir;
            LPSTR Module = new CHAR[nLen];
            memcpy(Module, Redir, nLen);
            Module[nLen] = '\0';
            // Try to patch redirected function
            HMODULE ModHandle = GetModuleHandle(Module); 
            delete[] Module; 
            return mz_DetourFn(ModHandle, Dot + 1, pHijacker, ppOrig);
        }
        
        if (ppOrig)
            *ppOrig = MakePtr(LPVOID, hModule, pdwAddrs[dwAddrIndex]);
        
        DWORD dwOffset = DWORD_PTR(pHijacker) - DWORD_PTR(hModule);
        
        // write to write-protected memory
        hr = WriteProtectedMemory(pdwAddrs + dwAddrIndex, &dwOffset, sizeof(LPVOID));
    }
    
    return hr;
}

HRESULT ApiHijackImports(
                         HMODULE hModule,
                         LPSTR szVictim,
                         LPSTR szEntry,
                         LPVOID pHijacker,
                         LPVOID *ppOrig
                         )
{
    // Check args
    if (::IsBadStringPtrA(szVictim, -1) ||
        (!IS_INTRESOURCE(szEntry) && ::IsBadStringPtrA(szEntry, -1)) ||
        ::IsBadCodePtr(FARPROC(pHijacker)))
    {
        return E_INVALIDARG;
    }
    PIMAGE_DOS_HEADER pDosHeader = PIMAGE_DOS_HEADER(hModule);
    if (::IsBadReadPtr(pDosHeader, sizeof(IMAGE_DOS_HEADER)) ||
        IMAGE_DOS_SIGNATURE != pDosHeader->e_magic)
    {
        return E_INVALIDARG;
    }
    PIMAGE_NT_HEADERS pNTHeaders = 
        MakePtr(PIMAGE_NT_HEADERS, hModule, pDosHeader->e_lfanew);
    if (::IsBadReadPtr(pNTHeaders, sizeof(IMAGE_NT_HEADERS)) ||
        IMAGE_NT_SIGNATURE != pNTHeaders->Signature)
    {
        return E_INVALIDARG;
    }
    HRESULT hr = E_UNEXPECTED;
    // Locate the victim
    IMAGE_DATA_DIRECTORY& impDir = 
        pNTHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
    PIMAGE_IMPORT_DESCRIPTOR pImpDesc = 
        MakePtr(PIMAGE_IMPORT_DESCRIPTOR, hModule, impDir.VirtualAddress),
        pEnd = pImpDesc + impDir.Size / sizeof(IMAGE_IMPORT_DESCRIPTOR) - 1;
    while(pImpDesc < pEnd)
    {
        if (0 == ::lstrcmpiA(MakePtr(LPSTR, hModule, pImpDesc->Name), szVictim))
        {
            if (0 == pImpDesc->OriginalFirstThunk)
            {
                // no import names table
                return E_UNEXPECTED;
            }
            // Locate the entry
            PIMAGE_THUNK_DATA pNamesTable =
                MakePtr(PIMAGE_THUNK_DATA, hModule, pImpDesc->OriginalFirstThunk);
            if (IS_INTRESOURCE(szEntry))
            {
                // By ordinal
                while(pNamesTable->u1.AddressOfData)
                {
                    if (IMAGE_SNAP_BY_ORDINAL(pNamesTable->u1.Ordinal) &&
                        WORD(szEntry) == IMAGE_ORDINAL(pNamesTable->u1.Ordinal))
                    {
                        hr = S_OK;
                        break;
                    }
                    pNamesTable++;
                }
            }
            else
            {
				int nCount = 0; 
                // By name
                while(pNamesTable->u1.AddressOfData)
                {
                    if (!IMAGE_SNAP_BY_ORDINAL(pNamesTable->u1.Ordinal))
                    {
                        PIMAGE_IMPORT_BY_NAME pName = MakePtr(PIMAGE_IMPORT_BY_NAME,
                            hModule, pNamesTable->u1.AddressOfData);
					
					//	_DebugAlert("%d. %s\n", ++nCount, LPSTR(pName->Name)); 
                        if (0 == ::lstrcmpiA(LPSTR(pName->Name), szEntry))
                        {
                            hr = S_OK;
                            break;
                        }
                    }
                    pNamesTable++;
                }
            }
            if (SUCCEEDED(hr))
            {
                // Get address
                LPVOID *pProc = MakePtr(LPVOID *, pNamesTable,
                    pImpDesc->FirstThunk - pImpDesc->OriginalFirstThunk);
                // Save original handler
                if (ppOrig)
                    *ppOrig = *pProc;
                // write to write-protected memory
                return WriteProtectedMemory(pProc, &pHijacker, sizeof(LPVOID));
            }
            break;
        }
        pImpDesc++;
    }
    return hr;
}