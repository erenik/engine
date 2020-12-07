/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#ifndef UI_INPUT_H
#define UI_INPUT_H

#include "UI/UIElement.h"
#include "NavigateDirection.h"

/// Base class for input. Will yield control and process its onTrigger message once ENTER is pressed. Will only yield control if ESC is pressed.
class UIInput : public UIElement 
{
public:
	UIInput(String name = "");
	virtual ~UIInput();

	virtual void OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement) override;

	/** Called by OS-functions to query if the UI wants to process drag-and-drop files. If so the active element where the mouse is hovering may opt to do magic with it.
		If no magic, or action, is taken, it will return false, at which point the game state should be called to handle general drag-and-drop files.
	*/
	virtual bool HandleDADFiles(List<String> files) override;

	// When clicking/Enter pressed on keyboard.
	virtual UIElement * Click(int mouseX, int mouseY) override;
	// When button is released.
	virtual UIElement* Activate(GraphicsState* graphicsState) override;

	// When navigating, either via control, or arrow keys or whatever.
	virtual void Navigate(NavigateDirection direction) override;

	/// Default calls parent class RemoveState. If the Active flag is removed, input is also halted/cancelled.
	virtual void RemoveState(int state, bool recursive = false) override;

	// For sub-classes to adjust children as needed (mainly for input elements).
	virtual void OnStateAdded(int state) override;

	// Used for handling things like drag-n-drop and copy-paste operations, etc. as willed.
	virtual void ProcessMessage(Message * message) override;

	/// Calls UIElement::SetText in addition to setting the editText to the same value if force is true.
	virtual void SetText(CTextr newText, bool force = false) override;
	/// Sets edit text. Resets text size so that it should be visible straight away.
	virtual void SetEditText(CTextr newText);

	/// Called once this element is no longer visible for any reason. E.g. switching game states to display another UI, or when this or a parent has been popped from the ui.
	virtual void OnExitScope(bool forced) override;

	/** Used by input-captuing elements. Should not be called for any base UI elements(?). 
		Return value of significance, but not defined yet.
	*/
	virtual UIInputResult OnKeyDown(GraphicsState* graphicsState, int keyCode, bool downBefore) override;
	/// Used for getting text. This will be local translated language key codes?
	virtual UIInputResult OnChar(int asciiCode) override;
	/// Begins input, returns false if not possible (e.g. non-activatable StringLabel input)
	bool BeginInput();
	/// Halts input and removes Active state.
	virtual void StopInput() override;

	// Creates default elements for a label and input one-liner input element. Used by Integer, String, Float inputs.
	static UIColumnList * CreateDefaultColumnList(UIElement * parent);
	static UILabel * CreateDefaultLabel(UIColumnList * box, String text, float sizeX);
	static UIInput * CreateDefaultInput(UIColumnList * box, String inputName, float sizeX);
	static float DefaultSpacePerElement(float padding);

	// Used for numbersOnly input fields.
	void IncrementValue();
	void DecrementValue();

	// Parses int value from this element's text.
	const int ParseInt();
	// Parses float value from this element's text.
	const float ParseFloat();

	// For setting static colors.
	virtual void SetTextColor(Vector4f newOverrideTextColor);

	virtual void SetRange(int min, int max);

	/// For making it a float/integer-only thingy.
	bool numbersOnly;
	/// For any valid alpha-numeric + a few key signs mathematical expressions.
	bool mathematicalExpressionsOnly;
	/// If this is a password-type thingy, this will convert so that the characters are always rendered as e.g. stars.
	bool concealCharacters;
	// Default false.
	bool rememberPreviousInputs; 

	// Is it currently active for input?
	const bool InputActive() const { return inputActive; }

protected:
	/// For handling text-input
	void OnBackspace();
	// sends message to update the ui with new caret and stuff.
	void OnTextUpdated();

	/// Necessary for text-editing.
	int caretPosition;
	/// Text before editing begun, for when canceling an edit.
	Text previousText;
	/// Text used during editing, used to update the rendered and used text-string, but not one and the same!
	Text editText;

	// For range
	int min, max;

private:
	// Add variables that limit what can be entered into this input?
	// If currently active.
	bool inputActive;
};


#endif
