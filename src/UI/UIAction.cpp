/// Emil Hedemalm
/// 2015-11-30
/// Actions for UI. Meant to be used primarily on graphics/UI-thread.

#include "UIAction.h"
#include "UI/UIElement.h"
#include "UI/UITypes.h"
#include "UI/UIInputs.h"
#include "UI/UIFileBrowser.h"
#include "UserInterface.h"

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

UIAction::UIAction(int inType, UIElement * inTargetElement, String inArgumentStr)
{
	Nullify();
	type = inType;
	targetElement = inTargetElement;
	argumentStr = inArgumentStr;
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
		case POP_UI:
			if (targetElement)
				targetElement->ui->PopFromStack(graphicsState, targetElement);
			break;
		// Drop down menu
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
			break;
			// File Browser
		case SELECT_FILE_BROWSER_FILE:
			if (targetElement->type == UIType::FILE_BROWSER)
				((UIFileBrowser*)targetElement)->SetActiveFile(graphicsState, argumentStr);
			break;
		case SELECT_FILE_BROWSER_DIRECTORY:
			if (targetElement->type == UIType::FILE_BROWSER) {
				UIFileBrowser* fileBrowser = ((UIFileBrowser*)targetElement);
				if (forElement->type == UIType::STRING_INPUT)
					fileBrowser->SetPath(((UIStringInput*)forElement)->GetValue(), graphicsState);
				else
					fileBrowser->UpdatePath(argumentStr, graphicsState);
			}
			break;
		case CONFIRM_FILE_BROWSER_SELECTION:
			if (targetElement->type == UIType::FILE_BROWSER)
				((UIFileBrowser*)targetElement)->ConfirmSelection(graphicsState);
			else if (targetElement->type == UIType::FILE_INPUT) {
				UIFileBrowser* browser = (UIFileBrowser*)forElement;
				((UIFileInput*)targetElement)->SetValue(browser->GetFileSelection()[0]);
			}
			break;
		case SET_FILE_BROWSER_FILE_FROM_INPUT:
			if (targetElement->type == UIType::FILE_BROWSER)
				((UIFileBrowser*)targetElement)->SetActiveFileFromInput(graphicsState);
			break;
		// FileInput file browser usage
		case PUSH_FILE_BROWSER:
			if (targetElement->type == UIType::FILE_INPUT)
				((UIFileInput*)targetElement)->PushFileBrowser(graphicsState);
			break;
		// Vector input 
		case BEGIN_VECTOR_INPUT:
			if (targetElement->type == UIType::VECTOR_INPUT)
				((UIVectorInput*)targetElement)->BeginInput(graphicsState);
			break;
	}
}
