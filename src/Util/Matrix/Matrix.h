/// Emil Hedemalm
/// 2014-07-28
/// A configurable/custom matrix class for storing arbitrary data (E.g. mat-data)

#ifndef MATRIX_H
#define MATRIX_H

#include <cstdlib>
#include <cstdarg>
#include "MathLib.h"

#define CLEAR_AND_DELETE(p) {for(int i = 0; i < p.Size(); ++i) delete p[i]; p.Clear();}
#define DELETE_LIST_IF_VALID(p) {if(p) CLEAR_AND_DELETE((*p)); delete p; p = NULL;}

/** A custom matrix class for handling grid-based data or objects.
	It is implemented as a dynamic array.
	The items stored need to have = operator overloaded as well as base constructors!

	Basic usage assumes using the double operator [x][y] to access elements (for 2D matrices).

	Suggested approach:

		Matrix myMatrix;
		myMatrix.SetDefaultValue(myValue);
		myMatrix.Allocate(mySize);
*/
template <class T>
class Matrix
{
public:
	typedef T value_type;

	T defaultValue;

	/// Empty constructor. Must call SetSize first before usage.
	Matrix();
	/// Initializes the matrix to given size (calling SetSize)
	Matrix(Vector2i size);
	/// Destructor. De-allocates the matrix but not hte individual objects (if created on the heap).
	~Matrix();
	/// Copy constructor
	Matrix(const Matrix &otherMatrix);

	/// Sets the default value with is written to each node upon allocation.
	void SetDefaultValue(T defaultValue);

	/// Resizing function for 2D matrices.
	void Allocate(Vector2i size);
	/// Resizing function for 3D matrices.
	void Allocate(Vector3i size);
	/// Same as allocate..
	virtual void SetSize(Vector2i size);
	
	/// Loads target list as elements into the matrix.
	bool Load(List<value_type> & listIntoMatrix);

	/// Prints conents, along with position (x/y)
	void PrintContents();

	/// Operator overloading
//	operator bool() const { return currentItems? true : false; }; // <- Was causing loads of shitty errors
	/// Assignment operator overloading
	const Matrix<T> & operator = (const Matrix &otherList);
	
	/*
	const List * operator += (const List &otherList);
	const List operator += (const T &newItem);
	const List operator -= (const T &itemToRemove);

	/// Const-operator overloads
	const List operator + (const List &otherList) const;
	const List operator - (const List &otherList) const;
	const List operator + (const T &newItem) const;
	const List operator - (const T &itemToRemove) const;
*/

	T At(int x, int y = 0, int z = 0);
	T At(Vector3i pos);
	void Set(Vector3i pos, T toT);

	/// Puts item into element at target location.
	bool Add(value_type item, Vector2i atLocation);
	/// Adds an item to the list
	bool Add(value_type item, int atX, int andY);

	/// Remove target item, searching for it.
	bool Remove(value_type item);
	/// Removes item at target index.
	bool RemoveItem(Vector2i atLocation);
	/// Removes item at target index.
	bool RemoveItem(int atX, int andY);

	/// Clears the list of all entries. Returns amount of removed item
	int Clear();
	/// Clears the list of entries and deletes every single one of them. USE WITH CAUTION and only on matrices using heap-allocated data!
	int ClearAndDelete();

	/// Swaps items at given locations.
	bool Swap(Vector2i loc1, Vector2i loc2);

	/// If wanting to save/load it, allow this.
	value_type * GetArray() { return arr; };
	/// Returns size of the matrix.
	const Vector3i & Size() const;

	/// Polls the existance/copy of target item in the queue.
	bool Exists(value_type item) const;
	/// Returns content by index. Similar to the [] operator.
	value_type & GetItem(Vector3i atLocation);

	/// Returns target element as if treating the matrix data as a long list.
	T & Element(int arrayIndex);
	/// Same as Size(), returns the number of elements in the matrix (the length of the element-array).
	int Elements();

	/// Polls the existance/copy of target item in the queue. Returns it's index if so and -1 if not.
	Vector3i GetLocationOf(value_type item) const;

/// To be inherited by other classes...
protected:

	/// Allocates and initializes values no matter dimensionality.
	void Allocate(int listSize);

	Vector3i size;
	value_type * arr;
	int currentItems;
	int arrLength;
	bool inUse;		// Defines that the list is currently being popped or pushed.
};


/// Implementation below because I'm too lazy to find how to include a separate blerargh template file in OSX.

/// Emil Hedemalm
/// 2014-07-28

#include <iostream>
#include <cassert>

/// Default constructor
template <class T>
Matrix<T>::Matrix()
{
	arr = NULL;
	arrLength = 0;
	currentItems = 0;
	inUse = false;
}

template <class T>
Matrix<T>::~Matrix()
{
	delete[] arr;
	arrLength = 0;
}

/// Sets the default value with is written to each node upon allocation.
template <class T>
void Matrix<T>::SetDefaultValue(T newDefaultValue)
{
	this->defaultValue = newDefaultValue;
}


template <class T>
void Matrix<T>::Allocate(Vector2i size)
{
	assert(size[0] && size[1]);
	if (arr)
		delete[] arr;
	arrLength = size[0] * size[1];
	// 0-size?
	if (!arrLength)
		return;
	Allocate(arrLength);
	// Set actual matrix size.
	this->size = size;
}

/// Resizing function for 3D matrices.
template <class T>
void Matrix<T>::Allocate(Vector3i size)
{
	assert(size.Length());
	if (arr) delete[] arr;
	arrLength = size[0] * size[1] * size[2];
	Allocate(arrLength);
	this->size = size;
}

/// Allocates and initializes values no matter dimensionality.
template <class T>
void Matrix<T>::Allocate(int listSize)
{
	arrLength = listSize;
	arr = new T [arrLength];
	// Write the default value over the new array if possible.
	for (int i = 0; i < arrLength; ++i)
	{
		arr[i] = defaultValue;
	}
	currentItems = 0;
}


/// Same as allocate..
template <class T>
void Matrix<T>::SetSize(Vector2i size)
{
	Allocate(size);
}

/// Loads target list as elements into the matrix.
template <class T>
bool Matrix<T>::Load(List<T> & listIntoMatrix)
{
	int matrixSize = size[0] * size[1];
	assert(listIntoMatrix.Size() == matrixSize);
	if(listIntoMatrix.Size() != matrixSize)
		return false;
	int i = 0;
	for (int y = 0; y < size[1]; ++y)
	{
		for (int x = 0; x < size[0]; ++x)
		{
			int arrIndex = y * size[0] + x;
			arr[arrIndex] = listIntoMatrix[i++];
		}
	}
	return true;
}

	
/*
/// Prints conents, along with position (x/y)
template <class T>
void Matrix<T>::PrintContents()
{
	for (int x = 0; x < size[0]; ++x)
	{
		for (int y = 0; y < size[1]; ++y)
		{
			int arrIndex = x * size[1] + y;
			std::cout<<"\nMatrix x"<<x<<" y"<<y<<": "<<arr[arrIndex];
		}
	}
}*/

/// Returns size of the matrix.
template <class T>
const Vector3i & Matrix<T>::Size() const
{
	return size;
}

/// Returns target element as if treating the matrix data as a long list.
template <class T>
T & Matrix<T>::Element(int arrayIndex)
{
	// Check that it's within bounds.
	assert(arrayIndex < arrLength);
	assert(arrayIndex >= 0);
	return arr[arrayIndex];
}

/// Same as Size(), returns the number of elements in the matrix (the length of the element-array).
template <class T>
int Matrix<T>::Elements()
{
	return arrLength;
}

/// Polls the existance/copy of target item in the queue. Returns it's index if so and -1 if not.
template <class T>
Vector3i Matrix<T>::GetLocationOf(T item) const
{
	for (int i = 0; i < arrLength; ++i)
	{
		if (arr[i] == item)
		{
			return Vector3i(i % size[0], (i / size[0]) % size[1], i / (size[0] * size[1]));
		}
	}
	return Vector3i(-1,-1,-1);
}

template <class T>
T Matrix<T>::At(Vector3i pos)
{	
	int index = pos[2] * size[0] * size[1] + pos[1] * size[0] + pos[0];
	return arr[index];
}

template <class T>
T Matrix<T>::At(int x, int y, int z)
{
	int index = z * size[0] * size[1] + y * size[0] + x;
	return arr[index];
}


template <class T>
void Matrix<T>::Set(Vector3i pos, T toT)
{
	int index = pos[2] * size[0] * size[1] + pos[1] * size[0] + pos[0];
	arr[index] = toT;
}

/// Clears the list of entries and deletes every single one of them. USE WITH CAUTION and only on matrices using heap-allocated data!
template <class T>
int Matrix<T>::ClearAndDelete()
{
	int deleted = 0;
	for (int i = 0; i < arrLength; ++i)
	{
		delete arr[i];
		arr[i] = NULL;
		++deleted;
	}
	size = Vector3i(0,0,0);
	arrLength = 0;
	return deleted;
}



#endif
