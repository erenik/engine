/// Emil Hedemalm
/// 2013-03-01

#ifndef LIST_H
#define LIST_H

#include <cstdlib>

#define CLEAR_AND_DELETE(p) {for(int i = 0; i < p.Size(); ++i) delete p[i]; p.Clear();}
#define DELETE_LIST_IF_VALID(p) {if(p) CLEAR_AND_DELETE((*p)); delete p; p = NULL;}

namespace ListOption {
    /// Used for certain options when removing/adding, in order to simplify sorting possibilities.
    enum listOptions {
        NULL_OPTION = 0,
        RETAIN_ORDER = 1,
    };
};

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
	~List();
	List(const List &otherList);
	List(const T & initialItem);

	/// Operator overloading
	operator bool() const { return currentItems? true : false; };
	/// Assignment operator overloading
	const List<T> * operator = (const List &otherList);
	const List * operator += (const List &otherList);
	const List operator += (const T &newItem);
	const List operator -= (const T &itemToRemove);
	/// Const-operator overloads
	const List operator + (const List &otherList) const;
	const List operator - (const List &otherList) const;
	const List operator + (const T &newItem) const;
	const List operator - (const T &itemToRemove) const;

	/// Array-indexing operator, varying version
	T& operator[](int index);
	/// Array-indexing operator, const version
	const T& operator[](int index) const;
	/// Get last one.
	const T Last() const;

	/** Inserts an item into the list at target index. All items beyond this index will be pushed back by 1 unit. 
		E.g. placing it at index 0 will add it at the beginning of the list.
		It is recommended to use Add for adding to the end of the list.
	*/
	bool Insert(value_type item, int atIndex);
	/// Adds an item to the list
	bool Add(value_type item);
	/// Adds an item to the list at the requested index, pushing along the rest. Used for keeping them sorted or stuff.
	bool Add(value_type item, int requestedIndex);
	/// Remove target item, searching for it.
	bool Remove(value_type item);
	/// Can be called with ListOption::RETAIN_ORDER to guarantee that internal order is preserved.
	T Remove(value_type item, int removeOption);
	/// Removes item at target index.
	bool RemoveIndex(int index);
	/// Can be called with ListOption::RETAIN_ORDER to guarantee that internal order is preserved.
	bool RemoveIndex(int index, int removeOption);
	/// Clears the list of all entries. Returns amount of removed item
	int Clear();
	/// Clears the list of entries and deletes every single one of them. USE WITH CAUTION!
	int ClearAndDelete();

    /// Resizes the array. Recommended to use before filling it to avoid re-sizings later which might slow down the system if used repetitively.
    void Allocate(int newSize);

	value_type * GetArray() { return arr; };
	/// Returns size of the list
	const int & Size() const;

	/// Polls the existance/copy of target item in the queue.
	bool Exists(value_type item) const;
	/// Polls the existance/copy of target item in the queue. Returns it's index if so and -1 if not.
	int GetIndex(value_type item) const;

/// To be inherited by other classes...
protected:
	/// Resizing function
	void Resize(int newSize);

	value_type * arr;
	int currentItems;
	int arrLength;
	bool inUse;		// Defines that the list is currently being popped or pushed.
};


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
	arr = NULL;
	arrLength = 0;
	currentItems = 0;
	inUse = false;
}
/// Destructor
template <class T>
List<T>::~List(){
	if (arr)
		delete[] arr;
	arr = NULL;
	arrLength = 0;
}
/// Copy constructor
template <class T>
List<T>::List(const List & otherList){
	arrLength = otherList.arrLength;
	arr = new T[arrLength];
	currentItems = otherList.currentItems;
	/// Transfer items
	for (int i = 0; i < currentItems; ++i){
		arr[i] = otherList.arr[i];
	}
}
template <class T>
List<T>::List(const T & initialItem){
	arr = NULL;
	arrLength = 0;
	currentItems = 0;
	inUse = false;
	Add(initialItem);
}

/// Operator overloading
template <class T>
const List<T> * List<T>::operator = (const List &otherList){
	if (arr)
		delete[] arr;
	arr = NULL;
	arrLength = 0;
	currentItems = 0;
	assert(otherList.arrLength >= otherList.currentItems);
	if (otherList.arrLength > 0){
		arrLength = otherList.arrLength;
		arr = new T[arrLength];
		for (int i = 0; i < otherList.currentItems; ++i){
			arr[i] = otherList.arr[i];
		}
		currentItems = otherList.currentItems;
	}
	return this;
}
template <class T>
const List<T> * List<T>::operator += (const List<T> &otherList) {
	/// Check if resizing is needed
	if (currentItems + otherList.currentItems > arrLength)
		Resize(currentItems + otherList.currentItems);
	// Ship over stuff
	for (int i = 0; i < otherList.currentItems; ++i){
		Add(otherList.arr[i]);
	}
	return this;
}
template <class T>
const List<T> List<T>::operator += (const T &newItem){
	if (currentItems + 1 > arrLength)
		Resize(currentItems * 2);
	Add(newItem);
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
T& List<T>::operator[](int index) {
	return arr[index];
}

template <class T>
const T& List<T>::operator[](int index) const {
	return arr[index];
}

/// Get last one.
template <class T>
const T List<T>::Last() const {
	return arr[currentItems-1];
}

/** Inserts an item into the list at target index. All items beyond this index will be pushed back by 1 unit. 
	E.g. placing it at index 0 will add it at the beginning of the list.
	It is recommended to use Add for adding to the end of the list.
*/
template <class T>
bool List<T>::Insert(T item, int atIndex)
{
	if (currentItems == arrLength){
		try {
			Resize(currentItems*2);
		} catch (...){
			std::cout << "\nUnable to allocate larger size array!";
			return false;
		}
	}
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
bool List<T>::Add(T item) {
	if (currentItems == arrLength){
		try {
			Resize(currentItems * 2);
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
bool List<T>::Add(T item, int requestedIndex) {
	if (currentItems == arrLength){
		try {
			Resize(currentItems * 2);
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
bool List<T>::Remove(T item){
	for (int i = 0; i < currentItems; ++i){
		/// found it!
		if (arr[i] == item){
			currentItems--;
			arr[i] = arr[currentItems];
			return true;
		}
	}
	return false;
}

/// Remove target item or index, keeping the internal order of the remaining objects!
template <class T>
T List<T>::Remove(T item, int removeOption){
    assert(removeOption == ListOption::RETAIN_ORDER);
	for (int i = 0; i < currentItems; ++i){
		/// found it!
		if (arr[i] == item){
		    /// Move down the remaining objects.
		    for (int j = i+1; j < currentItems; ++j){
                arr[j-1] = arr[j];
		    }
			currentItems--;
			return item;
		}
	}
	return NULL;
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
bool List<T>::RemoveIndex(int index, int removeOption){
	assert(index < currentItems && "Index out of bounds! You're probably doing something wrong.");
	if (index >= currentItems)
		return false;

    assert(removeOption == ListOption::RETAIN_ORDER);

    /// Move down the remaining objects.
	for (int i = index; i < currentItems; ++i){
		arr[i] = arr[i+1];
	}
	--currentItems;
	return true;
}


/// Clears the list of all entries. Returns amount of removed items.
template <class T>
int List<T>::Clear(){
	currentItems = 0;
	int itemsRemoved = currentItems;
	currentItems = 0;
	return itemsRemoved;
}

/// Clears the list of entries and deletes every single one of them. USE WITH CAUTION!
/// Doesn't work in GCC apparently, maybe bad move.

template <class T>
int List<T>::ClearAndDelete(){
	for (int i = 0; i < currentItems; ++i)
		delete arr[i];
	int itemsRemoved = currentItems;
	currentItems = 0;
	return itemsRemoved;
}


/// Returns size of the list
template <class T>
const int & List<T>::Size() const {
	return currentItems;
}

/// Polls the existance/copy of target item in the queue.
template <class T>
bool List<T>::Exists(T item) const {
	for (int i = 0; i < currentItems; ++i){
		if (arr[i] == item)
            return true;
	}
	return false;
}

/// Polls the existance/copy of target item in the queue. Returns it's index if so and -1 if not.
template <class T>
int List<T>::GetIndex(T item) const {
	for (int i = 0; i < currentItems; ++i){
		if (arr[i] == item)
            return i;
	}
	return -1;
}

/// Resizes the array. Recommended to use before filling it to avoid re-sizings later which might slow down the system if used repetitively.
template <class T>
void List<T>::Allocate(int newSize){
	Resize(newSize);
}

/// Resizing function
template <class T>
void List<T>::Resize(int newSize){
	if (newSize == 0)
		newSize = 8;
	assert(newSize >= currentItems);
	T * newArr = new T[newSize];
	for (int i = 0; i < currentItems; ++i)
		newArr[i] = arr[i];
	if (arr)
		delete[] arr;
	arr = newArr;
	arrLength = newSize;
}


#endif
