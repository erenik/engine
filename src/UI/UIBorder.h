/// Emil Hedemalm
/// 2020-12-12
/// Border elements and specific rendering around, inside, and outside edges of existing UI elements.

#pragma once

#include "UIElement.h"

class UIBorder : public UIElement {
public:
	virtual ~UIBorder();

	/// Adjusts the UI element size and position relative to new AppWindow size
	void AdjustToWindow(GraphicsState& graphicsState, const UILayout& windowLayout) override;

};
