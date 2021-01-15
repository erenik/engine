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

// A button composed of a bunch of underlying objects, as defined in their respective .gui file. Highlight and active states are propagated to all children, but interactions only handled on the CompositeButton.
class UICompositeButton : public UIButton {
public:
	UICompositeButton(String name);
	virtual ~UICompositeButton();

	/// Creates a deep copy of self and all child elements (where possible).
	virtual UIElement * Copy() override;

	// For sub-classes to adjust children as needed (mainly for input elements).
	virtual void OnStateAdded(int state) override;

	/// For example UIState::HOVER, if recursive will apply to all children.
	virtual void RemoveState(int state, bool recursive = false) override;

	virtual bool AddChild(GraphicsState* graphicsState, UIElement *in_child) override;
	
	/// Callback-function for sub-classes to implement own behaviour to run within the UI-class' code. Return true if it did something.
	bool OnProceed(GraphicsState* graphicsState);

};

#endif