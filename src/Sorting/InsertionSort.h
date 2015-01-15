/// Emil Hedemalm
/// 2014-10-24
/// Simple insertion sort.

#ifndef INSERTION_SORT_H
#define INSERTION_SORT_H

#include "SortingAlgorithm.h"

class InsertionSort : public SortingAlgorithm
{
public:
	/** List should be a filled list of objects to sort. The sorted list should be an empty list, but may be emptied by the sorting algorithm if needed as well.
		No objects are created through the sorter, except re-allocating the lists. I.e. the objects in the sorted list will be the same as in the given list.
	*/
	virtual void Sort(const List<Sortable*> & list, List<Sortable*> & sortedList, bool leastFirst = false);
};

#endif