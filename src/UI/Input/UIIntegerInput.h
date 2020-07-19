/// Emil Hedemalm
/// 2014-08-25
/// Combination element dedicated to handling input of integers.

#ifndef UI_INTEGER_INPUT_H
#define UI_INTEGER_INPUT_H

#include "UI/Input/UIInput.h"

/// Class for 1 Integer inputs.
class UIIntegerInput : public UIInput
{
public:
	UIIntegerInput(String name, String onTrigger);
	virtual ~UIIntegerInput();

	/** For mouse-scrolling. By default calls it's parent's OnScroll. Returns true if the element did anything because of the scroll.
		The delta corresponds to amount of "pages" it should scroll.
	*/
	virtual bool OnScroll(GraphicsState* graphicsState, float delta) override;
	/** Used by input-capturing elements. Calls recursively upward until an element wants to respond to the input.
		Returns 1 if it processed anything, 0 if not.
	*/
	virtual int OnKeyDown(GraphicsState* graphicsState, int keyCode, bool downBefore) override;
	
	/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference). Called from render thread.
	virtual void OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement) override;

	/// Creates the label and input.
	void CreateChildren(GraphicsState* graphicsState = nullptr) override;
	/// Getter/setter for the input element.
	int GetValue();
	void SetValue(int value);

	/// Same as onTrigger, set to all inputs.
	String action;
	/// for eased access.
	int maxDecimals;
	/// Should not be set from outside graphics thread (after it is made visible and rendering).
	UIInput * input;
	/// If true, input is disabled from the gui. Default false.
	bool guiInputDisabled; 

	/// If true, will accept and interpret any input as a mathematical expression, evaluating it as such.
	bool acceptMathematicalExpressions;

private:
};

#endif
