/// Emil Hedemalm
/// 2014-08-31
/// Aggregate input class which handles a number of buttons

#ifndef UI_RADIO_BUTTONS_H
#define UI_RADIO_BUTTONS_H

#include "UI/Lists/UIColumnList.h"
#include "UI/UIButtons.h"
#include "UI/Buttons/UIToggleButton.h"

/// Aggregate input class which handles a number of buttons
class UIRadioButtons : public UIColumnList 
{
public:
	/// o.o
	UIRadioButtons(int numberOfButtons, String name, String action);
	virtual ~UIRadioButtons();

	/// Creates the actual buttons.
	virtual void CreateChildren(GraphicsState * graphicsState) override;
	/// Sets the texts of the children.
	void SetTexts(List<String> texts);
	// Set texture for all elements.
	void SetTextureSource(String source);

	// Sets color for the toggle-buttons/selections
	void SetSelectionsTextColor(Color color);

	// Sets textures to be used for the elements, assuming one texture per button.
	void SetTextures(List<String> textureSourcesOrNames);

	// Sent when a child checkbox is toggled. 
	virtual void OnToggled(UIToggleButton * toggleButton);

	// Returns true if it adjusted any UI state.
	bool OnNavigate(GraphicsState* graphicsState, NavigateDirection navigateDirection);
	virtual bool OnProceed(GraphicsState* graphicsState) override;

	void OnStateAdded(int state) override;

	/// Toggles appropriately.
	void SetValue(GraphicsState* graphicsState, int v);

	/// Sets activationMessage/onActivate for all buttons.
	String action;

	static Color toggledTextColor;
	static Color notToggledTextColor;

protected:
	Color selectionsTextColor;
	int numButtons;
	int toggledIndex;
	List<String> buttonTexts;
	List<UIToggleButton*> buttons;
	List<String> textureSourcesOrNames;
};

#endif

