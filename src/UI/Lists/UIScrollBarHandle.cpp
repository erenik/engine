/// Emil hedemalm
/// 2016-04-22
/// UI scroll-bar

#include "UIList.h"

#include "InputState.h"
#include "UI/UITypes.h"
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
	sizeRatioX = 0.8f;
    selectable = true;
    hoverable = true;
	navigatable = true;
    activateable = true;
    isSysElement = true;
	highlightOnHover = true;
    activationMessage = "BeginScroll(this)";

	topBorderTextureSource = "ui/border_top_4";
	rightBorderTextureSource = "ui/border_right_4";
	topRightCornerTextureSource = "ui/top_right_corner_8x8";
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
	// Just do default, make us highlighted?
	UIElement::Hover(mouseX, mouseY);
	/* Moved to be handled within OnMouseMove
	if (element == this && InputMan.lButtonDown)
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

	alignment = SCROLL_BAR_Y;

    selectable = true;
    hoverable = true;
	navigatable = true;
    activateable = true;
    isSysElement = true;
	highlightOnHover = false;
	highlightOnActive = false;

    activationMessage = "BeginScroll(this)";
}
