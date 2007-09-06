#include "AutoResource.h"
#include "winutils.h"


void DisposeHInternet( HINTERNET handle );		//Uses InternetCloseHandle
void DisposeArray(void* pObj);					//uses delete[] on array
void DisposeHandle( HANDLE handle );			//uses CloseHandle

typedef CAutoResource< HANDLE, DisposeHandle >  CAutoHandle;
typedef CAutoResource< HINTERNET, DisposeHInternet >  CAutoHInternet;
typedef CAutoResource< LPVOID, DisposeArray > CAutoArray;
