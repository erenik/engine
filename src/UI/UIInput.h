/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#ifndef UI_INPUT_H
#define UI_INPUT_H

#include "UIElement.h"

class UIImage;

/// Base class for input. Will yield control and process its onTrigger message once ENTER is pressed. Will only yield control if ESC is pressed.
class UIInput : public UIElement {
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
	virtual UIElement * Activate();


	// Used for handling things like drag-n-drop and copy-paste operations, etc. as willed.
	virtual void ProcessMessage(Message * message);

	/// Called once this element is no longer visible for any reason. E.g. switching game states to display another UI, or when this or a parent has been popped from the ui.
	virtual void OnExitScope();

	/** Used by input-captuing elements. Should not be called for any base UI elements(?). 
		Return value of significance, but not defined yet.
	*/
	virtual int OnKeyDown(int keyCode, bool downBefore);
	/// Used for getting text. This will be local translated language key codes?
	virtual int OnChar(int asciiCode);
	/// Begins input! >)
	void BeginInput();
	/// For making it a float/integer-only thingy.
	bool numbersOnly;
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

/// Class for 1 string input with label before it.
class UIStringInput : public UIElement {
public:
	UIStringInput(String name, String onTrigger);
	virtual ~UIStringInput();

	/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
	virtual void OnInputUpdated(UIInput * inputElement);

	/// Creates the label and input.
	void CreateChildren();
	/// Getter/setter for the input element.
	String GetValue();
	void SetValue(String value);

	/// Same as onTrigger, set to all inputs.
	String action;
	UIInput * input;
private:
};

class UITextureInput : public UIElement {
public:
	UITextureInput(String name, String onTrigger);
	virtual ~UITextureInput();

	/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
	virtual void OnInputUpdated(UIInput * inputElement);

	/** Called by OS-functions to query if the UI wants to process drag-and-drop files. If so the active element where the mouse is hovering may opt to do magic with it.
		If no magic, or action, is taken, it will return false, at which point the game state should be called to handle general drag-and-drop files.
	*/
	virtual bool HandleDADFiles(List<String> files);


	/// Creates the label and input.
	void CreateChildren();
	/// Getter/setter for the input element.
	Texture * GetTexture();
	String GetTextureSource();
	
	/// To be called only from the RenderThread! Use GMSetUIs with TEXTURE_INPUT_SOURCE target!
	void SetTextureSource(String source);
	/// Same as onTrigger, set to all inputs.
	String action;
	/// for eased access.
	UIInput * input;
	UIImage * uiImage;
private:
};

/// Class for 1 float inputs.
class UIFloatInput : public UIElement {
public:
	UIFloatInput(String name, String onTrigger);
	virtual ~UIFloatInput();

	/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
	virtual void OnInputUpdated(UIInput * inputElement);

	/// Creates the label and input.
	void CreateChildren();
	/// Getter/setter for the input element.
	float GetValue();
	void SetValue(float value);

	/// Same as onTrigger, set to all inputs.
	String action;
	/// for eased access.
	int maxDecimals;
private:
	UIInput * input;
};


/// Class for 1 Integer inputs.
class UIIntegerInput : public UIElement {
public:
	UIIntegerInput(String name, String onTrigger);
	virtual ~UIIntegerInput();

	/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
	virtual void OnInputUpdated(UIInput * inputElement);

	/// Creates the label and input.
	void CreateChildren();
	/// Getter/setter for the input element.
	int GetValue();
	void SetValue(int value);

	/// Same as onTrigger, set to all inputs.
	String action;
	/// for eased access.
	int maxDecimals;
	/// Should not be set from outside graphics thread (after it is made visible and rendering).
	UIInput * input;
private:
};

/// Class that uses X UIInputs and a label to work.
class UIVectorInput : public UIElement {
public:
	/// Amount of elements in the vector first (primarily 3 or 4)
	UIVectorInput(int numInputs, String name, String onTrigger);
	virtual ~UIVectorInput();

	/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
	virtual void OnInputUpdated(UIInput * inputElement);

	/// Creates ze children!
	void CreateChildren();

	/// Getters
	Vector2i GetValue2i();
	Vector3f GetValue3f();
	Vector4f GetValue4f();
	
	/// Setters
	void SetValue4f(Vector4f vec);
	void SetValue3f(Vector3f vec);
	void SetValue2i(Vector2i vec);
	
	/// Action to be taken when any of the fields are triggered.
	String action;
	int numInputs;
	int maxDecimals;
private:
	/// For eased access.
	List<UIInput*> inputs;
};


/// Element made for use for any kind of text, like a random text-editor?
class UITextField : public UIInput{
public:
	UITextField();
};


#endif
