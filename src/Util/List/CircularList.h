/// Emil Hedemalm
/// 2014-07-04
/// Circular list based on the List class.
/** The main difference is that all array-access functions based on indices will instead work in a circular fashion, making something like
	the following viable no matter the size or length of the contents within, as they will wrap automatically:

	for(int i = 0; true; ++i)
	{
		// do something with circularList[i]; 
	}
*/

#ifndef CIRCULAR_LIST_H
#define CIRCULAR_LIST_H

#include "List.h"

template<class T>

class CircularList : public List<T>
{
public:
	typedef T value_type;
	/// Array-indexing operator, varying version
	virtual T& operator[](int index);
	/// Array-indexing operator, const version
	virtual const T& operator[](int index) const;
	/// Returns content by index. Similar to the [] operator.
	value_type & GetIndex(int index);
};

/// Implementation below.


/// Emil Hedemalm
/// 2014-07-04


#include <iostream>
#include <cassert>


template <class T>
T& CircularList<T>::operator[](int index) 
{
	while(index < 0)
		index += currentItems;
	return arr[index % currentItems];
}

template <class T>
const T& CircularList<T>::operator[](int index) const 
{
	while(index < 0)
		index += currentItems;
	return arr[index % currentItems];
}




#endif


