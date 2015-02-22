/// Emil Hedemalm
/// 2013-03-01

#ifndef LIST_H
#define LIST_H

#include <cstdlib>
#include <cstdarg>

#define CLEAR_AND_DELETE(p) {for(int i = 0; i < p.Size(); ++i) delete p[i]; p.Clear();}
#define DELETE_LIST_IF_VALID(p) {if(p) CLEAR_AND_DELETE((*p)); delete p; p = NULL;}

namespace ListOption {
    /// Used for certain options when removing/adding, in order to simplify sorting possibilities.
    enum listOptions {
        NULL_OPTION = 0,
        RETAIN_ORDER = 1,
    };
};


/// Align all allocation in the List class to 16 byte intervals, in order to support SSE/SIMD?
/// 	_arr2 = (float*) _aligned_malloc(ARRAY_SIZE * sizeof(float), 16);
///	_aligned_free(_arr2);


/** A custom list class for handling arbitrary amounts of objects easily!
	It is implemented as a dynamic array.
	The items stored need to have = operator overloaded as well as base constructors!
*/
template <class T>
class List
{
public:
	typedef T value_type;

	List();
	List(const List &otherList);
	List(const T & initialItem);
	/// Creates a new list of specific amount of items.
	List(const T & item, const T & item2) { Nullify(); AddItem(item); AddItem(item2); };
	List(const T & item, const T & item2, const T & item3) { Nullify(); Add(item, item2, item3); };
	List(const T & item, const T & item2, const T & item3, const T & item4) { Nullify(); Add(item, item2, item3, item4);};
	void Nullify();
	~List();

	/** Sets currentItems to arraySize. 
		Useful if using Allocate() followed by a memory operation using the GetArray()'s pointer, 
		instead of filling the list one object at a time.
	*/
	void SetFull();

	/// Operator overloading
//	operator bool() const { return currentItems? true : false; }; // <- Was causing loads of shitty errors
	/// Assignment operator overloading
	const List<T> & operator = (const List &otherList);
	const List * operator += (const List &otherList);
	const List operator += (const T &newItem);
	const List operator -= (const T &itemToRemove);
	/// Const-operator overloads
	const List operator + (const List &otherList) const;
	const List operator - (const List &otherList) const;
	const List operator + (const T &newItem) const;
	const List operator - (const T &itemToRemove) const;

	/** Extraction operator for when the list has 1 element (similar but inverted relationship of when creating a list from a single element of class T)
		Will throw errors or assertions if the list has any number of elements except 1.
	*/
	operator const T & ();

	/// Array-indexing operator, varying version
	virtual T& operator[](int index);
	/// Array-indexing operator, const version
	virtual const T& operator[](int index) const;
	/// Get last one.
	T& Last();

	/** Inserts an item into the list at target index. All items beyond this index will be pushed back by 1 unit. 
		E.g. placing it at index 0 will add it at the beginning of the list.
		It is recommended to use Add for adding to the end of the list.
	*/
	bool Insert(const value_type & item, int atIndex);
	/// Adds an item to the list
	bool AddItem(const value_type & item);
	/// Adds an item to the list
	bool Add(const List<value_type> & item);
	/// Adds an item to the list at the requested index, pushing along the rest. Used for keeping them sorted or stuff.
	bool Add(const value_type & item, int requestedIndex);
	void Add(const T & item, const T & item2) { Add(item); Add(item2);};
	void Add(const T & item, const T & item2, const T & item3) { Add(item); Add(item2); Add(item3);};
	void Add(const T & item, const T & item2, const T & item3, const T & item4) { Add(item); Add(item2); Add(item3); Add(item4); };
	void Add(const T & item, const T & item2, const T & item3, const T & item4, const T & item5) { Add(item); Add(item2); Add(item3); Add(item4); Add(item5); };
	/// Adds multiple items to the list
	bool AddArray(int numItems, T * itemArray);
	/// Removes, not caring for internal order. Quicker.
	bool RemoveItemUnsorted(const value_type & item);
	/// Removes, retains internal order.
	T RemoveItem(const value_type & item);
	/// Removes all occurences of any items in the sublist in this list. Does not retain order. Quicker.
	int RemoveUnsorted(const List<T> & subListToRemove);
	/// Removes all occurences of any items in the sublist in this list. Retains order.
	int Remove(const List<T> & subListToRemove);
	/// Removes item at target index.
	bool RemoveIndex(int index);
	/// Can be called with ListOption::RETAIN_ORDER to guarantee that internal order is preserved.
	bool RemoveIndex(int index, int removeOption);
	/// Clears the list of all entries. Returns amount of removed item
	int Clear();
	/// Clears the list of entries and deletes every single one of them. USE WITH CAUTION!
	int ClearAndDelete();

	/// Returns a part of this list (sub-list?), including all indices between the range, including the start and stop indices. A -1 for stopIndex will be replaced with the length.
	List<T> Part(int startIndex, int stopIndex = -1) const;
	/// Removes the part of the list spanning within the designated indices (inclusive of the edge-indices).
	void RemovePart(int fromIndex, int toIndex);
					

	/// Swaps items at given indices.
	bool Swap(int index, int otherIndex);

    /// Resizes the array. Recommended to use before filling it to avoid re-sizings later which might slow down the system if used repetitively.
    void Allocate(int newSize, bool setFull = false);
	/// Deallocates the array. Use in destructors. If items stored within are allocated on the heap, it is recommended to call ClearAndDelete() first.
	void Deallocate();

	value_type * GetArray() { return arr; };
	/// Returns size of the list
	const int & Size() const;

	/// Polls the existance/copy of target item in the queue.
	bool Exists(const value_type & item) const;
	/// Polls the existance/copy of all items in target list within this list. Returns false if any items are missing.
	bool Exists(const List<value_type> & subList) const;
	/// Checks occurances of target item in the list.
	int Occurances(const value_type & item) const;
	/// Checks for duplicates in the list.
	int Duplicates();
	/// Returns content by index. Similar to the [] operator.
	value_type & GetIndex(int index);

	/// Polls the existance/copy of target item in the queue. Returns it's index if so and -1 if not.
	int GetIndexOf(const value_type & item) const;

	/// Allocates a new array.
	static T * AllocateArray(int size);
	/// Deletes the array and calls the destructor on all objects. Does NOT set currentItems nor arrLength to 0! Deallocate does that.
	static void DeleteArray(T * arr, int arrLength);

/// To be inherited by other classes...
protected:
	/// Resizing function
	void Resize(int newSize);

	value_type * arr;
	int currentItems;
	int arrLength;
	bool inUse;		// Defines that the list is currently being popped or pushed.
};

// Conversion of a list's contents from class T to class B. o.o
template<class T, class B> 
List<B> ConvertList(List<T> tList)
{
	List<B> bList;
	for (int i = 0; i < tList.Size(); ++i)
	{
		bList.AddItem(tList[i]);
	}
	return bList;
}

/// Implementation below because I'm too lazy to find how to include a separate blerargh template file in OSX.

//#include "List.template"


/// Emil Hedemalm
/// 2013-03-01

#include <iostream>
#include <cassert>

/// Default constructor
template <class T>
List<T>::List()
{
	Nullify();	
}
/// Copy constructor
template <class T>
List<T>::List(const List & otherList)
{
	arrLength = otherList.arrLength;

	arr = AllocateArray(arrLength);

	currentItems = otherList.currentItems;
	/// Transfer items
	for (int i = 0; i < currentItems; ++i){
		arr[i] = otherList.arr[i];
	}
}

template <class T>
List<T>::List(const T & initialItem)
{
	Nullify();
	AddItem(initialItem);
}

template <class T>
void List<T>::Nullify()
{
	arr = NULL;
	arrLength = 0;
	currentItems = 0;
	inUse = false;
}	
/// Destructor
template <class T>
List<T>::~List()
{
	assert(arrLength >= 0 && arrLength < 10000000);
	Deallocate();
	arr = NULL;
	arrLength = 0;
}

/** Sets currentItems to arraySize. 
	Useful if using Allocate() followed by a memory operation using the GetArray()'s pointer, 
	instead of filling the list one object at a time.
*/
template <class T>
void List<T>::SetFull()
{
	currentItems = arrLength;
}

/// Operator overloading
template <class T>
const List<T> & List<T>::operator = (const List &otherList)
{
	// Re-allocate if needed only.
	if (arrLength < otherList.arrLength)
	{
		Resize(otherList.currentItems);
	}
	assert(otherList.arrLength >= otherList.currentItems);
	// Copy the items.
	for (int i = 0; i < otherList.currentItems; ++i)
	{
		arr[i] = otherList.arr[i];
	}
	currentItems = otherList.currentItems;
	return *this;
}

template <class T>
const List<T> * List<T>::operator += (const List<T> &otherList) 
{
	/// Check if resizing is needed
	if (currentItems + otherList.currentItems > arrLength)
		Resize(currentItems + otherList.currentItems);
	// Ship over stuff
	for (int i = 0; i < otherList.currentItems; ++i){
		AddItem(otherList.arr[i]);
	}
	return this;
}
template <class T>
const List<T> List<T>::operator += (const T &newItem)
{
	if (currentItems + 1 > arrLength)
		Resize(currentItems * 2 + 1);
	AddItem(newItem);
	return *this;
}
template <class T>
const List<T> List<T>::operator -= (const T &itemToRemove)
{
	Remove(itemToRemove);
	List<T> newList;
	newList += *this;
	return newList;
}
template <class T>
const List<T> List<T>::operator + (const List<T> &otherList) const {
	List<T> newList;
	newList += *this;
	newList += otherList;
	return newList;
}
template <class T>
const List<T> List<T>::operator - (const List<T> &otherList) const 
{
	List<T> newList;
	newList += *this;
	for (int i = 0; i < otherList.Size(); ++i){
		newList.Remove(otherList[i]);
	}	
	return newList;
}
template <class T>
const List<T> List<T>::operator + (const T &newItem) const 
{
	List<T> newList;
	newList += *this;
	newList.AddItem(newItem);
	return newList;
}

/** Extraction operator for when the list has 1 element (similar but inverted relationship of when creating a list from a single element of class T)
	Will throw errors or assertions if the list has any number of elements except 1.
*/
template <class T>
List<T>::operator const T & () 
{
	if (this->currentItems != 1)
	{
		assert(false && "Number of items not 1. Handle with some other functions that the simple conversion operator.");
		throw 3;
	}
	return arr[0];
}


template <class T>
T& List<T>::operator[](int index) 
{
	assert(index >= 0 && index < arrLength);
	return arr[index];
}

template <class T>
const T& List<T>::operator[](int index) const 
{
	assert(index >= 0 && index < arrLength);
	return arr[index];
}

/// Get last one.
template <class T>
T& List<T>::Last() {
	return arr[currentItems-1];
}

/** Inserts an item into the list at target index. All items beyond this index will be pushed back by 1 unit. 
	E.g. placing it at index 0 will add it at the beginning of the list.
	It is recommended to use Add for adding to the end of the list.
*/
template <class T>
bool List<T>::Insert(const T & item, int atIndex)
{
	if (currentItems == arrLength){
		try {
			Resize(currentItems*2);
		} catch (...){
			std::cout << "\nUnable to allocate larger size array!";
			return false;
		}
	}
	if (arrLength == 0)
		Resize(1);
	/// Move everything back 1 step.
	for (int i = currentItems; i > atIndex; --i)
	{
		arr[i] = arr[i-1];
	} 
	/// Insert it.
	arr[atIndex] = item;
	++currentItems;
	return true;
}

/// Adds an item to the list
template <class T>
bool List<T>::AddItem(const T & item) 
{
	if (currentItems == arrLength)
	{
		try {
			Resize(currentItems * 2 + 1);
		} catch (...){
			std::cout << "\nUnable to allocate larger size array!";
			return false;
		}
	}
	arr[currentItems] = item;
	++currentItems;
	return true;
}

/// Adds an item to the list
template <class T>
bool List<T>::Add(const List<T> & items)
{
	// Resize as needed.
	Resize(currentItems + items.Size());
	// And add 'em.
	for (int i = 0; i < items.Size(); ++i)
	{
		arr[currentItems + i] = items[i];
	}
	currentItems = currentItems + items.Size();
	return true;
}
	
template <class T>
bool List<T>::AddArray(int numItems, T * itemArray) 
{
	// Resize as needed.
	Resize(currentItems + numItems);
	// And add 'em.
	for (int i = 0; i < numItems; ++i)
	{
		arr[currentItems + i] = itemArray[i];
	}
	currentItems = currentItems + numItems;
	return true;
	/*
	Add(item);
	Add(item2);
	// Variable length arguments.
	// http://www.cprogramming.com/tutorial/c/lesson17.html
	// Initializing arguments to store all values after num 
	va_list arguments;    
	int extraArgs = numItems - 2;
    va_start ( arguments, item2);
    // Sum all the inputs; we still rely on the function caller to tell us how
     // many there are 
    for ( int x = 0; x < extraArgs; x++ )        
    {
		T itemt = va_arg (arguments, T);
        Add(itemt);
    }
    va_end ( arguments );      // Cleans up the list
	*/
	return true;
}

/// Adds an item to the list
template <class T>
bool List<T>::Add(const T & item, int requestedIndex) 
{
	if (currentItems == arrLength){
		try {
			Resize(currentItems * 2 + 1);
		} catch (...){
			std::cout << "\nUnable to allocate larger size array!";
			return false;
		}
	}
	assert(requestedIndex >= 0 && requestedIndex <= currentItems);
	/// Move along stuff.
	for (int i = currentItems; i > requestedIndex; --i){
        arr[i] = arr[i-1];
	}
	/// Place it at the requested index.
	arr[requestedIndex] = item;
	++currentItems;
	return true;
}

/// Remove target item or index
template <class T>
bool List<T>::RemoveItemUnsorted(const T & item)
{
	for (int i = 0; i < currentItems; ++i)
	{
		/// found it!
		if (arr[i] == item)
		{
			currentItems--;
			arr[i] = arr[currentItems];
			return true;
		}
	}
	return false;
}

/// Remove target item or index, keeping the internal order of the remaining objects!
template <class T>
T List<T>::RemoveItem(const T & item)
{
	for (int i = 0; i < currentItems; ++i)
	{
		/// found it!
		if (arr[i] == item)
		{
		    /// Move down the remaining objects.
		    for (int j = i+1; j < currentItems; ++j)
			{
                arr[j-1] = arr[j];
		    }
			currentItems--;
			return item;
		}
	}
	return NULL;
}

/// Removes all occurences of any items in the sublist in this list.
template <class T>
int List<T>::RemoveUnsorted(const List<T> & subListToRemove)
{  
	int removed = 0;
	for (int i = 0; i < currentItems; ++i)
	{
		/// found it?
		if (subListToRemove.Exists(arr[i]))
		{
			currentItems--;
			arr[i] = arr[currentItems];
			--i;
			++removed;
		}
	}
	return removed;
}

/// Removes all occurences of any items in the sublist in this list.
template <class T>
int List<T>::Remove(const List<T> & subListToRemove)
{  
	int removed = 0;
	for (int i = 0; i < currentItems; ++i)
	{
		/// found it?
		if (subListToRemove.Exists(arr[i]))
		{
		    /// Move down the remaining objects.
		    for (int j = i+1; j < currentItems; ++j){
                arr[j-1] = arr[j];
		    }
			currentItems--;
			++removed;
		}
	}
	return removed;
}
	

template <class T>
bool List<T>::RemoveIndex(int index){
	assert(index < currentItems && "Index out of bounds! You're probably doing something wrong.");
	if (index >= currentItems)
		return false;
	currentItems--;
    arr[index] = arr[currentItems];
	return true;
}

template <class T>
bool List<T>::RemoveIndex(int index, int removeOption)
{
	assert(index < currentItems && "Index out of bounds! You're probably doing something wrong.");
	if (index >= currentItems)
		return false;

    assert(removeOption == ListOption::RETAIN_ORDER);

    /// Move down the remaining objects.
	for (int i = index; i < currentItems - 1; ++i){
		arr[i] = arr[i+1];
	}
	--currentItems;
	return true;
}


/// Clears the list of all entries. Returns amount of removed items.
template <class T>
int List<T>::Clear()
{
	int itemsRemoved = currentItems;
	currentItems = 0;
	return itemsRemoved;
}

/// Clears the list of entries and deletes every single one of them. USE WITH CAUTION!
/// Doesn't work in GCC apparently, maybe bad move.

template <class T>
int List<T>::ClearAndDelete()
{
	for (int i = 0; i < currentItems; ++i)
		delete arr[i];
	int itemsRemoved = currentItems;
	currentItems = 0;
	return itemsRemoved;
}

/// Returns a part of this list (sub-list?), including all indices between the range, including the start and stop indices.
template <class T>
List<T> List<T>::Part(int startIndex, int stopIndex) const
{
	List<T> partList;
	if (stopIndex == -1)
		stopIndex = currentItems - 1;
	for (int i = startIndex; i <= stopIndex; ++i)
	{
		partList.AddItem(arr[i]);
	}
	return partList;
}

/// Removes the part of the list spanning within the designated indices (inclusive of the edge-indices).
template <class T>
void List<T>::RemovePart(int fromIndex, int toIndex)
{
	/// Move down the list, retaining order by default.
	int itemsRemoved = toIndex - fromIndex + 1;
	/// Move back the ENTIRE array. Nost just the same amount as removed...
	for (int j = fromIndex; j < currentItems - itemsRemoved; ++j)
	{
		arr[j] = arr[j + itemsRemoved];
	}
	currentItems -= itemsRemoved;
}	

/// Swaps items at given indices.
template <class T>
bool List<T>::Swap(int index, int otherIndex)
{
	if (index < 0 || otherIndex < 0 ||
		index > currentItems || otherIndex >= currentItems)
		return false;
	T tmp = arr[index];
	arr[index] = arr[otherIndex];
	arr[otherIndex] = tmp;
	return true;
}

/// Returns size of the list
template <class T>
const int & List<T>::Size() const {
	return currentItems;
}

/// Polls the existance/copy of target item in the queue.
template <class T>
bool List<T>::Exists(const T & item) const {
	for (int i = 0; i < currentItems; ++i){
		if (arr[i] == item)
            return true;
	}
	return false;
}

/// Checks occurances of target item in the list.
template <class T>
int List<T>::Occurances(const value_type & item) const
{
	int occurances = 0;
	for (int i = 0; i < currentItems; ++i)
	{
		if (arr[i] == item)
			++occurances;
	}
	return occurances;
}
	
/// Checks for duplicates in the list.
template <class T>
int List<T>::Duplicates()
{
	int duplicates = 0;
	for (int i = 0; i < currentItems; ++i)
	{
		T item = arr[i];
		for (int j = i + 1; j < currentItems; ++j)
		{
			T item2 = arr[j];
			if (item == item2)
				++duplicates;
		}
	}
	return duplicates;
}

/// Polls the existance/copy of all items in target list within this list. Returns false if any items are missing.
template <class T>
bool List<T>::Exists(const List<T> & subList) const
{
	int found = 0;
	for (int i = 0; i < subList.Size(); ++i)
	{
		if (!Exists(subList[i]))
			return false;
	}
	/// If the entire sublist is now empty, it means the same items are present in this main list.
	return true;
}



/// Polls the existance/copy of target item in the queue. Returns it's index if so and -1 if not.
template <class T>
T & List<T>::GetIndex(int index) 
{
	assert(index >= 0 && index < currentItems);
	return arr[index];
}

/// Polls the existance/copy of target item in the queue. Returns it's index if so and -1 if not.
template <class T>
int List<T>::GetIndexOf(const T & item) const {
	for (int i = 0; i < currentItems; ++i){
		if (arr[i] == item)
            return i;
	}
	return -1;
}

/// Resizes the array. Recommended to use before filling it to avoid re-sizings later which might slow down the system if used repetitively.
template <class T>
void List<T>::Allocate(int newSize, bool setFull)
{
	Resize(newSize);
	if (setFull)
		currentItems = newSize;
}

/// Deallocates the array. Use in destructors. If items stored within are allocated on the heap, it is recommended to call ClearAndDelete() first.
template <class T>
void List<T>::Deallocate()
{
	DeleteArray(arr, arrLength);
	arrLength = 0;
	currentItems = 0;
}


/// Resizing function
template <class T>
void List<T>::Resize(int newSize)
{
	// If same size, return. Already got the full length. Waste of time to delete and re-allocate.
	if (newSize == arrLength)
		return;
	// Just remove if 0.
	if (newSize == 0)
	{
		newSize = 8;
	}
	T * newArr = AllocateArray(newSize);
	// If new array is larger, copy over old items.
	if (newSize >= currentItems)
	{
		for (int i = 0; i < currentItems; ++i)
			newArr[i] = arr[i];
	}
	// Delete old array.
	DeleteArray(arr, arrLength);
	// Paste in the new one.
	arr = newArr;
	arrLength = newSize;
}


template <class T>
T * List<T>::AllocateArray(int num)
{
#ifdef USE_SSE
	T * newArr = (T*) _aligned_malloc(newSize * sizeof(T), 16);
	// Run constructor on the new objects?
	for (int i = 0; i < newSize; ++i)
		new((void*)&newArr[i]) T();
	return newArr;
#else
	return new T[num];
#endif
}

/// Deletes the array and calls the destructor on all objects. Does NOT set currentItems nor arrLength to 0! Deallocate does that.
template <class T>
void List<T>::DeleteArray(T * arr, int arrLength)
{
	if (arr)
	{
#ifdef USE_SSE
		for (int i = 0; i < arrLength; ++i)
			arr[i].~T();
		_aligned_free(arr);
#else
		delete[] arr;
#endif
		arr = 0;
	}
}


#endif
