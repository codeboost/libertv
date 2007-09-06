#ifndef FUPNPdotH
#define FUPNPdotH
#include "FFirewall.h"
#include "Utils.h"

class IUPnPNotify
{
public:
	virtual void OnPortsMapped(BOOL bSuccess ) = 0; 
};

class FUPnPThread : public mz_Thread
{
	void Thread(void* p)
	{

		IUPnPNotify* pNotify = (IUPnPNotify*)p; 
		int nRes = m_Event.Wait(INFINITE); 
		if (nRes != WAIT_OBJECT_0)
			return; 

		CoInitialize(NULL); 
		for (;;)
		{
			FFirewall aFirewall;
			std::vector<long> ports; 
			ports.push_back((long)g_AppSettings.m_ListenPort);

			if (!aFirewall.AddPortMapping(ports, "TCP", "LiberTV Incoming"))
			{
				CoUninitialize();
				_DBGAlert("FUPnP Thread: Unable to map ports\n");
				if (pNotify)
					pNotify->OnPortsMapped(FALSE); 
				return ; 
			}
			else
			{
				if (pNotify)
					pNotify->OnPortsMapped(TRUE); 
			}

			int nRes = m_Event.Wait(INFINITE, true); 
			aFirewall.RemovePortMapping();
			if (nRes != WAIT_OBJECT_0)
				break; 

		}
		_DBGAlert("FUPnP Thread: Terminating\n");
		CoUninitialize();
	}
public:
	mz_Event m_Event; 

	void UpdatePortMapping()
	{
		m_Event.SetEvent();
	}

	FUPnPThread()
	{
		m_Event.Create(false);
	}
	~FUPnPThread()
	{
		StopThread();
		Wait(INFINITE);
	}
};
#endif