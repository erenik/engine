// Emil Hedemalm
// 2020-08-20
// A checkbox, containing the label, and a toggle-button.

#pragma once

#include "../UIElement.h"


// A compound element containing a label, a toggle button, inside a columnlist
class UICheckbox : public UIElement {
public:
	UICheckbox(String name = "");
	virtual ~UICheckbox();
	virtual UIElement* Activate(GraphicsState* graphicsState) override;

	void OnStateAdded(int state) override;

	virtual void SetToggled(bool value);

	virtual void OnToggled(UIToggleButton * button) override;
	virtual void CreateChildren();
private:
	UILabel * label;
	UIToggleButton * button;
};