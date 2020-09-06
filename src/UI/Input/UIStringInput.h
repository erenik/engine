/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#ifndef UI_STRING_INPUT_H
#define UI_STRING_INPUT_H

#include "UI/Input/UIInput.h"

/// Class for 1 string input with label before it.
class UIStringInput : public UIInput {
public:
	UIStringInput(String name, String onTrigger);
	virtual ~UIStringInput();

	/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
	virtual void OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement) override;
	virtual UIInputResult OnKeyDown(GraphicsState* graphicsState, int keyCode, bool downBefore) override; // For managing old texts.
	/// Used for getting text. This will be local translated language key codes?
	virtual UIInputResult OnChar(int asciiCode) override;

	// For sub-classes to adjust children as needed (mainly for input elements).
	virtual void OnStateAdded(int state) override;

	/// Creates the label and input.
	void CreateChildren(GraphicsState* graphicsState) override;
	/// Getter/setter for the input element.
	String GetValue();
	void SetValue(String value);

	/// Same as onTrigger, set to all inputs.
	String action;
	UIInput * input;
	bool rememberPreviousInputs; // Default true.
	/// If true, input is disabled from the gui. Default false.
	bool guiInputDisabled; 
private:
	/// For when cycling old inputs.
	int index;
};

#endif
