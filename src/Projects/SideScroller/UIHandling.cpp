/// Emil Hedemalm
/// 2015-02-07
/// All code pertaining to updating UI... to separate it from the rest.

#include "SideScroller.h"
#include "File/SaveFile.h"
#include "Application/Application.h"

#include "UI/UIList.h"
#include "UI/UIImage.h"
#include "UI/UIButtons.h"

void LoadOptions();

extern bool inGameMenuOpened;

/// Updates ui depending on state.
void SideScroller::UpdateUI()
{
	String toPush;

	List<String> stateSpecificUI;
	stateSpecificUI.Add("gui/MainMenu.gui", "gui/HUD.gui", "gui/Shop.gui");
	stateSpecificUI.Add("gui/GameOver.gui", "gui/BuyTaco.gui");
	for (int i = 0; i < stateSpecificUI.Size(); ++i)
	{
		// Pop 'em.
		String ui = stateSpecificUI[i];
		MesMan.ProcessMessage("PopUI("+ui+")");
	}

	// Reveal specifics?
	switch(state)
	{
		case MAIN_MENU: toPush = "gui/MainMenu.gui"; break;
		case PLAYING_LEVEL:	toPush = "gui/HUD.gui"; break;
		case IN_SHOP: toPush = "gui/Shop.gui"; break;
		case GAME_OVER: toPush = "gui/GameOver.gui"; break;
		case TACO_TIME: toPush = "gui/BuyTaco.gui"; break;

			// old below?
		case EDITING_OPTIONS: toPush = "gui/Options.gui"; break;
		case NEW_GAME: toPush = "gui/NewGame.gui"; break;
		case LOAD_SAVES: toPush = "gui/LoadScreen.gui"; break;
		case LEVEL_CLEARED: toPush = "gui/LevelStats.gui"; break;
		case IN_LOBBY: toPush = "gui/Lobby.gui"; break;
		case BUYING_GEAR: toPush = "gui/Shop.gui"; break;
		case SHOWING_LEVEL_STATS: toPush = "gui/LevelStats.gui"; break;
		default:
			assert(false);
//			uis.Add("MainMenu");
			break;
	}
	if (toPush.Length())
		MesMan.ProcessMessage("PushUI("+toPush+")");
	
	switch(state)
	{
		case NEW_GAME:  LoadDefaultName(); break;
		case EDITING_OPTIONS: LoadOptions(); break;
		case PLAYING_LEVEL:
			// Update all stats.
			UpdateMunny();
			UpdateAttempts();
			UpdateDistance();
//			UpdateUIPlayerHP(); 
//			UpdateUIPlayerShield();
			if (inGameMenuOpened)
				MesMan.ProcessMessage("PushUI(gui/InGameMenu.gui)");
			else
				MesMan.ProcessMessage("PopUI(gui/InGameMenu.gui)");
			break;
		case IN_SHOP:
			UpdateShopMasks();
			break;
		case LOAD_SAVES: OpenLoadScreen(); break;
	//	case BUYING_GEAR: UpdateGearList(); break;
		case SHOWING_LEVEL_STATS: ShowLevelStats(); break;
	};
}

void SideScroller::UpdateMunny()
{
	QueueGraphics(new GMSetUIs("Pesos", GMUI::TEXT, String(munny)));
}
void SideScroller::UpdateAttempts()
{
	QueueGraphics(new GMSetUIs("Attempts", GMUI::TEXT, String(attempts->iValue)));
}
void SideScroller::UpdateDistance()
{
	static int lastDistance = 0;
	if ((int)distance != lastDistance)
	{
		QueueGraphics(new GMSetUIs("DistanceTraveled", GMUI::TEXT, String((int)distance)));
		lastDistance = (int)distance;
	}
}

List<Mask> masks;

void SideScroller::UpdateShopMasks()
{
	int masksOwned = 0;

	/// Update matrix size first.
	int num = masks.Size();
	Vector2i size(1,1);
	while(size.GeometricSum() < num)
	{
		size.x += 1;
		size.y += 1;
	}
	QueueGraphics(new GMSetUIv2i("MaskMatrix", GMUI::MATRIX_SIZE, size));


	/// Create buttons/image previews of all masks in the grid and send them to the grid!
	List<UIElement*> maskPreviewButtons;
	for (int i = 0; i < masks.Size(); ++i)
	{	
		Mask & mask = masks[i];
		UIButton * maskButton = new UIButton("ShopMask: "+mask.name);
		maskButton->textureSource = "0x55AA";
		maskButton->onHover = "ShopMaskHover: "+mask.name;
		maskButton->text = "";

		UIImage * image = new UIImage(mask.textureSource);
//		image->hoverable = true;
//		image->highlightOnHover = true;

		if (mask.purchased)
		{
			if (equippedMask == &mask)
			{
				// Highlight background of the image of equipped one.
				maskButton->textureSource = "0x99AA";
			}
			else 
			{
				// Highlight background of the image of owned ones. NO.
//				maskButton->textureSource = "0x77AA";		
			}
			masksOwned++;
		}
		else if (mask.price <= totalMunny->iValue)
		{
			// Slightly greyed-out - possible purchases
			image->color.z = 0.75f;
		}
		/// Grey out masks that have not yet been purchased!
		else 
		{
//			float greyed = 0.8f, alpha = 1.f;
			float greyed = 1.f, alpha = 0.5;
			image->color = Vector4f(greyed, greyed, greyed, alpha);

		}

		maskButton->AddChild(image);
		maskPreviewButtons.AddItem(maskButton);
	}
	QueueGraphics(new GMSetUIContents(maskPreviewButtons, "MaskMatrix"));

	/// Update player munny and owned masks.
	QueueGraphics(new GMSetUIs("ShopPesos", GMUI::TEXT, String(totalMunny->iValue)));
	QueueGraphics(new GMSetUIs("MasksOwned", GMUI::TEXT, String(masksOwned)+"/"+String(masks.Size())));
}

void SideScroller::UpdateSelectedMask(String maskName)
{
//	static String lastMask;
//	if (maskName == lastMask)
//		return;
//	lastMask = maskName;

	for (int i = 0; i < masks.Size(); ++i)
	{
		Mask & mask = masks[i];
		if (mask.name != maskName)
			continue;
		/// Update UI!
		QueueGraphics(new GMSetUIs("MaskName", GMUI::TEXT, mask.name));
		QueueGraphics(new GMSetUIs("MaskPreview", GMUI::TEXTURE_SOURCE, mask.textureSource));
		QueueGraphics(new GMSetUIs("lPrice", GMUI::TEXT, "Precio"));
		// Affordable?
		// If price is too high..
		if (mask.purchased)
		{
			if (&mask == equippedMask)
			{
				QueueGraphics(new GMSetUIs("lPrice", GMUI::TEXT, "Lucha!"));
				QueueGraphics(new GMSetUIs("Price", GMUI::TEXT, "Venga!"));
				QueueGraphics(new GMSetUIv4f("Price", GMUI::TEXT_COLOR, Vector4f(1,1,1,1)));
				QueueGraphics(new GMSetUIs("clPrice", GMUI::TEXTURE_SOURCE, "0xFFFF77AA"));	
			}
			else 
			{
				QueueGraphics(new GMSetUIs("Price", GMUI::TEXT, "Owned ("+String(mask.price)+")"));						
				QueueGraphics(new GMSetUIv4f("Price", GMUI::TEXT_COLOR, Vector4f(1,1,1,1)));
				QueueGraphics(new GMSetUIs("clPrice", GMUI::TEXTURE_SOURCE, "0xAA77"));
			}
		}
		else 
		{
			// Display price.
			QueueGraphics(new GMSetUIs("Price", GMUI::TEXT, String(mask.price)));
			if (mask.price > totalMunny->GetInt())
			{
				float nonRed = 0.1f;
				QueueGraphics(new GMSetUIv4f("Price", GMUI::TEXT_COLOR, Vector4f(1,nonRed,nonRed,1)));
				QueueGraphics(new GMSetUIs("clPrice", GMUI::TEXTURE_SOURCE, "0x4477"));
			}
			else 
			{
				QueueGraphics(new GMSetUIv4f("Price", GMUI::TEXT_COLOR, Vector4f(1,1,1,1)));
				// Affordable?
				QueueGraphics(new GMSetUIs("clPrice", GMUI::TEXTURE_SOURCE, "0xAAFFAA77"));
			}
		}
	}
}


void LoadOptions()
{
	// Load settings.
}

void SideScroller::LoadDefaultName()
{
	SetState(NEW_GAME, false);
	GraphicsMan.QueueMessage(new GMSetUIs("PlayerName", GMUI::STRING_INPUT_TEXT, playerName->strValue));
	GraphicsMan.QueueMessage(new GMSetUIi("Difficulty", GMUI::INTEGER_INPUT, difficulty->iValue));
}

void SideScroller::ShowLevelStats()
{
	state = SHOWING_LEVEL_STATS;
	GraphicsMan.QueueMessage(new GMSetUIs("LevelKills", GMUI::TEXT, LevelKills()->ToString()));
	GraphicsMan.QueueMessage(new GMSetUIs("LevelScore", GMUI::TEXT, LevelScore()->ToString()));
	GraphicsMan.QueueMessage(new GMSetUIs("ScoreTotal", GMUI::TEXT, score->ToString()));
}

/// Returns a list of save-files.
void SideScroller::OpenLoadScreen()
{
	state = LOAD_SAVES;
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



