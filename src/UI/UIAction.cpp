/// Emil Hedemalm
/// 2015-11-30
/// Actions for UI. Meant to be used primarily on graphics/UI-thread.

#include "UIAction.h"
#include "UI/UIElement.h"
#include "UI/UITypes.h"
#include "UI/UIInputs.h"

UIAction::UIAction()
{
	Nullify();
}

UIAction::UIAction(int inType, int inTarget)
{
	Nullify();
	type = inType;
	target = inTarget;
}

UIAction::UIAction(int inType, UIElement * inTargetElement)
{
	Nullify();
	type = inType;
	targetElement = inTargetElement;
}

UIAction::UIAction(int inType, UIElement * inTargetElement, int inArgument)
{
	Nullify();
	type = inType;
	targetElement = inTargetElement;
	argument = inArgument;
}

void UIAction::Nullify()
{
	type = target = 0;
	targetElement = 0;
}

void UIAction::Process(GraphicsState* graphicsState, UIElement * forElement)
{
	if (targetElement == 0)
	{
		if (target == SELF)
			targetElement = forElement;
		else if (target == PARENT)
			targetElement = forElement->parent;
	}
	switch(type)
	{
		case OPEN_DROP_DOWN_MENU:
			if (targetElement->type == UIType::DROP_DOWN_MENU)
				((UIDropDownMenu*)targetElement)->Open(graphicsState);
			break;
		case CLOSE_DROP_DOWN_MENU:
			if (targetElement->type == UIType::DROP_DOWN_MENU)
				((UIDropDownMenu*)targetElement)->Close(graphicsState);
			break;
		case SELECT_DROP_DOWN_MENU:
			if (targetElement->type == UIType::DROP_DOWN_MENU)
				((UIDropDownMenu*)targetElement)->Select(argument);
	}
}
