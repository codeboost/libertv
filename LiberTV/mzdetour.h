#ifndef __MZDETOUR_H__
#define __MZDETOUR_H__
#include <windows.h>
/*
    Detours an API Function by modifying the module's IAT.
    ppOrigFn contains the original function (make sure the args are correct)

    Example: Detour MessageBeep(DWORD dwType) -> MyMessageBeep(DWORD);
    HRESULT res = mz_DetourFn(GetModuleHandle(NULL), "USER32.DLL", "MessageBeep", MyMessageBeep, (PVOID*)&RealBeep);
    Note the A and W suffixes of some API functions (Ansi or Wide character)
    
*/
HRESULT mz_DetourFn(HMODULE hModule, LPSTR szFnName, LPVOID pNewFn, LPVOID *ppOrigFn);
void ReplaceIATEntryInOneMod(PCSTR pszCalleeModName, 
   PROC pfnCurrent, PROC pfnNew, HMODULE hmodCaller);

HRESULT ApiHijackImports(
                         HMODULE hModule,
                         LPSTR szVictim,
                         LPSTR szEntry,
                         LPVOID pHijacker,
                         LPVOID *ppOrig
                         );
#endif //__MZDETOUR_H__