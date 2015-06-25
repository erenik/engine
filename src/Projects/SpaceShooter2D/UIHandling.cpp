/// Emil Hedemalm
/// 2015-02-07
/// All code pertaining to updating UI... to separate it from the rest.

#include "SpaceShooter2D.h"
#include "File/SaveFile.h"
#include "Application/Application.h"

#include "Window/AppWindow.h"
#include "UI/UIList.h"
#include "UI/UIUtil.h"
#include "UI/UIInputs.h"

void LoadOptions();

extern bool inGameMenuOpened;

/// Updates ui depending on mode.
void SpaceShooter2D::UpdateUI()
{
	/// Pop stuff.
	List<String> uis("gui/MainMenu.gui", "gui/HUD.gui", "gui/Hangar.gui");
	for (int i = 0; i < uis.Size(); ++i)
	{
		PopUI(uis[i]);
	}
	String toPush;	
	// Reveal specifics?
	switch(mode)
	{
		case MAIN_MENU: toPush = "gui/MainMenu.gui"; break;
		case EDITING_OPTIONS: toPush = "gui/Options.gui"; break;
		case NEW_GAME: toPush = "gui/NewGame.gui"; break;
		case LOAD_SAVES: toPush = "gui/LoadScreen.gui"; break;
		case GAME_OVER: 
		case PLAYING_LEVEL:	toPush = "gui/HUD.gui"; break;
		case LEVEL_CLEARED: toPush = "gui/LevelStats.gui"; break;
		case IN_LOBBY: toPush = "gui/Lobby.gui"; break;
		case IN_HANGAR: toPush = "gui/Hangar.gui"; break;
		case IN_WORKSHOP: toPush = "gui/Workshop.gui"; break;
		case BUYING_GEAR: toPush = "gui/Shop.gui"; break;
		case SHOWING_LEVEL_STATS: toPush = "gui/LevelStats.gui"; break;
		default:
			assert(false);
//			uis.Add("MainMenu");
			break;
	}
	if (toPush.Length())
		MesMan.ProcessMessage("PushUI("+toPush+")");
	
	switch(mode)
	{
		case NEW_GAME:  LoadDefaultName(); break;
		case EDITING_OPTIONS: LoadOptions(); break;
		case PLAYING_LEVEL: 
			UpdateHUDGearedWeapons();
			UpdateUIPlayerHP(); 
			UpdateUIPlayerShield();
			if (inGameMenuOpened)
				MesMan.ProcessMessage("PushUI(gui/InGameMenu.gui)");
			else
				MesMan.ProcessMessage("PopUI(gui/InGameMenu.gui)");
			break;
		case LOAD_SAVES: OpenLoadScreen(); break;
		case BUYING_GEAR: UpdateGearList(); break;
		case SHOWING_LEVEL_STATS: ShowLevelStats(); break;
	};
}

void LoadOptions()
{
	// Load settings.
}

void RequeueHUDUpdate()
{
	/// Queue an update for later?
	Message * msg = new Message("UpdateHUDGearedWeapons");
	msg->timeToProcess = Time::Now() + Time::Milliseconds(1000);
	MesMan.QueueDelayedMessage(msg);
}

/// Update UI
void SpaceShooter2D::UpdateHUDGearedWeapons()
{
	// Fetch the names of the checkboxes.
	UserInterface * ui = MainWindow()->ui;
	if (!ui)
	{
		RequeueHUDUpdate();
		return;
	}
	UIElement * activeWeapon = ui->GetElementByName("ActiveWeapon");
	if (!activeWeapon)
	{
		RequeueHUDUpdate();
		return;
	}
	// Fetch children.
	assert(activeWeapon);
	List<UIElement*> children = activeWeapon->GetChildren();
	List<Weapon*> & weapons = playerShip->weapons;
	for (int i = 0; i < children.Size(); ++i)
	{
		UIElement * child = children[i];
		if (i >= weapons.Size())
		{
			QueueGraphics(new GMSetUIs(child->name, GMUI::TEXT, "."));
			QueueGraphics(new GMSetUIb(child->name, GMUI::ACTIVATABLE, false));
			QueueGraphics(new GMSetUIb(child->name, GMUI::HOVERABLE, false));
			continue;
		}
		Weapon * weapon = weapons[i];
		QueueGraphics(new GMSetUIs(child->name, GMUI::TEXT, weapon->name));
		QueueGraphics(new GMSetUIb(child->name, GMUI::ACTIVATABLE, true));
		QueueGraphics(new GMSetUIb(child->name, GMUI::HOVERABLE, true));
	}
}
void SpaceShooter2D::UpdateUIPlayerHP()
{
	GraphicsMan.QueueMessage(new GMSetUIi("HP", GMUI::INTEGER_INPUT, playerShip->hp));	
}
void SpaceShooter2D::UpdateUIPlayerShield()
{
	GraphicsMan.QueueMessage(new GMSetUIi("Shield", GMUI::INTEGER_INPUT, (int)playerShip->shieldValue));
}

void SpaceShooter2D::UpdateHUDSkill()
{
	QueueGraphics(new GMSetUIs("Skill", GMUI::TEXT, playerShip->skillName));
	QueueGraphics(new GMSetUIs("Skill", GMUI::TEXTURE_SOURCE, playerShip->activeSkill != NO_SKILL? "0x00FF00FF" : "0x44AA"));		
}

void SpaceShooter2D::LoadDefaultName()
{
	SetMode(NEW_GAME, false);
	GraphicsMan.QueueMessage(new GMSetUIs("PlayerName", GMUI::STRING_INPUT_TEXT, playerName->strValue));
	GraphicsMan.QueueMessage(new GMSetUIi("Difficulty", GMUI::INTEGER_INPUT, difficulty->iValue));
}

void SpaceShooter2D::ShowLevelStats()
{
	mode = SHOWING_LEVEL_STATS;
	GraphicsMan.QueueMessage(new GMSetUIs("LevelKills", GMUI::TEXT, LevelKills()->ToString()));
	GraphicsMan.QueueMessage(new GMSetUIs("LevelScore", GMUI::TEXT, LevelScore()->ToString()));
	GraphicsMan.QueueMessage(new GMSetUIs("ScoreTotal", GMUI::TEXT, score->ToString()));
}

/// Returns a list of save-files.
void SpaceShooter2D::OpenLoadScreen()
{
	mode = LOAD_SAVES;
	List<SaveFileHeader> headers;
	/// Returns list of all saves, in the form of their SaveFileHeader objects, which should include all info necessary to judge which save to load!
	headers = SaveFile::GetSaves(Application::name);
	// Clear old list.
	GraphicsMan.QueueMessage(new GMClearUI("SavesCList"));
	/// Sort saves by date?
	for (int i = 0; i < headers.Size(); ++i)
	{
		SaveFileHeader & header = headers[i];
		for (int j = i + 1; j < headers.Size(); ++j)
		{
			SaveFileHeader & header2 = headers[j];
			if (header2.dateSaved > header.dateSaved)
			{
				// Switch places.
				SaveFileHeader tmp = header2;
				header2 = header;
				header = tmp;
			}
		}
	}
	// List 'em.
	List<UIElement*> saves;
	for (int i = 0; i < headers.Size(); ++i)
	{
		SaveFileHeader & h = headers[i];
		UIList * list = new UIList();
		
		UILabel * label = new UILabel();
		label->text = h.customHeaderData;
		label->sizeRatioY = 0.5f;
		label->hoverable = false;
		label->textureSource = "NULL";
		list->AddChild(label);

		list->highlightOnHover = true;
		list->sizeRatioX = 0.33f;
		list->activateable = true;
		list->highlightOnActive = true;
		list->activationMessage = "LoadGame("+h.saveName+")";
		list->textureSource = "0x3366";
		saves.Add(list);
	}
	GraphicsMan.QueueMessage(new GMAddUI(saves, "SavesCList"));
	/// Move cursor to first save in the list.
	if (saves.Size())
	{
		saves[0]->Hover();
	}
	else 
	{
		GraphicsMan.QueueMessage(new GMSetHoverUI("Back"));
	}
}

void SpaceShooter2D::UpdateGearList()
{
	GraphicsMan.QueueMessage(new GMSetUIs("wsMunny", GMUI::TEXT, "Money: "+String(money->iValue)));

	String gearListUI = "GearList";
	GraphicsMan.QueueMessage(new GMClearUI(gearListUI));

	List<Gear> gearList = Gear::GetType(gearCategory);
	// Sort by price.
	for (int i = 0; i < gearList.Size(); ++i)
	{
		Gear & gear = gearList[i];
		for (int j = i + 1; j < gearList.Size(); ++j)
		{
			Gear & gear2 = gearList[j];
			if (gear2.price < gear.price)
			{
				Gear tmp = gear;
				gear = gear2;
				gear2 = tmp;
			}
		}
	}

	List<UIElement*> toAdd;
	for (int i = 0; i < gearList.Size(); ++i)
	{
		Gear & gear = gearList[i];
		if (gear.name.Length() == 0)
			continue;
		UIColumnList * list = new UIColumnList();
		if (gear.price < money->iValue)
		{
			list->hoverable = true;
			list->activateable = true;
			list->textureSource = "0x3344";
		}
		else 
		{
			list->textureSource = "0x1544";
		}
		list->sizeRatioY = 0.2f;
		list->padding = 0.02f;
		list->onHover = "ShowGearDesc:"+gear.description;
		list->activationMessage = "BuyGear:"+gear.name;
		// First a label with the name.
		UILabel * label = new UILabel(gear.name);
		label->sizeRatioX = 0.3f;
		label->hoverable = false;
		list->AddChild(label);
		// Add stats?
		switch(gearCategory)
		{
			// Weapons:
			case 0:
			{
				break;
			}
			// Shields
			case 1:
			{
				label = new UILabel("Max Shield: "+String(gear.maxShield));
				label->hoverable = false;
				label->sizeRatioX = 0.2f;
				list->AddChild(label);
				label = new UILabel("Regen: "+String(gear.shieldRegen));
				label->hoverable = false;
				label->sizeRatioX = 0.1f;
				list->AddChild(label);
				break;
			}
			// Armors
			case 2:
			{
				label = new UILabel("Max HP: "+String(gear.maxHP));
				label->hoverable = false;
				label->sizeRatioX = 0.15f;
				list->AddChild(label);
				label = new UILabel("Toughness: "+String(gear.toughness));
				label->hoverable = false;
				label->sizeRatioX = 0.1f;
				list->AddChild(label);
				label = new UILabel("Reactivity: "+String(gear.reactivity));
				label->hoverable = false;
				label->sizeRatioX = 0.1f;
				list->AddChild(label);
				break;		
			}
		}
		// Add price.
		label = new UILabel(String(gear.price));
		label->hoverable = false;
		label->sizeRatioX = 0.2f;
		list->AddChild(label);

		// Add buy button
		toAdd.Add(list);
	}
	GraphicsMan.QueueMessage(new GMAddUI(toAdd, gearListUI));
}


void SpaceShooter2D::OpenJumpDialog()
{
	// Create a string-input.
	static UIStringInput * jumpDialog = 0;
	if (!jumpDialog)
	{
		jumpDialog = new UIStringInput("JumpTo", "JumpToTime");
		jumpDialog->textureSource = "0x44AA";
		jumpDialog->onTrigger += "PopUI(JumpTo)&ResumeGame";
		jumpDialog->sizeRatioX = 0.5;
		jumpDialog->sizeRatioY = 0.1;
		jumpDialog->CreateChildren();
		jumpDialog->input->BeginInput(); // Make its input active straight away.
		// Add it to the main UI.
		QueueGraphics(new GMAddUI(jumpDialog, "root"));
	}
	else {
		jumpDialog->input->BeginInput(); // Make its input active straight away.
		QueueGraphics(new GMPushUI(jumpDialog, MainWindow()->ui));
	}
		// Close it afterwards.
}