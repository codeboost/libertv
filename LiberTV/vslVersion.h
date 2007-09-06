#ifndef __VSLVERSION_H__
#define __VSLVERSION_H__
struct vslVersion
{
	long	m_vMajor; 
	long	m_vMinor; 
	long	m_vRev; 
	long	m_vBuild;

	//Returns:
	//0 - Version are identical
	//>0 - Version is greater than pVer
	//<0 - Version is smaller than pVer
	int VersionDif (vslVersion& pVer)
	{
		int	res		= m_vMajor - pVer.m_vMajor;

		if (res == 0)
		{
			res = m_vMinor - pVer.m_vMinor;
			if (res == 0)
			{
				res = m_vRev - pVer.m_vRev;
				if (res == 0)
				{
					res = m_vBuild - pVer.m_vBuild;
				}
			}
		}
		return res;
	}

	bool operator == (vslVersion& pVer)
	{
		return (VersionDif(pVer) == 0); 
	}

	bool operator > (vslVersion & pVer)
	{
		return (VersionDif(pVer) > 0);
	}

	bool operator <(vslVersion & pVer)
	{
		return (VersionDif(pVer) < 0);
	}

	bool operator >=(vslVersion& pVer)
	{
		int nRes = VersionDif(pVer); 
		return nRes >=0; 
	}

	bool operator <=(vslVersion& pVer)
	{
		int nRes = VersionDif(pVer);
		return nRes<=0; 
	}

	bool operator != (vslVersion & pVer)
	{
		return !operator==(pVer); 
	}

	vslVersion()
	{
		m_vMajor = m_vMinor = m_vRev = m_vBuild = 0; 
	}
	vslVersion& operator = (const char* pStr);
	vslVersion(const char* pStrVer);
};



#endif //__VSLVERSION_H__