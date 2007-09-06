#ifndef __MZ_THREAD_H__
#define __MZ_THREAD_H__

#include "mz_Synchro.h"
#include <vector>

class MZHTTP_API mz_Thread : public mz_WaitHandle
{
							MZ_INCLUDE_RTTI (mz_Thread)
	static dword far pascal	_ThreadFunction (void* pParameter); 
	void*					m_UserParameter; 
	dword					m_dwThreadID; 
	mz_Event				m_kThreadStarted; 
public:

	mz_Thread (): m_dwThreadID(0), m_UserParameter(NULL)
	{
	};
	virtual ~mz_Thread ();; 
	bool	Create (void* pParameter, bool Suspended = false); 
	void	Suspend (); 
	void	Resume (bool bWaitUntilStarted = false); 

	/*
		If bWaitUntilCreated is true, Create() does not return until the thread has started and
		is ready to pass execution to the thread proc.
		This is implemented using an event which is created in Create()
		and set by the thread function. while Create() waits on it.
		This is needed to implement proper thread pool creation and destruction.
		FI: During engine startup, Bind() may fail, which triggers engine destruction.
		It may so happen that the Stop() IPC function is called *before* all the threads
		in the thread pool have been created. In this case the threads won't stop.
	*/

	void	Terminate (); 
	inline dword GetThreadID ()
	{
		return m_dwThreadID;
	}
	/*

		This function Queues a user IPC and returns
		You should use WaitForSingleObjectEx/WaitForMultipleObjectsEx with bAlterable=TRUE and process 
		WAIT_IO_COMPLETION if you want to stop the thread using this function (See Programming Server-Side applications by Richter and Clark, under
		QueueUserAPC description for an explanation on this method).
		Example:
		for(;;)
		{
			int nRes = WaitForSingleObjectEx(m_hSomeEvent, INFINITE, TRUE);
			if (nRes == WAIT_IO_COMPLETION)
				break; // Stop the thread
		}
		*/
	void			StopThread (); 


protected:
	//Override to implement thread functionality
	virtual void	Thread (void* pParameter) = 0;
};


class MZHTTP_API mz_ThreadPool : public mz_Object
{
	MZ_INCLUDE_RTTI (mz_ThreadPool)
	struct MZHTTTP_API_EX mz_PoolThread : public mz_Thread
	{
		void	Thread (void* pParam);
	};
	struct MZHTTTP_API_EX mz_PoolThreadItem
	{
		mz_ThreadPool*						m_kThis; 
		void*								m_kUser;
		mz_PoolThread						m_kThread;
	};
	mz_Sync m_ArraySync; 
	std::vector<mz_PoolThreadItem*>	m_kaThreads;
public:
	mz_ThreadPool ()
	{
	}
	virtual ~mz_ThreadPool ()
	{
	}

	bool			Create (void* pThreadParam, bool bSuspended, int nThreads);		//Creates the threads
	bool			AddWorker (int nWorkers, void* pThreadParam);		//Adds a new worker thread to the thread pool
	//Uses QueueUserAPC to stop the threads. Make sure you process WAIT_IO_COMPLETION as a stop event
	//in the thread function.
	int				StopAndWait (int pTimeout);									//Waits for the threads to shut down
	virtual void	Thread (void* pParam) = 0;	//worker thread
	size_t			GetThreadCount();
};




#endif //__MZ_THREAD_H__
