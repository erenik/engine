/// Emil Hedemalm
/// 2014-08-31
/// Aggregate input class which handles a number of buttons

#ifndef UI_RADIO_BUTTONS_H
#define UI_RADIO_BUTTONS_H

#include "UI/Lists/UIColumnList.h"
#include "UI/UIButtons.h"

/// Aggregate input class which handles a number of buttons
class UIRadioButtons : public UIColumnList 
{
public:
	/// o.o
	UIRadioButtons(int numberOfButtons, String name, String action);
	virtual ~UIRadioButtons();

	/// Creates the actual buttons.
	virtual void CreateChildren();
	/// Sets the texts of the children.
	void SetTexts(List<String> texts);
	// Set texture for all elements.
	void SetTextureSource(String source);

	// Sent when a child checkbox is toggled. 
	virtual void OnToggled(UICheckBox * box);

	/// Toggles appropriately.
	void SetValue(int v);

	/// Sets activationMessage/onActivate for all buttons.
	String action;

protected:
	int numButtons;
	List<String> names;
	List<UICheckBox*> buttons;
};

#endif

