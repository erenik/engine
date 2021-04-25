// Emil Hedemalm
// 2013-08-03

#include "UITypes.h"
#include "UIButtons.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"
#include "File/LogFile.h"

UIButton::UIButton(String i_name)
: UIElement()
{
	type = UIType::BUTTON;
	selectable = true;
	hoverable = true;
	navigatable = true;
	activateable = true;
	SetText(i_name);
	name = activationMessage = i_name;
	textureSource = defaultTextureSource;
};

UIButton::~UIButton()
{
//	std::cout<<"\nUIButton destructor";
}

/// Callback-function for sub-classes to implement own behaviour to run within the UI-class' code. Return true if it did something.
bool UIButton::OnProceed(GraphicsState* graphicsState) {
	if (HasState(UIState::DISABLED)) {
		LogGraphics("Button "+name+" disabled. ignoring OnProceed", INFO);
		return false;
	}

	bool didSomething = false;
	if (activationMessage.Length() > 0) {
		MesMan.QueueMessages(activationMessage);
		didSomething = true;
	}
	if (activationActions.Size() > 0) {
		for (int i = 0; i < activationActions.Size(); ++i) {
			activationActions[i].Process(graphicsState, this);
			didSomething = true;
		}
	}
	if (didSomething)
		return true;
	return UIElement::OnProceed(graphicsState);
}

UICompositeButton::UICompositeButton(String name) 
: UIButton(name) {
	SetText("");
}

UICompositeButton::~UICompositeButton() {
}

/// Creates a deep copy of self and all child elements (where possible).
UIElement * UICompositeButton::Copy(){
	UICompositeButton * copy = new UICompositeButton(name);
	//*copy = *this; // Copy all variables?
	assert(false && "Implement deep copy");
	CopyChildrenInto(copy);
	return copy;
}

// For sub-classes to adjust children as needed (mainly for input elements).
void UICompositeButton::OnStateAdded(GraphicsState* graphicsState, int state) {
	for (int i = 0; i < children.Size(); ++i) {
		UIElement * child = children[i];
		child->AddState(graphicsState, state, true); // Always add state.
	}
}

/// For example UIState::HOVER, if recursive will apply to all children.
void UICompositeButton::RemoveState(int state, bool recursive) {
	UIButton::RemoveState(state, true); // Always remove state.
}

bool UICompositeButton::AddChild(GraphicsState* graphicsState, UIElement *in_child) {
	bool result = UIButton::AddChild(graphicsState, in_child);
	in_child->hoverable = in_child->activateable = false;
	return result;
}

/// Callback-function for sub-classes to implement own behaviour to run within the UI-class' code. Return true if it did something.
bool UICompositeButton::OnProceed(GraphicsState* graphicsState)
{
	MesMan.QueueMessages(activationMessage);
	return true;
}

