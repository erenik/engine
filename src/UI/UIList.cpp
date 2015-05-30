// Emil Hedemalm
// 2013-07-10

#include "InputState.h"
#include "UITypes.h"
#include "UIList.h"
#include "Graphics/Messages/GMUI.h"
#include "GraphicsState.h"
#include "MathLib/Rect.h"
#include "Input/InputManager.h"
#include "Message/MessageManager.h"

UIScrollBarHandle::UIScrollBarHandle()
: UIElement()
{
    type = UIType::SCROLL_HANDLE;
    name = "Scroll bar handle";
    selectable = true;
    hoverable = true;
    activateable = true;
    isSysElement = true;
	highlightOnHover = true;
    activationMessage = "BeginScroll(this)";
}

UIScrollBarHandle::~UIScrollBarHandle()
{
//	std::cout<<"\nUIScrollBarHandle destructor.";
}

float UIScrollBarHandle::Move(float distance)
{
	float alignmentPre = alignmentY;
    alignmentY += distance;
	SetScrollPosition(alignmentY);
	float moved = alignmentY - alignmentPre;
	return moved;
}

float UIScrollBarHandle::GetScrollPosition()
{
	return alignmentY;
}

void UIScrollBarHandle::SetScrollPosition(float f)
{
    alignmentY = f;
    if (alignmentY > 1.0f - sizeRatioY * 0.5f)
        alignmentY = 1.0f - sizeRatioY * 0.5f;
    else if (alignmentY < 0.0f + sizeRatioY * 0.5f)
        alignmentY = 0.0f + sizeRatioY * 0.5f;
	
	// Recalculate position of the element to render correctly.
//	this->QueueBuffering();
	this->isBuffered = false;
}



/// Activation functions
UIElement * UIScrollBarHandle::Hover(int mouseX, int mouseY)
{
	UIElement * element = UIElement::Hover(mouseX, mouseY);
	/* Moved to be handled within OnMouseMove
	if (element == this && Input.lButtonDown)
	{
		float alignmentY = (mouseY - parent->bottom) / (float)this->parent->sizeY;
		this->SetAlignmentY(alignmentY);
	}
	*/
	return element;
}
	

// Returns true once the highest-level appropriate element has been found.
// No co-ordinates are required since we will instead require the element to already
// be highlighted/hovered above.
UIElement * UIScrollBarHandle::Click(int mouseX, int mouseY)
{
	UIElement * clickElement = UIElement::Click(mouseX, mouseY);
	if (clickElement == this)
	{
		float alignmentY = (mouseY - parent->bottom) / (float)this->parent->sizeY;
		this->SetAlignmentY(alignmentY);
	}
	return clickElement;
}

/// Used by e.g. ScrollBarHandle's in order to slide its content according to mouse movement, even when the mouse extends beyond the scope of the element.
void UIScrollBarHandle::OnMouseMove(Vector2i activeWindowCoords)
{
	float alignmentY = (activeWindowCoords[1]  - parent->bottom) / (float) this->parent->sizeY;
	SetAlignmentY(alignmentY);
}

// Setting
void UIScrollBarHandle::SetAlignmentY(float y)
{
	float halfSizeY = this->sizeRatioY * 0.5f;
	if (y > 1.f - halfSizeY)
		y = 1.f - halfSizeY;
	if (y < 0.f + halfSizeY)
		y = 0.f + halfSizeY;
	
	alignmentY = y;
	this->isBuffered = false;
}

UIScrollBar::UIScrollBar()
: UIElement()
{
    type = UIType::SCROLL_BAR;
    name = "Scroll bar";
    handle = NULL;

    selectable = true;
    hoverable = true;
    activateable = true;
    isSysElement = true;
	highlightOnHover = false;
	highlightOnActive = false;

    activationMessage = "BeginScroll(this)";
}

UIScrollBar::~UIScrollBar()
{
//	std::cout<<"\nUIScrollBar destructor.";
}

void UIScrollBar::CreateHandle()
{
    assert(handle == NULL);
    handle = new UIScrollBarHandle();
    handle->textureSource = "Gray";
 //   handle->text = "Joooooo";
    AddChild(handle);
    previousSize = 1.0f;
}

void UIScrollBar::OnMouseY(int y)
{
	float pageSize = this->PageSize();
	float halfPageSize = pageSize * 0.5f;
	float pos = (y + halfPageSize)/ (float)this->sizeY;
// Ensure.. within bounds?
	handle->SetAlignmentY(pos);
}

/// Activation functions
UIElement * UIScrollBar::Hover(int mouseX, int mouseY)
{
	// OnActivate()?
	// Update scroll position!
	UIElement * element = UIElement::Hover(mouseX, mouseY);
	if (element == this && inputState->lButtonDown)
		OnMouseY(mouseY - bottom);
	return element;
}


// Returns true once the highest-level appropriate element has been found.
// No co-ordinates are required since we will instead require the element to already
// be highlighted/hovered above.
UIElement * UIScrollBar::Click(int mouseX, int mouseY)
{
	UIElement * result = 0;
	// Don't process invisible UIElements, please.
	if (visible == false)
		return false;

	// Check if the mouse is outside the element's boundaries.
	if (mouseX > right || mouseX < left ||
		mouseY > top || mouseY < bottom){
			// Return false if we are outside of the boundaries,
			// since we haven't found the selected element.
			state = UIState::IDLE;
			//	if(child != NULL)
			return NULL;
	}

	// Check axiomaticness (direct-activation without further processing)
	if (axiomatic){
		if (activateable){
			state |= UIState::ACTIVE;
			return this;
		}
		return NULL;
	}

	// OnActivate()?
	// Update scroll position!
	OnMouseY(mouseY - bottom);


	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = children.Size()-1; i >= 0; --i){
		UIElement * child = children[i];
	    if (!child->visible)
            continue;
		result = child->Click(mouseX, mouseY);
		if (result != NULL){
			// The active element has been found further down the tree,
			// so we can return true.
			state = UIState::IDLE;
			return result;
		}
	}

	
	// Check the element's StateMan. If it is hovered over, we've found it.
	if (this->activateable && state & UIState::HOVER){
		state |= UIState::ACTIVE;
		return this;
	}
	// If not, return false, since we haven't foun the right element.
	return NULL;
}


void UIScrollBar::Update(float newSize)
{
//    std::cout<<"\nUIScrollBar::Update New size: "<<newSize;
    float newPreviousSize = newSize;
    /// Adjust the handle's size and position.
    if (newSize <= 1.0f){
        handle->sizeRatioY = 1.0f;
        handle->alignmentY = 0.5f;
        visible = false;
    }
    else {
        float top = handle->alignmentY + handle->sizeRatioY * 0.5f;
        handle->sizeRatioY = 1.0f / newSize;
     //   std::cout<<"\nHandle sizeRatioY: "<<handle->sizeRatioY;
        handle->alignmentY = top - handle->sizeRatioY * 0.5f;
     //   std::cout<<"\nHandle alignmentY: "<<handle->alignmentY;
        visible = true;
    }
    previousSize = newPreviousSize;
}

float UIScrollBar::Move(float distance)
{
    return handle->Move(distance);
}

/// Returns current scroll position, based on the handle.
float UIScrollBar::GetScrollPosition()
{
	return handle->GetScrollPosition();
}

void UIScrollBar::SetScrollPosition(float f)
{
	handle->SetScrollPosition(f);
}


/// Returns the current relative start of the contents
float UIScrollBar::GetStart()
{
    float pageSize = handle->sizeRatioY;
    float handleStart = handle->alignmentY + handle->sizeRatioY * 0.5f;
    /// Divide by the size ratio to get how many pages down we are!
    float start = (1.0f - handleStart) / handle->sizeRatioY;
/*    std::cout<<"\nHandle alignmentY: "<<handle->alignmentY<<" sizeRatioY: "<<handle->sizeRatioY;
    std::cout<<"\nPage size: "<<pageSize;
    std::cout<<"\nHandle start: "<<handleStart;
    std::cout<<"\nStart: "<<start;
    */
    return start;
}
/// Returns the current relative start of the contents
float UIScrollBar::GetStop(){
    float contentSize = 1.0f / handle->sizeRatioY;
    float handleStop = handle->alignmentY - handle->sizeRatioY * 0.5f;
    float stop = (1.0f - handleStop) * contentSize;
    return stop;
}

/// Returns size of the scroll bar handle, equivalent to how big a page is relative to the total contents.
float UIScrollBar::PageSize(){
	if (!handle)
		return 1.0f;
	return this->handle->sizeRatioY;
}

void UIScrollBar::PrintDebug()
{
    float pageSize = handle->sizeRatioY;
    float handleStart = handle->alignmentY + handle->sizeRatioY * 0.5f;
    float start = (1.0f - handleStart) * pageSize;

    std::cout<<"\nHandle alignmentY: "<<handle->alignmentY<<" sizeRatioY: "<<handle->sizeRatioY;
    std::cout<<"\nPage size: "<<pageSize;
    std::cout<<"\nHandle start: "<<handleStart;
    std::cout<<"\nStart: "<<start;
}


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
bool UIColumnList::AddChild(UIElement* child)
{
	/// Automate name if none was given.
	if (child->name.Length() < 1)
		child->name = this->name + "Element"+String::ToString(children.Size());

    /// Inherit up- and down- neighbours.
	child->InheritNeighbours(this);

	/// If first child, place it at the top.
	if (children.Size() == 0){
		UIElement::AddChild(child);
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
	UIElement::AddChild(child);
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
