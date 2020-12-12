/// Emil hedemalm
/// 2016-04-22
/// UI scroll-bar

#include "UIScrollBarHandle.h"

#include "InputState.h"
#include "UI/UITypes.h"
#include "Graphics/Messages/GMUI.h"
#include "GraphicsState.h"
#include "MathLib/Rect.h"
#include "Input/InputManager.h"
#include "Message/MessageManager.h"
#include "UI/UIBorder.h"

String UIScrollBarHandle::defaultTextureSource = "#FFFF";

String UIScrollBarHandle::defaultTopBorder = "";
String UIScrollBarHandle::defaultRightBorder = "";
String UIScrollBarHandle::defaultBottomBorder = "";
String UIScrollBarHandle::defaultTopRightCorner = "";

int UIScrollBarHandle::defaultLockWidth = 0,
UIScrollBarHandle::defaultLockHeight = 0,
 UIScrollBarHandle::defaultBorderOffset = 0;

UIScrollBarHandle::UIScrollBarHandle(String parentName)
: UIElement()
{
    type = UIType::SCROLL_HANDLE;
    name = parentName + "Scroll bar handle";

	borderOffset = defaultBorderOffset;
	
	lockWidth = defaultLockWidth;
	if (lockWidth > 0) {
		sizeRatioX = 1.0f;
		lockSizeX = true;
		sizeX = lockWidth;
	}
	else {
		sizeRatioX = 0.8f;
	}

    selectable = true;
    hoverable = true;
	navigatable = true;
    activateable = true;
    isSysElement = true;
	highlightOnHover = true;
    activationMessage = "BeginScroll(this)";

	topBorderTextureSource = defaultTopBorder;
	bottomBorderTextureSource = defaultBottomBorder;

	rightBorderTextureSource = defaultRightBorder;
	topRightCornerTextureSource = "";

	textureSource = defaultTextureSource;

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

#include "TextureManager.h"
#include "File/LogFile.h"

UIElement * UIScrollBarHandle::CreateBorderElement(String textureSource, char alignment) {
	UIBorder * borderElement = new UIBorder();
	borderElement->name = parent->name + " border " + String(alignment);
	borderElement->textureSource = textureSource;
	borderElement->SetParent(this);
	Texture * texture = TexMan.GetTexture(textureSource);
	if (texture == nullptr) {
		LogGraphics("Unable to fetch texture for border: " + textureSource + " for element " + name, INFO);
		delete borderElement;
		return nullptr;
	}
	borderElement->alignment = alignment;
	borderElement->highlightOnHover = true;

	// Give top and bottom borders a size proportionate to what they have.
	float ratioHandleToTextureX = sizeX /  texture->size.x;

	switch (alignment) {
	case TOP:
		borderElement->sizeY = texture->size.y * ratioHandleToTextureX;
		borderElement->lockSizeY = true;
		break;
	case BOTTOM:
		borderElement->sizeY = texture->size.y * ratioHandleToTextureX;
		borderElement->lockSizeY = true;
		break;
	case RIGHT:
		borderElement->sizeX = texture->size.x;
		borderElement->lockSizeX = true;
		break;
	case TOP_RIGHT:
		borderElement->sizeY = texture->size.y;
		borderElement->lockSizeY = true;
		borderElement->sizeX = texture->size.x;
		borderElement->lockSizeX = true;
		break;
	default:
		assert(false && "Implemenet");
	}
	borderElements.Add(borderElement);
	return borderElement;
}