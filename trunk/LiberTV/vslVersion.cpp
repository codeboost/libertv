#include "stdafx.h"
#include "vslVersion.h"

vslVersion& vslVersion::operator = (const char* pStr)
{
	if (NULL != pStr)
		sscanf(pStr, "%d.%d.%d.%d", &m_vMajor, &m_vMinor, &m_vRev, &m_vBuild);

	return *this; 
}

vslVersion::vslVersion(const char *pStr)
{
	m_vMinor = m_vMajor = m_vRev = m_vBuild = 0; 
	if (NULL != pStr)
		sscanf(pStr, "%d.%d.%d.%d", &m_vMajor, &m_vMinor, &m_vRev, &m_vBuild);
}

bool IsVersionGreater(const char* pStrVersion1, const char* pStrVersion2)
{
	vslVersion v1 = pStrVersion1; 
	vslVersion v2 = pStrVersion2; 
	return v1 > v2; 
}