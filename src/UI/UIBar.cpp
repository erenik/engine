/// Emil Hedemalm
/// 2020-07-19
/// A resizable UI-Bar used to represent HP, Shield, et al. Preferably used in combination with an IntegerLabel, default Horizontal

#include "UIBar.h"

UIBar::UIBar(String name) : UIElement()
{
	this->name = name;
	this->type = UIType::BAR;
	sizeRatioXBeforeFillRatio = 0;
	alignmentXBeforeFillRatio = 0;
}

UIBar::~UIBar() {

}

// Set it 0.0 for empty, 1.0 for full.
void UIBar::SetFill(GraphicsState* graphicsState, float ratio0to1) {
	if (sizeRatioXBeforeFillRatio == 0) {
		sizeRatioXBeforeFillRatio = layout.sizeRatioX;
		alignmentXBeforeFillRatio = layout.alignmentX;
	}

	this->layout.sizeRatioX = sizeRatioXBeforeFillRatio * ratio0to1;
//	this->sizeX = parent->sizeX * this->layout.sizeRatioX;
//	this->lockSizeX = true;

	// Align as needed.
	switch (layout.alignment) {
		case LEFT:
			//this->layout.alignmentX = alignmentXBeforeFillRatio - sizeRatioXBeforeFillRatio * 0.5f + this->layout.sizeRatioX * 0.5f;
			break;
	}

	AdjustToParent(graphicsState);
	ResizeGeometry(graphicsState); // Update the rect to be drawn.
}

