/// Emil Hedemalm
/// 2020-12-12
/// Border elements and specific rendering around, inside, and outside edges of existing UI elements.

#include "UIBorder.h"

#include "Mesh/Square.h"

UIBorder::~UIBorder() {

}

/// Adjusts the UI element size and position relative to new AppWindow size
void UIBorder::AdjustToWindow(GraphicsState& graphicsState, const UILayout& windowLayout) {
	/// Reset text-variables so that they are re-calculated before rendering again.
	layout.AdjustToParent(parent->layout, interaction.moveable);
	/// Set the new dimensions
	if (visuals.mesh)
		GetMesh()->SetDimensions((float)layout.left, (float)layout.right, (float)layout.bottom, (float)layout.top, layout.zDepth);
}


