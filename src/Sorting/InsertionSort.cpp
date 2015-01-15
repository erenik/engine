/// Emil Hedemalm
/// 2014-10-24
/// Simple insertion sort.

#include "InsertionSort.h"
#include "Sortable.h"

/** List should be a filled list of objects to sort. The sorted list should be an empty list, but may be emptied by the sorting algorithm if needed as well.
	No objects are created through the sorter, except re-allocating the lists. I.e. the objects in the sorted list will be the same as in the given list.
*/
void InsertionSort::Sort(const List<Sortable*> & list, List<Sortable*> & sortedList, bool leastFirst /*= false*/)
{
	sortedList.Clear();

	bool foundAPlace, insert;
	for (int i = 0; i < list.Size(); ++i)
	{
		Sortable * s = list[i];
		bool foundAPlace = false;
		// Find proper place for it.
		for (int j = 0; j < sortedList.Size(); ++j)
		{
			Sortable * t = sortedList[j];
		//	insert = (t->sortingValue < s->sortingValue);
		//	insert ^= leastFirst;
			if ((t->sortingValue < s->sortingValue) ^ leastFirst)
			{
				sortedList.Insert(s, j);
				foundAPlace = true;
				break;
			}
		}
		if (!foundAPlace)
			sortedList.Add(s);
	}
}
