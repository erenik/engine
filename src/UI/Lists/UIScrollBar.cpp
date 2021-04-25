/// Emil hedemalm
/// 2016-04-22
/// UI scroll-bar

#include "UIScrollBar.h"

#include "UIScrollBarHandle.h"
#include "InputState.h"
#include "UI/UITypes.h"
#include "Graphics/Messages/GMUI.h"
#include "GraphicsState.h"
#include "MathLib/Rect.h"
#include "Input/InputManager.h"
#include "Message/MessageManager.h"

String UIScrollBar::defaultTextureSource = "0x44AA";

UIScrollBar::UIScrollBar(String parentName)
	: UIElement()
{
	type = UIType::SCROLL_BAR;
	name = parentName + " Scroll bar";
	handle = NULL;

	alignment = SCROLL_BAR_Y;

	selectable = true;
	hoverable = true;
	navigatable = true;
	activateable = true;
	isSysElement = true;
	highlightOnHover = false;
	highlightOnActive = false;

	activationMessage = "BeginScroll(this)";

	textureSource = defaultTextureSource;
}

UIScrollBar::~UIScrollBar()
{
//	std::cout<<"\nUIScrollBar destructor.";
}

void UIScrollBar::CreateHandle()
{
    assert(handle == NULL);
    handle = new UIScrollBarHandle(parent->name);
	handle->sizeRatioX = 0.9f;
 //   handle->text = "Joooooo";
    AddChild(nullptr, handle);
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
UIElement * UIScrollBar::Hover(GraphicsState* graphicsState, int mouseX, int mouseY)
{
	// OnActivate()?
	// Update scroll position!
	UIElement * element = UIElement::Hover(graphicsState, mouseX, mouseY);
	if (element == this && inputState->lButtonDown)
		OnMouseY(mouseY - bottom);
	return element;
}


// Returns true once the highest-level appropriate element has been found.
// No co-ordinates are required since we will instead require the element to already
// be highlighted/hovered above.
UIElement * UIScrollBar::Click(GraphicsState* graphicsState, int mouseX, int mouseY)
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
			SetState(UIState::IDLE);
			//	if(child != NULL)
			return NULL;
	}

	// Check axiomaticness (direct-activation without further processing)
	if (axiomatic){
		if (activateable){
			AddState(graphicsState, UIState::ACTIVE);
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
		result = child->Click(graphicsState, mouseX, mouseY);
		if (result != NULL){
			// The active element has been found further down the tree,
			// so we can return true.
			SetState(UIState::IDLE);
			return result;
		}
	}

	
	// Check the element's state. If it is hovered over, we've found it.
	if (this->activateable && HasState(UIState::HOVER)){
		AddState(graphicsState, UIState::ACTIVE);
		return this;
	}
	// If not, return false, since we haven't foun the right element.
	return NULL;
}

// Updates to contents with given size in relative units.
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
		/*
		if (vboBuffer != -1)
			FreeBuffers();
	    if (mesh)
            DeleteGeometry();
			*/
        float top = handle->alignmentY + handle->sizeRatioY * 0.5f;
        handle->sizeRatioY = 1.0f / newSize;
     //   std::cout<<"\nHandle sizeRatioY: "<<handle->sizeRatioY;
        handle->alignmentY = top - handle->sizeRatioY * 0.5f;
		handle->ResizeGeometry();
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

