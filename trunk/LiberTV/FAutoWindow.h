#ifndef __FAUTOWINDOW_H__
#define __FAUTOWINDOW_H__
#pragma once

#include <atlbase.h>
#include <atlapp.h>
extern CMyServerAppModule _Module;
#include <atlwin.h>
#include <atlframe.h>

//Thin wrapper over a window class
//Create(): If the window is created and is alive, the window is set as a foreground window
//If it has been closed, the object is deleted and a fresh object is created with the window
//Used for windows with popup windows as class members (eg. FTrayWnd).
template <class _TWindow>
class FAutoWindow
{
public:
	_TWindow* pWindow;
	FAutoWindow()
	{
		pWindow = NULL;
	}

	~FAutoWindow()
	{
		DestroyWindow();
		delete pWindow; 
		pWindow = NULL;
	}

	operator HWND()
	{
		return pWindow?pWindow->m_hWnd:NULL;
	}


	HWND Create(HWND hWnd, LPRECT prc, const tchar* Name, DWORD dwStyles, DWORD dwExStyles = 0, HMENU hMenu = NULL, LPVOID lpv = NULL)
	{
		if (NULL != pWindow)
		{
			if (pWindow->IsWindow())
			{
				SetForegroundWindow(*pWindow); 
				return *pWindow; 
			}
			else
			{
				delete pWindow;
				pWindow = NULL;
			}
		}

		pWindow = new _TWindow; 
		return pWindow->Create(hWnd, prc, Name, dwStyles, dwExStyles, hMenu, lpv); 
	}

	_TWindow* operator->()
	{
		if (NULL == pWindow)
		{
			assert(0); 
			pWindow = new _TWindow; 
		}
		return pWindow; 
	}


	void DestroyWindow()
	{
		if (pWindow != NULL)
		{
			if (pWindow->IsWindow())
				pWindow->DestroyWindow(); 
			delete pWindow; 
			pWindow = NULL; 
		}
	}

	bool IsObjectAndWindow()
	{
		return pWindow && 
			   pWindow->IsWindow();
	}

	void ShowWindow(int nCmdShow)
	{
		if (IsObjectAndWindow())
			pWindow->ShowWindow(nCmdShow); 
	}
	_TWindow*	GetPtr(){
		return pWindow;
	}
};


template <class T>
class FWAutoPtr : public CAutoPtr<T>
{
public:

	~FWAutoPtr()
	{
		Destroy(); 
	}

	BOOL IsObjectAndWindow()
	{
		return m_p && m_p->IsWindow();
	}

	BOOL ShowWindow(int nShow)
	{
		if (m_p)
			return m_p->ShowWindow(nShow);
		return FALSE; 
	}

	HWND Create(HWND hWnd, LPRECT prc, const tchar* Name, DWORD dwStyles, DWORD dwExStyles = 0, HMENU hMenu = NULL, LPVOID lpv = NULL)
	{
		if (NULL != m_p)
		{
			if (m_p->IsWindow())
			{
				SetForegroundWindow(*pWindow); 
				return *m_p; 
			}
			else
			{
				m_p->Create(hWnd, prc, Name, dwStyles, dwExStyles, hMenu, lpv); 
			}
		}

		//Object must be Attached before calling Create()
		ATLASSERT(FALSE)
			return NULL; 
	}

	void Destroy()
	{
		if (IsObjectAndWindow())
			m_p->DestroyWindow(); 
		Free(); 
	}
};
#endif //__FAUTOWINDOW_H__