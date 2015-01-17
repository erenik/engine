/// Emil Hedemalm
/// 2015-01-17
/// 

#include "List.h"

#define LIST_VERSION 0

/// Used to write lists of pointer-classes using standard WriteTo, ReadFrom functions.
template<class T>
bool WriteListTo(const List<T * > & list, std::fstream & file)
{
	file.write((char*) &list.Size(), sizeof(int));
	for (int i = 0; i < list.Size(); ++i)
	{
		T * item = list[i];
		item->WriteTo(file);
	}
	return true;	
}

/** Used to write lists of pointer-classes using standard WriteTo, ReadFrom functions. 
	Creates the items and deletes the contents of the list as necessary.
*/
template<class T>
bool ReadListFrom(List<T *> & list, std::fstream & file)
{
	// Delete all items before loadnag new stuff into it.
	list.ClearAndDelete();
	int listSize;
	file.read((char*) &listSize, sizeof(int));
	for (int i = 0; i < listSize; ++i)
	{
		T * item = new T();
		item->ReadFrom(file);
		list.Add(item);
	}
	return true;	
}
