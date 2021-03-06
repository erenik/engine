// Emil Hedemalm
// 2020-08-20
// A checkbox, containing the label, and a toggle-button.

#include "UICheckbox.h"

#include "UI/UILists.h"
#include "UI/Buttons/UIToggleButton.h"
#include "UI/UIInputs.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"
#include "UI/UILabel.h"


UICheckbox::UICheckbox(String in_name /*= ""*/)
	: UIElement(in_name)
	, button(nullptr)
	, label(nullptr)
{
	Nullify();
	activationMessage = "SetBool:" + name;
	SetText("", false);
	displayText = name;
	type = UIType::CHECKBOX;
	interaction.selectable = true;
	interaction.hoverable = true;
	interaction.navigatable = true;
	interaction.activateable = true;
	interaction.togglable = true;

	/*
	// Set default texture to get proper animation for when hovering/activating it.
	//text = name;
	*/
};

UICheckbox::~UICheckbox()
{
	//	std::cout<<"\nUICheckBox destructor";
}

UIElement* UICheckbox::Activate(GraphicsState* graphicsState)
{
	UIElement* result = 0;
	// Don't process invisible UIElements, please.
	if (interaction.visible == false)
		return 0;

	// Check the element's state. If it is active, we've found it. Dialogues work too, or?
	if (HasState(UIState::ACTIVE))
	{
		/// Just unflag the active state, try ensure that the hover-state remains!
		RemoveState(UIState::ACTIVE);
		// Now return our message!
		if (interaction.selectable == true) {
			SetToggled(!button->IsToggled());
			OnToggled(button);
			// Send a message!
			BoolMessage * msg = new BoolMessage(activationMessage, button->IsToggled());
			MesMan.QueueMessage(msg);
		}
		else {
			// Element not activatable
		}
		return this;
	}
	// If not, return 0, since we haven't found the right element.
	return 0;
}


void UICheckbox::OnStateAdded(GraphicsState* graphicsState, int state) {
	if (state == UIState::ACTIVE) {
		button->SetToggled(!button->IsToggled());
		RemoveState(state);
	}
	button->AddState(graphicsState, state);
	if (label && state == UIState::HOVER)
		label->AddState(graphicsState, state, true);
}

void UICheckbox::SetToggled(bool value) {
	if (button)
		button->SetToggled(value);
}

void UICheckbox::SetText(CTextr text, bool force) {
	displayText = text;
	if (label)
		label->SetText(text);
}


void UICheckbox::OnToggled(UIElement * button) {
	switch(button->type) {
	case UIType::TOGGLE_BUTTON:
	case UIType::CHECKBOX:
		_onToggled();
		break;
	default:
		assert(false && "Implement");
	}
}

void UICheckbox::_onToggled() {
	// Check own button only.
	UIToggleButton* toggleButton = button;
	if (toggleButton == nullptr)
		return;
	BoolMessage * msg = new BoolMessage(activationMessage, toggleButton->IsToggled());
	MesMan.QueueMessage(msg);
	if (toggleTextOn.Length() > 0) {
		toggleButton->SetText(toggleButton->IsToggled() ? toggleTextOn : toggleTextOff);
	}
}

void UICheckbox::CreateChildren() {

	UIColumnList * box = UIInput::CreateDefaultColumnList(this);
	float spacePerElement = UIInput::DefaultSpacePerElement(layout.padding);
	label = UIInput::CreateDefaultLabel(box, displayText, spacePerElement);
	
	/// Create 3 children
	button = new UIToggleButton();
	box->InheritDefaults(button);
	button->name = name + "Toggle";
	button->layout.sizeRatioX = spacePerElement;
	button->onToggledTexture = "ui/checkbox_toggled";
	button->onNotToggledTexture = "ui/checkbox_not_toggled";
	button->visuals.retainAspectRatioOfTexture = true;
	box->AddChild(nullptr, button);
}

void UICheckbox::SetToggleTexts(String on, String off) {
	toggleTextOn = on;
	toggleTextOff = off;

	button->onToggledTexture = "";
	button->onNotToggledTexture = "";

	SetToggled(button->IsToggled());
}
