#ifndef __CONTAINER_H__
#define __CONTAINER_H__
#include "mzlist.h"
#include "mzarray.h"

//mzList. A slightly better mzListBase implementation
//Adds List<->Array functionality plus some operators

template <class T>
class MZHTTP_API mzList : public mzListBase<T>
{
public:

	void		ToArray (mzDynArray<T>& Array, bool ClearArray = false) const;  //adds list contents to Array. 
	void		FromArray (const mzDynArray<T>& Array, bool ClearList = false); //gets list contents from array
	void		RemoveItem (const T& Value);								  //make sure T has a properly defined operator==

	//This is highly experimental. Copies pointers to list items to the const T* array
	//Note that if an item is removed from the list and you use the pointer from the array, you're in trouble.
	//Make sure you keep the ptr array and list in sync. 
	void		ToPtrArray (mzDynArray<const T*>& Array, bool ClearArray = false);

	//Some useful operators
	mzList<T>&	operator= (const mzDynArray<T>& A); 				//gets contents from array
	mzList<T>&	operator+ (const mzDynArray<T>& A); 				//adds array contents to the list
	mzList<T>&	operator+= (const mzDynArray<T>& A);				 //same as operator+
	mzList<T>&	operator= (mzList<T>& rhs); 					   //copies contents from list rhs
	mzList<T>&	operator+ (const T& V); 						   //adds an item to the end of the list
	mzList<T>&	operator- (const T& V); 						   //removes an item from the list

	virtual ~mzList ()
	{
	}
};


template <class T>
void mzList<T>::ToArray (mzDynArray<T>& Array, bool Clear) const
{
	if (Clear)
	{
		Array.ClearAll();
	}

	Array.SetSize(Array.GetCount() + GetCount()); //to speed things up a little
	mzList<T>::mzListIterator Iter	= Iterate(); 
	while (Iter.Curr())
	{
		Array.Add(*Iter); 
		Iter++;
	}
}

template <class T>
void mzList<T>::FromArray (const mzDynArray<T>& Array, bool ClearList)
{
	if (ClearList)
	{
		Clear();
	} 

	for (int k = 0; k < Array.GetCount())
	{
		AddLast(Array[k]);
	}
}

template <class T>
mzList<T>& mzList<T> ::operator = (const mzDynArray<T>& A)
{
	FromArray(A, true);
	return *this;
}

template <class T>
mzList<T>& mzList<T> ::operator+ (const mzDynArray<T>& A)
{
	FromArray(A, false); 
	return *this;
}

template <class T>
mzList<T>& mzList<T> ::operator+= (const mzDynArray<T>& A)
{
	FromArray(A, false); 
	return *this;
}

template <class T>
mzList<T>& mzList<T> ::operator= (mzList<T>& rhs)
{
	if (this == &rhs)
	{
		return *this;
	} 

	Clear(); 
	mzList<T>::mzListIterator Iter	= rhs.Iterate(); 
	while (Iter.Curr())
	{
		AddLast(*Iter); 
		Iter++;
	}

	return *this;
}


template <class T>
mzList<T>& mzList<T> ::operator+ (const T& V)
{
	AddLast(T);
}

template <class T>
void mzList<T>::RemoveItem (const T& Value)
{
	mzList<T>::mzListIterator Iter	= Iterate(); 
	if (Iter.Find(Value))
	{
		Iter.RemoveCurrent();
	}
}

template <class T>
mzList<T>& mzList<T> ::operator- (const T& Value)
{
	RemoveItem(Value);
	return *this;
}

template <class T>
void mzList<T>::ToPtrArray (mzDynArray<const T*>& Array, bool Clear)
{
	if (Clear)
	{
		Array.Clear();
	}
	Array.SetSize(Array.GetCount() + GetCount()); 
	mzList<T>::mzListIterator Iter	= Iterate(); 
	while (Iter.Curr())
	{
		Array.Add(&(*Iter)); 
		Iter++;
	}
}

#endif //__CONTAINER_H__
