/// Emil Hedemalm
/// 2014-08-25
/// Combination element dedicated to handling input of integers.

#include "UIIntegerInput.h"

#include "MathLib/Expression.h"

#include "UI/UITypes.h"
#include "UI/UI.h"
#include "UI/UIList.h"
#include "UIInput.h"

#include "Input/Keys.h"

#include "Message/MathMessage.h"
#include "Message/MessageManager.h"

#include "Graphics/GraphicsManager.h"

/// Class for 1 Integer inputs.
UIIntegerInput::UIIntegerInput(String name, String onTrigger)
: UIElement(), action(onTrigger)
{
	this->type = UIType::INTEGER_INPUT;
	this->name = name;
	input = NULL;
	label = NULL;
	guiInputDisabled = false;
}
UIIntegerInput::~UIIntegerInput()
{
	// Nothing special, let base class handle children.
}

/** For mouse-scrolling. By default calls it's parent's OnScroll. Returns true if the element did anything because of the scroll.
	The delta corresponds to amount of "pages" it should scroll.
*/
bool UIIntegerInput::OnScroll(float delta)
{
	// Adjust if the input piece is being hovered over.
	if (input->state & UIState::HOVER)
	{
		int v = GetValue();
		if (delta > 0)
			++v;
		else 
			--v;
		SetValue(v);
		/// Notify game state etc. of the change.
		IntegerMessage * m = new IntegerMessage(action, GetValue());
		MesMan.QueueMessage(m);
		return true;
	}
	// If not, do as regular UIElements do, probably query parents..
	return UIElement::OnScroll(delta);
}

/** Used by input-capturing elements. Calls recursively upward until an element wants to respond to the input.
	Returns 1 if it processed anything, 0 if not.
*/
int UIIntegerInput::OnKeyDown(int keyCode, bool downBefore)
{
	int v = GetValue();
	switch(keyCode)
	{
		case KEY::DOWN:
			--v;
			SetValue(v);
			return 1;
		case KEY::UP:
			++v;
			SetValue(v);
			return 1;
	}
	UIElement::OnKeyDown(keyCode, downBefore);
	return 0;
}	

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIIntegerInput::OnInputUpdated(UIInput * inputElement)
{
	assert(inputElement == input);
	/// If we accept expressions, parse and evaluate it?
	if (acceptMathematicalExpressions)
	{
		String text = input->text;
		Expression exp(text);
		ExpressionResult er = exp.Evaluate();
		String result;
		switch(er.type)
		{
			case DataType::INTEGER:
				result = String(er.iResult);
				break;
			case DataType::FLOAT:
				result = String((int)RoundFloat(er.fResult));
				break;
			default:
				// Don't do anything.. it's bad input.
				std::cout<<"\nUIFloatInput: Bad input, skipping.";
				return;
		}
		Graphics.Pause();
		input->SetText(result, true);
		Graphics.Resume();
	}
	IntegerMessage * m = new IntegerMessage(action, GetValue());
	MesMan.QueueMessage(m);
}


/// Creates the label and input.
void UIIntegerInput::CreateChildren()
{
	if (childrenCreated)
		return;
	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = new UIColumnList();
	box->padding = this->padding;
	AddChild(box);

	int elements = 1 + 1;
	float spaceLeft = 1.0f - padding * elements;
	float spacePerElement = spaceLeft / elements;

	/// Create a label
	label = new UILabel();
	label->text = name;
	label->sizeRatioX = spacePerElement;
	box->AddChild(label);

	/// Create 3 children
	input = new UIInput();
	input->textureSource = textureSource;
	/// Set them to only accept floats?
	input->name = name + "Input";
	/// Any mathematical expression?
	if (acceptMathematicalExpressions)
		input->mathematicalExpressionsOnly = true;
	/// Numbers only
	else
		input->numbersOnly = true;
	input->text = "0";
	input->sizeRatioX = spacePerElement;
	if (guiInputDisabled)
	{
		input->activateable = false;
		input->highlightOnHover = false;
	}
//	input->onTrigger = "UIIntegerInput("+name+")";
	box->AddChild(input);
	childrenCreated = true;
}

/// Getter/setter for the input element.
int UIIntegerInput::GetValue()
{
	return input->text.ParseInt();
}
void UIIntegerInput::SetValue(int value)
{
	input->SetText(String::ToString(value));
}


