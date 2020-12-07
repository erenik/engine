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
