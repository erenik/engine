// Emil Hedemalm
// 2013-08-03

#include "UITypes.h"
#include "UIButtons.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"

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
	GetText().color = defaultTextColor;
};

UIButton::~UIButton()
{
//	std::cout<<"\nUIButton destructor";
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
	*copy = *this; // Copy all variables?
	CopyChildrenInto(copy);
	return copy;
}

// For sub-classes to adjust children as needed (mainly for input elements).
void UICompositeButton::OnStateAdded(int state) {
	for (int i = 0; i < children.Size(); ++i) {
		UIElement * child = children[i];
		child->AddState(state, true); // Always add state.
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
