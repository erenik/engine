// Emil Hedemalm
// 2020-08-20
// A button which is togglable, usually just using images or other graphics to show it is enabled / disabled.

#pragma once

#include "../UIElement.h"

class UIToggleButton : public UIElement {
public:
	UIToggleButton(String name = "");
	virtual ~UIToggleButton();
	virtual UIElement* Activate(GraphicsState* graphicsState) override;

	void SetToggled(bool value);
	// Sets the flag, but does not call the OnToggled event.
	void SetToggledSilently(bool value);
	/// Callback sent to parents once an element is toggled, in order to act upon it. Used by UIMatrix, can be ToggleButton, Checkbox et al
	void OnToggled(UIElement * element) override;
	const bool IsToggled() const { return toggled; }

	void OnStateAdded(GraphicsState* graphicsState, int state) override;
	/// For example UIState::HOVER, if recursive will apply to all children.
	virtual void RemoveState(int state, bool recursive = false);

	void UpdateTexture();

	static String defaultOnToggledTexture;
	static String defaultOnNotToggledTexture;

	String onToggledTexture;
	String onNotToggledTexture;

private:
	/// For checkboxes.
	bool toggled;

};