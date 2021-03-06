// Emil Hedemalm
// 2013-07-10

#include "UIList.h"

#include "UIScrollBar.h"
#include "UIScrollBarHandle.h"
#include "InputState.h"
#include "UI/UITypes.h"
#include "Graphics/Messages/GMUI.h"
#include "GraphicsState.h"
#include "MathLib/Rect.h"
#include "Input/InputManager.h"
#include "Message/MessageManager.h"


UIList::UIList(String name)
: UIElement(name)
{
    type = UIType::LIST;
    formatX = true;
    scrollBarX = scrollBarY = NULL;
	contentsSize = 0.0f;
	/// Should be hoverable so mouse interactions work.
	interaction.hoverable = true;
	/// But disable highlight on hover!
	visuals.highlightOnHover = false;

	// Default true.
	createScrollBarsAutomatically = true;
 
}

UIList::~UIList()
{
//	std::cout<<"\nUIList destructor.";
}


int UIList::CenteredContentParentPosX() const {
	if (scrollBarY && scrollBarY->interaction.visible)
		return layout.posX - layout.sizeX * scrollBarY->layout.sizeRatioX * 0.5f;
	return layout.posX;
}

// If parent is e.g. List, available size will vary depending on if scrollbar is present or not.
int UIList::AvailableParentSizeX() const {
	if (scrollBarY && scrollBarY->interaction.visible)
		return layout.sizeX - layout.sizeX * scrollBarY->layout.sizeRatioX;
	return layout.sizeX;
}

void UIList::RescaleChildrenY(float f)
{
	for (int i = 0; i < children.Size(); ++i)
	{
		UIElement * c = children[i];
		if (c->isSysElement)
			continue;
		c->layout.sizeRatioY = f;
		c->Bufferize();
	}
}

/// Deletes all children and content inside.
void UIList::Clear(GraphicsState& graphicsState)
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
        c->visuals.FreeBuffers(graphicsState);
        if (c->visuals.mesh)
            c->DeleteGeometry();
		delete c;
		c = NULL;
    }
	contentsSize = 0.0f;
    scrollBarX = scrollBarY = NULL;
}

void UIList::CreateScrollBarIfNeeded(GraphicsState* graphicsState)
{
	if (!scrollBarY)
	{
		float scrollBarWidth = 0.05;
		UIScrollBar * scroll = new UIScrollBar(name);
		scroll->parent = this;
		scroll->layout.sizeRatioX = scrollBarWidth;
		scroll->CreateHandle();
		scroll->Update(graphicsState, 1.0f - contentChildren.Last()->layout.posY);
		scroll->layout.alignmentX = 0.975f;
	    scrollBarY = scroll;
	    UIElement::AddChild(nullptr, scrollBarY);

		scroll->AdjustToParent(graphicsState);
		scroll->CreateGeometry(graphicsState);
		scroll->Bufferize();
	}
}

/** For some UI elements, such as lists, the list itself is not navigatable, so query it to get the first element in its list if so here.
		By default returns itself. */
UIElement * UIList::GetNavigationElement(NavigateDirection direction) {
	if (direction == NavigateDirection::Down)
	{
		return children[0];
	}
	else if (direction == NavigateDirection::Up) {
		return children[children.Size() - 1];
	}
	else
		return children[0];
}

// Adjusts hierarchy besides the regular addition
bool UIList::AddChild(GraphicsState * graphicsState, UIElement* child)
{
	/// Automate name if none was given.
	if (child->name.Length() < 1)
		child->name = this->name + "Element"+String::ToString(children.Size());

	contentChildren.AddItem(child);
    /// Inherit neighbours.
	child->InheritNeighbours(this);

	// Set it's parent to this.
	child->parent = this;
	child->ui = ui;

 //   std::cout<<"\nAdding child "<<child->name<<" text: "<<child->text;
	/// If first child, place it at the top.
	if (children.Size() == 0){
		UIElement::AddChild(nullptr, child);
		child->layout.alignmentY = 1.0f - child->layout.sizeRatioY * 0.5f;
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
		float childBottomEdge = c->layout.alignmentY - c->layout.sizeRatioY * 0.5f;
		if (childBottomEdge < bottom)
			bottom = childBottomEdge;
	}
	UIElement::AddChild(nullptr, child);
	child->layout.alignmentY = bottom - child->layout.sizeRatioY * 0.5f - layout.padding;

	/// Update bottom after this addition.
	bottom -= child->layout.sizeRatioY;

	// Update total contents size.
	contentsSize = 1 - bottom;

	/// Check if we should add scroll-bars to zis list!
	bool needScrollBarY = child->layout.alignmentY - child->layout.sizeRatioY * 0.5f < 0;
	needScrollBarY &= createScrollBarsAutomatically;
	if (needScrollBarY)
	{
		// Create if needed.
		CreateScrollBarIfNeeded(graphicsState);
		/// Update scroll-bar.
		UIElement * lastChild = contentChildren.Last();
		scrollBarY->Update(graphicsState, 1.0f - bottom);
   //     scroll->text = "Neeeeej";
        FormatElements(*graphicsState);
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
		child->EnsureVisibility(graphicsState);
	}

	return true;
}

#define RETURN_IF_OUTSIDE {if (mouseY > layout.top || mouseY < layout.bottom || \
                               mouseX > layout.right || mouseX < layout.left) return NULL;};

/// Activation functions
UIElement * UIList::Hover(GraphicsState* graphicsState, int mouseX, int mouseY)
{
	RETURN_IF_OUTSIDE
    float listX = (float)mouseX;
    float listY = (float)mouseY;
	if (onHover.Length())
		MesMan.QueueMessages(onHover);
    if (scrollBarY){
        listY -= scrollBarY->GetStart() * layout.sizeY;
    }
    UIElement * e = NULL;

	// Remove hover state first?
	this->RemoveState(UIState::HOVER);

    /// Check le children.
    for (int i = 0; i < children.Size(); ++i){
        UIElement * child = children[i];
        if (child->isSysElement)
            e = child->Hover(graphicsState, mouseX, mouseY);
        else
            e = child->Hover(graphicsState, RoundInt(listX), RoundInt(listY));
        if (e)
            break;
    }
	// Us? o.o;
    if (e == NULL)
	{
        e = this;
		if (!AddState(graphicsState, UIState::HOVER))
			return NULL;
	}
	// Odd-case, with lists acting as buttons, if it can be highlit and sub elements are not highlightable.
	else if (visuals.highlightOnHover && !e->visuals.highlightOnHover) {
		e = this;
		AddState(graphicsState, UIState::HOVER);
	}
	else 
	{
		//this->RemoveState(UIState::HOVER);
	}
    if (scrollBarY){
//        std::cout<<"\nUIList::Hover "<< (e ? e->name : "NULL") <<" listY: "<<listY<<" mouseY: "<<mouseY;
    }
    return e;
}
UIElement * UIList::Click(GraphicsState* graphicsState, int mouseX, int mouseY)
{
	state &= ~UIState::ACTIVE;
    RETURN_IF_OUTSIDE

	if (activationMessage) {
		MesMan.QueueMessages(activationMessage);
		return this;
	}

	float listX = (float)mouseX;
    float listY = (float)mouseY;
    if (scrollBarY)
        listY -= scrollBarY->GetStart() * layout.sizeY;

    UIElement * e = NULL;
    /// Check le children.
    for (int i = 0; i < children.Size(); ++i){
        UIElement * child = children[i];
        if (child->isSysElement)
            e = child->Click(graphicsState, mouseX, mouseY);
        else
            e = child->Click(graphicsState, RoundInt(listX), RoundInt(listY));
        if (e)
            break;
    }
	if (!e)
	{
        e = this;
		// If activatable, flag it.
		if (this->interaction.activateable){
			AddState(graphicsState, UIState::ACTIVE);
			return this;
		}
		// If not, skip it if it didn't activate.
		return nullptr;
	}
    if (scrollBarY){
        std::cout<<"\nUIList::Click "<< (e ? e->name : "NULL") <<" listY: "<<listY<<" mouseY: "<<mouseY;
    }
    return e;
}
/// GEtttererrr
UIElement * UIList::GetElement(GraphicsState* graphicsState, int mouseX, int mouseY){
    RETURN_IF_OUTSIDE
    float listX = (float)mouseX;
    float listY = (float)mouseY;
    if (scrollBarY)
        listY -= scrollBarY->GetStart() * layout.sizeY;
    UIElement * e = this;
    /// Check le children.
    for (int i = 0; i < children.Size(); ++i){
        UIElement * child = children[i];
        if (child->isSysElement)
            e = child->Click(graphicsState, mouseX, mouseY);
        else
            e = child->GetElement(listX, listY);
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
bool UIList::OnScroll(GraphicsState* graphicsState, float delta)
{
	if (children.Size() == 0)
		return false;
	float moved = 0;
	bool thisOrParentMoved = false;
    /// Move the slider and adjust content.
    if (scrollBarY)
	{
        float pageSize = scrollBarY->handle->layout.sizeRatioY;
        bool negative = delta < 0;
        float distance = AbsoluteValue(delta);
        float pixels = distance * layout.sizeY;
        if (pixels < 5){
            distance = 5.0f / layout.sizeY;
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
        moved += scrollBarY->Move(*graphicsState, distance);
    //    scrollBarY->PrintDebug();
    }
	if (moved)
	{
		thisOrParentMoved = true;
	}
    float moveRemaining = delta - moved;
	if (moveRemaining == delta)
		thisOrParentMoved |= UIElement::OnScroll(graphicsState, moveRemaining) != 0;
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
			children[0]->Hover(graphicsState);
		}
		else {
			LastChild()->Hover(graphicsState);
		}
	}
	return thisOrParentMoved;
}

/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour, and should be NULL for the initial call.
UIElement * UIList::GetUpNeighbour(GraphicsState* graphicsState, UIElement * referenceElement, bool & searchChildrenOnly)
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
				if (child->interaction.navigatable)
					return child;
				
				// Set to only search children now!
				searchChildrenOnly = true;
				UIElement * neighbour = child->GetUpNeighbour(graphicsState, referenceElement, searchChildrenOnly);
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
		Scroll(graphicsState, -500.f);
		// Get last child which is not a system element
		return LastChild();
	}
	// No children? Do we have a valid up-element?
	UIElement * upEle = UIElement::GetUpNeighbour(graphicsState, referenceElement, searchChildrenOnly);
	if (upEle)
		return upEle;

	// No good found? Return the caller element so the cursor stays in place.
	return referenceElement;
}

/// Suggests a neighbour which could be to the right of this element. Meant to be used for UI-navigation support. The reference element indicates the element to which we are seeking a compatible or optimum neighbour, and should be NULL for the initial call.
UIElement * UIList::GetDownNeighbour(GraphicsState* graphicsState, UIElement * referenceElement, bool & searchChildrenOnly)
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
				if (child->interaction.hoverable)
					return child;
				// Set to only search children now!
				searchChildrenOnly = true;
				UIElement * neighbour = child->GetElementClosestTo(referenceElement->layout.position, true);
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
	UIElement * downEle = UIElement::GetDownNeighbour(graphicsState, referenceElement, searchChildrenOnly);
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
		UIElement * child = this->GetElementClosestTo(referenceElement->layout.position, true);
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
		UIElement * child = this->GetElementClosestTo(referenceElement->layout.position, true);
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
void UIList::SetScrollPosition(GraphicsState& graphicsState, float fValue)
{
	if (scrollBarY)
		scrollBarY->SetScrollPosition(graphicsState, fValue);
	return;
}



/// Scroll, not capped.
bool UIList::Scroll(GraphicsState* graphicsState, float absoluteDistanceInPages)
{
    /// Move the slider and adjust content.
	if (scrollBarY)
	{
		float pageSize = scrollBarY->handle->layout.sizeRatioY;
		float distance = absoluteDistanceInPages * pageSize;
		scrollBarY->Move(*graphicsState, distance);
		//    scrollBarY->PrintDebug();
	}
	else
		parent->OnScroll(graphicsState, absoluteDistanceInPages);
    return true;
}

/// Adjusts positions and sizes acording to any attached scroll-bars or other system UI elements.
void UIList::FormatElements(GraphicsState& graphicsState){
    if (!formatX)
        return;
    for (int i = 0; i < children.Size(); ++i){
        UIElement * e = children[i];
        if (e->isSysElement)
            continue;
        if (scrollBarY){
			// Handled during rendering instead.
			/*
            if (e->layout.sizeRatioX > 1.0f - scrollBarY->layout.sizeRatioX){
                float newSizeRatioX = 1.0f - scrollBarY->layout.sizeRatioX;
                e->layout.alignmentX = e->layout.alignmentX + (newSizeRatioX - e->layout.sizeRatioX) * 0.5f;
                e->layout.sizeRatioX = newSizeRatioX;
            }
			*/
			/// Re-bufferize all elements that are formated!
			e->Bufferize();
        }
    }
}

/// Rendering
void UIList::Render(GraphicsState & graphicsState)
{
	UIElement::Render(graphicsState);
    // Render ourself and maybe children.
//    RenderSelf(graphicsState);
  //  if (children.Size())
    //    RenderChildren(graphicsState);
}

void UIList::RenderChildren(GraphicsState & graphicsState)
{
	Matrix4d modelMatrix = graphicsState.modelMatrixD;
	Matrix4d translatedModelMatrix = graphicsState.modelMatrixD;

	Vector4d initialPositionTopRight(layout.right, layout.top, 0, 1),
			initialPositionBottomLeft(layout.left, layout.bottom, 0, 1);
	Vector3f currTopRight = graphicsState.modelMatrixF * initialPositionTopRight;
	Vector3f currBottomLeft = graphicsState.modelMatrixF * initialPositionBottomLeft;

	Rect previousScissor = graphicsState.scissor;
	Rect uiRect(currBottomLeft[0], currBottomLeft[1], currTopRight[0], currTopRight[1]);
	Rect uiScissor = previousScissor.Intersection(uiRect);
	
	// Set scissor! o.o
	graphicsState.SetGLScissor(uiScissor);
   

	float pageBeginY = 0.f, pageBeginYPixels = 0;
    /// If we got a scrollbar, apply a model matrix to make us render at the proper location.
    if (scrollBarY && scrollBarY->interaction.visible)
	{
        pageBeginY = scrollBarY->GetStart();
        pageBeginYPixels = pageBeginY * layout.sizeY;
        translatedModelMatrix *= Matrix4d::Translation(0, pageBeginYPixels, 0);
		this->pageBegin[1] = pageBeginYPixels;
    }
	float pageMiddleY = pageBeginY - 0.5f;

    /// Render all children
	for (int i = 0; i < children.Size(); ++i)
	{
        UIElement * child = children[i];
        if (!child->interaction.visible)
            continue;

        /// Set model matrix depending on child-type.
        if (child->isSysElement){
        //    std::cout<<"\nChild: "<<child->name<<" is sys element.";
            graphicsState.modelMatrixF = modelMatrix;
        }
		else {
			/// Skip if outside of screen.
			if (AbsoluteValue(child->layout.alignmentY + pageMiddleY) > 0.8f)
				continue;
            graphicsState.modelMatrixF = translatedModelMatrix;
		}
		// Render
        child->Render(graphicsState);
        
		// Reset scissor in-case child-elements were manipulatng it.
		graphicsState.SetGLScissor(uiScissor);
	}
	// Reset scissor to how it was before.
	graphicsState.SetGLScissor(previousScissor);
	// Reset model-matrix to original one.
	graphicsState.modelMatrixF = graphicsState.modelMatrixD = modelMatrix;
}


/// Called to ensure visibility of target element.
void UIList::EnsureVisibility(GraphicsState* graphicsState, UIElement * element)
{
	if (!element)
		return;
	// Check if we need to do anything at all (if small list)
	if (contentsSize <= 1.01f)
		return;

	// Do stuff! o.o
	float alignmentY = 1 - element->layout.alignmentY;
//	std::cout<<"\nAlignmentY: "<<alignmentY;
	float halfSizeY = element->layout.sizeRatioY * 0.5f;
	float top = alignmentY - halfSizeY;
	float bottom = alignmentY + halfSizeY;

	// Extract positions for this element.
	if (scrollBarY == nullptr)
		return;
	float pageTop = this->scrollBarY->GetStart();
	float pageStop = this->scrollBarY->GetStop();

	// If elements top exceeding current viewable top, scroll up.
	if (top <= pageTop){
		float dist = pageTop - top;
		this->Scroll(graphicsState, dist);
	//	std::cout<<"\nScrolling up.";
	}
	// And if element's bottom exceeding current viewable bottom, scroll down..!
	else if (bottom >= pageStop){
		float dist = pageStop - bottom;
		this->Scroll(graphicsState, dist);
//		std::cout<<"\nScrolling down.";
	}

//	float


	// Compare with scroll-bar position.

	//this->Scroll(-0.1f);
}


