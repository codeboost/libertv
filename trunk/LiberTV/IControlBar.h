#ifndef __ICONTROLBAR_H__
#define __ICONTROLBAR_H__
#include "IVideoPlayer.h"
#include "IStatusBar.h"

#define id_Scroll_SeekBar	1
#define id_Scroll_Volume	2

class IFVolume
{
public:
	virtual int		GetVolume() = 0; 
	virtual void	SetVolume(int Volume) = 0; 
	virtual void	IncreaseVolume() = 0; 
	virtual	void	DecreaseVolume() = 0; 
	virtual BOOL	IsMuted() = 0; 
	virtual ~IFVolume(){}
};

class IFSeekBar
{
public:
	//Seekbar
	virtual REFERENCE_TIME	
		SeekBar_GetPos() = 0; 
	virtual void	SeekBar_SetPos(__int64 nPos) = 0; 
	virtual void	SeekBar_Enable(bool bEnable) = 0; 
	virtual void	SeekBar_SetRange(__int64 Min, __int64 Max) = 0; 
	virtual void	SeekBar_SetAvail(__int64 nAvail) = 0; 
	virtual ~IFSeekBar(){}
};


class IFStatusBar
{
public:
	virtual ~IFStatusBar(){}
	//Statusbar
	virtual void	Status_SetTimer(const tchar* pStrTimer) = 0; 
	virtual void	Status_SetMessage(int idi, const tchar* pStrMsg) = 0; 
	void			Status_SetTimer(REFERENCE_TIME rtCur, REFERENCE_TIME rtMax, bool fHighPrecision, int nDisplayMode = 0);
	void			Status_FormatMessage(int idi, const char* Fmt, ...); 
	static void		FormatStatusTimer(FString& posstr, FString& durstr, REFERENCE_TIME rtNow, REFERENCE_TIME rtDur, int nDisplayMode);
	virtual void	ShowBuffering(BOOL bShow) = 0; 
};

class IControlBar : public IFVolume, 
					public IFSeekBar, 
					public IFStatusBar
{
public:
	virtual ~IControlBar(){}
	//Misc
	virtual void	SetPlayer(IVideoPlayer* pPlayer) = 0; 
	virtual void	SetPlayState(BOOL bPlaying) = 0; 
    virtual void    OnStop() = 0; 
	virtual void	ShowNextPrev(BOOL bShow) = 0; 

	//Internal
	void			SaveSettings();
	void			LoadSettings(); 
};




#endif //__ICONTROLBAR_H__