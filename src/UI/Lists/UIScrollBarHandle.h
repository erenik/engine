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
	float Move(float distance);
	float GetScrollPosition();
	void SetScrollPosition(float f);

	/// Activation functions
	virtual UIElement * Hover(int mouseX, int mouseY);
	// Returns true once the highest-level appropriate element has been found.
	// No co-ordinates are required since we will instead require the element to already
	// be highlighted/hovered above.
	virtual UIElement * Click(int mouseX, int mouseY);

	/// Used by e.g. ScrollBarHandle's in order to slide its content according to mouse movement, even when the mouse extends beyond the scope of the element.
	virtual void OnMouseMove(Vector2i activeWindowCoords);

	UIElement * CreateBorderElement(String textureSource, char alignment) override;

	// Sets alignment, queueing rebuffering.
	void SetAlignmentY(float y);

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
