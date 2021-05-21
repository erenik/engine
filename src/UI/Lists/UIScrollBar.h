/// Emil Hedemalm
/// 2020-12-11
/// Handle for scroll-bars, moved out of UIList.h

#pragma once

#include "UI/UIElement.h"

class UIScrollBarHandle;

class UIScrollBar : public UIElement {
	friend class UIList;
public:
	UIScrollBar(String parentName);
	virtual ~UIScrollBar();
	void CreateHandle();

	/// For updating scroll-position based on Mouse coordinate
	void OnMouseY(GraphicsState& graphicsState, int y);

	/// Activation functions
	virtual UIElement * Hover(GraphicsState* graphicsState, int mouseX, int mouseY) override;
	// Returns true once the highest-level appropriate element has been found.
	// No co-ordinates are required since we will instead require the element to already
	// be highlighted/hovered above.
	virtual UIElement * Click(GraphicsState* graphicsState, int mouseX, int mouseY) override;
	// Updates to contents with given size in relative units.
	void Update(GraphicsState* graphicsState, float newSize);

	/** Move the scrollbar, capping it depending on it's size (will never exceed the 0.0 - 1.0 limits)
		Returns the actual distance moved.
	*/
	float Move(GraphicsState& graphicsState, float distance);
	/// Returns current scroll position, based on the handle.
	float GetScrollPosition();
	void SetScrollPosition(GraphicsState& graphicsState, float f);

	/// Returns the current relative start of the contents, 0.0 being top and close to (pages) at the maximum, varying with the content length.
	float GetStart();
	/// Returns the current relative start of the contents
	float GetStop();
	/// Returns size of the scroll bar handle, equivalent to how big a page is relative to the total contents.
	float PageSize();
	void PrintDebug();

	static String defaultTextureSource;

private:
	UIScrollBarHandle * handle;
	float previousSize;
};

