/// Emil Hedemalm
/// 2014-08-25
/// Combination element dedicated to handling input of float point numbers.

#ifndef UI_FLOAT_INPUT_H
#define UI_FLOAT_INPUT_H

#include "UI/Input/UIInput.h"
class GraphicsState;

/// Class for 1 float inputs.
class UIFloatInput : public UIInput {
public:
	UIFloatInput(String name, String onTrigger);
	virtual ~UIFloatInput();

	/** For mouse-scrolling. By default calls it's parent's OnScroll. Returns true if the element did anything because of the scroll.
		The delta corresponds to amount of "pages" it should scroll.
	*/
	virtual bool OnScroll(GraphicsState* graphicsState, float delta) override;

	/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
	virtual void OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement) override;

	// For sub-classes to adjust children as needed (mainly for input elements).
	virtual void OnStateAdded(int state) override;

	/// Creates the label and input.
	void CreateChildren(GraphicsState* graphicsState) override;
	/// Getter/setter for the input element.
	float GetValue();
	void SetValue(float value);

	/// Same as onTrigger, set to all inputs.
	String action;
	/// for eased access.
	int maxDecimals;

	/// Used when scrolling, and maybe also when reacting to the up/down arrows when the element is active? Default 1.0f?
	float smallIncrement;

	/// If true, will accept and interpret any input as a mathematical expression, evaluating it as such.
	bool acceptMathematicalExpressions;

private:
	UIInput * input;
};


#endif
