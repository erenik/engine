// Emil Hedemalm
// 2015-10-04

#include "UIColumnList.h"
#include "InputState.h"
#include "UI/UITypes.h"
#include "UI/UILists.h"
#include "Graphics/Messages/GMUI.h"
#include "GraphicsState.h"
#include "MathLib/Rect.h"
#include "Input/InputManager.h"
#include "Message/MessageManager.h"


UIColumnList::UIColumnList(String name)
: UIElement()
{
	this->name = name;
    type = UIType::COLUMN_LIST;
}

UIColumnList::~UIColumnList()
{
//	std::cout<<"\nUIColumnList destructor.";
}

/// Deletes all children and content inside.
void UIColumnList::Clear(){
	while(children.Size()){
		UIElement * c = children[0];
		children.Remove(c);
		c->FreeBuffers();
		c->DeleteGeometry();
		delete c;
		c = NULL;
	}
}

// Adjusts hierarchy besides the regular addition
bool UIColumnList::AddChild(GraphicsState* graphicsState, UIElement* child)
{
	/// Automate name if none was given.
	if (child->name.Length() < 1)
		child->name = this->name + "Element"+String::ToString(children.Size());

    /// Inherit up- and down- neighbours.
	child->InheritNeighbours(this);

	/// If first child, place it at the top.
	if (children.Size() == 0){
		UIElement::AddChild(graphicsState, child);
		child->alignmentX = 0.0f + child->sizeRatioX * 0.5f + padding;
		return true;
	}
	// If not, find bottom child (or, bottom edge.!
	// Get bottom child
	float left = 0.f;
    // Bind UIs too, so save the rightmost child within for later.
    UIElement * rightmost = NULL;
	for (int i = 0; i < children.Size(); ++i){
		UIElement * c = children[i];
		rightmost = c;
		float childLeftEdge = c->alignmentX + c->sizeRatioX * 0.5f;
		if (childLeftEdge > left)
			left = childLeftEdge;
	}
	UIElement::AddChild(graphicsState, child);
	child->alignmentX = left + child->sizeRatioX * 0.5f + padding;

    // Bind them for proper navigation.
    if (rightmost){
		rightmost->rightNeighbour = child;
        rightmost->rightNeighbourName = child->name;
		child->leftNeighbour = rightmost;
        child->leftNeighbourName = rightmost->name;
    }

	/// Check if we should add scroll-bars to zis list!
	if (child->alignmentX + child->sizeRatioX * 0.5f > 1.0f){
		// assert(false && "Implement automatic scrollbars for your columns lists, yo!");
		std::cout<<"\nImplement automatic scrollbars for your columns lists, yo!";
	}
	return true;
}

UIElement * UIColumnList::GetLeftNeighbour(UIElement * referenceElement, bool & searchChildrenOnly)
{
	// Check if the element is part of this list.
	int index = BelongsToChildIndex(referenceElement);
	// Part of us somehow?
	if (index != -1)
	{
		// Part of this list!
		if (index > 0)
		{
			// Return child one step below it.
			for (int i = index - 1; i >= 0; --i)
			{
				// Or further below as needed..
				UIElement * child = children[i];
				// Skip the scroll-bars if they react to this...
				if (child->isSysElement)
					continue;
				/// If activatable? Return it straight away.
				if (child->activateable)
					return child;
				
				// Set to only search children now!
				searchChildrenOnly = true;
				// Fetch all activatable elements in the child.
				List<UIElement*> activatables;
				child->GetElementsByFlags(UIFlag::ACTIVATABLE, activatables);

				if (!child->HasActivatables())
					continue;

				UIElement * pew = child->GetElementClosestToInY(referenceElement->position, true);
				if (pew)
					return pew;
			}
		}
	}	
	// Not part of our list? Check the most bottom element in this list and return it then!
/*	else if (children.Size())
	{
		// But first. Scroll to the bottom, to ensure that it is made visible! owo
		Scroll(-500.f);
		// Get last child which is not a system element
		return LastChild();
	}
	*/
	// No children? Do we have a valid up-element?
	UIElement * leftEle = NULL;
	// UIElement::GetLeftNeighbour(referenceElement, searchChildrenOnly);
	/// If not, check parent?, always go up until told otherwise!
	if (!leftEle)
	{
		searchChildrenOnly = false;
		leftEle = parent->UIElement::GetLeftNeighbour(referenceElement, searchChildrenOnly);
	}
	if (leftEle)
		return leftEle;

	// No good found? Return the caller element so the cursor stays in place.
	return referenceElement;
}

UIElement * UIColumnList::GetRightNeighbour(UIElement * referenceElement, bool & searchChildrenOnly)
{
	// Check if the element is part of this list.
	UIElement * search = referenceElement;
	// Check if the element is part of this list.
	int index = BelongsToChildIndex(referenceElement);
	// Part of us somehow?
	if (index != -1)
	{
		// Part of this list!
		if (index < children.Size() - 1)
		{
			// Return child one step below it.
			for (int i = index + 1; i < children.Size(); ++i)
			{
				// Or further below as needed..
				UIElement * child = children[i];
				// Skip the scroll-bars if they react to this...
				if (child->isSysElement)
					continue;
				/// If activatable? Return it straight away.
				if (child->activateable)
					return child;
				// Set to only search children now!
				searchChildrenOnly = true;
				UIElement * neighbour = child->GetElementClosestToInY(referenceElement->position, true);
		//		if (neighbour == child)
		//			continue;		
//					child->GetDownNeighbour(referenceElement, searchChildrenOnly);
				if (neighbour)
					return neighbour;
			}
		}
	}
	// No children? Do we have a valid up-element?
	UIElement * rightEle = NULL;
	if (!rightEle)
	{
		searchChildrenOnly = false;
		rightEle = parent->UIElement::GetRightNeighbour(referenceElement, searchChildrenOnly);
	}
	if (rightEle)
		return rightEle;

	// No good found? Return the caller element so the cursor stays in place.
	return referenceElement;
}
