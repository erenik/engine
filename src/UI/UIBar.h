/// Emil Hedemalm
/// 2020-07-19
/// A resizable UI-Bar used to represent HP, Shield, et al. Preferably used in combination with an IntegerLabel

#pragma once 

#include "UIElement.h"

class UIBar : public UIElement {
public:
	UIBar(String name);
	virtual ~UIBar();

	// Set it 0.0 for empty, 1.0 for full.
	void SetFill(float ratio0to1);
	
private:
	float sizeRatioXBeforeFillRatio;
	float alignmentXBeforeFillRatio;
};
