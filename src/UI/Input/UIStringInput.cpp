/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#include "UIStringInput.h"
#include "UI/UITypes.h"
#include "UI/UIList.h"
#include "UIInput.h"

#include "Message/Message.h"
#include "Message/MessageManager.h"

#include "Graphics/GraphicsManager.h"

UIStringInput::UIStringInput(String name, String onTrigger)
: UIElement(),action(onTrigger)
{
	this->type = UIType::STRING_INPUT;
	this->name = name;
	input = NULL;
	label = NULL;
}
UIStringInput::~UIStringInput()
{
}

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIStringInput::OnInputUpdated(UIInput * inputElement)
{
	// Only logical thing should be our input calling us straight away.
	assert(inputElement == input);
	// Post a SetString message accordingly.
	SetStringMessage * m = new SetStringMessage(action, GetValue());
	MesMan.QueueMessage(m);
	return;

}

/// Creates the label and input.
void UIStringInput::CreateChildren()
{
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
	label->sizeRatioX = 0.3f;
	box->AddChild(label);

	/// Create 3 children
	input = new UIInput();
	/// Set them to only accept floats?
	input->name = name + "Input";
	input->text = "";
	input->sizeRatioX = 0.65f;
	input->onTrigger = "UIStringInput("+name+")";
	box->AddChild(input);
}
/// Getter/setter for the input element.
String UIStringInput::GetValue()
{
	return input->text;
}
void UIStringInput::SetValue(String value)
{
	Graphics.Pause();
	this->SetText(value);
	Graphics.Resume();
}

