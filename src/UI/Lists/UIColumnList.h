// Emil Hedemalm
// 2015-10-04

#ifndef UI_COLUMN_LIST_H
#define UI_COLUMN_LIST_H

#include "UI/UIElement.h"

/** The UIColumnList class acts in such a way that it formats it's internals content after demand, by repositioning them as needed.
	Works similar to UIList but side-ways instead ^w^
*/
class UIColumnList : public UIElement 
{
public:
	UIColumnList(String name = "");
	virtual ~UIColumnList();
	/// Deletes all children and content inside.
	void Clear();
	// Adjusts hierarchy besides the regular addition
	virtual bool AddChild(UIElement* child); // Sets child pointer to child UI element, NULL if non

	/** Suggests a neighbour which could be to the right of this element. 
		Meant to be used for UI-navigation support. The reference element 
		indicates the element to which we are seeking a compatible or optimum neighbour, 
		and should be NULL for the initial call.

		If searchChildrenOnly is true, the call should not recurse to any parents. 
		This is set by special classes such as UIList and UIColumnList when they know
		a certain element should be or contain the correct neighbour element.
	*/
	virtual UIElement * GetLeftNeighbour(UIElement * referenceElement, bool & searchChildrenOnly);
	virtual UIElement * GetRightNeighbour(UIElement * referenceElement, bool & searchChildrenOnly);
};

#endif
