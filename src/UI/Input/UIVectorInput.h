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
	virtual void OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement) override;

	/// Creates ze children!
	void CreateChildren(GraphicsState* graphicsState) override;

	/// Calls UIElement::SetText in addition to setting the editText to the same value if force is true.
	void SetText(CTextr newText, bool force /*= false*/) override;

	/// Getters
	Vector2i GetValue2i();
	Vector2f GetValue2f();
	Vector3f GetValue3f();
	Vector4f GetValue4f();
	
	/// Setters
	void SetValue2i(Vector2i vec);
	void SetValue2f(Vector2f vec);
	void SetValue3f(const Vector3f & vec);
	void SetValue4f(const Vector4f & vec);
	
	/** For mouse-scrolling. By default calls it's parent's OnScroll. Returns true if the element did anything because of the scroll.
		The delta corresponds to amount of "pages" it should scroll.
	*/
	bool OnScroll(GraphicsState * graphicsState, float delta) override;
	
	/// See dataTypes below.
	void SetDataType(int dataType);

	/// Action to be taken when any of the fields are triggered.
	String action;
	int numInputs;
	int maxDecimals;

	/// Defines what messages and functions will be relevant for this specific input. Default is FLOATS
	enum {
		INTEGERS,
		FLOATS,
	};
	int dataType;

	/// If true, will accept and interpret any input as a mathematical expression, evaluating it as such.
	bool acceptMathematicalExpressions;

private:

	String labelText;
	/// For eased access.
	List<UIInput*> inputs;
};

#endif
