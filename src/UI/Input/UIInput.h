/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#ifndef UI_INPUT_H
#define UI_INPUT_H

#include "UI/UIElement.h"

/// Base class for input. Will yield control and process its onTrigger message once ENTER is pressed. Will only yield control if ESC is pressed.
class UIInput : public UIElement 
{
public:
	UIInput(String name = "");
	virtual ~UIInput();

	/** Called by OS-functions to query if the UI wants to process drag-and-drop files. If so the active element where the mouse is hovering may opt to do magic with it.
		If no magic, or action, is taken, it will return false, at which point the game state should be called to handle general drag-and-drop files.
	*/
	virtual bool HandleDADFiles(List<String> files);

	// When clicking/Enter pressed on keyboard.
	virtual UIElement * Click(int mouseX, int mouseY);
	// When button is released.
	virtual UIElement* Activate(GraphicsState* graphicsState) override;

	/// Default calls parent class RemoveState. If the Active flag is removed, input is also halted/cancelled.
	virtual void RemoveState(int state, bool recursive = false);

	// Used for handling things like drag-n-drop and copy-paste operations, etc. as willed.
	virtual void ProcessMessage(Message * message);

	/// Calls UIElement::SetText in addition to setting the editText to the same value if force is true.
	virtual void SetText(CTextr newText, bool force = false);
	/// Sets edit text. Resets text size so that it should be visible straight away.
	virtual void SetEditText(CTextr newText);

	/// Called once this element is no longer visible for any reason. E.g. switching game states to display another UI, or when this or a parent has been popped from the ui.
	virtual void OnExitScope(bool forced);

	/** Used by input-captuing elements. Should not be called for any base UI elements(?). 
		Return value of significance, but not defined yet.
	*/
	virtual int OnKeyDown(GraphicsState* graphicsState, int keyCode, bool downBefore);
	/// Used for getting text. This will be local translated language key codes?
	virtual int OnChar(int asciiCode);
	/// Begins input! >)
	void BeginInput();

	// Creates default elements for a label and input one-liner input element. Used by Integer, String, Float inputs.
	UIColumnList * CreateDefaultColumnList();
	UILabel * CreateDefaultLabel(UIColumnList * box, float sizeX);
	UIInput * CreateDefaultInput(UIColumnList * box, float sizeX);
	float DefaultSpacePerElement();

	/// For making it a float/integer-only thingy.
	bool numbersOnly;
	/// For any valid alpha-numeric + a few key signs mathematical expressions.
	bool mathematicalExpressionsOnly;
	/// If this is a password-type thingy, this will convert so that the characters are always rendered as e.g. stars.
	bool concealCharacters;
	// Default false.
	bool rememberPreviousInputs; 

protected:
	/// For handling text-input
	void OnBackspace();
	/// Halts input and removes Active state.
	void StopInput();
	// sends message to update the ui with new caret and stuff.
	void OnTextUpdated();

	/// Necessary for text-editing.
	int caretPosition;
	/// Text before editing begun, for when canceling an edit.
	Text previousText;
	/// Text used during editing, used to update the rendered and used text-string, but not one and the same!
	Text editText;

private:
	// Add variables that limit what can be entered into this input?
	// If currently active.
	bool inputActive;
};


#endif
