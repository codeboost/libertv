#ifndef __MZ_OBJECT_H__
#define __MZ_OBJECT_H__
#include "mz_Inc.h"
#include "mz_runtime.h"

//Base class for all MZ derived objects. 
//To include runtime information, do the following:
//in the .h file:
//class mz_Derived : mzObject
//{
//  MZ_INCLUDE_RTTI();
// ...normal class declaration
//};
//In the .cpp file:
//MZ_IMPLEMENT_RUNTIME(mz_Derived, mz_BaseClass, "mz_Derived");

//Note: This doesn't exactly work with multiple inheritance. 
//Avoid it (multiple inheritance) anyways, because it's a troublemaker.

class MZHTTP_API mz_Object
{
	static const mz_RuntimeClass*	m_pRTTI; 

public:

	mz_Object ()
	{
	}
	virtual ~mz_Object ()
	{
	}
};

#define MZ_INCLUDE_RTTI(classname) 
#define MZ_IMPLEMENT_RUNTIME(classname) 


class MZHTTP_API mz_StatsObject : public mz_Object
{
	MZ_INCLUDE_RTTI (mz_StatsObject)
public:
	mz_StatsObject ()
	{
	}
	virtual ~mz_StatsObject ()
	{
	}
	virtual void	Stats () = 0;
};
#endif //__MZ_OBJECT_H__
