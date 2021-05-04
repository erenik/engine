/// Emil Hedemalm
/// 2014-01-14
/// Input which also provides a button which pushes a FileBrowser for easy file-selection.

#include "UIFileInput.h"

#include "UI/Lists/UIColumnList.h"
#include "UI/UIButtons.h"
#include "UI/UserInterface.h"
#include "UI/UIFileBrowser.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"

UIFileInput::UIFileInput(String name, String onTrigger) : UIStringInput(name, onTrigger) 
{
	this->type = UIType::FILE_INPUT;
}
UIFileInput::~UIFileInput() {

}

/// Creates the label and input.
void UIFileInput::CreateChildren(GraphicsState* graphicsState) {
	/// Use a column-list to automatically get links between the elements, etc.
	UIColumnList * box = CreateDefaultColumnList(this);
	float spacePerElement = DefaultSpacePerElement(padding);
	divider = Vector2f(0.4f, 0.4f);
	label = CreateDefaultLabel(box, displayText, divider.x);
	label->rightBorderTextureSource = rightBorderTextureSource;

	input = CreateDefaultInput(box, name, divider.y);
	input->onTriggerActions = onTriggerActions;
	input->textureSource = inputTextureSource;

	/// Set them to only accept floats?
	input->SetText("");
	input->rememberPreviousInputs = true;
	input->textAlignment = LEFT;
	if (guiInputDisabled)
	{
		input->activateable = false;
		input->highlightOnHover = false;
		box->activateable = false;
		activateable = false;
		hoverable = false;
	}
	
	fileBrowserButton = new UIButton();
	InheritDefaults(fileBrowserButton);
	fileBrowserButton->textureSource = "img/ui/fileBrowserIcon";
	fileBrowserButton->retainAspectRatioOfTexture = true;
	fileBrowserButton->highlightOnHover = true;
	fileBrowserButton->sizeRatioX = 1 - divider.x - divider.y;
	box->AddChild(nullptr, fileBrowserButton);
	fileBrowserButton->activationActions.Add(UIAction(UIAction::PUSH_FILE_BROWSER, this));

	childrenCreated = true;

}

// Pushes a new file browser to relevant UI where this element resides.
void UIFileInput::PushFileBrowser(GraphicsState * graphicsState) {
	UIFileBrowser * newBrowser = new UIFileBrowser(name+"Browser", UIAction(UIAction::CONFIRM_FILE_BROWSER_SELECTION, this));
	newBrowser->SetFileFilter(fileFilter);
	InheritDefaults(newBrowser);
	newBrowser->sizeRatioX = 1.0f;
	newBrowser->sizeRatioY = 1.0f;

	UserInterface * ui = this->ui;
	ui->Root()->AddChild(graphicsState, newBrowser);
	newBrowser->CreateChildren(graphicsState);
	ui->PushToStack(newBrowser);
}

void UIFileInput::SetFileFilter(String filter) {
	fileFilter = filter;
}

void UIFileInput::SetValue(String value)
{
	input->SetText(value);
	SetStringMessage * m = new SetStringMessage(action, value);
	MesMan.QueueMessage(m);
}

