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

UIDropDownMenu::UIDropDownMenu(String inName)
	: UIElement()
{
	type = UIType::DROP_DOWN_MENU;
	this->name = inName;
	separateLabel = true;
	text = "";
	selectButton = 0;
	selectionList = 0;

	available.Add("Lall-i-lall", "Balloko", "Kalliji");
}

UIDropDownMenu::~UIDropDownMenu()
{
}

void UIDropDownMenu::CreateChildren()
{
	if (childrenCreated)
		return;
	childrenCreated = true;
	if (separateLabel)
	{
		this->label = new UILabel(name);
		label->sizeRatioX = 0.4f;
		label->sizeRatioY = 1.f;
		label->alignmentX = 0.2f;
		AddChild(label);
	}
	selectButton = new UIButton("<Placeholder>");
	selectButton->sizeRatioX = 1 - (label? label->sizeRatioX : 0);
	selectButton->alignmentX = 0.5 + (label? label->sizeRatioX * 0.5 : 0);
//	selectButton->activationMessage = "OpenDropDownMenu:"+name;
	UIAction uac(UIAction::OPEN_DROP_DOWN_MENU, UIAction::PARENT);
	selectButton->activationActions.AddItem(uac);

	AddChild(selectButton);
//	ShipTypeToSpawn
}

/// Sets text, queueing recalculation of the rendered variant. If not force, will ignore for active ui input elements.
void UIDropDownMenu::SetText(Text newText, bool force /*= false*/)
{
	label->SetText(newText, force);
}

void UIDropDownMenu::RenderText()
{
}

/// Call from graphics thread only. Or set via GMSetUIContents
void UIDropDownMenu::SetContents(List<String> contents)
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
	PopulateList();
}

/// Opens UI to select among children. How this will be done depends on settings.
void UIDropDownMenu::Open()
{
	// Create list to select. Cover entire screen. Stupid initial implementation :D
	if (!selectionList)
	{
		selectionList = new UIList();
		selectionList->textureSource = "0x22FF";
		PopulateList();
	}
	selectionList->visible = true;
	ui->Root()->AddChild(selectionList);
	this->ui->PushToStack(selectionList);
}

void UIDropDownMenu::PopulateList()
{
	if (!selectionList)
		return;
	selectionList->Clear();
	/// Populate the list.
	for (int i = 0; i < available.Size(); ++i)
	{
		UIButton * button = new UIButton(name+"ListButton"+i);
		button->text = available[i];
		button->sizeRatioY = 0.1f;
		UIAction ac(UIAction::CLOSE_DROP_DOWN_MENU, this);
		button->activationActions.AddItem(ac);
		button->activationActions.AddItem(UIAction(UIAction::SELECT_DROP_DOWN_MENU, this, i));
		selectionList->AddChild(button);
	}
}

void UIDropDownMenu::Close()
{
	this->ui->PopFromStack(selectionList);
	this->ui->Root()->RemoveChild(selectionList);
	selectionList->visible = false;
}

void UIDropDownMenu::Select(int index)
{
	selection = available[index];
	selectButton->SetText(selection);
	SetStringMessage * ssm = new SetStringMessage("DropDownMenuSelection:"+name, selection);
	MesMan.QueueMessage(ssm);
}


