/// Emil Hedemalm
/// 2014-08-31
/// Aggregate input class which handles a number of buttons

#include "UIRadioButtons.h"
#include "UI/UITypes.h"
#include "Message/MessageManager.h"
#include "Message/MathMessage.h"
// #include "Graphics/GraphicsManager.h"

/// o.o
UIRadioButtons::UIRadioButtons(int numberOfButtons, String name, String action)
: UIColumnList(), numButtons(numberOfButtons), action(action)
{
	this->name = name;
	this->type = UIType::RADIO_BUTTONS;
}

/// Creates the actual buttons.
void UIRadioButtons::CreateChildren()
{
	assert(children.Size() == 0);
	int elements = numButtons;
	float spaceLeft = 1.0f - padding * elements;
	float spacePerElement = spaceLeft / elements;

	for (int i = 0; i < numButtons; ++i)
	{
		/// Create 3 children
		UICheckBox * button = new UICheckBox();
		/// Set them to only accept floats?
		button->name = name + "Input";
		button->text = names.Size() > i ? names[i] : "NoName";
		button->sizeRatioX = spacePerElement;
		/// Pre-select first one always.
		if (i == 0)
			button->toggled = true;
	//	input->onTrigger = "UIFloatInput("+name+")";
		AddChild(button);
		buttons.Add(button);
	}
}


/// Sets the texts of the children.
void UIRadioButtons::SetTexts(List<String> texts)
{
	for (int i = 0; i < buttons.Size() && i < texts.Size(); ++i)
	{
		UICheckBox * button = buttons[i];
		String text = texts[i];
		button->SetText(text);
	}
}

/// Sent when a child checkbox is toggled. 
void UIRadioButtons::OnToggled(UICheckBox * box)
{
	assert(buttons.Exists(box));
	/// Send a message based on which button it was.
	int index = buttons.GetIndexOf(box);
	/// Set a message!
	IntegerMessage * im = new IntegerMessage(action, index);
	MesMan.QueueMessage(im);
	/// De-toggle the other elements.
	for (int i = 0; i < buttons.Size(); ++i)
	{
		UICheckBox * button = buttons[i];
		if (button == box)
			continue;
		button->toggled = false;
	}
}

