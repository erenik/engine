// Emil Hedemalm
// 2020-08-20
// A button which is togglable, usually just using images or other graphics to show it is enabled / disabled.

#include "UIToggleButton.h"

#include "Message/Message.h"
#include "Message/MessageManager.h"

String UIToggleButton::defaultOnToggledTexture = "0x77FF";
String UIToggleButton::defaultOnNotToggledTexture = "0x44FF";

UIToggleButton::UIToggleButton(String name /*= ""*/)
	: UIElement()
	, toggled(false)
{
	Nullify();
	// Set default texture to get proper animation for when hovering/activating it.
	textureSource = defaultTextureSource;

	// Default toggle button - bright and dark backgrounds.
	onToggledTexture = defaultOnToggledTexture;
	onNotToggledTexture = defaultOnNotToggledTexture;

	this->name = name;
	SetText(name);
	activationMessage = "SetBool:" + name;
	type = UIType::CHECKBOX;
	selectable = true;
	hoverable = true;
	navigatable = true;
	activateable = true;
	toggled = false;

	this->highlightOnActive = true;
	this->highlightOnHover = true;

	UpdateTexture();
	UpdateTextColor();
};

UIToggleButton::~UIToggleButton()
{
	//	std::cout<<"\nUICheckBox destructor";
}

UIElement* UIToggleButton::Activate(GraphicsState* graphicsState)
{
	UIElement* result = 0;
	// Don't process invisible UIElements, please.
	if (visible == false)
		return 0;

	// Assume no checkbox has any children.

	// Check the element's state. If it is active, we've found it. Dialogues work too, or?
	if (HasState(UIState::ACTIVE))
	{
		RemoveState(UIState::ACTIVE); // Logic is handled when Active state is removed.
		return this;
	}
	// If not, return 0, since we haven't found the right element.
	return 0;
}

void UIToggleButton::SetToggled(bool value) { 
	toggled = value; 
	this->OnToggled(this);
}

// Sets the flag, but does not call the OnToggled event.
void UIToggleButton::SetToggledSilently(bool value) {
	toggled = value;
	UpdateTexture();
	UpdateTextColor();
}

void UIToggleButton::OnToggled(UIToggleButton * button) {
	UpdateTexture(); 
	UpdateTextColor();

	// Inform parents if need be.
	UIElement::OnToggled(button);
}

void UIToggleButton::OnStateAdded(int state) {
	UIElement::OnStateAdded(state);
}

/// For example UIState::HOVER, if recursive will apply to all children.
void UIToggleButton::RemoveState(int state, bool recursive /* = false */ ) {
	if (state == UIState::ACTIVE && HasState(state)) {
		SetToggled(!toggled);
		parent->OnToggled(this);
		// Send a message!
		BoolMessage * msg = new BoolMessage(activationMessage, toggled);
		MesMan.QueueMessage(msg);
	}
	UIElement::RemoveState(state, recursive);
}


void UIToggleButton::UpdateTexture() {
	this->texture = nullptr; // Force update of pointer to texture next frame.
	textureSource = toggled? onToggledTexture : onNotToggledTexture;
}

void UIToggleButton::UpdateTextColor() {
	SetTextColor(toggled ? toggledTextColor : notToggledTextColor);
}

