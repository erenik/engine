/// Emil Hedemalm
/// 2015-01-20
/// Space shooter.
/// For the Karl-Emil SpaceShooter project, mainly 2014-2015/

#include "SideScroller.h"

/// Particle system for sparks/explosion-ish effects.
bool paused = false;
float breatherBlockSize = 5.f;
Random levelRand;
Random sfxRand;
/// Starts at 0, increments after calling BreatherBlock and AddLevelPart
float levelLength;

/// 4 entities constitude the blackness.
List<Entity*> blacknessEntities;

void SetApplicationDefaults()
{
	Application::name = "SideScroller";
	Application::quitOnHide = false;
	TextFont::defaultFontSource = "img/fonts/font3.png";
	PhysicsProperty::defaultUseQuaternions = false;
	Viewport::defaultRenderGrid = false;
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
Entity * paco = NULL, * taco = NULL;
/** Dynamically created ones, to be cleaned up as we go.
	Blocks, pesos, clouds all fit therein?
*/
Entities levelEntities;
Entities pesos, clouds;

Mask * equippedMask = NULL;
int munny = 0;
GameVar * totalMunny = 0; // Total munny from all attempts. Summed up upon death? And auto-saved then?
GameVar * purchasedMasks = 0;
GameVar * equippedMaskName = 0;
GameVar * attempts = 0;
float distance = 0;

/// Toggled when skipping parts. Defaulf true. When false no block should be created as to speed up processing.
bool blockCreationEnabled = true; 

enum 
{
	PERSP_CAMERA,
	ORTHO_CAMERA,
	CAMERA_TYPES,
};
int camera = ORTHO_CAMERA;

void CycleCamera(int toWhich = -1)
{
	if (toWhich > -1)
		camera = toWhich % CAMERA_TYPES;
	if (!levelCamera)
		levelCamera = CameraMan.NewCamera("Level Camera");
	// General properties.
	levelCamera->trackingMode = TrackingMode::ADD_POSITION;
	levelCamera->entityToTrack = playerEntity;
	levelCamera->position = Vector3f(0,0,0);
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


class PesoProperty : public EntityProperty 
{
public:
	PesoProperty(Entity * owner)
		: EntityProperty("PesoProp", EP_PESO, owner)
	{
		value = 1;
		sleeping = false;
	};
	virtual void OnCollisionCallback(CollisionCallback * cc)
	{
		if (sleeping)
			return;
		sleeping = true;
		munny += value;
		sideScroller->UpdateMunny();
		pesos.RemoveItemUnsorted(owner); // Remove self from clean-up procedure.
		MapMan.DeleteEntity(owner); // Remove self.
	}
	bool sleeping;
	int value;
};


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

SideScroller::SideScroller()
{
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
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void SideScroller::OnEnter(AppState * previousState)
{
	/// Enable Input-UI navigation via arrow-keys and Enter/Esc.
	Input.ForceNavigateUI(true);

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
		}
	}
}

void SideScroller::ProcessLevel(int timeInMs)
{
	// Extend arena as necessary.
	while(CreateNextLevelParts())
		;

}

/// Creates the next level parts.
bool SideScroller::CreateNextLevelParts()
{
	float distToEdge = distToEdge = levelLength - playerEntity->position.x;
	/// If skipping level, just move past some blocks without actually creating them (since creation/destruction lags).
	if (distToEdge < -55)
	{
		blockCreationEnabled = false;
	}
	else if (distToEdge < 25.f)
	{
		blockCreationEnabled = true;
	}
	else
		return false;

	/// If approaching a 1k mark.
	int modK = (int)levelLength % 1000;
	if (modK > 975)
	{
		// 20 breather-blocks.
		BreatherBlock();
		PacoTaco();
	}
	else 
	{
		// Create moar!
		BreatherBlock();
		AddLevelPart();
		// Clean-up past level-parts
		CleanupOldBlocks();
	}
//	distToEdge = levelLength - playerEntity->position.x;
	return true;
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
				QueuePhysics(new PMSetEntity(playerEntity, PT_POSITION_X, posKX * 1000 + 1000));
				QueuePhysics(new PMSetEntity(playerEntity, PT_POSITION_Y, 3.f));
				QueuePhysics(new PMSetEntity(playerEntity, PT_VELOCITY, Vector3f()));
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
			if (msg.Contains("StartMoveShip"))
			{
				String dirStr = msg - "StartMoveShip";
				int dir = Direction::Get(dirStr);
				movementDirections.Add(dir);
				UpdatePlayerVelocity();
			}
			else if (msg.Contains("StopMoveShip"))
			{
				String dirStr = msg - "StopMoveShip";
				int dir = Direction::Get(dirStr);
				while(movementDirections.Remove(dir));
				UpdatePlayerVelocity();
			}
			else if (msg == "ToggleBlackness")
			{
				if (blacknessEntities.Size())
				{
					bool visible = blacknessEntities[0]->IsVisible();
					GraphicsMan.QueueMessage(new GMSetEntityb(blacknessEntities, GT_VISIBILITY, !visible));
					Viewport * viewport = MainWindow()->MainViewport();
					viewport->renderGrid = visible;
				}
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
		MapMan.DeleteAllEntities();
		levelEntities.Clear();
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
	BreatherBlock();
	// Add initial level part
	AddLevelPart();

	// Play script for animation or whatever.
	ScriptMan.PlayScript("scripts/NewGame.txt");

	// Attach camera?
	CycleCamera();

	// Set sun on top?
//	MesMan.QueueMessages("SetSunTime(12:00)");

	// Resume physics/graphics if paused.
	Resume();

	/// Update state, update gui
	SetState(PLAYING_LEVEL, true);
	// Start movin'!
	munny = 0;
	UpdatePlayerVelocity();
	MesMan.QueueMessages("PopUI(GameOver)");
	MesMan.QueueMessages("PopUI(Shop)");
	paco = taco = NULL;
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

	playerEntity->properties.AddItem(new LuchadorProperty(playerEntity));

	// Set up sprite-animation o.o'
	AnimationSet * set = AnimationMan.GetAnimationSet("Luchador");
	GraphicsProperty * gp = playerEntity->graphics; 
	gp->animationSet = set;
	gp->animStartTime = 0;
	gp->currentAnimation = set->GetAnimation("Run");
	
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

/// In the current sub-level
List<Entity*> blocksAdded;
float blockSize = 2.f;
String colorStr = "0xb19d7c";

// Appends a block. Default size 2 (blockSize?).
void Block(float size = 2) 
{
	if (size <= 0)
		size = blockSize;
//	"0x55"
	if (blockCreationEnabled)
	{
		Entity * block = EntityMan.CreateEntity("LevelPart-block", ModelMan.GetModel("cube.obj"), TexMan.GetTexture(colorStr));
		Vector3f position;
		position.x += levelLength;
		position.x += size * 0.5f;

		/// Scale up ground-tiles.
		float scaleY = 25.f;
		position.y -= scaleY * 0.5f - 0.5f;

		block->position = position;
		block->Scale(Vector3f(size, scaleY, 1));
		PhysicsProperty * pp = block->physics = new PhysicsProperty();
		pp->shapeType = ShapeType::AABB;
		pp->collisionCategory = CC_ENVIRONMENT;
		pp->collisionFilter = CC_PLAYER;
		MapMan.AddEntity(block);
		levelEntities.AddItem(block);
		blocksAdded.AddItem(block);
	}
	levelLength += size;
}

/// Creates the framework for a custom block.
Entity * RampUpBlock(float width = 2.f, float height = 2.f)
{
	String rampColor = "0x55FF";
	Texture * rampTex = TexMan.GetTexture(rampColor);
	Entity * block = EntityMan.CreateEntity("LevelPart-block", 
		ModelMan.GetModel("rampUp1x1.obj"), 
		rampTex);
	Vector3f position;
	position.x += levelLength;
	position.x += width * 0.5f;

	/// Scale up ground-tiles.
	float scaleY = height;
	position.y -= scaleY * 0.5f - 0.5f;

	block->position = position;
	block->position.y += height;
	block->Scale(Vector3f(width, height, 1));
	PhysicsProperty * pp = block->physics = new PhysicsProperty();
	
	pp->shapeType = ShapeType::MESH;
	pp->friction = 0.f; // no friction.
	pp->restitution = 1.f; // Maximum bounce.
	
	pp->collisionCategory = CC_ENVIRONMENT;
	pp->collisionFilter = CC_PLAYER;
	MapMan.AddEntity(block);
	levelEntities.AddItem(block);
//	blocksAdded.AddItem(block);
	
	// Add extension-block beneath it?
	block = EntityMan.CreateEntity("UnderRamp",
		ModelMan.GetModel("cube.obj"),
		rampTex);
	position.y -= 5.f;
	block->SetScale(Vector3f(width, 5.f, 1.f));
	MapMan.AddEntity(block, true, true);
	levelEntities.AddItem(block);

	return block;
}

/// Creates the framework for a custom block.
Entity * CustomBlock(float width = 2.f, float height = 0.f)
{
	Entity * block = EntityMan.CreateEntity("LevelPart-block", 
		ModelMan.GetModel("cube.obj"), 
		TexMan.GetTexture(colorStr));
	Vector3f position;
	position.x += levelLength;
	position.x += width * 0.5f;

	/// Scale up ground-tiles.
	float scaleY = 25.f;
	position.y -= scaleY * 0.5f - 0.5f;

	block->position = position;
	block->position.y += height;
	block->Scale(Vector3f(width, scaleY, 1));
	PhysicsProperty * pp = block->physics = new PhysicsProperty();
	pp->shapeType = ShapeType::AABB;
	pp->collisionCategory = CC_ENVIRONMENT;
	pp->collisionFilter = CC_PLAYER;
	MapMan.AddEntity(block);
	levelEntities.AddItem(block);
	blocksAdded.AddItem(block);
	return block;
}

void Hole(float holeSize) // Appends a hole, default side 2.
{
	if (blockCreationEnabled)
	{
		/// Add graphical-only blocks where regular blocks are missing.
		Entity * block = EntityMan.CreateEntity("LevelPart-block", ModelMan.GetModel("cube.obj"), TexMan.GetTexture(colorStr));
		Vector3f position;
		position.x += levelLength;
		position.x += holeSize * 0.5f;

		/// Scale up ground-tiles.
		float scaleY = 25.f;
		position.y -= scaleY * 0.5f - 0.5f; 
		position.y -= 3.0f; // 2 down additionally to simulate hole.

		block->position = position;
		block->Scale(Vector3f(holeSize, scaleY, 1));

		MapMan.AddEntity(block, true, false);
		levelEntities.AddItem(block);
	}
//	blocksAdded.AddItem(block);
	// Increment level-length.
	levelLength += holeSize;
}

void SideScroller::FlatPart() // Just flat, 10 pieces.
{
	for (int i = 0; i < 10; ++i)
	{
		Block();
	}
}

void SideScroller::LinearHoles(int numHoles) // With a number of holes at varying positions, always with 1 block in between. Max 5 holes.
{
	for (int i = 0; i < 10; ++i)
	{
		if (i % 2 == 0 || numHoles < 0)
			Block();
		else 
		{
			Hole(blockSize);
			--numHoles;
		}
	}
}

void SideScroller::DoubleHoles(int numHoles) // With a number of holes at verying positions, always 1 block in between. Max ... 3 holes? 2 + 1 + 2 + 1 + 2
{
	for (int i = 0; i < 10; ++i)
	{
		if (i%3 == 0 || numHoles < 0)
			Block();
		else
		{
			Hole(blockSize);
			--numHoles;
		}
	}
}
	
void SideScroller::TripleHoles(int numHoles) // With a number of holes at varying positions, always 1 block in between. Max 2 holes. 3 + 2 + 3
{
	for (int i = 0; i < 10; ++i)
	{
		if (i % 4 == 0 || numHoles < 0)
			Block();
		else
		{
			Hole(blockSize);
			--numHoles;
		}
	}	
}

/// A big hole, with a platform before it.
void SideScroller::BigHole(float holeSize)
{
	RampUpBlock(2, 1.f);
//	CustomBlock(2, 2.f);
	Hole(holeSize);
}

void SideScroller::BreatherBlock(float width /* = 5.f*/)
{
	Block(width);
}

void SideScroller::PacoTaco()
{
	if (!paco)
	{
		paco = CreateSprite("img/Outdoor - Mexican town/Taco_guy.png");
		Texture * tex = paco->diffuseMap;
		paco->position.z = -0.01f;
		paco->SetScale(Vector3f(tex->size * 0.005f, 1));
		SetOnGround(paco);
		
		// Set scale accordingly.
		taco = CreateSprite("img/Outdoor - Mexican town/Taco_stand.png");
		taco->position.z = -0.02f;
		taco->SetScale(Vector3f(taco->diffuseMap->size * 0.005f, 1));
		SetOnGround(taco);
		// Add to map.
		MapMan.AddEntity(paco, true, false);
		MapMan.AddEntity(taco, true, false);
	}
	// Set position if far away.
	float dist = AbsoluteValue(paco->position.x - levelLength);
	if (dist > 100)
	{
		// Get closest K.
		int k = (levelLength + 500) / 1000;
		float posX = k * 1000;
		QueuePhysics(new PMSetEntity(paco, PT_POSITION_X, posX));
		QueuePhysics(new PMSetEntity(taco, PT_POSITION_X, posX + 2.0f));
	} 
}


// Add some pesos!
void AddPesos()
{
	// o.o
	for (int i = 0; i < blocksAdded.Size(); ++i)
	{
		Entity * block = blocksAdded[i];
		Entity * peso = CreateSprite("img/Pesos/Peso_Mexicano_1921_dos_cont.png");
		peso->properties.Add(new PesoProperty(peso));
		peso->Scale(0.6f);
		peso->SetPosition(block->position + Vector3f(0, block->scale.y * 0.5f + 2,0));
		PhysicsProperty * pp = peso->physics = new PhysicsProperty();
		pp->shapeType = ShapeType::SPHERE;
		pp->recalculatePhysicalRadius = false;
		pp->physicalRadius = 0.5f;
		pp->noCollisionResolutions = true;
		pp->collissionCallback = true;
		pp->collisionCategory = CC_PESO;
		pp->collisionFilter = CC_PLAYER;
		MapMan.AddEntity(peso);

		pesos.AddItem(peso); // Add so it is cleaned up later on as we go.
	//	LogMain("Creating peso "+String(((int)peso)%1000)+" at "+String(peso->position.x), INFO);
		assert(pesos.Duplicates() == 0);
	}
}

void AddClouds()
{
	List<String> cloudTextures;
	cloudTextures.Add("img/Clouds/Cloud1.png",
		"img/Clouds/Cloud2.png");

	// Add some random amount of clouds. Give them some speed.
	int numC = levelRand.Randi(5);
	for (int i = 0; i < numC; ++i)
	{
		int whichCloud = levelRand.Randi(cloudTextures.Size() + 1) % cloudTextures.Size();
		Texture * tex = TexMan.GetTexture(cloudTextures[whichCloud]);
		Entity * cloud = CreateSprite("0xFFAA");
		cloud->diffuseMap = tex;
		cloud->position.x = levelLength + levelRand.Randi(50.f);
		cloud->position.y = levelRand.Randi(25) + 5.f;
		cloud->position.z = -0.9f;
		cloud->SetScale(Vector3f(tex->size, 1) * 0.02f);
		/// No collisions! But some speed o-o
		PhysicsProperty * pp = cloud->physics = new PhysicsProperty();
		pp->collisionsEnabled = false;
		pp->currentVelocity = pp->velocity = Vector3f(-(1 + levelRand.Randf()) * 0.2f,0,0);
		pp->type = PhysicsType::KINEMATIC;

		MapMan.AddEntity(cloud, true, true);
		levelEntities.AddItem(cloud);
	}
}

void LongCactus(Entity * aboveBlock)
{
	Entity * cactus = CreateSprite("img/Outdoor - Mexican town/Big_fucking_cactus.png");
	float cactusSize = 1.5f + levelRand.Randf(1.f);
	cactus->position.x = aboveBlock->position.x;
	cactus->position.y = aboveBlock->position.y + aboveBlock->scale.y * 0.5f + cactusSize * 0.5f;
	cactus->position.z = -0.1f;
	cactus->Scale(Vector3f(1,cactusSize,1));
	MapMan.AddEntity(cactus, true, false);
}

void ShortCactus(Entity * aboveBlock)
{
	// Randomize amount?
	int cactii = levelRand.Randi(5) + 1;
	int initialSign = levelRand.Randi(10) > 5? 1 : -1;
	float initialSize = 0.5f + levelRand.Randf(0.5f);
	for (int i = 0; i < cactii; ++i)
	{
		Entity * cactus = CreateSprite("img/Outdoor - Mexican town/Barrel_cactus.png");
		cactus->position.x = aboveBlock->position.x;
		float scale = (1.f - i * 0.2f) * initialSize;
		cactus->scale = Vector3f(1,1,1) * scale; // scale down steadily
		// If non-1 cactii, offset X-position a bit.
		if (i > 0)
		{
			float offset = (i+1) / 2 * 0.5f;
			if (i % 2 == 0)
				offset *= -1;
 			cactus->position.x += offset * initialSign * initialSize;
		}
		cactus->position.y = aboveBlock->position.y + aboveBlock->scale.y * 0.5f + scale * 0.5f;
		cactus->position.z = -0.1f + i * 0.01f; // move steadily forward.
		cactus->hasRescaled = true;
		cactus->RecalculateMatrix();
		// depth-sort when rendering.
		MapMan.AddEntity(cactus, true, false);
	}
}

/// Creates a 20+ meters level-part.
void SideScroller::AddLevelPart()
{
	blocksAdded.Clear();
	// Depending on level.. or possibly random chance.
	int k = levelLength / 1000;
	float r = levelRand.Randf();
	switch(k)
	{
		case 0: /// 0 to 1000 meters.
			LinearHoles(levelRand.Randi(6)); break;
		case 1: /// 1k to 2k - introduce the double-holes..!
			if (r > 0.5f)
				LinearHoles(levelRand.Randi(6));
			else
				DoubleHoles(levelRand.Randi(5));
			break;
		case 2: /// 2k to 3k - introduce triple-holes..!
			if (r > 0.7f)
				LinearHoles(levelRand.Randi(6));
			else if (r > 0.5f)
				DoubleHoles(levelRand.Randi(5));
			else
				TripleHoles(levelRand.Randi(4));
			break;
		case 3: /// 3k to 4k - Big holes!
			if (r > 0.5f)
				BigHole(levelRand.Randi(10.f));
			else
				LinearHoles(levelRand.Randi(4));
			break;
		default:
			LinearHoles(levelRand.Randi(6));
	}
	// Add some pesos!
	AddPesos();
	AddClouds();

	// Add some cacti.
	for (int i = 0; i < blocksAdded.Size(); ++i)
	{
		// Random chance.
		float r = levelRand.Randf(1.f);
		if (r < 0.6f)
			continue;
		Entity * block = blocksAdded[i];
		r = levelRand.Randf(1.f);
		if (r > 0.5f) // 0.9 to 1.0
			LongCactus(block);
		else // from 0.7 to 0.9
			ShortCactus(block);
	}

}

void AddDBLPart() // Difficulty-By-Length, randomly generated. Used in initial test
{
	float partLength = 20.f;
	// Fetch start and end bounds.
	float startBound = levelLength,
		endBound = levelLength + partLength;
	
	// Create some blocks based on how large this part is.
	float blockSize = 2.f;
	int blocks = partLength / blockSize;
	
	int blocksToCreate = blocks;
	/// Decrease chance of blocks spawning based on distance traveled.
	float ratioBlocks = 0.9f - levelLength * 0.001f;
	for (int i = 0; i < blocksToCreate; ++i)
	{
		/// Check if should place here.
		if (levelRand.Randf(1.f) > ratioBlocks)
		{
			Hole(blockSize);
			continue;
		}
		Block();
	}
}

// Clean-up past level-parts
void SideScroller::CleanupOldBlocks()
{
	assert(levelEntities.Duplicates() == 0);
	assert(pesos.Duplicates() == 0);

	float cleanupX = playerEntity->position.x - 20.f;
	// Check all entities?
	for (int i = 0; i < levelEntities.Size(); ++i)
	{
		Entity * entity = levelEntities[i];
		if (entity->position.x < cleanupX)
		{
			levelEntities.RemoveItem(entity);
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
		ScriptMan.PlayScript("scripts/GameOver.txt");
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
	Vector3f totalVec;
	for (int i = 0; i < movementDirections.Size(); ++i)
	{
		Vector3f vec = Direction::GetVector(movementDirections[i]);
		totalVec += vec;
	}
	totalVec.Normalize();

	totalVec = Vector3f(1,0,0);
	/// o.o
	float playerSpeed = 15.f;
	if (equippedMask)
		playerSpeed += equippedMask->speedBonus;
	totalVec *= playerSpeed;
	bool moving = true;
	// Set acceleration?
	if (moving)
	{
		QueuePhysics(new PMSetEntity(playerEntity, PT_ACCELERATION, totalVec));
	}
	else 
		QueuePhysics(new PMSetEntity(playerEntity, PT_ACCELERATION, Vector3f()));
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

