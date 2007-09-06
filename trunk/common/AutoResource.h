#pragma once
///////////////////////////////////////////////////////////
// Class CAutoResource
//
// An object of this template class holds a resource, and will dispose of
// the resource when it is destructed.  This class is a generalization of
// auto_ptr in STL and CAutoPtr in ATL.  The user must supply the function
// doing the disposition as a template parameter.
//
// Examples where this class can be useful include file/window/registry
// handles and pointers.

template< typename T, void (*Dispose)( T ), const T NullResource = 0>
class CAutoResource
{
public:
	typedef T ResourceType;
	CAutoResource( T Resource = NullResource )
		: m_Resource( Resource )
	{
	}
	~CAutoResource()
	{
		if ( NullResource != m_Resource )
		{
			Dispose( m_Resource );
		}
	}
	operator T() const
	{
		return m_Resource;
	}
	T Get() const
	{
		return m_Resource;
	}
	T operator =(T rh)
	{
		Attach(rh);
		return m_Resource; 
	}
	bool operator == (T rh)
	{
		return m_Resource == rh; 
	}
	void Attach( T NewResource )
	{
		Dispose( m_Resource );
		m_Resource = NewResource;
	}
	// The assert on operator & usually indicates a bug.
	T * operator & ()
	{
		_ASSERTE( NullResource == m_Resource );
		return & m_Resource;
	}
private:
	T   m_Resource;
	CAutoResource( const CAutoResource& );
	CAutoResource& operator = ( const CAutoResource& );
};