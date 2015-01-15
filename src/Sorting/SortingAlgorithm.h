/// Emil Hedemalm
/// 2014-10-24
/// Parent class for sorting algorithms that will support objects derived from the Sortable class.

#ifndef SORTING_ALGORITHM
#define SORTING_ALGORITHM

#include "List/List.h"

class Sortable;

class SortingAlgorithm 
{
public:
	/** List should be a filled list of objects to sort. The sorted list should be an empty list, but may be emptied by the sorting algorithm if needed as well.
		No objects are created through the sorter, except re-allocating the lists. I.e. the objects in the sorted list will be the same as in the given list.
	*/
	virtual void Sort(const List<Sortable*> & list, List<Sortable*> & sortedList, bool leastFirst = false) = 0;
};

#endif

