/// Emil Hedemalm
/// 2014-10-24
/// Parent class for all objects which want to be able to be sorted by the SortingAlgorithm-derived sorting class functions

#ifndef SORTABLE_H
#define SORTABLE_H

class Sortable 
{
public:
	/// Value used when sorting. Should be set for each object before a sorting algorithm is called to sort them!
	int sortingValue;
};

#endif