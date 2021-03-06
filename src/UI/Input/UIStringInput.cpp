/// Emil Hedemalm
/// 2014-01-14
/// Multi-purpose input element.

#include "UIStringInput.h"
#include "UI/UITypes.h"
#include "UI/UILists.h"
#include "UIInput.h"

#include "Input/Keys.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"

#include "Graphics/GraphicsManager.h"
#include "UI/UILabel.h"

UIStringInput::UIStringInput(String name, String onTrigger)
: UIInput(), action(onTrigger)
{
	this->type = UIType::STRING_INPUT;
	this->name = name;
	input = NULL;
	label = NULL;
	labelText = name;
	rememberPreviousInputs = true;
	guiInputDisabled = false;
	index = 0;
	
	// Don't highlight this, highlight the sub-input element
	visuals.highlightOnHover = false;
	visuals.highlightOnActive = false;
	visuals.textureSource = "0x00000000";
}
UIStringInput::~UIStringInput()
{
}

List<std::pair<String, List<Text> > > previousUIInputTexts;

std::pair<String, List<Text>> * GetListForUI(String name)
{
	for (int i = 0; i < previousUIInputTexts.Size(); ++i)
	{
		std::pair<String, List<Text>> & p = previousUIInputTexts[i];
		if (p.first == name)
			return &p;
	}
	return 0;
}

/// Sent by UIInput elements upon pressing Enter and thus confirmign the new input, in case extra actions are warranted. (e.g. UITextureInput to update the texture provided as reference).
void UIStringInput::OnInputUpdated(GraphicsState* graphicsState, UIInput * inputElement)
{
	std::cout << "\nOnInputUpdated..." << inputElement;
	// Only logical thing should be our input calling us straight away.
	assert(inputElement == input);
	Text text = GetValue();
	/// For remembered inputs.
	if (rememberPreviousInputs)
	{
		// Save into list.
		bool saved = false;
		std::pair<String, List<Text>> * list = GetListForUI(name);
		if (list)
		{
			list->second.Insert(text, 0);
			if (list->second.Size() > 20)
				list->second.RemoveLast();
		}
		else
			previousUIInputTexts.AddItem(std::pair<String, List<Text>>(name, text));
	}
	index = -1;

	for (int i = 0; i < onTriggerActions.Size(); ++i)
		onTriggerActions[i].Process(graphicsState, this);

	// Post a SetString message accordingly.
	SetStringMessage * m = new SetStringMessage(action, text);
	MesMan.QueueMessage(m);
	/// Queue onTrigger messages too, if any.
	if (onTrigger.Length())
		MesMan.QueueMessages(onTrigger);
	return;

}

// For managing old texts.
UIInputResult UIStringInput::OnKeyDown(GraphicsState* graphicsState, int keyCode, bool downBefore)
{
	// Check for previous texts?
	int indexDesired = index;
	switch(keyCode)
	{
		case KEY::UP:  ++indexDesired; break;
		case KEY::DOWN: --indexDesired; break;
		default:
			UIInputResult result = input->OnKeyDown(graphicsState, keyCode, downBefore);
			if (result == UIInputResult::InputStopped)
				RemoveState(UIState::ACTIVE);
			return result;
	}
	if (indexDesired != index)
	{
		// Search n set.
		std::pair<String, List<Text>> * list = GetListForUI(name);
		if (list)
		{
			List<Text> & texts = list->second;
			ClampInt(indexDesired, -1, texts.Size()-1);
			Text t;
			if (indexDesired < 0)
				t = "";
			else 
				t = texts[indexDesired];
			input->SetEditText(t);
			index = indexDesired;
		}
	}
	return UIInputResult::TextUpdated;
}

/// Used for getting text. This will be local translated language key codes?
UIInputResult UIStringInput::OnChar(int asciiCode) {
	return input->OnChar(asciiCode);
}

// For sub-classes to adjust children as needed (mainly for input elements).
void UIStringInput::OnStateAdded(GraphicsState* graphicsState, int state) {
	label->AddStateSilently(state, true);
	input->AddState(graphicsState, state);
	UIInput::OnStateAdded(graphicsState, state);
}

bool UIStringInput::BeginInput(GraphicsState* graphicsState) {
	if (!input->InputActive())
		return input->BeginInput(graphicsState);
	else
		input->StopInput();
	return false;
}

/// Creates the label and input.
void UIStringInput::CreateChildren(GraphicsState * graphicsState)
{
	if (childrenCreated)
		return;

	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = CreateDefaultColumnList(this);
	float spacePerElement = DefaultSpacePerElement(layout.padding);
	label = CreateDefaultLabel(box, displayText, divider.x);
	label->rightBorderTextureSource = rightBorderTextureSource;

	input = CreateDefaultInput(box, name, 1 - divider.x);
	input->onTriggerActions = onTriggerActions;
	input->visuals.textureSource = inputTextureSource;

	/// Set them to only accept floats?
	input->SetText("");
	input->rememberPreviousInputs = true;
	input->text.alignment = LEFT;
	if (guiInputDisabled)
	{
		input->interaction.activateable = false;
		input->visuals.highlightOnHover = false;
		box->interaction.activateable = false;
		interaction.activateable = false;
		interaction.hoverable = false;
	}
	childrenCreated = true;
}
/// Getter/setter for the input element.
String UIStringInput::GetValue()
{
	return input->GetText();
}
void UIStringInput::SetValue(String value)
{
	input->SetText(value);
}

/// Calls UIElement::SetText in addition to setting the editText to the same value if force is true.
void UIStringInput::SetText(CTextr newText, bool force) {
	displayText = newText;
	if (label)
		label->SetText(newText, force);
}


