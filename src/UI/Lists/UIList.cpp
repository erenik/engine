// Emil Hedemalm
// 2013-07-10

#include "InputState.h"
#include "UI/UITypes.h"
#include "UIList.h"
#include "Graphics/Messages/GMUI.h"
#include "GraphicsState.h"
#include "MathLib/Rect.h"
#include "Input/InputManager.h"
#include "Message/MessageManager.h"


UIList::UIList()
: UIElement()
{
    type = UIType::LIST;
    formatX = true;
    scrollBarX = scrollBarY = NULL;
	contentsSize = 0.0f;
	/// Should be hoverable so mouse interactions work.
	hoverable = true;
	/// But disable highlight on hover!
	highlightOnHover = false;
	// Clear initial stuff! As no elements should be present this should work.
	Clear();

	// Default true.
	createScrollBarsAutomatically = true;
 
}

UIList::~UIList()
{
//	std::cout<<"\nUIList destructor.";
}

/// Deletes all children and content inside.
void UIList::Clear()
{
	// Delete everything. Including side-bars!
    for (int i = 0; i < children.Size(); /* Empty */){
    	UIElement * c = children[i];
		/*
		if (c->isSysElement){
    	    ++i;
            continue;
        }
		*/
		children.Remove(c);
		if (c->vboBuffer)
            c->FreeBuffers();
        if (c->mesh)
            c->DeleteGeometry();
		delete c;
		c = NULL;
    }
	// Reset contents size and stuff too!
	contentsSize = 0.0f;
	// Reset pointers o.o
    scrollBarX = scrollBarY = NULL;
}

// Adjusts hierarchy besides the regular addition
bool UIList::AddChild(UIElement* child)
{
	/// Automate name if none was given.
	if (child->name.Length() < 1)
		child->name = this->name + "Element"+String::ToString(children.Size());

    /// Inherit neighbours.
	child->InheritNeighbours(this);

 //   std::cout<<"\nAdding child "<<child->name<<" text: "<<child->text;
	/// If first child, place it at the top.
	if (children.Size() == 0){
		UIElement::AddChild(child);
		child->alignmentY = 1.0f - child->sizeRatioY * 0.5f;
		return true;
	}
	// If not, find bottom child (or, bottom edge.!
	// Get bottom child
	float bottom = 1.0f;
	UIElement * bottomElement = NULL;
	for (int i = 0; i < children.Size(); ++i){
		UIElement * c = children[i];
		/// Skip 'system' elements like scroll-bars and handles that will be deleted shortly!
		if (c->isSysElement)
            continue;
		bottomElement = c;
	//	std::cout<<"\nBottomElement name: "<<bottomElement->name;
		float childBottomEdge = c->alignmentY - c->sizeRatioY * 0.5f;
		if (childBottomEdge < bottom)
			bottom = childBottomEdge;
	}
	UIElement::AddChild(child);
	child->alignmentY = bottom - child->sizeRatioY * 0.5f - padding;

	// Update total contents size.
	contentsSize = 1 - child->alignmentY + child->sizeRatioY * 0.5f;

	/// Check if we should add scroll-bars to zis list!
	bool needScrollBarY = child->alignmentY - child->sizeRatioY * 0.5f < 0;
	needScrollBarY &= createScrollBarsAutomatically;
	if (needScrollBarY)
	{
	    if (scrollBarY)
		{
	        children.Remove(scrollBarY);
	   //     std::cout<<"\nTrying to delete stuff?";
			if (scrollBarY->vboBuffer != -1)
				scrollBarY->FreeBuffers();
	        if (scrollBarY->mesh)
                scrollBarY->DeleteGeometry();
			UIElement * scrollBar = scrollBarY;
			scrollBarY = NULL;
			delete scrollBar;
	      //  scrollBarY->Update(bottom + child->sizeRatioY);
          //  scrollBarY->visible = true;
          //  FormatElements();
          //  return;
        }
        UIScrollBar * scroll = new UIScrollBar();
        scroll->CreateHandle();
        scroll->Update(1.0f - (bottom - child->sizeRatioY));
        scroll->sizeRatioX = 0.1f;
        scroll->alignmentX = 0.95f;
        scroll->textureSource = "img/80Gray50Alpha.png";
   //     scroll->text = "Neeeeej";
        scrollBarY = scroll;
        UIElement::AddChild(scrollBarY);
        FormatElements();
	//	assert(false && "Implement automatic scrollbars for your lists, yo!");
	}

	// Bind them for proper navigation.
    if (bottomElement)
	{
	//	std::cout<<"\nBottomElement name: "<<bottomElement;

		// Let navigation use the UIList to determine who should be the up/down neighbour. Don't assign that here!
   //     bottomElement->downNeighbourName = child->name;
	//	child->upNeighbourName = bottomElement->name;
    }
	
	// If adding a child with hover state, assume it should be in focus.
	if (child->state & UIState::HOVER)
	{
		child->EnsureVisibility();
	}

	return true;
}

#define RETURN_IF_OUTSIDE {if (mouseY > top || mouseY < bottom || \
                               mouseX > right || mouseX < left) return NULL;};

/// Activation functions
UIElement * UIList::Hover(int mouseX, int mouseY)
{
	RETURN_IF_OUTSIDE
    float listX = (float)mouseX;
    float listY = (float)mouseY;
	if (onHover.Length())
		MesMan.QueueMessages(onHover);
    if (scrollBarY){
        listY -= scrollBarY->GetStart() * sizeY;
    }
    UIElement * e = NULL;
    /// Check le children.
    for (int i = 0; i < children.Size(); ++i){
        UIElement * child = children[i];
        if (child->isSysElement)
            e = child->Hover(mouseX, mouseY);
        else
            e = child->Hover(RoundInt(listX), RoundInt(listY));
        if (e)
            break;
    }
	// Us? o.o;
    if (e == NULL)
	{
        e = this;
		if (!AddState(UIState::HOVER))
			return NULL;
	}
	else 
	{
		this->RemoveState(UIState::HOVER);
	}
    if (scrollBarY){
//        std::cout<<"\nUIList::Hover "<< (e ? e->name : "NULL") <<" listY: "<<listY<<" mouseY: "<<mouseY;
    }
    return e;
}
UIElement * UIList::Click(int mouseX, int mouseY)
{
	state &= ~UIState::ACTIVE;
    RETURN_IF_OUTSIDE
    float listX = (float)mouseX;
    float listY = (float)mouseY;
    if (scrollBarY)
        listY -= scrollBarY->GetStart() * sizeY;

    UIElement * e = NULL;
    /// Check le children.
    for (int i = 0; i < children.Size(); ++i){
        UIElement * child = children[i];
        if (child->isSysElement)
            e = child->Click(mouseX, mouseY);
        else
            e = child->Click(RoundInt(listX), RoundInt(listY));
        if (e)
            break;
    }
	if (!e)
	{
        e = this;
		// If activatable, flag it.
		if (this->activateable){
			state |= UIState::ACTIVE;
			return this;
		}
	}
    if (scrollBarY){
        std::cout<<"\nUIList::Click "<< (e ? e->name : "NULL") <<" listY: "<<listY<<" mouseY: "<<mouseY;
    }
    return e;
}
/// GEtttererrr
UIElement * UIList::GetElement(int mouseX, int mouseY){
    RETURN_IF_OUTSIDE
    float listX = (float)mouseX;
    float listY = (float)mouseY;
    if (scrollBarY)
        listY -= scrollBarY->GetStart() * sizeY;
    UIElement * e = this;
    /// Check le children.
    for (int i = 0; i < children.Size(); ++i){
        UIElement * child = children[i];
        if (child->isSysElement)
            e = child->Click(mouseX, mouseY);
        else
            e = child->GetElement(RoundInt(listX), RoundInt(listY));
        if (e)
            break;
    }
    if (!e)
        e = this;
    if (scrollBarY){
        std::cout<<"\nUIList::GetElement "<< (e ? e->name : "NULL") <<" listY: "<<listY<<" mouseY: "<<mouseY;
    }
    return e;
}


/// Scroll ze listur!
bool UIList::OnScroll(float delta)
{
	float moved = 0;
	bool thisOrParentMoved = false;
    /// Move the slider and adjust content.
    if (scrollBarY)
	{
        float pageSize = scrollBarY->handle->sizeRatioY;
        bool negative = delta < 0;
        float distance = AbsoluteValue(delta);
        float pixels = distance * sizeY;
        if (pixels < 5){
            distance = 5.0f / sizeY;
        }
        float quarterPage = pageSize * 0.25f;
        std::cout<<"\nQuarterPage: "<<quarterPage;
        if (distance > quarterPage){
            distance = quarterPage;
        }
		/// Fix sign.
        if (negative)
            distance *= -1.0f;
        distance = pageSize * delta;
        moved += scrollBarY->Move(distance);
    //    scrollBarY->PrintDebug();
    }
	if (moved)
	{
		thisOrParentMoved = true;
	}
    float moveRemaining = delta - moved;
	if (moveRemaining == delta)
		thisOrParentMoved |= UIElement::OnScroll(moveRemaining) != 0;
	else {
		// Did all the moving here.
		std::cout<<"\ndid all the moving.";
	}
	// If no movement was done, and a child is selected in this list, go to the top or bottom child instead.
	UIElement * hoverElement = GetElementByState(UIState::HOVER);
	if (!thisOrParentMoved && hoverElement)
	{
		hoverElement->RemoveState(UIState::HOVER);
		if (delta > 0)
		{
			children[0]->Hover();
		}
		else {
			LastChild()->Hover();
		}
	}
	return thisOrParentMoved;
}

/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour, and should be NULL for the initial call.
UIElement * UIList::GetUpNeighbour(UIElement * referenceElement, bool & searchChildrenOnly)
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
				UIElement * neighbour = child->GetUpNeighbour(referenceElement, searchChildrenOnly);
			//	if (neighbour == child)
			//		continue;
				if (neighbour)
					return neighbour;
			}
		}
	}	
	// Not part of our list? Check the most bottom element in this list and return it then!
	else if (children.Size())
	{
		// But first. Scroll to the bottom, to ensure that it is made visible! owo
		Scroll(-500.f);
		// Get last child which is not a system element
		return LastChild();
	}
	// No children? Do we have a valid up-element?
	UIElement * upEle = UIElement::GetUpNeighbour(referenceElement, searchChildrenOnly);
	if (upEle)
		return upEle;

	// No good found? Return the caller element so the cursor stays in place.
	return referenceElement;
}

/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour, and should be NULL for the initial call.
UIElement * UIList::GetDownNeighbour(UIElement * referenceElement, bool & searchChildrenOnly)
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
				UIElement * neighbour = child->GetElementClosestTo(referenceElement->position, true);
		//		if (neighbour == child)
		//			continue;		
//					child->GetDownNeighbour(referenceElement, searchChildrenOnly);
				if (neighbour)
					return neighbour;
			}
		}
	}
	// Not part of our list? Check the most top-most element in this list and return it then!
	else if (children.Size())
	{
		return children[0];
	}
	// No children? Do we have a valid up-element?
	UIElement * downEle = UIElement::GetDownNeighbour(referenceElement, searchChildrenOnly);
	if (downEle)
		return downEle;

	// No good found? Return the caller element so the cursor stays in place.
	return referenceElement;
}

UIElement * UIList::GetLeftNeighbour(UIElement * referenceElement, bool & searchChildrenOnly)
{
	// Grab best on in Y- that corresponds to it?
	if (searchChildrenOnly)
	{
		UIElement * child = this->GetElementClosestTo(referenceElement->position, true);
		return child;
	}
	/// If not going down, continue up til a ColumnList is hit?
	return parent->GetLeftNeighbour(referenceElement, searchChildrenOnly);
}
UIElement * UIList::GetRightNeighbour(UIElement * referenceElement, bool & searchChildrenOnly)
{
	// Grab best on in Y- that corresponds to it?
	if (searchChildrenOnly)
	{
		UIElement * child = this->GetElementClosestTo(referenceElement->position, true);
		return child;	
	}
	/// If not going down, continue up til a ColumnList is hit?
	return parent->GetRightNeighbour(referenceElement, searchChildrenOnly);
}


// Get last child which is not a system element
UIElement * UIList::LastChild()
{
	for (int i = children.Size() - 1; i >= 0; --i)
	{
		UIElement * child = children[i];
		if (child->isSysElement)
			continue;
		return child;
	}
	return NULL;
}

/// Returns the current scroll position.
float UIList::GetScrollPosition()
{
	if (scrollBarY)
		return scrollBarY->GetScrollPosition();
	return 0.f;
}

/// Set current scroll position.
void UIList::SetScrollPosition(float fValue)
{
	if (scrollBarY)
		scrollBarY->SetScrollPosition(fValue);
	return;
}



/// Scroll, not capped.
bool UIList::Scroll(float absoluteDistanceInPages)
{
    /// Move the slider and adjust content.
    if (scrollBarY)
	{
        float pageSize = scrollBarY->handle->sizeRatioY;
        float distance = absoluteDistanceInPages * pageSize;
        scrollBarY->Move(distance);
    //    scrollBarY->PrintDebug();
    }
    return true;
}

/// Adjusts positions and sizes acording to any attached scroll-bars or other system UI elements.
void UIList::FormatElements(){
    if (!formatX)
        return;
    for (int i = 0; i < children.Size(); ++i){
        UIElement * e = children[i];
        if (e->isSysElement)
            continue;
        if (scrollBarY){
            if (e->sizeRatioX > 1.0f - scrollBarY->sizeRatioX){
                float newSizeRatioX = 1.0f - scrollBarY->sizeRatioX;
                e->alignmentX = e->alignmentX + (newSizeRatioX - e->sizeRatioX) * 0.5f;
                e->sizeRatioX = newSizeRatioX;
            }
        }
		/// Re-queue bufferization of all elements that are formated!
		e->isBuffered = false;
    }
}

/// Rendering
void UIList::Render(GraphicsState & graphicsState)
{
    // Render ourself and maybe children.
    RenderSelf(graphicsState);
    if (children.Size())
        RenderChildren(graphicsState);
}

void UIList::RenderChildren(GraphicsState & graphicsState)
{
	Matrix4d modelMatrix = graphicsState.modelMatrixD;
	Matrix4d translatedModelMatrix = graphicsState.modelMatrixD;

	Vector4d initialPositionTopRight(right, top, 0, 1), 
			initialPositionBottomLeft(left, bottom, 0, 1);
	Vector3f currTopRight = graphicsState.modelMatrixF * initialPositionTopRight;
	Vector3f currBottomLeft = graphicsState.modelMatrixF * initialPositionBottomLeft;

	Rect previousScissor = graphicsState.scissor;
	Rect uiRect(currBottomLeft[0], currBottomLeft[1], currTopRight[0], currTopRight[1]);
	Rect uiScissor = previousScissor.Intersection(uiRect);
	
	// Set scissor! o.o
	graphicsState.SetGLScissor(uiScissor);
   

    /// If we got a scrollbar, apply a model matrix to make us render at the proper location.
    if (scrollBarY && scrollBarY->visible)
	{
        float pageBeginY = scrollBarY->GetStart();
        pageBeginY *= sizeY;
        translatedModelMatrix *= Matrix4d::Translation(0, pageBeginY, 0);
		this->pageBegin[1] = pageBeginY;
    }

    /// Render all children
	for (int i = 0; i < children.Size(); ++i)
	{
        UIElement * child = children[i];
        if (!child->visible)
            continue;

        /// Set model matrix depending on child-type.
        if (child->isSysElement){
        //    std::cout<<"\nChild: "<<child->name<<" is sys element.";
            graphicsState.modelMatrixF = graphicsState.modelMatrixD = modelMatrix;
        }
        else
            graphicsState.modelMatrixF = graphicsState.modelMatrixD = translatedModelMatrix;

		// Render
        child->Render(graphicsState);
        
		// Reset scissor in-case child-elements were manipulatng it.
		graphicsState.SetGLScissor(uiScissor);
	}
	// Reset scissor to how it was before.
	graphicsState.SetGLScissor(previousScissor);
}


/// Called to ensure visibility of target element.
void UIList::EnsureVisibility(UIElement * element)
{
	if (!element)
		return;
	// Check if we need to do anything at all (if small list)
	if (contentsSize <= 1.01f)
		return;

	// Do stuff! o.o
	float alignmentY = 1 - element->alignmentY;
//	std::cout<<"\nAlignmentY: "<<alignmentY;
	float halfSizeY = element->sizeRatioY * 0.5f;
	float top = alignmentY - halfSizeY;
	float bottom = alignmentY + halfSizeY;

	// Extract positions for this element.
	float pageTop = this->scrollBarY->GetStart();
	float pageStop = this->scrollBarY->GetStop();

	// If elements top exceeding current viewable top, scroll up.
	if (top <= pageTop){
		float dist = pageTop - top;
		this->Scroll(dist);
	//	std::cout<<"\nScrolling up.";
	}
	// And if element's bottom exceeding current viewable bottom, scroll down..!
	else if (bottom >= pageStop){
		float dist = pageStop - bottom;
		this->Scroll(dist);
//		std::cout<<"\nScrolling down.";
	}

//	float


	// Compare with scroll-bar position.

	//this->Scroll(-0.1f);
}


