// Emil Hedemalm
// 2020-08-20
// A checkbox, containing the label, and a toggle-button.

#include "UICheckbox.h"

#include "UI/UILists.h"
#include "UI/Buttons/UIToggleButton.h"
#include "UI/UIInputs.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"


UICheckbox::UICheckbox(String in_name /*= ""*/)
	: UIElement(in_name)
	, button(nullptr)
{
	Nullify();
	textureSource = defaultTextureSource;
	activationMessage = "SetBool:" + name;
	text = name;
	type = UIType::CHECKBOX;
	selectable = true;
	hoverable = true;
	navigatable = true;
	activateable = true;

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
	if (visible == false)
		return 0;

	// Check the element's state. If it is active, we've found it. Dialogues work too, or?
	if (HasState(UIState::ACTIVE))
	{
		/// Just unflag the active state, try ensure that the hover-state remains!
		RemoveState(UIState::ACTIVE);
		// Now return our message!
		if (selectable == true) {
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


void UICheckbox::OnStateAdded(int state) {
	if (state == UIState::ACTIVE) {
		button->SetToggled(!button->IsToggled());
		RemoveState(state);
	}
	if (label)
		label->AddState(state);
}

void UICheckbox::SetToggled(bool value) {
	if (button)
		button->SetToggled(value);
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
	float spacePerElement = UIInput::DefaultSpacePerElement(padding);
	label = UIInput::CreateDefaultLabel(box, displayText, spacePerElement);
	
	/// Create 3 children
	button = new UIToggleButton();
	box->InheritDefaults(button);
	button->name = name + "Toggle";
	button->sizeRatioX = spacePerElement;
	button->onToggledTexture = "ui/checkbox_toggled";
	button->onNotToggledTexture = "ui/checkbox_not_toggled";
	button->retainAspectRatioOfTexture = true;
	box->AddChild(nullptr, button);
}

void UICheckbox::SetToggleTexts(String on, String off) {
	toggleTextOn = on;
	toggleTextOff = off;

	button->onToggledTexture = "";
	button->onNotToggledTexture = "";

	SetToggled(button->IsToggled());
}
