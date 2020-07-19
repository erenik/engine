/// Emil Hedemalm
/// 2014-08-25
/// Combination element dedicated to handling input of float point numbers.


#include "UIFloatInput.h"
#include "MathLib/Expression.h"

#include "UI/UITypes.h"
#include "UI/UI.h"
#include "UI/UILists.h"
#include "UIInput.h"

#include "Message/MathMessage.h"
#include "Message/MessageManager.h"

#include "Graphics/GraphicsManager.h"

/// Class for 1 float inputs.
UIFloatInput::UIFloatInput(String name, String onTrigger)
: UIInput(), action(onTrigger)
{
	this->type = UIType::FLOAT_INPUT;
	this->name = name;
	input = NULL;
	label = NULL;
	maxDecimals = -1;
	acceptMathematicalExpressions = UI::defaultAcceptMathematicalExpressions;
	smallIncrement = 1.f;
}

UIFloatInput::~UIFloatInput(){
	// Nothing special, let base class handle children.
}

/** For mouse-scrolling. By default calls it's parent's OnScroll. Returns true if the element did anything because of the scroll.
	The delta corresponds to amount of "pages" it should scroll.
*/
bool UIFloatInput::OnScroll(GraphicsState* graphicsState, float delta)
{
	// Adjust if the input piece is being hovered over.
	if (input->HasState(UIState::HOVER))
	{
		float v = GetValue();
		if (delta > 0)
			v += smallIncrement;
		else 
			v -= smallIncrement;
		SetValue(v);
		/// Notify game state etc. of the change.
		FloatMessage * m = new FloatMessage(action, GetValue());
		MesMan.QueueMessage(m);
		return true;
	}
	// If not, do as regular UIElements do, probably query parents..
	return UIElement::OnScroll(graphicsState, delta);
}

/// Sent by UIInput elements upon pressing Enter and thus confirming the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIFloatInput::OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement)
{
	// Only logical thing should be our input calling us straight away.
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
				result = String(er.fResult);
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
	// Post a SetString message accordingly.
	FloatMessage * m = new FloatMessage(action, GetValue());
	MesMan.QueueMessage(m);
}


/// Creates the label and input.
void UIFloatInput::CreateChildren(GraphicsState* graphicsState)
{
	if (childrenCreated)
		return;

	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = CreateDefaultColumnList();
	float spacePerElement = DefaultSpacePerElement();
	label = CreateDefaultLabel(box, spacePerElement);
	input = CreateDefaultInput(box, spacePerElement);

	/// Inherit texture from parent by default, so highlighting works accordingly? Or highlight without any texture base...?
	/// ...
	/// Any mathematical expression?
	if (acceptMathematicalExpressions)
		input->mathematicalExpressionsOnly = true;
	/// Numbers only
	else
		input->numbersOnly = true;
	input->text = "0";
	input->sizeRatioX = spacePerElement;
//	input->onTrigger = "UIFloatInput("+name+")";
	childrenCreated = true;
}

/// Getter/setter for the input element.
float UIFloatInput::GetValue()
{
	return input->text.ParseFloat();
}
void UIFloatInput::SetValue(float value)
{
	input->SetText(String::ToString(value, maxDecimals));
}


