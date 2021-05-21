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
#include "UI/UILabel.h"

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
	float spacePerElement = DefaultSpacePerElement(layout.padding);
	divider = Vector2f(0.4f, 0.4f);
	label = CreateDefaultLabel(box, displayText, divider.x);
	label->rightBorderTextureSource = rightBorderTextureSource;

	input = CreateDefaultInput(box, name, divider.y);
	input->onTriggerActions = onTriggerActions;
	input->visuals.textureSource = inputTextureSource;

	/// Set them to only accept floats?
	input->SetText("");
	input->rememberPreviousInputs = true;
	input->text.alignment = LEFT;
	if (guiInputDisabled)
	{
		input->interaction.activateable = false;
		input->visuals.highlightOnHover = false;
		box->interaction.activateable = false;
		interaction.activateable = false;
		interaction.hoverable = false;
	}
	
	fileBrowserButton = new UIButton();
	InheritDefaults(fileBrowserButton);
	fileBrowserButton->visuals.textureSource = "img/ui/fileBrowserIcon";
	fileBrowserButton->visuals.retainAspectRatioOfTexture = true;
	fileBrowserButton->visuals.highlightOnHover = true;
	fileBrowserButton->layout.sizeRatioX = 1 - divider.x - divider.y;
	box->AddChild(nullptr, fileBrowserButton);
	fileBrowserButton->activationActions.Add(UIAction(UIAction::PUSH_FILE_BROWSER, this));

	childrenCreated = true;

}

// Pushes a new file browser to relevant UI where this element resides.
void UIFileInput::PushFileBrowser(GraphicsState * graphicsState) {
	UIFileBrowser * newBrowser = new UIFileBrowser(name+"Browser", UIAction(UIAction::CONFIRM_FILE_BROWSER_SELECTION, this));
	newBrowser->SetFileFilter(fileFilter);
	InheritDefaults(newBrowser);
	newBrowser->layout.sizeRatioX = 1.0f;
	newBrowser->layout.sizeRatioY = 1.0f;

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

