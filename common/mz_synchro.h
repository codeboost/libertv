#ifndef __MZ_SYNCHRO_H__
#define __MZ_SYNCHRO_H__

/*
mz_Syncro.h
Synchronization Objects:
mz_Sync:		Critical Section
mz_WaitHandle:	Generic waitable handle 
mz_Event:		Event
mz_Semaphore:	Semaphore
*/
#include <assert.h>
#include "mz_Object.h"


class mz_Sync; 
class mz_Event; 
class mz_Semaphore; 
class mz_WaitHandleArray; 

#define SYNC_MAX_WAIT_OBJECTS	8			//Can be up to 64, but usually no more than 8 events are used
#define SYNC_MAX_SEM_COUNT		2147483647L 

#define SYNC_ERROR				(long)-1

//Critical Section object
class  MZHTTP_API mz_Sync : public mz_Object
{
						MZ_INCLUDE_RTTI (mz_Sync)
private:
	CRITICAL_SECTION	m_CriticalSection; 
public:
						mz_Sync (int nSpinCount = 4000);
						~mz_Sync ();
	void				Lock () ; 
	void				Unlock (); 
#ifdef _DEBUG
	int					m_kSyncLocks;
#endif
};

//Waitable Handle
class  MZHTTP_API mz_WaitHandle : public mz_Object
{
					MZ_INCLUDE_RTTI (mz_WaitHandle) 

	friend class	mz_WaitHandleArray;
public:
	mz_WaitHandle (const MZHANDLE hHandle = NULL): m_hHandle(hHandle){
	}
	virtual	~mz_WaitHandle ();
	inline void SetHandle (const MZHANDLE hHandle)
	{
		m_hHandle = hHandle;
	}
	int	Wait (const int pTimeout, bool bAlterable = true) const; 
	inline MZHANDLE GetHandle () const
	{
		return m_hHandle;
	}
	inline operator MZHANDLE ()  const
	{
		return GetHandle();
	}
	inline mz_WaitHandle& operator = (const MZHANDLE hHandle)
	{
		SetHandle(hHandle);return *this;
	}
	void		CloseHandle ();

protected:
	MZHANDLE	m_hHandle;
};

class  MZHTTP_API mz_Event : public mz_WaitHandle
{
			MZ_INCLUDE_RTTI (mz_Event)
public:
			mz_Event (bool bManual, bool bState, const MZCHAR* pName); 
			mz_Event (); 
			~mz_Event (); 
	bool	Create (bool bManual = true, bool bState = false, const MZCHAR* pName = NULL); 
	void	SetEvent ();
	void	ResetEvent(); 
};

class  MZHTTP_API mz_Semaphore : public mz_WaitHandle
{
			MZ_INCLUDE_RTTI (mz_Semaphore) 
public:
			mz_Semaphore (long pInitialCount, long pMaxCount = SYNC_MAX_SEM_COUNT, const MZCHAR* pName = NULL); 
			mz_Semaphore (); 
			~mz_Semaphore (); 
	bool	Create (long pInitialCount = 0, long pMaxCount = SYNC_MAX_SEM_COUNT, const MZCHAR* pName = NULL);
	long	Release (long pCount = 1); //return previous count or SYNC_ERROR
};

class  MZHTTP_API mz_WaitHandleArray : public mz_Object
{
			MZ_INCLUDE_RTTI (mz_WaitHandleArray) 
	HANDLE	m_Array[SYNC_MAX_WAIT_OBJECTS]; 
	int		m_nCount; 
public:
			mz_WaitHandleArray (); 
	inline void AddHandle (const mz_WaitHandle* pObj)
	{
		assert(m_nCount < SYNC_MAX_WAIT_OBJECTS); 
		m_Array[m_nCount++] = pObj->GetHandle();
	}
	inline void AddHandle (const MZHANDLE pHandle)
	{
		assert(m_nCount < SYNC_MAX_WAIT_OBJECTS); 
		m_Array[m_nCount++] = pHandle;
	}
	int		WaitMultiple (bool pWaitForAll, dword pTimeOut, bool bAlterable = true); 
	void	Clear ();
};

class  MZHTTP_API mz_AutoSync : public mz_Object
{
	mz_Sync&	m_kSync; 
	mz_AutoSync&operator = (const mz_AutoSync& pSync); 
public:
				mz_AutoSync (mz_Sync& pSync);
				~mz_AutoSync ();
};

#define SynchronizeThread(pSync) mz_AutoSync _A_((pSync));

class MZHTTP_API mz_InterlockedLong
{
	LONG	m_kValue; 
public:
	mz_InterlockedLong ()
	{
		operator = (0);
	}
	inline LONG operator ++ ()
	{
		return InterlockedIncrement(&m_kValue);
	}
	inline LONG operator ++ (int)
	{
		return operator++();
	}
	inline LONG operator -- ()
	{
		return InterlockedDecrement(&m_kValue);
	}
	inline LONG operator -- (int)
	{
		return operator--();
	}
	inline LONG operator + (LONG Value)
	{
		return InterlockedExchangeAdd(&m_kValue, Value);
	}
	inline operator LONG ()
	{
		LONG	pValue; InterlockedExchange(&pValue, m_kValue); return pValue;
	}
	inline LONG operator = (LONG pValue)
	{
		return InterlockedExchange(&m_kValue, pValue);
	}
	inline LONG operator+=(dword dwValue)
	{
		return InterlockedExchangeAdd(&m_kValue, (LONG)dwValue);
	}
};

#endif //__MZ_SYNCHRO_H__
