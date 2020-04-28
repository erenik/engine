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
	activateable = true;
	name = text = activationMessage = i_name;
	textureSource = defaultTextureSource;
	text.color = defaultTextColor;
};

UIButton::~UIButton()
{
//	std::cout<<"\nUIButton destructor";
}

UICheckBox::UICheckBox(String name /*= ""*/)
: UIElement()
{
	Nullify();
	// Set default texture to get proper animation for when hovering/activating it.
	textureSource = defaultTextureSource;
	this->name = name;
	text = name;
	activationMessage = "SetBool:"+name;
	type = UIType::CHECKBOX;
	selectable = true;
	hoverable = true;
	activateable = true;
	toggled = false;
};

UICheckBox::~UICheckBox()
{
//	std::cout<<"\nUICheckBox destructor";
}

UIElement* UICheckBox::Activate(GraphicsState* graphicsState)
{
	UIElement* result = 0;
	// Don't process invisible UIElements, please.
	if (visible == false)
		return 0;
	
	// Assume no checkbox has any children.

	// Check the element's state. If it is active, we've found it. Dialogues work too, or?
	if (state & UIState::ACTIVE)
	{
		/// Just unflag the active state, try ensure that the hover-state remains!
		state &= ~UIState::ACTIVE;
		// Now return our message!
		if (selectable == true){
			toggled = !toggled;
			OnToggled(this);
			// Send a message!
			BoolMessage * msg = new BoolMessage(activationMessage, toggled);
			MesMan.QueueMessage(msg);
			/*
			if (type== UIType::CHECKBOX){
				selected = !selected;
			}
			else if (type== UIType::RADIOBUTTON){
				parent->DeselectAll();
				selected = true;
			}
			else {
				//	selected = true;
			}
			*/
		}
		else {
			// Element not activatable
		}
		return this;
	}
	// If not, return 0, since we haven't found the right element.
	return 0;
}