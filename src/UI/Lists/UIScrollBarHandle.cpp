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

	layout.borderOffset = defaultBorderOffset;
	
	lockWidth = defaultLockWidth;
	if (lockWidth > 0) {
		layout.sizeRatioX = 1.0f;
		layout.lockSizeX = true;
		layout.sizeX = lockWidth;
	}
	else {
		layout.sizeRatioX = 0.8f;
	}

    interaction.selectable = true;
	interaction.hoverable = true;
	interaction.navigatable = true;
	interaction.activateable = true;
    isSysElement = true;
	visuals.highlightOnHover = true;
    activationMessage = "BeginScroll(this)";

	topBorderTextureSource = defaultTopBorder;
	bottomBorderTextureSource = defaultBottomBorder;

	rightBorderTextureSource = defaultRightBorder;
	topRightCornerTextureSource = "";

	visuals.textureSource = defaultTextureSource;

}

UIScrollBarHandle::~UIScrollBarHandle()
{
//	std::cout<<"\nUIScrollBarHandle destructor.";
}

float UIScrollBarHandle::Move(GraphicsState& graphicsState, float distance)
{
	float alignmentPre = layout.alignmentY;
	layout.alignmentY += distance;
	SetScrollPosition(graphicsState, layout.alignmentY);
	float moved = layout.alignmentY - alignmentPre;
	return moved;
}

float UIScrollBarHandle::GetScrollPosition()
{
	return layout.alignmentY;
}

void UIScrollBarHandle::SetScrollPosition(GraphicsState& graphicsState, float f)
{
	layout.alignmentY = f;
    if (layout.alignmentY > 1.0f - layout.sizeRatioY * 0.5f)
		layout.alignmentY = 1.0f - layout.sizeRatioY * 0.5f;
    else if (layout.alignmentY < 0.0f + layout.sizeRatioY * 0.5f)
		layout.alignmentY = 0.0f + layout.sizeRatioY * 0.5f;
	
	// Recalculate position of the element to render correctly.
//	this->QueueBuffering();
	Rebufferize(graphicsState);
}



/// Activation functions
UIElement * UIScrollBarHandle::Hover(GraphicsState* graphicsState, int mouseX, int mouseY)
{
	UIElement * element = UIElement::Hover(graphicsState, mouseX, mouseY);
	// Just do default, make us highlighted?
	UIElement::Hover(graphicsState, mouseX, mouseY);
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
UIElement * UIScrollBarHandle::Click(GraphicsState* graphicsState, int mouseX, int mouseY)
{
	UIElement * clickElement = UIElement::Click(graphicsState, mouseX, mouseY);
	if (clickElement == this)
	{
		float alignmentY = (mouseY - parent->layout.bottom) / (float)this->parent->layout.sizeY;
		this->SetAlignmentY(*graphicsState, alignmentY);
	}
	return clickElement;
}

/// Used by e.g. ScrollBarHandle's in order to slide its content according to mouse movement, even when the mouse extends beyond the scope of the element.
void UIScrollBarHandle::OnMouseMove(GraphicsState& graphicsState, Vector2i activeWindowCoords)
{
	float alignmentY = (activeWindowCoords[1]  - parent->layout.bottom) / (float) this->parent->layout.sizeY;
	SetAlignmentY(graphicsState, alignmentY);
}

// Setting
void UIScrollBarHandle::SetAlignmentY(GraphicsState& graphicsState, float y)
{
	float halfSizeY = this->layout.sizeRatioY * 0.5f;
	if (y > 1.f - halfSizeY)
		y = 1.f - halfSizeY;
	if (y < 0.f + halfSizeY)
		y = 0.f + halfSizeY;
	
	layout.alignmentY = y;
	Rebufferize(graphicsState);
}

#include "TextureManager.h"
#include "File/LogFile.h"

UIElement * UIScrollBarHandle::CreateBorderElement(GraphicsState* graphicsState, String textureSource, char alignment) {
	UIBorder * borderElement = new UIBorder();
	borderElement->name = parent->name + " border " + String(alignment);
	borderElement->visuals.textureSource = textureSource;
	borderElement->SetParent(this);
	Texture * texture = TexMan.GetTexture(textureSource);
	if (texture == nullptr) {
		LogGraphics("Unable to fetch texture for border: " + textureSource + " for element " + name, INFO);
		delete borderElement;
		return nullptr;
	}
	borderElement->layout.alignment = alignment;
	borderElement->visuals.highlightOnHover = true;

	// Give top and bottom borders a size proportionate to what they have.
	float ratioHandleToTextureX = layout.sizeX /  texture->size.x;

	switch (alignment) {
	case TOP:
		borderElement->layout.sizeY = texture->size.y * ratioHandleToTextureX;
		borderElement->layout.lockSizeY = true;
		break;
	case BOTTOM:
		borderElement->layout.sizeY = texture->size.y * ratioHandleToTextureX;
		borderElement->layout.lockSizeY = true;
		break;
	case RIGHT:
		borderElement->layout.sizeX = texture->size.x;
		borderElement->layout.lockSizeX = true;
		break;
	case TOP_RIGHT:
		borderElement->layout.sizeY = texture->size.y;
		borderElement->layout.lockSizeY = true;
		borderElement->layout.sizeX = texture->size.x;
		borderElement->layout.lockSizeX = true;
		break;
	default:
		assert(false && "Implemenet");
	}
	borderElements.Add(borderElement);
	return borderElement;
}