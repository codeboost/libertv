#pragma once

#include "FWebWindow.h"

class TestWindow : public CFrameWindowImpl<TestWindow>
{
public:
    DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME)
    BEGIN_MSG_MAP(TestWindow)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_SIZE, OnSize); 
        CHAIN_MSG_MAP(CFrameWindowImpl<TestWindow>)
    END_MSG_MAP()

    FIEWindow    m_IEWindow; 
    

    LRESULT OnCreate(UINT, WPARAM, LPARAM, BOOL&)
    {
        CreateSimpleStatusBar();
        RECT rcClient; 
        GetClientRect(&rcClient); 
        m_IEWindow.Create(m_hWnd, rcClient, "IEHolder", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, WS_EX_CLIENTEDGE );
        return 0;
    }



	LRESULT OnSize(UINT, WPARAM, LPARAM, BOOL&)
    {
        RECT rcWindow;
        GetClientRect( &rcWindow );
        if  ( m_IEWindow.m_hWnd )
            m_IEWindow.MoveWindow( &rcWindow, TRUE );

		UpdateLayout(TRUE); 
        return 0; 
    }
};