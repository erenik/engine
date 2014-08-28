/// Emil Hedemalm
/// 2014-08-25
/// Combination element dedicated to handling a series of numbers (usually 2-4)

#ifndef UI_VECTOR_INPUT_H
#define UI_VECTOR_INPUT_H

#include "UI/UIElement.h"

/// Class that uses X UIInputs and a label to work.
class UIVectorInput : public UIElement {
public:
	/// Amount of elements in the vector first (primarily 2 to 4)
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

	/// If true, will accept and interpret any input as a mathematical expression, evaluating it as such.
	bool acceptMathematicalExpressions;

private:
	/// For eased access.
	List<UIInput*> inputs;
};

#endif
