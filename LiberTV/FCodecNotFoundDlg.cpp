#include "stdafx.h"
#include "FCodecNotFoundDlg.h"
#include "FMainFrame.h"

extern FAutoWindow<FMainFrame> g_MainFrame; 

void FCodecNotFoundDlg::OnCreated()
{
	SetMinSize(640, 480); 
	SetIcon(LoadIcon(_Module.get_m_hInst(), MAKEINTRESOURCE(IDI_MAIN)));

	FString Str;
	GetWindowText(Str);

	SetWindowText("LiberTV: Codec not found"); 

	if (FAILED(m_pW.Navigate(g_AppSettings.AppDir(Str))))
	{
		MessageBox("Cannot load error file.", "Error loading dialog", MB_OK | MB_ICONERROR); 
		DestroyWindow();
	}

	SetFocus(); 
	SetActiveWindow(); 
}

void FCodecNotFoundDlg::OnLoadComplete()
{

}

LRESULT FCodecNotFoundDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	FIEDialog::OnDestroy(uMsg, wParam, lParam, bHandled); 
	return 0; 
}