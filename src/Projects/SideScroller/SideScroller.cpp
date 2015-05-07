/// Emil Hedemalm
/// 2015-01-20
/// Space shooter.
/// For the Karl-Emil SpaceShooter project, mainly 2014-2015/

#include "SideScroller.h"
#include "Input/Keys.h"
#include "InputState.h"

/// Particle system for sparks/explosion-ish effects.
bool paused = false;
float breatherBlockSize = 5.f;
Random levelRand, sfxRand;
/// Starts at 0, increments after calling BreatherBlock and AddLevelPart
float levelLength;
int pacoTacoX = 0;
int pacoTacoAnnouncementX = 0;
int lastK = 0;

/// 4 entities constitude the blackness.
List<Entity*> blacknessEntities;

void SetApplicationDefaults()
{
	Application::name = "SideScroller";
	Application::quitOnHide = false;
	TextFont::defaultFontSource = "img/fonts/Font_Mexicano.png";
	PhysicsProperty::defaultUseQuaternions = false;
	Viewport::defaultRenderGrid = false;
	inputState->demandActivatableForHoverElements = true; // Force elements to be activatable before UI elements are highlighted/steal focus.
	inputState->demandHighlightOnHoverForHoverElements = true;
	inputState->demandHoverElement = true;
}

// Global variables.
SideScroller * sideScroller = NULL;
/// The level entity, around which the playing field and camera are based upon.
Entity * levelEntity = NULL;
Vector2f playingFieldSize;
Vector2f playingFieldHalfSize;
float playingFieldPadding;
/// All ships, including player.
List<Entity*> shipEntities;
List<Entity*> projectileEntities;
String playerName;
/// o.o
Time startDate;
bool inGameMenuOpened = false;
bool showLevelStats = false;

/// Main level camera, following player.
Camera * levelCamera = NULL;
Entity * playerEntity = NULL;
LuchadorProperty * player = NULL;
Entity * paco = NULL, * taco = NULL;
/** Dynamically created ones, to be cleaned up as we go.
	Blocks, pesos, clouds all fit therein?
*/
List<Entity*> levelEntities;

void AddForCleanup(List<Entity*> entities)
{
	for (int i = 0; i < entities.Size(); ++i)
	{
		Entity * entity = entities[i];
		if (levelEntities.Exists(entity))
		{
			assert(false && "Trying to re-add");
			return;
		}
		levelEntities.AddItem(entity);
		int dupAfter = levelEntities.Duplicates();
		if (dupAfter)
			LogMain("Duplicates found..", WARNING);
	}
}

Entities pesos, clouds;

Mask * equippedMask = NULL;
int munny = 0;
GameVar * totalMunny = 0; // Total munny from all attempts. Summed up upon death? And auto-saved then?
GameVar * purchasedMasks = 0;
GameVar * equippedMaskName = 0;
GameVar * attempts = 0;
float distance = 0;

enum 
{
	PERSP_CAMERA,
	ORTHO_CAMERA,
	CAMERA_TYPES,
};
int camera = ORTHO_CAMERA;


class BGM 
{
public:
	BGM()
	{
		startDistance = stopDistance = -1;
	};
	void Play()
	{
		QueueAudio(new AMPlayBGM(source));
	};
	static bool Play(String byName);
	String name, source;
	int startDistance; // in K, -1 if not used.
	int stopDistance;
};
List<BGM*> bgms;
BGM * currentBGM = NULL;

bool BGM::Play(String byName)
{
	for (int i = 0; i < bgms.Size(); ++i)
	{
		BGM * bgm = bgms[i];
		if (bgm->name == byName)
		{
			bgm->Play();
			return true;
		}
	}	
	return false;
}

void LoadBGMList()
{
	bgms.Clear();
	List<String> lines = File::GetLines("bgm/BGMList.txt");
	BGM * bgm = NULL;
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		if (line.StartsWith("BGM"))
		{
			/// Add previous one.
			if (bgm)
				bgms.AddItem(bgm);
			bgm = new BGM();
			bgm->name = line - "BGM ";
		}
		if (!bgm)
			continue;
		if (line.StartsWith("Source"))
		{
			assert(bgm);
			bgm->source = line - "Source ";
		}
		else if (line.StartsWith("StartDistance"))
		{
			assert(bgm);
			bgm->startDistance = (line - "StartDistance ").ParseInt();
		}
	}
	// Add final one.
	if (bgm)
		bgms.AddItem(bgm);
	// go through all, calc stop X
	BGM * previous = NULL;
	for (int i = 0; i < bgms.Size(); ++i)
	{
		BGM * current = bgms[i];
		if (previous)
			previous->stopDistance = current->startDistance - 1;
		previous = current;
	}
}



void CycleCamera(int toWhich = -1)
{
	if (toWhich > -1)
		camera = toWhich % CAMERA_TYPES;
	if (!levelCamera)
		levelCamera = CameraMan.NewCamera("Level Camera");
	// General properties.
	levelCamera->trackingMode = TrackingMode::ADD_POSITION;
	levelCamera->entityToTrack = playerEntity;
	levelCamera->position = Vector3f(5,0,0);
	levelCamera->distanceFromCenterOfMovement = 10.f;
	switch(camera)
	{
		case PERSP_CAMERA:
			levelCamera->projectionType = Camera::PROJECTION_3D;
			levelCamera->zoom = 0.1f;
			levelCamera->rotation = Vector3f(PI*0.25f, PI*0.125f, 0);
			break;
		case ORTHO_CAMERA:
			levelCamera->projectionType = Camera::ORTHOGONAL;
			levelCamera->zoom = 10.f;
			levelCamera->rotation = Vector3f();
			break;
	}
	// Set camera
	QueueGraphics(new GMSetCamera(levelCamera));
}



void RegisterStates()
{
	sideScroller = new SideScroller();
	StateMan.RegisterState(sideScroller);
	StateMan.QueueState(sideScroller);
}

Model * sprite = NULL;

void SetAlphaBlending(Entity * entity)
{
	if (!entity->graphics)
	{
		entity->graphics = new GraphicsProperty(entity);
	}
	entity->graphics->flags |= RenderFlag::ALPHA_ENTITY;
}

/// Creates a new sprite featuring alpha-blending (as should be). Using the default sprite.obj, which is a 1x1 XY plane centered on 0,0 (0.5 in each direction).
Entity * CreateSprite(String textureSource)
{
	Entity * entity = EntityMan.CreateEntity("Sprite", sprite, TexMan.GetTexture(textureSource));
	SetAlphaBlending(entity);
	return entity;
}

void SetOnGround(Entity * entity)
{
	/// Default ground-level.
	entity->position.y = entity->scale.y * 0.5f + 0.5f;
}

/// Last time we got offered to buy tacos.
int buyTaco = 0;

SideScroller::SideScroller()
{
	keyPressedCallback = true;
	levelCamera = NULL;
	SetPlayingFieldSize(Vector2f(30,20));
	levelEntity = NULL;
	playingFieldPadding = 1.f;
	gearCategory = 0;
	previousState = state = -1;
	sprite = ModelMan.GetModel("sprite.obj");
	
	// Game/Save-vars
	totalMunny = GameVars.CreateInt("TotalMunny", 0);
	attempts = GameVars.CreateInt("Attempts", 0);
	equippedMaskName = GameVars.CreateString("EquippedMaskName", "");
	purchasedMasks = GameVars.CreateString("PurchasedMasks", "");

	/// Load masks data.
	Mask::LoadFromCSV("data/Masks.csv");
	// Default masks.
	if (masks.Size() == 0)
	{
		masks.AddItem(Mask("Grey", "img/Masks/Gray_mask.png", 100, 1));
		masks.AddItem(Mask("Red", "img/Masks/Red_mask.png", 500, 2));
		masks.AddItem(Mask("Yellow", "img/Masks/Yellow_mask.png", 1000, 3));
	}

	/// Load game if existing.
	AutoLoad();
}

SideScroller::~SideScroller()
{
	bgms.ClearAndDelete();
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void SideScroller::OnEnter(AppState * previousState)
{
	/// Enable Input-UI navigation via arrow-keys and Enter/Esc.
	InputMan.ForceNavigateUI(true);

	/// Create game variables.
	playTime = GameVars.CreateInt("playTime", 0);
	gameStartDate = GameVars.CreateTime("gameStartDate");

	AppWindow * w = MainWindow();
	assert(w);
	Viewport * vp = w->MainViewport();
	assert(vp);
//	vp->renderGrid = false;	

	// Add custom render-pass to be used.
	RenderPass * rs = new RenderPass();
	rs->type = RenderPass::RENDER_APP_STATE;
	GraphicsMan.QueueMessage(new GMAddRenderPass(rs));

	// Create.. the sparks! o.o
	// New global sparks system.
//	sparks = new Sparks(true);
	// Register it for rendering.
//	Graphics.QueueMessage(new GMRegisterParticleSystem(sparks, true));
	
//	stars = new Stars(true);
//	stars->deleteEmittersOnDeletion = true;
//	Graphics.QueueMessage(new GMRegisterParticleSystem(stars, true));
	
	/// Add emitter
//	starEmitter = new StarEmitter(Vector3f());
//	Graphics.QueueMessage(new GMAttachParticleEmitter(starEmitter, stars));

	// Remove overlay.
	// Set up ui.
//	if (!ui)
//		CreateUserInterface();
	// Set UI without delay.
//	GraphicsMan.ProcessMessage(new GMSetUI(ui));
	GraphicsMan.ProcessMessage(new GMSetOverlay(NULL));

	// Load Space Race integrator
	PhysicsMan.QueueMessage(new PMSet(new FirstPersonIntegrator()));
	PhysicsMan.QueueMessage(new PMSet(new FirstPersonCD()));
	PhysicsMan.QueueMessage(new PMSet(new FirstPersonCR()));

	PhysicsMan.checkType = AABB_SWEEP;

	// Run OnStartApp script.
	ScriptMan.PlayScript("scripts/OnStartApp.txt");

	/// Enter main menu
//	OpenMainMenu();

	/// Load BGM list
	LoadBGMList();

	// Run OnEnter.ini start script if such a file exists.
	Script * script = new Script();
	script->Load("OnEnter.ini");
	ScriptMan.PlayScript(script);

	// Remove initial cover screen.
	QueueGraphics(new GMSetOverlay(NULL));
}

Time now;
// int64 nowMs;
int timeElapsedMs;
#include "OS/Sleep.h"

Time lastJumpPlacementInput = Time::Now();
int jumpPlacement = 0;

/// Main processing function, using provided time since last frame.
void SideScroller::Process(int timeInMs)
{
	SleepThread(10);
	if (!MainWindow()->InFocus())
		SleepThread(40);
	else 
	{
//		std::cout<<"\nIn focus o.op";
	}
//	std::cout<<"\nSS2D entities: "<<shipEntities.Size() + projectileEntities.Size() + 1;
//	if (playerShip) std::cout<<"\nPlayer position: "<<playerShip.position;

	now = Time::Now();
	timeElapsedMs = timeInMs;
	
//	Cleanup();

	switch(state)
	{
		case PLAYING_LEVEL:
		{
			if (paused)
				return;
			ProcessLevel(timeInMs);
			/// Jumping?
			if (jumpPlacement != 0)
			{
				// Check time.
				int msSinceLastInput = (now - lastJumpPlacementInput).Milliseconds();
				if (msSinceLastInput > 1000)
				{
					// Jump.
					JumpTo(jumpPlacement);
					jumpPlacement = 0;
				}
			}
		}
	}
}

void SideScroller::ProcessLevel(int timeInMs)
{
	// Won?
	

	// Extend arena as necessary.
	while(CreateNextLevelParts())
		;

	float playerX = playerEntity->position.x;
	if (lastK != 0 && playerX > lastK * 1000)
	{
		// Won.
		player->Stop();
		// Switch state for display of UI.
		this->SetState(LEVEL_CLEARED);
		
		// Play winner BGM!
		BGM::Play("Winner");
		return;
	}
	
	/// Check if we should switch bgm?
	
	float bgmX = 0, bgmStopX = 0;
	if (currentBGM)
	{
		bgmX = currentBGM->startDistance;
		bgmStopX = currentBGM->stopDistance; // Calculated when loading.
	}
	if (playerX > bgmStopX && bgmStopX != -1)
	{
		// Next one.
		int index = bgms.GetIndexOf(currentBGM);
		BGM * next = bgms[index + 1];
		next->Play();
		currentBGM = next;
	}
	
	/// Adjust ambient color as we go.
	static Vector3f previousAmbient;
	Vector3f ambient(1,1,1);

	float hours = playerX / 3000.f * 12.f;
	float hour = hours;
	hour += 6;
	while(hour > 24)
		hour -= 24;
	
	Texture * ambientMap = TexMan.GetTexture("img/Sky_colors.png");
	float day = hour / 24.f;
	Vector3f sample = ambientMap->Sample(day * ambientMap->width, 0);
	sample += Vector3f(1,1,1) * 0.2f;
	ambient = sample;
	if (ambient != previousAmbient)
	{
		previousAmbient = ambient;
		QueueGraphics(new GMSetAmbience(ambient));
	}

}

/// Function when leaving this state, providing a pointer to the next StateMan.
void SideScroller::OnExit(AppState * nextState)
{
	levelEntity = NULL;
	SleepThread(50);
	// Register it for rendering.
	MapMan.DeleteAllEntities();
	SleepThread(100);
}

/// Callback from the Input-manager, query it for additional information as needed.
void SideScroller::KeyPressed(int keyCode, bool downBefore)
{
	switch(state)
	{
		case PLAYING_LEVEL:
		{
			/// Numbers to jump to a certain place.
			if (keyCode >= KEY::ZERO && keyCode <= KEY::NINE)
			{
				lastJumpPlacementInput = Time::Now();
				jumpPlacement *= 10;
				jumpPlacement += keyCode - KEY::ZERO;
				std::cout<<"\nJump placement: "<<jumpPlacement;
			}
		}
	}
}



/// Creates the user interface for this state
void SideScroller::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->CreateRoot();
//	ui->Load("gui/MainMenu.gui");
}


/// Callback function that will be triggered via the MessageManager when messages are processed.
void SideScroller::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::ON_UI_ELEMENT_HOVER:
		{
			if (msg.StartsWith("ShopMaskHover:"))
			{
				String maskName = msg - "ShopMaskHover: ";
				UpdateSelectedMask(maskName);
			}
			break;
		}
		case MessageType::SET_STRING:
		{
			SetStringMessage * strMes = (SetStringMessage *) message;
			playerName->strValue = strMes->value;
			break;
		}
		case MessageType::INTEGER_MESSAGE:
		{
			IntegerMessage * im = (IntegerMessage*) message;
			if (msg == "SetDifficulty")
			{
				difficulty->iValue = im->value;
			}
			else if (msg == "SetMasterVolume")
			{
				QueueAudio(new AMSet(AT_MASTER_VOLUME, im->value * 0.01f));
			}
			break;
		}
		case MessageType::COLLISSION_CALLBACK:
		{

			CollisionCallback * cc = (CollisionCallback*) message;
			Entity * one = cc->one;
			Entity * two = cc->two;
#define PLAYER 0
#define PROJ 1
//			std::cout<<"\nColCal: "<<cc->one->name<<" & "<<cc->two->name;

			Entity * playerEntity = NULL;
			Entity * other = NULL;
			int oneType = (one == playerEntity) ? PLAYER : PROJ;
			int twoType = (two == playerEntity) ? PLAYER : PROJ;
			int types[5] = {0,0,0,0,0};
			++types[oneType];
			++types[twoType];
			
			switch(one->physics->collisionCategory)
			{
				case CC_PESO:
				case CC_PLAYER:
					one->OnCollisionCallback(cc);
			}
			switch(two->physics->collisionCategory)
			{
				case CC_PESO:
				case CC_PLAYER:
					two->OnCollisionCallback(cc);
			}
			break;
		}
		case MessageType::STRING:
		{
			msg.RemoveSurroundingWhitespaces();
			int found = msg.Find("//");
			if (found > 0)
				msg = msg.Part(0,found);
			if (msg == "MainMenu")
			{	
				SetState(MAIN_MENU, true);
			}
			if (msg == "NewGame" || msg == "Retry")
				NewGame();
			if (msg == "BuyTaco")
			{
				// o.o
				player->BuyTaco();
			}
			if (msg == "OfferTacos")
			{
				state = TACO_TIME;
				// Force-feed. First stop.
				Message stop("Stop");
				playerEntity->ProcessMessage(&stop);
				// Push UI.
				MesMan.QueueMessages("PushUI(gui/BuyTaco.gui)");
				// Update buyTaco variable.
				buyTaco += 1000;
			}
			if (msg == "Run")
			{
				state = PLAYING_LEVEL;
				playerEntity->ProcessMessage(message);
			}
			else if (msg == "RecreateLevelParts")
			{
				RecreateLevelParts();
			}
			else if (msg == "QuitGame")
			{
				float r = sfxRand.Randf();
				if (r > 0.5f)
					QueueAudio(new AMPlaySFX("sfx/Adios.wav"));
				else 
					QueueAudio(new AMPlaySFX("sfx/Adios amigo.wav"));
				// Stop music?
				QueueAudio(new AMStopBGM());
				Message * msg = new Message("QuitApplication");
				msg->timeToProcess = Time::Now() + Time::Milliseconds(2000);
				MesMan.QueueDelayedMessage(msg);
				// Hide the main window straight away - so it doesn't interrupt the user if they are rushed?
				MainWindow()->Hide();
			}
			else if (msg == "ReturnToPreviousState")
			{
				switch(previousState)
				{
					case PLAYING_LEVEL:
					case GAME_OVER:
						NewGame();
						break;
					case MAIN_MENU:
						SetState(previousState, true);
						break;
				}
			}
			else if (msg == "Jump")
				Jump();
			else if (msg == "CycleCamera")
				CycleCamera(camera+1);
			else if (msg == "OnReloadUI")
			{
				UpdateUI();
			}
			else if (msg == "Shop")
			{
				SetState(IN_SHOP, true);
			}
			else if (msg.StartsWith("ShopMask: "))
			{
				String maskName = msg - "ShopMask: ";
				Mask * mask = Mask::GetByName(maskName);
				if (!mask)
				{
					assert(false && "No such mask");
					return;
				}				
				bool warrantsSaving = false;
				bool warrantsUIUpdate = false;
				if (mask->purchased)
				{
					// Lucha?.
					if (equippedMask == mask)
					{
						// Already equipped? Lucha!
						MesMan.QueueMessages("NewGame");
						return;
					}
					/// Equip!
					float r = sfxRand.Randf();
					if (r > 0.5f)
						QueueAudio(new AMPlaySFX("sfx/Buena eleccion.wav"));
					else 
						QueueAudio(new AMPlaySFX("sfx/Sabia decision.wav"));
					equippedMask = mask;
					equippedMaskName->strValue = mask->name;
					warrantsSaving = true;
					warrantsUIUpdate = true;
				}
				else 
				{
					// Try buy it.
					if (mask->price < totalMunny->iValue)
					{
						mask->purchased = true;
						purchasedMasks->strValue += ";" + mask->name;
						totalMunny->iValue -= mask->price;
						QueueAudio(new AMPlaySFX("sfx/Buena compra.wav"));
						// Autosave?
						warrantsSaving = true;
						warrantsUIUpdate = true;
					}
					// Error sound of insufficient pesos?
					else 
					{
						// Play sfx
						float r = sfxRand.Randf();
						if (r > 0.7f)
							QueueAudio(new AMPlaySFX("sfx/Caro.wav"));
						else if (r > 0.4f)
							QueueAudio(new AMPlaySFX("sfx/Tan caro.wav"));
						else
							QueueAudio(new AMPlaySFX("sfx/Demasiado caro.wav"));
					}
				}
				// Autosave
				if (warrantsSaving)
					AutoSave();
				// Update UI.
				if (warrantsUIUpdate)
				{
					UpdateUI();
					// Hover to same element as we were on earlier.
					QueueGraphics(new GMSetHoverUI(message->element->name));
				}
			}
			else if (msg == "NextK")
			{
				if (state != PLAYING_LEVEL)
					return;
				// Move the player to the next K mark (1k, 2k, etc.)
				// Move player up in the air a bit, as needed.
				int posKX = playerEntity->position.x;
				posKX /= 1000;
				int newX = posKX * 1000 + 1000;
				JumpTo(newX);
			}
			else if (msg.Contains("AutoSave"))
			{
				bool silent = msg.Contains("(silent)");
				bool ok = SaveGame();
				if (ok)
				{
					if (silent)
						return;
					GraphicsMan.QueueMessage(new GMSetUIs("CenterText", GMUI::TEXT, "Auto-save: Progress saved"));
					ScriptMan.NewScript(List<String>("Wait(1500)", "ClearCenterText"));
				}
				else 
				{
					GraphicsMan.QueueMessage(new GMSetUIs("CenterText", GMUI::TEXT, "Auto-save: Failed. Details: "+lastError));	
					ScriptMan.NewScript(List<String>("Wait(6000)", "ClearCenterText"));
				}
			}
			else if (msg == "OpenLoadScreen")
			{
				OpenLoadScreen();
			}
			else if (msg.Contains("LoadGame("))
			{
				bool ok = LoadGame(msg.Tokenize("()")[1]);
				if (ok)
				{
					// Data loaded. Check which state we should enter?
					if (currentLevel->iValue < 4)
					{
						// Enter the next-level straight away.
						MesMan.QueueMessages("NextLevel");
					}
					else 
					{
						SetState(IN_LOBBY);
					}
				}
				else {
					GraphicsMan.QueueMessage(new GMSetUIs("CenterText", GMUI::TEXT, "Load failed. Details: "+lastError));	
					ScriptMan.NewScript(List<String>("Wait(6000)", "ClearCenterText"));
				}
			}
			else if (msg == "OpenMainMenu" || msg == "Back" ||
				msg == "GoToMainMenu")
				OpenMainMenu();
			else if (msg == "LoadDefaultName")
			{
				LoadDefaultName();
			}
			else if (msg == "GoToPreviousstate")
			{
				SetState(previousState);
			}
			else if (msg == "OpenOptionsScreen")
			{
				SetState(EDITING_OPTIONS);
			}
			else if (msg == "ToggleMenu")
			{
				switch(state)
				{
					case PLAYING_LEVEL:
					case LEVEL_CLEARED:
						break;
					default:
						return;
				}
				// Pause the game.
				if (!paused)
				{
					Pause();
					// Bring up the in-game menu.
					OpenInGameMenu();
				}
				else 
				{
					inGameMenuOpened = false;
					UpdateUI();
					Resume();
				}
			}
			else if (msg.StartsWith("ShowLevelStats"))
			{
				showLevelStats = true;
				// Add level score to total upon showing level stats. o.o
				score->iValue += LevelScore()->iValue;
				ShowLevelStats();
			}
			else if (msg.StartsWith("HideLevelStats"))
			{
				showLevelStats = false;
				UpdateUI();
			}
			else if (msg == "Pause/Break")
			{
				TogglePause();
			}
			else if (msg == "ListEntitiesAndRegistrations")
			{
				std::cout<<"\nGraphics entities "<<GraphicsMan.RegisteredEntities()<<" physics "<<PhysicsMan.RegisteredEntities()
					<<" projectiles "<<projectileEntities.Size()<<" ships "<<shipEntities.Size();
			}
			else if (msg == "ReloadLevel")
			{
				LoadLevel();
			}
			else if (msg == "NextLevel")
			{
				// Next level? Restart?
				++currentLevel->iValue;
				if (currentLevel->iValue > 4)
				{
					currentLevel->iValue = 4;
				}
				LoadLevel();
			}
			else if (msg == "NextStage")
			{
				if (currentStage->iValue >= 8)
					return;
				++currentStage->iValue;
				currentLevel->iValue = 1;
				if (currentLevel->iValue > 8)
				{
					currentLevel->iValue = 8;
				}
				LoadLevel();
			}
			else if (msg == "PreviousLevel")
			{
				--currentLevel->iValue;
				if (currentLevel->iValue < 1)
				{
					std::cout<<"\nCannot go to previous level, already at level 1. Try switching stage.";
					currentLevel->iValue = 1;
				}
				LoadLevel();
			}
			else if (msg == "ClearLevel")
			{
				// Move the level-entity, the player will follow.
//				PhysicsMan.QueueMessage(new PMSetEntity(levelEntity, PT_POSITION, Vector3f(level.goalPosition - 5.f, 10, 0)));
//				GraphicsMan.QueueMessage(new GMSetCamera(levelCamera, CT_POSITION, Vector3f(level.goalPosition - 5.f, 10, 0)));
			}
			else if (msg == "FinishStage")
			{
				/// Add munny o.o
				money->iValue += 1000 +  (currentStage->iValue - 1) * 2000;
				ScriptMan.PlayScript("scripts/FinishStage.txt");
			}
			else if (msg.Contains("GoToLobby"))
			{
				SetState(IN_LOBBY);
			}
			else if (msg.StartsWith("ShowGearDesc:"))
			{
				String text = msg;
				text.Remove("ShowGearDesc:");
				GraphicsMan.QueueMessage(new GMSetUIs("GearInfo", GMUI::TEXT, text));
			}
			else if (msg.StartsWith("BuyGear:"))
			{
				// do stuff
				// Play some SFX too?
				
				// Auto-save.
				MesMan.QueueMessages("AutoSave(silent)");
			}
			else if (msg.Contains("ExitToMainMenu"))
			{
				SetState(MAIN_MENU);
			}
			if (msg.StartsWith("DisplayCenterText"))
			{
				String text = msg;
				if (text.Contains("$-L"))
				{
					text.Replace("$-L", currentStage->ToString()+"-"+currentLevel->ToString());
				}
				text.Replace("Stage $", "Stage "+currentStage->ToString());
				text.Remove("DisplayCenterText");
				text.RemoveInitialWhitespaces();
				GraphicsMan.QueueMessage(new GMSetUIs("CenterText", GMUI::TEXT, text));
			}
			else if (msg == "ClearCenterText")
				GraphicsMan.QueueMessage(new GMSetUIs("CenterText", GMUI::TEXT, Text()));
			else if (msg == "ResetCamera")
			{
				ResetCamera();
			}
			break;
		}
	}
}

/// Called from the render-thread for every viewport/AppWindow, after the main rendering-pipeline has done its job.
void SideScroller::Render(GraphicsState * graphicsState)
{
	switch(state)
	{
		case PLAYING_LEVEL:	
			if (!levelEntity)
				return;
//			RenderInLevel(graphicsState);
			break;
		default:
			return;
	}
}

/// o.o
void SideScroller::JumpTo(int newX)
{
	std::cout<<"\nJumping to "<<newX;
	if (newX <= playerEntity->position.x)
	{
		std::cout<<"\nClearng old content since jumping back.";
		/// Reset level creation then?
		levelLength = newX - 1000;
		/// Delete old level-parts so we get no duplicates.
		MapMan.DeleteEntities(levelEntities);
		levelEntities.Clear();
	}

	buyTaco = newX;
	// Pause physics. Set position.
	Physics.Pause();
	Graphics.Pause();
	playerEntity->position.x = newX;
	playerEntity->position.y = 3.f;
	playerEntity->RecalculateMatrix();
	playerEntity->physics->velocity = Vector3f();
	Physics.Resume();
	Graphics.Resume();
//	QueuePhysics(new PMSetEntity(playerEntity, PT_POSITION_X, newX));
//	QueuePhysics(new PMSetEntity(playerEntity, PT_POSITION_Y, 3.f));
//	QueuePhysics(new PMSetEntity(playerEntity, PT_VELOCITY, Vector3f()));
	/// Move camera position.
	QueueGraphics(new GMSetCamera(levelCamera, CT_SMOOTHED_POSITION, Vector3f(newX,0,0)));
	
}

// Update ui
void SideScroller::OnScoreUpdated()
{

}

/// Level score. If -1, returns current.
GameVariable * SideScroller::LevelScore(int stage, int level)
{
	String name = "Level "+String(stage)+"-"+String(level)+" score";
	GameVar * gv = GameVars.Get(name);
	if (!gv)
		gv = GameVars.CreateInt(name, 0);
	return gv;
}

/// Level score. If -1, returns current.
GameVariable * SideScroller::LevelKills(int stage, int level)
{
	String name = "Level "+String(stage)+"-"+String(level)+" kills";
	GameVar * gv = GameVars.Get(name);
	if (!gv)
		gv = GameVars.CreateInt(name, 0);
	return gv;
}

Entity * sky = NULL;
#include "Graphics/Animation/AnimationManager.h"

/// Starts a new game. Calls LoadLevel
void SideScroller::NewGame()
{	
	buyTaco = 0;
	pacoTacoX = 0;
	currentBGM = NULL;
	/// Play SFX
	float r = sfxRand.Randf();
	if (r > 0.8f)
		QueueAudio(new AMPlaySFX("sfx/Venga.wav"));
	else if (r > 0.6f)
		QueueAudio(new AMPlaySFX("sfx/Venga 2.wav"));
	else if (r > 0.4f)
		QueueAudio(new AMPlaySFX("sfx/Lucha.wav"));
	else if (r > 0.2f)
		QueueAudio(new AMPlaySFX("sfx/Venga lucha.wav"));
	else
		QueueAudio(new AMPlaySFX("sfx/Muchacho venga.wav"));

	AnimationMan.LoadFromDirectory("anim");

	/// Clear all pre-existing shit as needed.
	if (playerEntity)
	{
		levelEntities.Clear();
		MapMan.DeleteAllEntities();
		pesos.Clear();
		clouds.Clear();
		playerEntity = NULL;
	}

	levelLength = - breatherBlockSize * 0.5f;

	// Create sky - again. All entities are cleared on new game.
	sky = EntityMan.CreateEntity("Sky", ModelMan.GetModel("sprite.obj"), TexMan.GetTexture("0x44AAFF"));
	sky->position = Vector3f(500, 20, -2);
	sky->SetScale(Vector3f(200000, 40, 1));
	MapMan.AddEntity(sky, true, false);

	// Create player.
	NewPlayer();
	startDate = Time::Now();

	/// Create breather-block.
//	BreatherBlock();
	// Add initial level part
//	AddLevelPart();

	// Play script for animation or whatever.
	ScriptMan.PlayScript("scripts/NewGame.txt");

	// Attach camera?
	CycleCamera();
	/// Move camera to the player - disable or increase the smoothing parameter for an instant.
	QueueGraphics(new GMSetCamera(levelCamera, CT_SMOOTHING, 0.05f));
	/// Force render 1 frame to move the camera?
	QueueGraphics(new GMSetCamera(levelCamera, CT_SMOOTHED_POSITION, Vector3f(0,0,0)));

	// Set sun on top?
//	MesMan.QueueMessages("SetSunTime(12:00)");

	// Resume physics/graphics if paused.
	Resume();

	/// Update state, update gui
	SetState(PLAYING_LEVEL, true);
	// Start movin'!
	munny = 0;
	
	/// Let algorithm sort level-creation.
	JumpTo(0);
	player->Run();

	MesMan.QueueMessages("PopUI(GameOver)");
	MesMan.QueueMessages("PopUI(Shop)");
	paco = taco = NULL;
}

void SideScroller::RecreateLevelParts()
{
	MapMan.DeleteEntities(levelEntities);
	levelEntities.Clear();
	// reset created level distance and it should hopefully recreate itself.. lol
	levelLength = 0;
}

void SideScroller::Jump()
{
	Message jump("Jump");
	if (playerEntity)
		playerEntity->ProcessMessage(&jump);
}

/// o.o
void SideScroller::NewPlayer()
{
	// Add the player!
	playerEntity = CreateSprite("0xFF");
	playerEntity->position.y = 3.f;
//	playerEntity->Scale(0.5f);
	// Set up physics.
	PhysicsProperty * pp = playerEntity->physics = new PhysicsProperty();
	pp->requireGroundForLocalAcceleration = true;
	// Manually set up physics for it.
	pp->shapeType = ShapeType::SPHERE;
	pp->physicalRadius = 0.5f;
	pp->recalculatePhysicalRadius = false;
	pp->onCollision = true;
	pp->collisionFilter = CC_PESO | CC_ENVIRONMENT;
	pp->collisionCategory = CC_PLAYER;

	player = new LuchadorProperty(playerEntity);
	playerEntity->properties.AddItem(player);

	// Set up sprite-animation o.o'
	AnimationSet * set = AnimationMan.GetAnimationSet("Luchador");
	GraphicsProperty * gp = playerEntity->graphics; 
	gp->animationSet = set;
	gp->animStartTime = 0;
	gp->currentAnimation = set->GetAnimation("Run");
//	gp->temporalAliasingEnabled = true; // Smooth!
	gp->flags = RenderFlag::ALPHA_ENTITY;


	MapMan.AddEntity(playerEntity);
}


void SideScroller::Pause()
{
	paused = true;
	OnPauseStateUpdated();
}
void SideScroller::Resume()
{
	paused = false;
	OnPauseStateUpdated();
}

void SideScroller::TogglePause()
{
	paused = !paused;
	OnPauseStateUpdated();
}

void SideScroller::OnPauseStateUpdated()
{
	if (paused)
	{
		GraphicsMan.QueueMessage(new GraphicsMessage(GM_PAUSE_PROCESSING));
		PhysicsMan.Pause();
	}
	else {
		GraphicsMan.QueueMessage(new GraphicsMessage(GM_RESUME_PROCESSING));
		PhysicsMan.Resume();
	}
}

// Clean-up past level-parts
void SideScroller::CleanupOldBlocks()
{
	float cleanupX = levelLength - 100.f;
	// Check all entities?
	for (int i = 0; i < levelEntities.Size(); ++i)
	{
		Entity * entity = levelEntities[i];
		if (entity->position.x < cleanupX)
		{
			List<Entity*> subList = levelEntities;
			int duplicates = levelEntities.Duplicates();
			// Remove duplicates at the same time..
			levelEntities.RemoveItemUnsorted(entity);
			int after = levelEntities.Duplicates();
			assert(duplicates == after);

			assert(subList.Exists(levelEntities));

//			std::cout<<"\nDeleting "<<entity->name<<" at "<<entity->position;
			MapMan.DeleteEntity(entity);
			--i;
		}
	}

	// Clean up pesos?
	for (int i = 0; i < pesos.Size(); ++i)
	{	
		Entity * peso = pesos[i];
		if (peso->position.x < cleanupX)
		{
//			LogMain("Deleting peso "+String(((int)peso)%1000)+" at "+String(peso->position.x), INFO);		
//			std::cout<<"\nDeleting peso "<<peso<<" at "<<peso->position.x;
			pesos.RemoveItemUnsorted(peso);
			MapMan.DeleteEntity(peso);
			--i;
		}
	}

}


/// Loads target level. The source and separate .txt description have the same name, just different file-endings, e.g. "Level 1.png" and "Level 1.txt"
void SideScroller::LoadLevel(String fromSource)
{
	// Reset stats for this specific level.
	LevelKills()->iValue = 0;
	LevelScore()->iValue = 0;

	QueueGraphics(new GMSetUIb("LevelMessage", GMUI::VISIBILITY, false));


	showLevelStats = false;
	inGameMenuOpened = false;
	if (fromSource == "CurrentStageLevel")
	{
		fromSource = "Levels/Stage "+currentStage->ToString()+"/Level "+currentStage->ToString()+"-"+currentLevel->ToString();
	}		
	this->levelSource = fromSource;
	// Delete all entities.
	MapMan.DeleteAllEntities();
	shipEntities.Clear();
	projectileEntities.Clear();
	
	GraphicsMan.PauseRendering();
	PhysicsMan.Pause();

	/// o.o


	/// Add emitter for stars at player start.

	/// Add entity to track for both the camera, blackness and player playing field.
	Vector3f initialPosition = Vector3f(0,10,0);
	if (!levelEntity)
	{
		levelEntity = EntityMan.CreateEntity("LevelEntity", NULL, NULL);
		levelEntity->position = initialPosition;
		PhysicsProperty * pp = levelEntity->physics = new PhysicsProperty();
		pp->collisionsEnabled = false;
		pp->type = PhysicsType::KINEMATIC;
		// Set level camera to track the level entity.
		GraphicsMan.QueueMessage(new GMSetCamera(levelCamera, CT_ENTITY_TO_TRACK, levelEntity));
	}

	GraphicsMan.ResumeRendering();
	PhysicsMan.Resume();
	// Set state! UI updated from within.
	SetState(PLAYING_LEVEL);
	// Run start script.
	ScriptMan.PlayScript("scripts/OnLevelStart.txt");

	// o.o
	this->Resume();
}

void SideScroller::GameOver()
{
	if (state != GAME_OVER)
	{
		SetState(GAME_OVER);
		// Play script for animation or whatever.
		ScriptMan.PlayScript("scripts/OnDeath.txt");
		// End script by going back to menu or playing a new game.
	}
}

void SideScroller::LevelCleared()
{
	if (state != LEVEL_CLEARED)
	{
		SetState(LEVEL_CLEARED);
		ScriptMan.PlayScript("scripts/LevelComplete.txt");
	}
}

/// Opens main menu.
void SideScroller::OpenMainMenu()
{
	SetState(MAIN_MENU);
}

// Bring up the in-game menu.
void SideScroller::OpenInGameMenu()
{
	inGameMenuOpened = true;
	UpdateUI();
}

/// Auto-saves.
bool SideScroller::AutoSave()
{
	SaveFile save(Application::name, "AutoSave");
	String customHeaderData = "None";
	bool ok = save.OpenSaveFileStream(customHeaderData, true);
	if (!ok)
	{
		assert(false);
		return false;
	}
	std::fstream & stream = save.GetStream();	
	ok = GameVars.WriteTo(stream);
	if (ok)
	{
		std::cout<<"\nAuto-saved successfully.";
		return true;
	}
	LogMain("Auto-save failed", ERROR);
	return false;
}

bool SideScroller::AutoLoad()
{
	SaveFile save(Application::name, "AutoSave");
	bool ok = save.OpenLoadFileStream();
	if (!ok)
	{
		LogMain("Failed to open stream to AutoSave ("+save.Path()+"). New game it is.", INFO);
		return false;
	}
	std::fstream & stream = save.GetStream();	
	ok = GameVars.ReadFrom(stream);

	/// Flags masks as purchased as based on the save-file.
	List<String> maskNames = purchasedMasks->strValue.Tokenize(";");
	for (int j = 0; j < maskNames.Size(); ++j)
	{
		for (int i = 0; i < masks.Size(); ++i)
		{
			Mask & mask = masks[i];
			// Equipped?
			if (equippedMaskName->strValue == mask.name)
				equippedMask = &mask;
			String name = maskNames[j];
			if (mask.name == name)
				mask.purchased = true;
		}
	}
	return ok;
}


/// Saves current progress.
bool SideScroller::SaveGame()
{
	SaveFile save(Application::name, playerName->strValue + gameStartDate->ToString());
	String customHeaderData = "Name: "+playerName->strValue+"\nStage: "+currentStage->ToString()+" Level: "+currentLevel->ToString()+
		"\nScore: " + String(score->iValue) + 
		"\nSave date: " + Time::Now().ToString("Y-M-D") + 
		"\nStart date: " + startDate.ToString("Y-M-D");
	
	bool ok = save.OpenSaveFileStream(customHeaderData, true);
	if (!ok)
	{
		lastError = save.lastError;
		return false;
	}
	assert(ok);
	std::fstream & stream = save.GetStream();
	if (!GameVars.WriteTo(stream))
	{
		lastError = "Unable to save GameVariables to stream.";
		return false;
	}
	// Close the stream.
	stream.close();
	// Save custom data.
	return true;
}

/// Loads progress from target save.
bool SideScroller::LoadGame(String saveName)
{
	SaveFile save(Application::name, saveName);
	String cHeaderData;
	bool ok = save.OpenLoadFileStream();
	if (!ok)
	{
		lastError = save.lastError;
		return false;
	}
	std::fstream & stream = save.GetStream();
	if (!GameVars.ReadFrom(stream))
	{
		lastError = "Unable to read GameVariables from save.";
		return false;
	}
	// Close the stream.
	stream.close();
	return true;
}



void SideScroller::UpdatePlayerVelocity()
{
	assert(playerEntity);
	Message upVel("UpdateVelocity");
	playerEntity->ProcessMessage(&upVel);
}

void SideScroller::ResetCamera()
{
	levelCamera->projectionType = Camera::ORTHOGONAL;
	levelCamera->zoom = 15.f;
}

void SideScroller::SetPlayingFieldSize(Vector2f newSize)
{
	playingFieldSize = newSize;
	playingFieldHalfSize = newSize * .5f;
}

/// Saves previousState
void SideScroller::SetState(int newState, bool updateUI)
{
	if (newState != state)
	{
		// New previous-state?
		if (previousState != state)
			previousState = state;
		state = newState;
		// Update UI automagically?
		if (updateUI)
			UpdateUI();
		
		// Play music?
		switch(newState)
		{
			case IN_SHOP:
				QueueAudio(new AMPlaySFX("sfx/Bienvenido al mercado.wav"));
				break;
			case MAIN_MENU:
				QueueAudio(new AMPlayBGM("bgm/2015-04-13_mexican.ogg"));
				break;
			case PLAYING_LEVEL:
				QueueAudio(new AMPlayBGM("bgm/2015-04-28_Mariachi.ogg"));
				break;
		}

	}
}

