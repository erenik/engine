/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#include "UIVectorInput.h"
#include "UI/UI.h"
#include "UI/UITypes.h"
#include "UIInput.h"
#include "UI/UIList.h"

#include "Message/MathMessage.h"
#include "Message/MessageManager.h"

#include "MathLib/Expression.h"

///
UIVectorInput::UIVectorInput(int numInputs, String name, String onTrigger)
: UIElement()
{
	type = UIType::VECTOR_INPUT;
	this->numInputs = numInputs;
	this->name = name;
	this->action = onTrigger;
	maxDecimals = 3;
	dataType = FLOATS;
}
UIVectorInput::~UIVectorInput()
{

}

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIVectorInput::OnInputUpdated(UIInput * inputElement)
{
	assert(this->inputs.Exists(inputElement));
	/// Fetch vector data from the input first.
	VectorMessage * m = NULL;
	switch(numInputs)
	{
		case 2:
			if (dataType == INTEGERS)
				m = new VectorMessage(action, GetValue2i());
			else 
				m = new VectorMessage(action, GetValue2f());
			break;
		case 3:
			m = new VectorMessage(action, GetValue3f());
			break;
		case 4:
			m = new VectorMessage(action, GetValue4f());
			break;
		default:
			assert(false && "implement");
			break;
	}
	if (m)
		MesMan.QueueMessage(m);
}

/// Creates ze children!
void UIVectorInput::CreateChildren()
{
	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = new UIColumnList();
	box->padding = this->padding;
	AddChild(box);

	int elements = numInputs + 1;
	float spaceLeft = 1.0f - padding * elements;
	float spacePerElement = spaceLeft / elements;

	/// Create a label
	label = new UILabel();
	label->text = name;
	label->sizeRatioX = spacePerElement;
	box->AddChild(label);

	/// Create 3 children
	for (int i = 0; i < numInputs; ++i){
		UIInput * input = new UIInput();
		/// Set them to only accept floats?
		input->name = name + "Input"+String::ToString(i);
		input->numbersOnly = true;
		input->text = "0";
		input->sizeRatioX = spacePerElement;
	//	input->onTrigger = "UIVectorInput("+name+")";
		box->AddChild(input);
		inputs.Add(input);
	}
}	
/// Getters
Vector2i UIVectorInput::GetValue2i()
{
	int arr[2];
	for (int i = 0; i < inputs.Size() && i < 2; ++i){
		arr[i] = inputs[i]->text.ParseInt();
	}
	return Vector2i(arr);
}
Vector2f UIVectorInput::GetValue2f()
{
	float arr[2];
	for (int i = 0; i < inputs.Size() && i < 2; ++i){
		arr[i] = inputs[i]->text.ParseFloat();
	}
	return Vector2f(arr);
}
Vector3f UIVectorInput::GetValue3f()
{
	float arr[3];
	for (int i = 0; i < inputs.Size() && i < 3; ++i){
		arr[i] = inputs[i]->text.ParseFloat();
	}
	return Vector3f(arr);
}
Vector4f UIVectorInput::GetValue4f()
{
	float arr[4];
	for (int i = 0; i < inputs.Size() && i < 4; ++i){
		arr[i] = inputs[i]->text.ParseFloat();
	}
	return Vector4f(arr);
}	
/// Setters
void UIVectorInput::SetValue2i(Vector2i vec)
{
	for (int i = 0; i < inputs.Size() && i < 2; ++i){
		String s = String::ToString(vec[i]);
		inputs[i]->SetText(s);
	}
}
void UIVectorInput::SetValue2f(Vector2f vec)
{
	for (int i = 0; i < inputs.Size() && i < 2; ++i){
		String s = String::ToString(vec[i], maxDecimals);
		inputs[i]->SetText(s);
	}
}

void UIVectorInput::SetValue3f(Vector3f vec)
{
	for (int i = 0; i < inputs.Size() && i < 3; ++i){
		String s = String::ToString(vec[i], maxDecimals);
		inputs[i]->SetText(s);
	}
}
void UIVectorInput::SetValue4f(Vector4f vec)
{
	for (int i = 0; i < inputs.Size() && i < 4; ++i){
		String s = String::ToString(vec[i], maxDecimals);
		inputs[i]->SetText(s);
	}
}



/** For mouse-scrolling. By default calls it's parent's OnScroll. Returns true if the element did anything because of the scroll.
	The delta corresponds to amount of "pages" it should scroll.
*/
bool UIVectorInput::OnScroll(float delta)
{
	for (int i = 0; i < inputs.Size(); ++i)
	{
		UIInput * input = inputs[i];
		// Adjust if the input piece is being hovered over.
		if (input->state & UIState::HOVER)
		{
			if (dataType == INTEGERS)
			{
				int v = input->text.ParseInt();
				if (delta > 0)
					++v;
				else 
					--v;
				input->SetText(String(v));
			}
			// Floating point.
			else 
			{
				float diff = 0.05f;
				float v = input->text.ParseFloat();
				if (delta > 0)
					v += diff;
				else 
					v -= diff;
				input->SetText(String(v, maxDecimals));	
			}
			/// Notify game state etc. of the change.
			OnInputUpdated(input);
			return true;
		}
	}
	// If not, do as regular UIElements do, probably query parents..
	return UIElement::OnScroll(delta);
}