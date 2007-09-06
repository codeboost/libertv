#include "stdafx.h"
#include "mz_Thread.h"

MZ_IMPLEMENT_RUNTIME (mz_Thread); 
MZ_IMPLEMENT_RUNTIME (mz_ThreadPool);

void WINAPI APCFunc (ULONG_PTR)
{
	//Nothing here. Used to get the threads out of the wait state (if bAlterable==TRUE).
	//See StopThread() comment for more.
}

dword pascal mz_Thread::_ThreadFunction (void* pParameter)
{
	mz_Thread* O= (mz_Thread*) pParameter; 

		if (O->m_kThreadStarted.GetHandle() != NULL)
			O->m_kThreadStarted.SetEvent();

		O->Thread(O->m_UserParameter); 


	ExitThread(0); 
	return 0;
}


bool mz_Thread::Create (void* pParameter, bool Suspended)
{
	m_UserParameter = pParameter; 
	m_hHandle = CreateThread(NULL, 0, _ThreadFunction, this, Suspended?CREATE_SUSPENDED:0, &m_dwThreadID); 
	return (NULL != m_hHandle);
}

void mz_Thread::Suspend ()
{
	SuspendThread(m_hHandle);
}

void mz_Thread::Resume (bool bWaitUntilStarted)
{
	if (NULL == m_hHandle)
		return ; 

	if (bWaitUntilStarted)
		m_kThreadStarted.Create(); 

	int nResCount = ResumeThread(m_hHandle);

	if (bWaitUntilStarted && nResCount == 1)
	{
		m_kThreadStarted.Wait(INFINITE); 
	}
}

void mz_Thread::Terminate ()
{
	TerminateThread(m_hHandle, 0);
}

void mz_Thread::StopThread ()
{
	QueueUserAPC(APCFunc, m_hHandle, 0);
}

mz_Thread::~mz_Thread()
{
}
//////////////////////////////////////////////////////////////////////////

bool mz_ThreadPool::Create (void* pUserParam, bool bSuspended, int nThreads)
{
	return AddWorker(nThreads, pUserParam);
}

bool mz_ThreadPool::AddWorker (int nWorkers, void* pUserParam)
{
	
	for (int k = 0; k < nWorkers; k++)
	{
		mz_PoolThreadItem* Item	= new mz_PoolThreadItem;
		Item->m_kThis = this;
		Item->m_kUser = pUserParam;

		if (!Item->m_kThread.Create(Item, false))
		{
			delete Item;
			return false;
		}
		m_ArraySync.Lock();
		m_kaThreads.push_back(Item);
		m_ArraySync.Unlock(); 
	}

	SynchronizeThread(m_ArraySync); 
	for (int j = 0; j < nWorkers; j++)
	{
		m_kaThreads[j]->m_kThread.Resume(true);
	}
	return true;
}

void mz_ThreadPool::mz_PoolThread::Thread (void* pParam)
{
	mz_PoolThreadItem* pItem= (mz_PoolThreadItem*) pParam;
	pItem->m_kThis->Thread(pItem->m_kUser);
}

int mz_ThreadPool::StopAndWait (int pTimeout)
{
	int res	= 0; 
	SynchronizeThread(m_ArraySync); 
	for (size_t k = 0; k < m_kaThreads.size(); k++)
	{
		m_kaThreads[k]->m_kThread.StopThread(); 

		if (m_kaThreads[k]->m_kThread.Wait(pTimeout) == WAIT_TIMEOUT)
		{
			res = WAIT_TIMEOUT;
		}
		delete m_kaThreads[k];
	}
	m_kaThreads.clear();
	return res;
}

size_t mz_ThreadPool::GetThreadCount()
{
	SynchronizeThread(m_ArraySync); 
	return m_kaThreads.size(); 
}


