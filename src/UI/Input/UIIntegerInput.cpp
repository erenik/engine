/// Emil Hedemalm
/// 2014-08-25
/// Combination element dedicated to handling input of integers.

#include "UIIntegerInput.h"

#include "MathLib/Expression.h"

#include "UI/UITypes.h"
#include "UI/UI.h"
#include "UI/UILists.h"
#include "UIInput.h"

#include "Input/Keys.h"

#include "Message/MathMessage.h"
#include "Message/MessageManager.h"

#include "Graphics/GraphicsManager.h"

/// Class for 1 Integer inputs.
UIIntegerInput::UIIntegerInput(String name, String onTrigger)
: UIInput(), action(onTrigger)
{
	this->type = UIType::INTEGER_INPUT;
	this->name = name;
	input = NULL;
	label = NULL;
	guiInputDisabled = false;
	acceptMathematicalExpressions = false;
	textColor = nullptr;
}
UIIntegerInput::~UIIntegerInput()
{
	// Nothing special, let base class handle children.
}

/** For mouse-scrolling. By default calls it's parent's OnScroll. Returns true if the element did anything because of the scroll.
	The delta corresponds to amount of "pages" it should scroll.
*/
bool UIIntegerInput::OnScroll(GraphicsState* graphicsState, float delta)
{
	if (guiInputDisabled)
	{
		/// Go to parent as usual.
		return UIElement::OnScroll(graphicsState, delta);
	}
	// Adjust if the input piece is being hovered over.
	if (input->HasState(UIState::HOVER))
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
	return UIElement::OnScroll(graphicsState, delta);
}

/** Used by input-capturing elements. Calls recursively upward until an element wants to respond to the input.
	Returns 1 if it processed anything, 0 if not.
*/
UIInputResult UIIntegerInput::OnKeyDown(GraphicsState* graphicsState, int keyCode, bool downBefore)
{
	if (input->InputActive()) {
		return input->OnKeyDown(graphicsState, keyCode, downBefore);
	}

	return UIElement::OnKeyDown(graphicsState, keyCode, downBefore);
}	

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIIntegerInput::OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement)
{
	assert(inputElement == input);
	/// If we accept expressions, parse and evaluate it?
	if (acceptMathematicalExpressions)
	{
		String text = input->GetText();
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
		input->SetText(result, true);
	}
	IntegerMessage * m = new IntegerMessage(action, GetValue());
	MesMan.QueueMessage(m);
}

// For sub-classes to adjust children as needed (mainly for input elements).
void UIIntegerInput::OnStateAdded(int state) {
	if (state == UIState::HOVER)
		input->AddState(state);
	else if (state == UIState::ACTIVE) {
		input->BeginInput();
	}
}

void UIIntegerInput::SetRange(int newMin, int newMax) {
	min = newMin;
	max = newMax;
	input->SetRange(min, max);
}


// When navigating, either via control, or arrow keys or whatever.
void UIIntegerInput::Navigate(NavigateDirection direction) {
	if (HasStateRecursive(UIState::ACTIVE)) {
		input->Navigate(direction);
	}
}

/// Halts input and removes Active state.
void UIIntegerInput::StopInput() {
	input->StopInput();
}


/// Creates the label and input.
void UIIntegerInput::CreateChildren(GraphicsState* graphicsState)
{
	if (childrenCreated)
		return;
	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = CreateDefaultColumnList(this);
	float spacePerElement = DefaultSpacePerElement(padding);
	label = CreateDefaultLabel(box, displayText, divider.x);
	label->rightBorderTextureSource = rightBorderTextureSource;
	label->textAlignment = LEFT;
	//label->textureSource = "0x554433";
	input = CreateDefaultInput(box, name, 1 - divider.x);
	input->textureSource = inputTextureSource;
	//input->textureSource = "0x334455";

	label->SetTextColor(textColor);
	input->SetTextColor(textColor);

	/// Set them to only accept floats?
	/// Any mathematical expression?
	if (acceptMathematicalExpressions)
		input->mathematicalExpressionsOnly = true;
	/// Numbers only
	else
		input->numbersOnly = true;
	input->SetText("0");
	input->textAlignment = RIGHT;
	if (guiInputDisabled)
	{
		input->activateable = false;
		input->highlightOnHover = false;
	}
//	input->onTrigger = "UIIntegerInput("+name+")";
	childrenCreated = true;
}

/// Getter/setter for the input element.
int UIIntegerInput::GetValue()
{
	return input->GetText().ParseInt();
}
void UIIntegerInput::SetValue(int value)
{
	input->SetText(String::ToString(value));
}


