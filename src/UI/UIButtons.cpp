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
