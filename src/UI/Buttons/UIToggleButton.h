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
	const bool IsToggled() const { return toggled; }

	void OnStateAdded(int state) override;

	void UpdateTexture();

	String onToggledTexture;
	String onNotToggledTexture;

private:
	/// For checkboxes.
	bool toggled;

};