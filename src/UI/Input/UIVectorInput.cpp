/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#include "UIVectorInput.h"
#include "UI/UI.h"
#include "UI/UITypes.h"
#include "UIInput.h"
#include "UI/Lists/UIColumnList.h"

#include "Message/MathMessage.h"
#include "Message/MessageManager.h"

#include "MathLib/Expression.h"

///
UIVectorInput::UIVectorInput(int numInputs, String name, String onTrigger)
: UIInput()
{
	type = UIType::VECTOR_INPUT;
	this->numInputs = numInputs;
	this->name = name;
	this->action = onTrigger;
	maxDecimals = -1;
	dataType = FLOATS;

	labelText = name; // Default value.

	activationActions.Add(UIAction(UIAction::BEGIN_VECTOR_INPUT, this));

}
UIVectorInput::~UIVectorInput()
{

}

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIVectorInput::OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement)
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
void UIVectorInput::CreateChildren(GraphicsState* graphicsState)
{
	if (childrenCreated)
		return;

	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = new UIColumnList();
	box->padding = this->padding;
	AddChild(nullptr, box);



	/// Create a label
	label = new UILabel();
	this->InheritDefaults(label);
	label->SetText(labelText);
	label->sizeRatioX = divider.x;
	box->AddChild(nullptr, label);

	float spaceLeft = 1.0f - divider.x - padding * numInputs;
	float spacePerElement = spaceLeft / numInputs;

	/// Create 2-3 children
	for (int i = 0; i < numInputs; ++i)
	{
		UIInput * input = new UIInput();
		this->InheritDefaults(input);

		/// Set them to only accept floats?
		input->name = name + "Input"+String::ToString(i);
		input->numbersOnly = true;
		input->SetText("0");
		input->sizeRatioX = spacePerElement;
		input->textAlignment = RIGHT;
	//	input->onTrigger = "UIVectorInput("+name+")";
		box->AddChild(nullptr, input);
		inputs.Add(input);
	}
	childrenCreated = true;
}	

/// Begins input, returns false if not possible (e.g. non-activatable StringLabel input)
bool UIVectorInput::BeginInput(GraphicsState* graphicsState) {
	assert(inputs.Size());
	RemoveState(UIState::ACTIVE); // Remove active from self.
	return inputs[0]->BeginInput(graphicsState); // Begin input on child.
}

/// Calls UIElement::SetText in addition to setting the editText to the same value if force is true.
void UIVectorInput::SetText(CTextr newText, bool force /*= false*/) {
	labelText = newText;
	if (label)
		label->SetText(newText);
}

/// Getters
Vector2i UIVectorInput::GetValue2i()
{
	int arr[2];
	for (int i = 0; i < inputs.Size() && i < 2; ++i){
		arr[i] = inputs[i]->ParseInt();
	}
	return Vector2i(arr);
}
Vector2f UIVectorInput::GetValue2f()
{
	float arr[2];
	for (int i = 0; i < inputs.Size() && i < 2; ++i){
		arr[i] = inputs[i]->ParseFloat();
	}
	return Vector2f(arr);
}
Vector3f UIVectorInput::GetValue3f()
{
	float arr[3];
	for (int i = 0; i < inputs.Size() && i < 3; ++i){
		arr[i] = inputs[i]->ParseFloat();
	}
	return Vector3f(arr);
}
Vector4f UIVectorInput::GetValue4f()
{
	float arr[4];
	for (int i = 0; i < inputs.Size() && i < 4; ++i){
		arr[i] = inputs[i]->ParseFloat();
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

void UIVectorInput::SetValue3f(const Vector3f & vec)
{
	for (int i = 0; i < inputs.Size() && i < 3; ++i)
	{
		String s = String::ToString(vec[i], maxDecimals);
		inputs[i]->SetText(s);
	}
}
void UIVectorInput::SetValue4f(const Vector4f & vec)
{
	for (int i = 0; i < inputs.Size() && i < 4; ++i){
		String s = String::ToString(vec[i], maxDecimals);
		inputs[i]->SetText(s);
	}
}



/** For mouse-scrolling. By default calls it's parent's OnScroll. Returns true if the element did anything because of the scroll.
	The delta corresponds to amount of "pages" it should scroll.
*/
bool UIVectorInput::OnScroll(GraphicsState* graphicsState, float delta)
{
	for (int i = 0; i < inputs.Size(); ++i)
	{
		UIInput * input = inputs[i];
		// Adjust if the input piece is being hovered over.
		if (input->HasState(UIState::HOVER))
		{
			if (dataType == INTEGERS)
			{
				int v = input->ParseInt();
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
				float v = input->ParseFloat();
				if (delta > 0)
					v += diff;
				else 
					v -= diff;
				input->SetText(String(v, maxDecimals));	
			}
			/// Notify game state etc. of the change.
			OnInputUpdated(graphicsState, input);
			return true;
		}
	}
	// If not, do as regular UIElements do, probably query parents..
	return UIElement::OnScroll(graphicsState, delta);
}

/// See dataTypes below.
void UIVectorInput::SetDataType(int newDataType)
{
	this->dataType = newDataType;
}

// For sub-classes to adjust children as needed (mainly for input elements).
void UIVectorInput::OnStateAdded(GraphicsState* graphicsState, int state) {
	if (state == UIState::HOVER)
		label->AddStateSilently(state, true);
	for (int i = 0; i < inputs.Size(); ++i)
		inputs[i]->AddStateSilently(state);
	UIElement::OnStateAdded(graphicsState, state);
}

