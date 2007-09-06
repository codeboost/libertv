#ifndef FTemplates_dot_H
#define FTemplates_dot_H

#include <assert.h>
#include "mz_synchro.h"

template <class T>
class FFastStack
{
public:
	T*	m_kArray;
	int	m_kIndex; 
	int	m_kSize; 
	mz_Semaphore m_Sema;
public:
	FFastStack (int nMaxSize = 0)
	{
		m_kIndex = 0;
		m_kArray = NULL; 
		m_kSize = 0; 
		SetSize(nMaxSize); 
	}
	~FFastStack ()
	{
		delete[] m_kArray;
	}
	inline void SetSize(int nSize)
	{
		assert(m_kArray == NULL); 
		if (nSize > 0)
		{
			m_kArray = new T[nSize];
			m_kSize = nSize; 
		}
	}

	inline void Clear ()
	{
		m_kIndex = 0;
	}

	inline bool Push (T& V)
	{
		if (m_kIndex < m_kSize)
		{
			m_kArray[m_kIndex++] = V;
			return true;
		}
		return false;
	}
	inline bool Pop (T& V)
	{
		if (m_kIndex == 0)
		{
			return false;
		} 

		V = m_kArray[--m_kIndex];
		return true;
	}
	inline int GetSize ()
	{
		return m_kIndex;
	}
};

template <class DataType>
class FFastSyncSemaStack
{
	FFastStack<DataType> m_Stack;
	mz_Sync				 m_Sync; 
	mz_Semaphore		 m_Sema;
public:

	FFastSyncSemaStack()
	{
		m_Sema.Create();
	}

	inline int GetSize()
	{
		SynchronizeThread(m_Sync); 
		return m_Stack.GetSize(); 
	}

	inline void SetSize(int nSize)
	{
		SynchronizeThread(m_Sync); 
		return m_Stack.SetSize(nSize);
	}

	inline void Clear()
	{
		SynchronizeThread(m_Sync); 
		m_Stack.Clear(); 
	}
	
	inline int PopWait(DataType& V, bool &bPopped, int nTimeout = INFINITE)
	{
		int nRes = m_Sema.Wait(nTimeout); 
		bPopped = false; 
		if (nRes == WAIT_OBJECT_0)
		{
			SynchronizeThread(m_Sync);
			bPopped = m_Stack.Pop(V); 
		}
		return nRes; 
	}

	inline bool Push(DataType& V)
	{
		m_Sync.Lock(); 
		bool bRes = m_Stack.Push(V); 
		m_Sync.Unlock(); 
		m_Sema.Release(); 
		return bRes; 
	}

	//Lock/Unlock - ensure that we pop/push multiple items on the same thread.
	inline void Lock()
	{
		m_Sync.Lock(); 
	}

	inline void Unlock() 
	{
		m_Sync.Unlock(); 
	}

};

























#endif