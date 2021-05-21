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

	layout.alignment = SCROLL_BAR_Y;

	interaction.DefaultTrue();	
	isSysElement = true;

	visuals.highlightOnHover = false;
	visuals.highlightOnActive = false;

	activationMessage = "BeginScroll(this)";

	visuals.textureSource = UIScrollBar::defaultTextureSource;
}

UIScrollBar::~UIScrollBar()
{
//	std::cout<<"\nUIScrollBar destructor.";
}

void UIScrollBar::CreateHandle()
{
    assert(handle == NULL);
    handle = new UIScrollBarHandle(parent->name);
	handle->layout.sizeRatioX = 0.9f;
 //   handle->text = "Joooooo";
    AddChild(nullptr, handle);
    previousSize = 1.0f;
}

void UIScrollBar::OnMouseY(GraphicsState& graphicsState, int y)
{
	float pageSize = this->PageSize();
	float halfPageSize = pageSize * 0.5f;
	float pos = (y + halfPageSize)/ (float)layout.sizeY;
// Ensure.. within bounds?
	handle->SetAlignmentY(graphicsState, pos);
}

/// Activation functions
UIElement * UIScrollBar::Hover(GraphicsState* graphicsState, int mouseX, int mouseY)
{
	// OnActivate()?
	// Update scroll position!
	UIElement * element = UIElement::Hover(graphicsState, mouseX, mouseY);
	if (element == this && inputState->lButtonDown)
		OnMouseY(*graphicsState, mouseY - layout.bottom);
	return element;
}


// Returns true once the highest-level appropriate element has been found.
// No co-ordinates are required since we will instead require the element to already
// be highlighted/hovered above.
UIElement * UIScrollBar::Click(GraphicsState* graphicsState, int mouseX, int mouseY)
{
	UIElement * result = 0;
	// Don't process invisible UIElements, please.
	if (interaction.visible == false)
		return false;

	// Check if the mouse is outside the element's boundaries.
	if (mouseX > layout.right || mouseX < layout.left ||
		mouseY > layout.top || mouseY < layout.bottom){
			// Return false if we are outside of the boundaries,
			// since we haven't found the selected element.
			SetState(UIState::IDLE);
			//	if(child != NULL)
			return NULL;
	}

	// Check axiomaticness (direct-activation without further processing)
	if (interaction.axiomatic){
		if (interaction.activateable){
			AddState(graphicsState, UIState::ACTIVE);
			return this;
		}
		return NULL;
	}

	// OnActivate()?
	// Update scroll position!
	OnMouseY(*graphicsState, mouseY - layout.bottom);


	// Alright, the mouse is inside this element!
	// Do we have children?
	for (int i = children.Size()-1; i >= 0; --i){
		UIElement * child = children[i];
	    if (!child->interaction.visible)
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
	if (this->interaction.activateable && HasState(UIState::HOVER)){
		AddState(graphicsState, UIState::ACTIVE);
		return this;
	}
	// If not, return false, since we haven't foun the right element.
	return NULL;
}

// Updates to contents with given size in relative units.
void UIScrollBar::Update(GraphicsState* graphicsState, float newSize)
{
//    std::cout<<"\nUIScrollBar::Update New size: "<<newSize;
    float newPreviousSize = newSize;
    /// Adjust the handle's size and position.
    if (newSize <= 1.0f){
        handle->layout.sizeRatioY = 1.0f;
        handle->layout.alignmentY = 0.5f;
        interaction.visible = false;
    }
    else {
		/*
		if (vboBuffer != -1)
			FreeBuffers();
	    if (mesh)
            DeleteGeometry();
			*/
        float top = handle->layout.alignmentY + handle->layout.sizeRatioY * 0.5f;
        handle->layout.sizeRatioY = 1.0f / newSize;
     //   std::cout<<"\nHandle sizeRatioY: "<<handle->layout.sizeRatioY;
        handle->layout.alignmentY = top - handle->layout.sizeRatioY * 0.5f;
		handle->ResizeGeometry(graphicsState);
     //   std::cout<<"\nHandle alignmentY: "<<handle->layout.alignmentY;
        interaction.visible = true;
    }
    previousSize = newPreviousSize;
}

float UIScrollBar::Move(GraphicsState& graphicsState, float distance)
{
    return handle->Move(graphicsState, distance);
}

/// Returns current scroll position, based on the handle.
float UIScrollBar::GetScrollPosition()
{
	return handle->GetScrollPosition();
}

void UIScrollBar::SetScrollPosition(GraphicsState& graphicsState, float f)
{
	handle->SetScrollPosition(graphicsState, f);
}


/// Returns the current relative start of the contents
float UIScrollBar::GetStart()
{
    float pageSize = handle->layout.sizeRatioY;
    float handleStart = handle->layout.alignmentY + handle->layout.sizeRatioY * 0.5f;
    /// Divide by the size ratio to get how many pages down we are!
    float start = (1.0f - handleStart) / handle->layout.sizeRatioY;
/*    std::cout<<"\nHandle alignmentY: "<<handle->layout.alignmentY<<" sizeRatioY: "<<handle->layout.sizeRatioY;
    std::cout<<"\nPage size: "<<pageSize;
    std::cout<<"\nHandle start: "<<handleStart;
    std::cout<<"\nStart: "<<start;
    */
    return start;
}
/// Returns the current relative start of the contents
float UIScrollBar::GetStop(){
    float contentSize = 1.0f / handle->layout.sizeRatioY;
    float handleStop = handle->layout.alignmentY - handle->layout.sizeRatioY * 0.5f;
    float stop = (1.0f - handleStop) * contentSize;
    return stop;
}

/// Returns size of the scroll bar handle, equivalent to how big a page is relative to the total contents.
float UIScrollBar::PageSize(){
	if (!handle)
		return 1.0f;
	return this->handle->layout.sizeRatioY;
}

void UIScrollBar::PrintDebug()
{
    float pageSize = handle->layout.sizeRatioY;
    float handleStart = handle->layout.alignmentY + handle->layout.sizeRatioY * 0.5f;
    float start = (1.0f - handleStart) * pageSize;

    std::cout<<"\nHandle alignmentY: "<<handle->layout.alignmentY<<" sizeRatioY: "<<handle->layout.sizeRatioY;
    std::cout<<"\nPage size: "<<pageSize;
    std::cout<<"\nHandle start: "<<handleStart;
    std::cout<<"\nStart: "<<start;
}

