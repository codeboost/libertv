#include "stdafx.h"
#include "mz_Synchro.h"

MZ_IMPLEMENT_RUNTIME (mz_Sync);


mz_Sync::mz_Sync (int nSpinCount)
{
	nSpinCount;
	//mz_InitializeCriticalSection(&m_CriticalSection); 
	//InitializeCriticalSectionAndSpinCount(&m_CriticalSection, nSpinCount);
	InitializeCriticalSection(&m_CriticalSection); 
#ifdef _DEBUG
	m_kSyncLocks = 0; 
#endif
}

mz_Sync::~mz_Sync ()
{
	//assert(m_kSyncLocks==0); 
	DeleteCriticalSection(&m_CriticalSection);
}

void mz_Sync::Lock ()
{
	EnterCriticalSection(&m_CriticalSection); 
#ifdef _DEBUG
	m_kSyncLocks++;
#endif
}

void mz_Sync::Unlock ()
{
#ifdef _DEBUG
	assert(m_kSyncLocks > 0); 
	m_kSyncLocks--;
#endif
	LeaveCriticalSection(&m_CriticalSection);
}

MZ_IMPLEMENT_RUNTIME (mz_WaitHandle);

int mz_WaitHandle::Wait (const int pTimeOut, bool bAlterable) const
{
	return WaitForSingleObjectEx(m_hHandle, pTimeOut, bAlterable ? TRUE : FALSE);
}

void mz_WaitHandle::CloseHandle ()
{
	::CloseHandle(m_hHandle); 
	m_hHandle = NULL;
}

mz_WaitHandle::~mz_WaitHandle ()
{
	if (m_hHandle)
		CloseHandle();
}

//-------------------------------------------------------------------------------------------------

MZ_IMPLEMENT_RUNTIME (mz_Event); 


mz_Event::mz_Event ()
{
	m_hHandle = NULL;
}

mz_Event::mz_Event (bool bManual, bool bState, const MZCHAR* pName)
{
	m_hHandle = NULL;
	Create(bManual, bState, pName);
}

bool mz_Event::Create (bool bManual, bool bState, const MZCHAR* pName)
{
	m_hHandle = CreateEvent(NULL, bManual, bState, pName); 
	return (m_hHandle != NULL);
}

void mz_Event::SetEvent ()
{
	::SetEvent(m_hHandle);
}

void mz_Event::ResetEvent()
{
	::ResetEvent(m_hHandle); 
}

mz_Event::~mz_Event ()
{
	CloseHandle();
}

MZ_IMPLEMENT_RUNTIME (mz_Semaphore); 


mz_Semaphore::mz_Semaphore ()
{
}

mz_Semaphore::mz_Semaphore (long pInitialCount, long pMaxCount, const MZCHAR* pName)
{
	Create(pInitialCount, pMaxCount, pName);
}

bool mz_Semaphore::Create (long pInitialCount, long pMaxCount /* = SYNC_MAX_SEM_COUNT */,
						   const MZCHAR* pName /* = NULL */)
{
	m_hHandle = CreateSemaphore(NULL, pInitialCount, pMaxCount, pName); 
	return (m_hHandle != NULL);
}

long mz_Semaphore::Release (long pCount)
{
	long lPreviousCount	= 0; 
	if (ReleaseSemaphore(m_hHandle, pCount, &lPreviousCount))
	{
		return lPreviousCount;
	} 

	return SYNC_ERROR;
}

mz_Semaphore::~mz_Semaphore ()
{
	CloseHandle();
}

MZ_IMPLEMENT_RUNTIME (mz_WaitHandleArray); 


mz_WaitHandleArray::mz_WaitHandleArray ()
{
	m_nCount = 0;
}

int mz_WaitHandleArray::WaitMultiple (bool pWaitForAll, dword pTimeOut, bool bAlterable)
{
	return WaitForMultipleObjectsEx(m_nCount, m_Array, pWaitForAll ? TRUE : FALSE, pTimeOut, bAlterable ? TRUE : FALSE);
}

void mz_WaitHandleArray::Clear ()
{
	m_nCount = 0;
}

mz_AutoSync::mz_AutoSync (mz_Sync& pSync): m_kSync(pSync){
	m_kSync.Lock();
}

mz_AutoSync::~mz_AutoSync ()
{
	m_kSync.Unlock();
}