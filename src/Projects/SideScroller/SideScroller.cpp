/// Emil Hedemalm
/// 2015-01-20
/// Space shooter.
/// For the Karl-Emil SpaceShooter project, mainly 2014-2015/

#include "SideScroller.h"

#include "Application/Application.h"
#include "StateManager.h"

#include "Physics/Messages/CollisionCallback.h"
#include "Window/Window.h"
#include "Viewport.h"

#include "OS/OSUtil.h"
#include "File/SaveFile.h"

#include "Graphics/Messages/GMRenderPass.h"
#include "Render/RenderPass.h"

#include "Message/MathMessage.h"

#include "Input/InputManager.h"

#include "Physics/Integrators/FirstPersonIntegrator.h"
#include "Physics/CollisionDetectors/FirstPersonCD.h"
#include "Physics/CollisionResolvers/FirstPersonCR.h"

/// Particle system for sparks/explosion-ish effects.
Sparks * sparks = NULL;
Stars * stars = NULL;

ParticleEmitter * starEmitter = NULL;

bool paused = false;
float breatherBlockSize = 5.f;

/// 4 entities constitude the blackness.
List<Entity*> blacknessEntities;

void SetApplicationDefaults()
{
	Application::name = "SideScroller";
	TextFont::defaultFontSource = "img/fonts/font3.png";
	PhysicsProperty::defaultUseQuaternions = false;
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

void RegisterStates()
{
	sideScroller = new SideScroller();
	StateMan.RegisterState(sideScroller);
	StateMan.QueueState(sideScroller);
}

SideScroller::SideScroller()
{
	levelCamera = NULL;
	SetPlayingFieldSize(Vector2f(30,20));
	levelEntity = NULL;
	playingFieldPadding = 1.f;
	gearCategory = 0;
	previousState = state = 0;
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
	currentLevel = GameVars.CreateInt("currentLevel", 1);
	currentStage = GameVars.CreateInt("currentStage", 1);
	playerName = GameVars.CreateString("playerName", "Cytine");
	score = GameVars.CreateInt("score", 0);
	money = GameVars.CreateInt("money", 0);
	playTime = GameVars.CreateInt("playTime", 0);
	gameStartDate = GameVars.CreateTime("gameStartDate");
	difficulty = GameVars.CreateInt("difficulty", 1);

	Window * w = MainWindow();
	assert(w);
	Viewport * vp = w->MainViewport();
	assert(vp);
//	vp->renderGrid = false;	

	// Add custom render-pass to be used.
	RenderPass * rs = new RenderPass();
	rs->type = RenderPass::RENDER_APP_STATE;
	GraphicsMan.QueueMessage(new GMAddRenderPass(rs));

	// Set folder to use for saves.
	String homeFolder = OSUtil::GetHomeDirectory();
	homeFolder.Replace('\\', '/');
	SaveFile::saveFolder = homeFolder;

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

/// Main processing function, using provided time since last frame.
void SideScroller::Process(int timeInMs)
{
	Sleep(10);
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
	if (playerEntity->position.x > levelLength - 5.f)
	{
		// Create moar!
		BreatherBlock();
		AddLevelPart();
	}
	else if (playerEntity->position.y < -2.f)
	{
		// Deaded.
		QueuePhysics(new PMSetEntity(playerEntity, PT_PHYSICS_TYPE, PhysicsType::STATIC));
	}
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void SideScroller::OnExit(AppState * nextState)
{
	levelEntity = NULL;
	Sleep(50);
	// Register it for rendering.
	if (sparks)
		Graphics.QueueMessage(new GMUnregisterParticleSystem(sparks, true));
	if (stars)
		Graphics.QueueMessage(new GMUnregisterParticleSystem(stars, true));
	MapMan.DeleteAllEntities();
	Sleep(100);
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
		case MessageType::SET_STRING:
		{
			SetStringMessage * strMes = (SetStringMessage *) message;
			playerName->strValue = strMes->value;
			break;
		}
		case MessageType::INTEGER_MESSAGE:
		{
			IntegerMessage * im = (IntegerMessage*) message;
			if (msg == "SetGearCategory")
			{
				gearCategory = im->value;
				UpdateGearList();
			}
			else if (msg == "SetDifficulty")
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
		//	std::cout<<"\nCollision between "<<one->name<<" and "<<two->name;
			if (oneType == PLAYER)
			{
				one->OnCollisionCallback(cc);
			}
			else if (twoType == PLAYER)
			{
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
			if (msg == "NewGame")
				NewGame();
			else if (msg == "Jump")
				Jump();
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
			else if (msg == "OnReloadUI")
			{
				UpdateUI();
			}
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

/// Called from the render-thread for every viewport/window, after the main rendering-pipeline has done its job.
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


/// Starts a new game. Calls LoadLevel
void SideScroller::NewGame()
{	
	/// Clear all pre-existing shit as needed.
	if (playerEntity)
	{
		MapMan.DeleteAllEntities();
		playerEntity = NULL;
	}

	levelLength = - breatherBlockSize * 0.5f;

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
	if (!levelCamera)
		levelCamera = CameraMan.NewCamera("Level player-tracking camera");
	levelCamera->trackingMode = TrackingMode::ADD_POSITION;
	levelCamera->entityToTrack = playerEntity;
	levelCamera->position = Vector3f(0,0,0);
	levelCamera->distanceFromCenterOfMovement = 10.f;
	// Set camera
	QueueGraphics(new GMSetCamera(levelCamera));

	// Resume physics/graphics if paused.
	Resume();

	state = PLAYING_LEVEL;
	// Start movin'!
	UpdatePlayerVelocity();
}

Time lastJump = Time::Now();

void SideScroller::Jump()
{
	int jumpCooldownMs = 500;
	if ((now - lastJump).Milliseconds() < jumpCooldownMs)
		return;
	lastJump = now;
	QueuePhysics(new PMSetEntity(playerEntity, PT_VELOCITY, playerEntity->Velocity() + Vector3f(0,5.f,0)));
}


/// o.o
void SideScroller::NewPlayer()
{
	// Add the player!
	playerEntity = EntityMan.CreateEntity("Player", ModelMan.GetModel("Sphere.obj"), TexMan.GetTexture("0xFF"));
	playerEntity->position.y = 3.f;
	playerEntity->Scale(0.5f);
	// Set up physics.
	PhysicsProperty * pp = playerEntity->physics = new PhysicsProperty();
	pp->requireGroundForLocalAcceleration = true;

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


void SideScroller::BreatherBlock()
{
	Entity * block = EntityMan.CreateEntity("Block", ModelMan.GetModel("cube.obj"), TexMan.GetTexture("0x55"));
	PhysicsProperty * pp = block->physics = new PhysicsProperty();
	pp->shapeType = ShapeType::AABB;
	block->position.x = levelLength + breatherBlockSize * 0.5f;
	block->SetScale(Vector3f(breatherBlockSize, 1, 1));
	levelLength += breatherBlockSize;
	MapMan.AddEntity(block);
}

Random levelRand;

/// Creates a 20+ meters level-part.
void SideScroller::AddLevelPart()
{
	float partLength = 20.f;
	// Fetch start and end bounds.
	float startBound = levelLength,
		endBound = levelLength + partLength;
	
	// Create some blocks based on how large this part is.
	float blockSize = 2.f;
	int blocks = partLength / blockSize;
	
	float ratioBlocks = 0.7f;
	int blocksToCreate = blocks * ratioBlocks;
	for (int i = 0; i < blocksToCreate; ++i)
	{
		Entity * block = EntityMan.CreateEntity("LevelPart-block", ModelMan.GetModel("cube.obj"), TexMan.GetTexture("0x55"));
		Vector3f position;
		position.x = levelRand.Randf(partLength);
		position.x += levelLength;
		position.x += blockSize * 0.5f;
		block->position = position;
		block->Scale(Vector3f(blockSize, 1, 1));
		PhysicsProperty * pp = block->physics = new PhysicsProperty();
		pp->shapeType = ShapeType::AABB;
		MapMan.AddEntity(block);
	}
	levelLength += partLength;
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
void SideScroller::SetState(int newstate, bool updateUI)
{
	if (previousState != state)
		previousState = state;
	state = newstate;
	// Update UI automagically?
	if (updateUI)
		UpdateUI();
}

