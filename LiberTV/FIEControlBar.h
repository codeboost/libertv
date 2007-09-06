#ifndef __FIECONTROLBAR_H__
#define __FIECONTROLBAR_H__

#include "mz_Inc.h"
#include "Utils.h"
#include "FWebWindow.h"
#include "IControlBar.h"


class FIEControlBar : public FIEWindow, public IControlBar
{


public:
	BEGIN_MSG_MAP(FIEControlBar);
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy); 
		CHAIN_MSG_MAP(FIEWindow)
	END_MSG_MAP();

	void	OnDocumentComplete(DWORD dwID, BOOL bMainFrame);
	LRESULT	OnDestroy(UINT, WPARAM, LPARAM, BOOL&); 
	//Volume
	int		GetVolume(); 
	void	SetVolume(int Volume); 
	void	IncreaseVolume(); 
	void	DecreaseVolume(); 
	BOOL	IsMuted(); 


	//Seekbar
	 REFERENCE_TIME	
		SeekBar_GetPos(); 
	 void	SeekBar_SetPos(__int64 nPos); 
	 void	SeekBar_Enable(bool bEnable); 
	 void	SeekBar_SetRange(__int64 Min, __int64 Max); 
	 void	SeekBar_SetAvail(__int64 nAvail); 


	//Statusbar
	 void	Status_SetTimer(const tchar* pStrTimer); 
	 void	Status_SetMessage(int idi, const tchar* pStrMsg); 

	 void	SetPlayState(BOOL bPlaying); 

	//Misc
	 void	SetPlayer(IVideoPlayer* pPlayer){}

	 void	Init(); 
     void   OnStop(); 
	 void	ShowBuffering(BOOL bShow);
	 void	ShowNextPrev(BOOL bShow);
};

#endif //__FIECONTROLBAR_H__