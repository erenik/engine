/// Emil Hedemalm
/// 2015-11-30
/// Drop-down menu. Single-selection for starters.

#include "UIDropDownMenu.h"
#include "UI/UIButtons.h"
#include "UI/UserInterface.h"
#include "UI/UILists.h"
#include "UI/UITypes.h"
#include "Message/Message.h"
#include "Message/MessageManager.h"
#include "UI/Input/UIInput.h"

UIDropDownMenu::UIDropDownMenu(String inName)
	: UIInput()
{
	type = UIType::DROP_DOWN_MENU;
	this->name = inName;
	separateLabel = true;
	SetText("");
	selectButton = 0;
	selectionList = 0;

	available.Add("Lall-i-lall", "Balloko", "Kalliji");
}

UIDropDownMenu::~UIDropDownMenu()
{
}

void UIDropDownMenu::CreateChildren(GraphicsState* graphicsState)
{
	if (childrenCreated)
		return;
	childrenCreated = true;
	if (separateLabel)
	{
		this->label = new UILabel(name);
		this->InheritDefaults(label);
		label->hoverable = false;
		label->sizeRatioX = 0.4f;
		label->sizeRatioY = 1.f;
		label->alignmentX = 0.2f;
		AddChild(nullptr, label);
	}
	selectButton = new UIButton("<Placeholder>");
	this->InheritDefaults(selectButton);
	selectButton->sizeRatioX = 1 - (label? label->sizeRatioX : 0);
	selectButton->alignmentX = 0.5 + (label? label->sizeRatioX * 0.5 : 0);
//	selectButton->activationMessage = "OpenDropDownMenu:"+name;
	UIAction uac(UIAction::OPEN_DROP_DOWN_MENU, UIAction::PARENT);
	selectButton->activationActions.AddItem(uac);

	this->activationMessage = "";
	this->activationActions.Add(UIAction(UIAction::OPEN_DROP_DOWN_MENU, this)); // Set it for self as well.

	AddChild(graphicsState, selectButton);
//	ShipTypeToSpawn
}

/// Sets text, queueing recalculation of the rendered variant. If not force, will ignore for active ui input elements.
void UIDropDownMenu::SetText(CTextr newText, bool force /*= false*/)
{
	label = UIInput::CreateDefaultLabel(this, newText, 1.0f);
}

void UIDropDownMenu::RenderText()
{
}

/// Call from graphics thread only. Or set via GMSetUIContents
void UIDropDownMenu::SetContents(GraphicsState * graphicsState, List<String> contents)
{
	bool equal = false;
	if (available.Size() == contents.Size())
	{
		equal = true;
		for (int i = 0; i < available.Size(); ++i)
		{
			if (available[i] != contents[i])
			{
				equal = false;
				break;
			}
		}
	}
	if (equal)
		return; // No change needed.
	this->available = contents;
	PopulateList(graphicsState);
}

bool UIDropDownMenu::OnProceed(GraphicsState* graphicsState) {
	Open(graphicsState);
	return true;
}

/// Opens UI to select among children. How this will be done depends on settings.
void UIDropDownMenu::Open(GraphicsState* graphicsState)
{
	// Create list to select. Cover entire screen. Stupid initial implementation :D
	if (!selectionList)
	{
		selectionList = new UIList();
		this->InheritDefaults(selectionList);
		selectionList->visible = false;
		selectionList->textureSource = "0x22FF";
		PopulateList(graphicsState);
	}

	// Make it visible.
	if (!selectionList->visible)
	{
		selectionList->visible = true;
		//this->AddChild(graphicsState, selectionList);
		auto root = GetRoot();

		root->AddChild(graphicsState, selectionList);
		ui->PushToStack(selectionList);
	}

	//UIElement * element = new UIElement();
	//element->textureSource = "0xFFAA55FF";
	//GetRoot()->AddChild(graphicsState, element);

}

void UIDropDownMenu::PopulateList(GraphicsState* graphicsState)
{
	if (!selectionList)
		return;
	selectionList->Clear();
	/// Populate the list.
	for (int i = 0; i < available.Size(); ++i)
	{
		UIButton * button = new UIButton(name+"ListButton"+i);
		this->InheritDefaults(button);

		button->SetText(available[i]);
		button->sizeRatioY = 0.1f;
		UIAction ac(UIAction::CLOSE_DROP_DOWN_MENU, this);
		button->activationActions.AddItem(ac);
		button->activationActions.AddItem(UIAction(UIAction::SELECT_DROP_DOWN_MENU, this, i));
		selectionList->AddChild(graphicsState, button);
	}
}

void UIDropDownMenu::Close(GraphicsState* graphicsState)
{
	this->ui->PopFromStack(graphicsState, selectionList);
	this->ui->Root()->RemoveChild(graphicsState, selectionList);
	selectionList->visible = false;
}

void UIDropDownMenu::Select(int index, bool silent)
{
	selection = available[index];
	selectButton->SetText(selection);
	if (!silent) {
		SetStringMessage * ssm = new SetStringMessage("DropDownMenuSelection:" + name, selection);
		MesMan.QueueMessage(ssm);
	}
}

bool UIDropDownMenu::Select(String entry, bool silent) {
	for (int i = 0; i < available.Size(); ++i) {
		if (available[i] == entry) {
			Select(i, silent);
			return true;
		}
	}
	return false;
}

// For sub-classes to adjust children as needed (mainly for input elements).
void UIDropDownMenu::OnStateAdded(GraphicsState* graphicsState, int state) {
	if (state == UIState::HOVER)
		label->AddStateSilently(state, true);
	selectButton->AddStateSilently(state);
	UIElement::OnStateAdded(graphicsState, state);
}

