#ifndef __FCONTROLBARDEF_H__
#define __FCONTROLBARDEF_H__

class FIControlBar
{
public:
	virtual void SetSliderRange(__int64 rAvail, __int64 rMax); 
	virtual void SetSliderPos(__int64 rtNow);
};
	

#endif //__FCONTROLBARDEF_H__