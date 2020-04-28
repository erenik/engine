// Emil Hedemalm
// 2013-08-03
// Checkboxes n buttons of various sorts!

#ifndef UI_CHECKBOX_H
#define UI_CHECKBOX_H

#include "UIElement.h"

class UIButton : public UIElement{
public:
	UIButton(String name = "");
	virtual ~UIButton();
};

class UICheckBox : public UIElement {
public:
	UICheckBox(String name = "");
	virtual ~UICheckBox();
	virtual UIElement* Activate(GraphicsState* graphicsState) override;
private:
};

#endif