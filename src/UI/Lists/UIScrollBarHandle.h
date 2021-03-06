/// Emil Hedemalm
/// 2020-12-11
/// Handle for scroll-bars, moved out of UIList.h

#pragma once

#include "UI/UIElement.h"

class UIScrollBarHandle : public UIElement {
public:
	UIScrollBarHandle(String parentName);
	virtual ~UIScrollBarHandle();
	/// Returns the actual distance moved.
	float Move(GraphicsState& graphicsState, float distance);
	float GetScrollPosition();
	void SetScrollPosition(GraphicsState& graphicsState, float f);

	/// Activation functions
	virtual UIElement * Hover(GraphicsState* graphicsState, int mouseX, int mouseY);
	// Returns true once the highest-level appropriate element has been found.
	// No co-ordinates are required since we will instead require the element to already
	// be highlighted/hovered above.
	virtual UIElement * Click(GraphicsState* graphicsState, int mouseX, int mouseY);

	/// Used by e.g. ScrollBarHandle's in order to slide its content according to mouse movement, even when the mouse extends beyond the scope of the element.
	virtual void OnMouseMove(GraphicsState& graphicsState, Vector2i activeWindowCoords);

	UIElement * CreateBorderElement(GraphicsState* graphicsState, String textureSource, char alignment) override;

	// Sets alignment, queueing rebuffering.
	void SetAlignmentY(GraphicsState& graphicsState, float y);

	static String defaultTextureSource;

	static String defaultTopBorder,
		defaultRightBorder,
		defaultBottomBorder,
		defaultTopRightCorner;

	// If 0, ignore, if non-0, set width of all handles to given value and use it instead of relative sizing.
	static int defaultLockWidth,
		defaultLockHeight,
		defaultBorderOffset;

	int lockWidth;
	int lockHeight;
};
