/// Emil Hedemalm
/// 2014-04-08
/// General item class. Subclass to add further data.

#ifndef ITEM_H
#define ITEM_H

class Item 
{

	String name;
	/// Type of item, used primarily for sorting. Default is 0.
	int type;
	/// Sub-type of item.
	int subType;
};

#endif
