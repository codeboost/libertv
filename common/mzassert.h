#ifndef __MZASSERT_H__
#define __MZASSERT_H__

#if defined (IGNORE_STDC)
void	DoAssertion (long bExp);
#define assert(x) DoAssertion((x))
#else
#include <assert.h>
#endif


#endif //__MZASSERT_H__
