#ifndef __MZ_RUNTIME_H__
#define __MZ_RUNTIME_H__
#include "mz_Inc.h"
#include "mzString.h"


class MZHTTP_API mz_RuntimeClass
{
private:
	const mz_RuntimeClass*	m_BaseClass; 
	const mzString			m_Name; 
	mz_RuntimeClass&		operator = (mz_RuntimeClass& pNew);
							mz_RuntimeClass (mz_RuntimeClass& pNew);
public:
	mz_RuntimeClass (const char* szClassName, const mz_RuntimeClass* pBaseClass): m_Name(szClassName){
		m_BaseClass = pBaseClass;
	}


	const mz_RuntimeClass* GetBaseRTTI () const
	{
		return m_BaseClass;
	}
	const mzString& GetClassName () const
	{
		return m_Name;
	}
};
#endif //__MZ_RUNTIME_H__
