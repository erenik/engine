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

UIRadioButtons::~UIRadioButtons()
{
}


/// Creates the actual buttons.
void UIRadioButtons::CreateChildren()
{
	assert(children.Size() == 0);
	int elements; 
	float spaceLeft;
	float spacePerElement;
	float labelSize;
	labelSize = divider.x;
	elements = numButtons;
	spaceLeft = (1.0f - divider.x) - padding * elements;
	spacePerElement = spaceLeft / elements;
	// Add label!
	UILabel * label = new UILabel(text);
	label->sizeRatioX = labelSize;
	label->text = displayText;
	AddChild(label);

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

// Set texture for all elements.
void UIRadioButtons::SetTextureSource(String source)
{
	for (int i = 0; i < buttons.Size(); ++i)
	{
		UICheckBox * button = buttons[i];
		button->textureSource = source;
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
	bool somethingToggled = false;
	/// De-toggle the other elements.
	for (int i = 0; i < buttons.Size(); ++i)
	{
		UICheckBox * button = buttons[i];
		somethingToggled |= button->toggled;
		if (button == box)
			continue;
		button->toggled = false;
	}
	/// Ensure always one radio button is toggled.
	if (!somethingToggled)
		box->toggled = true;
}

/// Toggles appropriately.
void UIRadioButtons::SetValue(int v)
{
	if (v >= buttons.Size() || v < 0)
	{
		assert(false);
		return;
	}
	for (int i = 0; i < buttons.Size(); ++i)
	{
		buttons[i]->toggled = false;
	}
	buttons[v]->toggled = true;
}


