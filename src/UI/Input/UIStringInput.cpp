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
	std::cout<<"\nOnInputUpdated..."<<inputElement;
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
	// Post a SetString message accordingly.
	SetStringMessage * m = new SetStringMessage(action, text);
	MesMan.QueueMessage(m);
	/// Queue onTrigger messages too, if any.
	if (onTrigger.Length())
		MesMan.QueueMessages(onTrigger);
	return;

}

// For managing old texts.
int UIStringInput::OnKeyDown(GraphicsState* graphicsState, int keyCode, bool downBefore)
{
	// Check for previous texts?
	int indexDesired = index;
	switch(keyCode)
	{
		case KEY::UP:  ++indexDesired; break;
		case KEY::DOWN: --indexDesired; break;
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
	return 0;
}


/// Creates the label and input.
void UIStringInput::CreateChildren(GraphicsState * graphicsState)
{
	if (childrenCreated)
		return;

	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = CreateDefaultColumnList();
	float spacePerElement = DefaultSpacePerElement();
	label = CreateDefaultLabel(box, spacePerElement);
	input = CreateDefaultInput(box, spacePerElement);

	/// Set them to only accept floats?
	input->text = "";
	input->rememberPreviousInputs = true;
	input->textAlignment = CENTER;
	if (guiInputDisabled)
	{
		input->activateable = false;
		input->highlightOnHover = false;
		box->activateable = false;
		box->hoverable = false;
		activateable = false;
		hoverable = false;
	}
	childrenCreated = true;
}
/// Getter/setter for the input element.
String UIStringInput::GetValue()
{
	return input->text;
}
void UIStringInput::SetValue(String value)
{
	input->SetText(value);
}

