/// Emil Hedemalm
/// 2015-01-20
/// Space shooter.
/// For the Karl-Emil SpaceShooter project, mainly 2014-2015/

#include "SpaceShooter2D.h"

#include "Application/Application.h"
#include "StateManager.h"

#include "Input/InputManager.h"
#include "Input/Action.h"

#include "Physics/Messages/CollisionCallback.h"
#include "Window/Window.h"
#include "Viewport.h"

#include "OS/OSUtil.h"
#include "File/SaveFile.h"

#include "Graphics/Messages/GMRenderPass.h"
#include "Render/RenderPass.h"

#include "UI/UIList.h"

#include "Game/GameVariableManager.h"
#include "Message/MathMessage.h"

/// Particle system for sparks/explosion-ish effects.
Sparks * sparks = NULL;
Stars * stars = NULL;

ParticleEmitter * starEmitter = NULL;

bool paused = false;

List<Weapon> Weapon::types;
List<Ship> Ship::types;

/// 4 entities constitude the blackness.
List<Entity*> blacknessEntities;

void SetApplicationDefaults()
{
	Application::name = "SpaceShooter2D";
	TextFont::defaultFontSource = "img/fonts/font3.png";
	PhysicsProperty::defaultUseQuaternions = false;
}

// Global variables.
SpaceShooter2D * spaceShooter = NULL;
Ship playerShip;
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
int gearCategory = 0;

void RegisterStates()
{
	spaceShooter = new SpaceShooter2D();
	StateMan.RegisterState(spaceShooter);
	StateMan.QueueState(spaceShooter);
}

SpaceShooter2D::SpaceShooter2D()
{
	levelCamera = NULL;
	SetPlayingFieldSize(Vector2f(30,20));
	levelEntity = NULL;
	playingFieldPadding = 1.f;
}

SpaceShooter2D::~SpaceShooter2D()
{
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void SpaceShooter2D::OnEnter(AppState * previousState)
{
	/// Create game variables.
	currentLevel = GameVars.CreateInt("currentLevel", 1);
	currentStage = GameVars.CreateInt("currentStage", 1);
	playerName = GameVars.CreateString("playerName", "Cytine");
	score = GameVars.CreateInt("score", 0);
	money = GameVars.CreateInt("money", 0);
	playTime = GameVars.CreateInt("playTime", 0);
	gameStartDate = GameVars.CreateTime("gameStartDate");

	Window * w = MainWindow();
	assert(w);
	Viewport * vp = w->MainViewport();
	assert(vp);
	vp->renderGrid = false;	

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
	sparks = new Sparks(true);
	// Register it for rendering.
	Graphics.QueueMessage(new GMRegisterParticleSystem(sparks, true));
	
	stars = new Stars(true);
	stars->deleteEmittersOnDeletion = true;
	Graphics.QueueMessage(new GMRegisterParticleSystem(stars, true));
	
	/// Add emitter
	starEmitter = new ParticleEmitter();
	Graphics.QueueMessage(new GMAttachParticleEmitter(starEmitter, stars));


	// Remove overlay.
	// Set up ui.
	if (!ui)
		CreateUserInterface();
	GraphicsMan.QueueMessage(new GMSetUI(ui));
	GraphicsMan.QueueMessage(new GMSetOverlay(NULL));

	// Load Space Race integrator
	integrator = new SSIntegrator(0.f);
	PhysicsMan.QueueMessage(new PMSet(integrator));
	cd = new SpaceShooterCD();
	PhysicsMan.QueueMessage(new PMSet(cd));
	cr = new SpaceShooterCR();
	PhysicsMan.QueueMessage(new PMSet(cr));

	PhysicsMan.checkType = AABB_SWEEP;

	/// Enter main menu
	OpenMainMenu();

	// Run OnEnter.ini start script if such a file exists.
	Script * script = new Script();
	script->Load("OnEnter.ini");
	ScriptMan.PlayScript(script);
}


Time now;
int64 nowMs;
int timeElapsedMs;

/// Main processing function, using provided time since last frame.
void SpaceShooter2D::Process(int timeInMs)
{
	Sleep(10);
//	std::cout<<"\nSS2D entities: "<<shipEntities.Size() + projectileEntities.Size() + 1;
//	if (playerShip) std::cout<<"\nPlayer position: "<<playerShip.position;

	now = Time::Now();
	nowMs = now.Milliseconds();
	assert(nowMs >= 0);
	timeElapsedMs = timeInMs;
	
	Cleanup();

	switch(mode)
	{
		case PLAYING_LEVEL:
		{
			if (paused)
				return;
			level.Process(timeInMs);
		}
	}
}

void SpaceShooter2D::Cleanup()
{
	/// Remove projectiles which have been passed by.
	for (int i = 0; i < projectileEntities.Size(); ++i)
	{
		Entity * proj = projectileEntities[i];
		if (proj->position[0] < despawnPositionLeft ||
			proj->position[0] > spawnPositionRight ||
			proj->position[1] < -1.f ||
			proj->position[1] > playingFieldSize[1] + 2.f)
		{
			MapMan.DeleteEntity(proj);
			projectileEntities.Remove(proj);
			--i;
		}
	}

	/// Clean ships.
	for (int i = 0; i < level.ships.Size(); ++i)
	{
		Ship & ship = level.ships[i];
		if (!ship.entity)
			continue;
		// Check if it should de-spawn.
		if (ship.entity->position[0] < despawnPositionLeft)
		{
			MapMan.DeleteEntity(ship.entity);
			shipEntities.Remove(ship.entity);
			ship.entity = NULL;
		}
	}
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void SpaceShooter2D::OnExit(AppState * nextState)
{
	levelEntity = NULL;
	Sleep(50);
	// Register it for rendering.
	Graphics.QueueMessage(new GMUnregisterParticleSystem(sparks, true));
	Graphics.QueueMessage(new GMUnregisterParticleSystem(stars, true));
	MapMan.DeleteAllEntities();
	Sleep(100);
}


/// Creates the user interface for this state
void SpaceShooter2D::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/MainMenu.gui");
}


/// Callback function that will be triggered via the MessageManager when messages are processed.
void SpaceShooter2D::ProcessMessage(Message * message)
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
			break;
		}
		case MessageType::COLLISSION_CALLBACK:
		{

			CollisionCallback * cc = (CollisionCallback*) message;
			Entity * one = cc->one;
			Entity * two = cc->two;
#define SHIP 0
#define PROJ 1
//			std::cout<<"\nColCal: "<<cc->one->name<<" & "<<cc->two->name;

			Entity * shipEntity1 = NULL;
			Entity * other = NULL;
			int oneType = (one == playerShip.entity || shipEntities.Exists(one)) ? SHIP : PROJ;
			int twoType = (two == playerShip.entity || shipEntities.Exists(two)) ? SHIP : PROJ;
			int types[5] = {0,0,0,0,0};
			++types[oneType];
			++types[twoType];
		//	std::cout<<"\nCollision between "<<one->name<<" and "<<two->name;
			if (oneType == SHIP)
			{
				ShipProperty * shipProp = (ShipProperty*)one->GetProperty(ShipProperty::ID());
				shipProp->OnCollision(two);
			}
			else if (twoType == SHIP)
			{
				ShipProperty * shipProp = (ShipProperty*)two->GetProperty(ShipProperty::ID());
				shipProp->OnCollision(one);
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
			else if (msg == "AutoSave")
			{
				bool ok = SaveGame();
				if (ok)
				{
					GraphicsMan.QueueMessage(new GMSetUIs("CenterText", GMUI::TEXT, "Auto-save: Progress saved"));
					ScriptMan.NewScript(List<String>("Wait(3000)", "ClearCenterText"));
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
						mode = IN_LOBBY;
						UpdateUI();
					}
				}
				else {
					GraphicsMan.QueueMessage(new GMSetUIs("CenterText", GMUI::TEXT, "Load failed. Details: "+lastError));	
					ScriptMan.NewScript(List<String>("Wait(6000)", "ClearCenterText"));
				}
			}
			else if (msg == "Back")
			{
				// Default.
				OpenMainMenu();
			}
			else if (msg == "LoadDefaultName")
			{
				GraphicsMan.QueueMessage(new GMSetUIs("PlayerName", GMUI::STRING_INPUT_TEXT, playerName->strValue));
			}
			else if (msg == "GoToMainMenu")
			{
				OpenMainMenu();
			}
			else if (msg == "ToggleMenu")
			{
				switch(mode)
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
				GraphicsMan.QueueMessage(new GMSetUIs("LevelKills", GMUI::TEXT, LevelKills()->ToString()));
				GraphicsMan.QueueMessage(new GMSetUIs("LevelScore", GMUI::TEXT, LevelScore()->ToString()));
				GraphicsMan.QueueMessage(new GMSetUIs("ScoreTotal", GMUI::TEXT, score->ToString()));
				UpdateUI();
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
				level.ships.Clear();
				MapMan.DeleteEntities(shipEntities);
				// Move the level-entity, the player will follow.
				PhysicsMan.QueueMessage(new PMSetEntity(levelEntity, PT_POSITION, Vector3f(level.goalPosition - 5.f, 10, 0)));
//				GraphicsMan.QueueMessage(new GMSetCamera(levelCamera, CT_POSITION, Vector3f(level.goalPosition - 5.f, 10, 0)));
			}
			else if (msg == "FinishStage")
			{
				ScriptMan.PlayScript("scripts/FinishStage.txt");
			}
			else if (msg.Contains("GoToLobby"))
			{
				mode = IN_LOBBY;
				UpdateUI();
			}
			else if (msg.StartsWith("ShowGearDesc:"))
			{
				String text = msg;
				text.Remove("ShowGearDesc:");
				GraphicsMan.QueueMessage(new GMSetUIs("GearInfo", GMUI::TEXT, text));
			}
			else if (msg.Contains("ExitToMainMenu"))
			{
				mode = MAIN_MENU;
				UpdateUI();
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


/// Creates default key-bindings for the state.
void SpaceShooter2D::CreateDefaultBindings()
{
	List<Binding*> & bindings = this->inputMapping.bindings;
#define BINDING(a,b) bindings.Add(new Binding(a,b));
	BINDING(Action::CreateStartStopAction("MoveShipUp"), KEY::W);
	BINDING(Action::CreateStartStopAction("MoveShipDown"), KEY::S);
	BINDING(Action::CreateStartStopAction("MoveShipLeft"), KEY::A);
	BINDING(Action::CreateStartStopAction("MoveShipRight"), KEY::D);
	BINDING(Action::FromString("ResetCamera"), KEY::HOME);
	BINDING(Action::FromString("NewGame"), List<int>(KEY::N, KEY::G));
	BINDING(Action::FromString("ClearLevel"), List<int>(KEY::C, KEY::L));
	BINDING(Action::FromString("ListEntitiesAndRegistrations"), List<int>(KEY::L, KEY::E));
	BINDING(Action::FromString("ToggleBlackness"), List<int>(KEY::T, KEY::B));
	BINDING(Action::FromString("NextLevel"), List<int>(KEY::N, KEY::L));
	BINDING(Action::FromString("PreviousLevel"), List<int>(KEY::P, KEY::L));
	BINDING(Action::FromString("ToggleMenu"), KEY::ESCAPE);
}

/// Called from the render-thread for every viewport/window, after the main rendering-pipeline has done its job.
void SpaceShooter2D::Render(GraphicsState * graphicsState)
{
	switch(mode)
	{
		case PLAYING_LEVEL:	
			if (!levelEntity)
				return;
			break;
		default:
			return;
	}

	// Load default shader?
	ShadeMan.SetActiveShader(NULL);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixd(graphicsState->projectionMatrixD.getPointer());
	glMatrixMode(GL_MODELVIEW);
	Matrix4d modelView = graphicsState->viewMatrixD * graphicsState->modelMatrixD;
	glLoadMatrixd(modelView.getPointer());
	// Enable blending
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	float z = -4;
	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 0);
	graphicsState->currentTexture = NULL;
	// Disable lighting
	glDisable(GL_LIGHTING);
	glDisable(GL_COLOR_MATERIAL);
	// Ignore previous stuff there.
	glDisable(GL_DEPTH_TEST);
	// Specifies how the red, green, blue and alpha source blending factors are computed
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	Vector2f minField = levelEntity->position - playingFieldHalfSize - Vector2f(1,1);
	Vector2f maxField = levelEntity->position + playingFieldHalfSize + Vector2f(1,1);

	/// o.o
	for (int i = 0; i < shipEntities.Size(); ++i)
	{	
		// Grab the position
		Entity * e = shipEntities[i];
		Vector2f pos = e->position;
		// Check if outside boundary.
		if (pos > minField && pos < maxField)
		{
			continue; // Skip already visible ships.
		}
		// Clamp the position.
		pos.Clamp(minField, maxField);

		// Check direction from this position to the entity's actual position.
		Vector3f to = (e->position - pos);
		float dist = to.Length();
		Vector3f dir = to.NormalizedCopy();
		Vector3f a,b,c;
		// Move the position a bit out...?
		Vector3f center = pos;
		center.z = 7.f;

		// Center.
		a = b = c = center;
		// Move A away from the dir.
		a += dir * 0.7f;
		// Get side-dirs.
		Vector3f side = dir.CrossProduct(Vector3f(0,0,1)).NormalizedCopy();
		side *= 0.5f;
		b += side;
		c -= side;

		// Set color based on distance.
		float alpha = (1.f / dist) + 0.5f;
		glColor4f(1,1,1,alpha);

		// Draw stuff
		glBegin(GL_TRIANGLES);
	#define DRAW(a) glVertex3f(a.x,a.y,a.z)
			DRAW(a);
			DRAW(b);
			DRAW(c);
		glEnd();
	}

	glEnable(GL_DEPTH_TEST);
	CheckGLError("SpaceShooter2D::Render");
}

/// Update UI
void SpaceShooter2D::UpdateUIPlayerHP()
{
	GraphicsMan.QueueMessage(new GMSetUIs("HP", GMUI::TEXT, String(playerShip.hitPoints)));	
}
void SpaceShooter2D::UpdateUIPlayerShield()
{
	GraphicsMan.QueueMessage(new GMSetUIs("Shield", GMUI::TEXT, String((int)playerShip.shieldValue)));
}
// Update ui
void SpaceShooter2D::OnScoreUpdated()
{

}

/// o.o
Entity * SpaceShooter2D::OnShipDestroyed(Ship * ship)
{
		// Explode
//	Entity * explosionEntity = spaceShooter->NewExplosion(owner->position, ship);
//	game->explosions.Add(explosionEntity);
	return NULL;
}

/// Level score. If -1, returns current.
GameVariable * SpaceShooter2D::LevelScore(int stage, int level)
{
	String name = "Level "+String(stage)+"-"+String(level)+" score";
	GameVar * gv = GameVars.Get(name);
	if (!gv)
		gv = GameVars.CreateInt(name, 0);
	return gv;
}

/// Level score. If -1, returns current.
GameVariable * SpaceShooter2D::LevelKills(int stage, int level)
{
	String name = "Level "+String(stage)+"-"+String(level)+" kills";
	GameVar * gv = GameVars.Get(name);
	if (!gv)
		gv = GameVars.CreateInt(name, 0);
	return gv;
}


/// Starts a new game. Calls LoadLevel
void SpaceShooter2D::NewGame()
{
	startDate = Time::Now();
	score->iValue = 0;
	for (int i = 0; i < 8; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			LevelKills(i+1,j+1)->iValue = 0;
			LevelScore(i+1,j+1)->iValue = 0;
		}
	}
	
	/// Fetch file which dictates where to load weapons and ships from.
	List<String> lines = File::GetLines("ToLoad.txt");
	enum {
		SHIPS, WEAPONS
	};
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		if (line.StartsWith("//"))
			continue;
		if (line.Contains("Ships:"))
			mode = SHIPS;
		else if (line.Contains("Weapons:"))
			mode = WEAPONS;
		else if (mode == SHIPS)
			Ship::LoadTypes(line);	
		else if (mode == WEAPONS)
			Weapon::LoadTypes(line);	
	}
	/*
	// Load weapons.
	Weapon::LoadTypes("Ship Data/Human/Weapons.csv");
	// Load ship-types.
	Ship::LoadTypes("Ship Data/Human/Ships.csv");
*/

	if (playerShip.name.Length() == 0)
	{
		playerShip = Ship::New("Default");
		playerShip.ai = false;
		playerShip.allied = true;
		if (playerShip.weapons.Size() == 0)
		{
			std::cout<<"\nAdding default weapon.";
			Weapon weapon;
			weapon.damage = 24;
			weapon.cooldownMs = 200;
			weapon.projectileSpeed = 24.f;
			playerShip.weapons.Add(weapon);
			playerShip.maxHitPoints = 500;
		}
	}
	currentStage->iValue = 1;
	currentLevel->iValue = 1;
	LoadLevel();
	// Play script for animation or whatever.
	ScriptMan.PlayScript("scripts/NewGame.txt");
	// Resume physics/graphics if paused.
	Resume();
}

void SpaceShooter2D::Pause()
{
	paused = true;
	OnPauseStateUpdated();
}
void SpaceShooter2D::Resume()
{
	paused = false;
	OnPauseStateUpdated();
}

void SpaceShooter2D::TogglePause()
{
	paused = !paused;
	OnPauseStateUpdated();
}

void SpaceShooter2D::OnPauseStateUpdated()
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


/// Loads target level. The source and separate .txt description have the same name, just different file-endings, e.g. "Level 1.png" and "Level 1.txt"
void SpaceShooter2D::LoadLevel(String fromSource)
{
	// Reset stats for this specific level.
	LevelKills()->iValue = 0;
	LevelScore()->iValue = 0;

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

	level.Load(fromSource);
	level.SetupCamera();
	level.AddPlayer(&playerShip);
	// Reset player stats.
	playerShip.hitPoints = playerShip.maxHitPoints;
	playerShip.shieldValue = playerShip.maxShieldValue;


	/// Add emitter for stars at player start.

	float emissionSpeed = level.starSpeed.Length();
	Vector3f starDir = level.starSpeed.NormalizedCopy();

	ParticleEmitter * startEmitter = new ParticleEmitter();
	startEmitter->newType = true;
	startEmitter->instantaneous = true;
	startEmitter->constantEmission = 1400;
	startEmitter->positionEmitter.type = EmitterType::PLANE_XY;
	startEmitter->positionEmitter.SetScale(200.f);
	startEmitter->velocityEmitter.type = EmitterType::VECTOR;
	startEmitter->velocityEmitter.vec = starDir;
	startEmitter->SetEmissionVelocity(emissionSpeed);
	startEmitter->SetParticleLifeTime(20.f);
	startEmitter->SetScale(0.3f);
	startEmitter->SetColor(level.starColor);
	Graphics.QueueMessage(new GMAttachParticleEmitter(startEmitter, stars));


	starEmitter->newType = true;
	starEmitter->direction = starDir;
	starEmitter->SetEmissionVelocity(emissionSpeed);
	starEmitter->SetParticlesPerSecond(10);
	starEmitter->positionEmitter.type = EmitterType::PLANE_XY;
	starEmitter->positionEmitter.SetScale(40.f);
	starEmitter->velocityEmitter.type = EmitterType::VECTOR;
	starEmitter->velocityEmitter.vec = starDir;
	starEmitter->SetEmissionVelocity(emissionSpeed);
	starEmitter->SetParticleLifeTime(60.f);
	starEmitter->SetColor(level.starColor);
	starEmitter->SetScale(0.3f);


	/// Add entity to track for both the camera, blackness and player playing field.
	Vector3f initialPosition = Vector3f(0,10,0);
	if (!levelEntity)
	{
		levelEntity = EntityMan.CreateEntity("LevelEntity", NULL, NULL);
		levelEntity->position = initialPosition;
		PhysicsProperty * pp = levelEntity->physics = new PhysicsProperty();
		pp->collissionsEnabled = false;
		pp->type = PhysicsType::KINEMATIC;
		/// Add blackness to track the level entity.
		for (int i = 0; i < 4; ++i)
		{
			Entity * blackness = EntityMan.CreateEntity("Blackness"+String(i), ModelMan.GetModel("sprite.obj"), TexMan.GetTexture("0x0A"));
			float scale = 50.f;
			float halfScale = scale * 0.5f;
			blackness->scale = scale * Vector3f(1,1,1);
			Vector3f position;
			position[2] = 5.f; // Between game plane and camera
			switch(i)
			{
				case 0: position[0] += playingFieldHalfSize[0] + halfScale + playingFieldPadding; break;
				case 1: position[0] -= playingFieldHalfSize[0] + halfScale + playingFieldPadding; break;
				case 2: position[1] += playingFieldHalfSize[1] + halfScale + playingFieldPadding; break;
				case 3: position[1] -= playingFieldHalfSize[1] + halfScale + playingFieldPadding; break;
			}
			blackness->position = position;
			levelEntity->AddChild(blackness);
			blacknessEntities.Add(blackness);
		}
		// Register blackness entities for rendering.
		GraphicsMan.QueueMessage(new GMRegisterEntities(blacknessEntities));
		PhysicsMan.QueueMessage(new PMRegisterEntities(levelEntity));
		// Set level camera to track the level entity.
		GraphicsMan.QueueMessage(new GMSetCamera(levelCamera, CT_ENTITY_TO_TRACK, levelEntity));
	}
	// Track ... level with effects.
	starEmitter->entityToTrack = playerShip.entity;
	starEmitter->positionOffset = Vector3f(70.f,0,0);
//	GraphicsMan.QueueMessage(new GMSetParticleEmitter(starEmitter, GT_EMITTER_ENTITY_TO_TRACK, playerShip.entity));
//	GraphicsMan.QueueMessage(new GMSetParticleEmitter(starEmitter, GT_EMITTER_POSITION_OFFSET, Vector3f(70.f, 0, 0)));
	// Reset position of level entity if already created.
	levelEntity->position = initialPosition;
	levelEntity->physics->velocity = level.BaseVelocity();
//	PhysicsMan.QueueMessage(new PMSetEntity(levelEntity, PT_POSITION, initialPosition));
	// Set velocity of the game.
//	PhysicsMan.QueueMessage(new PMSetEntity(levelEntity, PT_VELOCITY, level.BaseVelocity()));
	// Reset position of player!
	playerShip.entity->position = initialPosition;
//	PhysicsMan.QueueMessage(new PMSetEntity(playerShip.entity, PT_POSITION, initialPosition));

	GraphicsMan.ResumeRendering();
	PhysicsMan.Resume();
	mode = PLAYING_LEVEL;
	UpdateUI();
	// Play music?
	if (level.music.Length())
	{
		AudioMan.QueueMessage(new AMPlay(AudioType::BGM, level.music, 1.f));
	}
	// Run start script.
	ScriptMan.PlayScript("scripts/OnLevelStart.txt");

	// o.o
	this->Resume();
}

void SpaceShooter2D::GameOver()
{
	if (mode != GAME_OVER)
	{
		mode = GAME_OVER;
		// Play script for animation or whatever.
		ScriptMan.PlayScript("scripts/GameOver.txt");
		// End script by going back to menu or playing a new game.
	}
}

void SpaceShooter2D::LevelCleared()
{
	if (mode != LEVEL_CLEARED)
	{
		mode = LEVEL_CLEARED;
		ScriptMan.PlayScript("scripts/LevelComplete.txt");
	}
}

/// Opens main menu.
void SpaceShooter2D::OpenMainMenu()
{
	mode = MAIN_MENU;
	inGameMenuOpened = false;
	showLevelStats = false;
	UpdateUI();
}

/// Where the ship will be re-fitted and new gear bought.
void SpaceShooter2D::EnterShipWorkshop()
{
	mode = IN_WORKSHOP;
	UpdateUI();
}
	
List<SaveFileHeader> headers;

/// Returns a list of save-files.
void SpaceShooter2D::OpenLoadScreen()
{
	mode = LOAD_SAVES;
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
		label->sizeRatioY = 0.1f;
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
	// Done.
	UpdateUI();
}

// Bring up the in-game menu.
void SpaceShooter2D::OpenInGameMenu()
{
	inGameMenuOpened = true;
	UpdateUI();
}

/// Saves current progress.
bool SpaceShooter2D::SaveGame()
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
bool SpaceShooter2D::LoadGame(String saveName)
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

/// Updates ui depending on mode.
void SpaceShooter2D::UpdateUI()
{
	/// o.o
	List<String> toHide, uis;
	toHide.Add("MainMenu", "HUD", "Saves", "InGameMenu", "Workshop");
	toHide.Add("Options", "LevelStats", "Lobby");
			
	// Reveal specifics?
	switch(mode)
	{
		case MAIN_MENU:
			uis.Add("MainMenu");
			break;
		case LOAD_SAVES:
			uis.Add("Saves");
			break;
		case GAME_OVER:
		case PLAYING_LEVEL:
		case LEVEL_CLEARED:
			uis.Add("HUD");
			UpdateUIPlayerHP();
			UpdateUIPlayerShield();
			break;
		case IN_LOBBY:
			uis.Add("Lobby");
			break;
		default:
			assert(false);
			uis.Add("MainMenu");
			break;
	}
	// o.o
	if (inGameMenuOpened)
		uis.Add("InGameMenu");
	if (showLevelStats)
		uis.Add("LevelStats");
	
	/// Remove those specifically stated to reveal.
	toHide = toHide - uis;
	/// o.o
	for (int i = 0; i < toHide.Size(); ++i)
	{
		GraphicsMan.QueueMessage(new GMSetUIb(toHide[i], GMUI::VISIBILITY, false));
	}
	for (int i = 0; i < uis.Size(); ++i)
	{
		GraphicsMan.QueueMessage(new GMSetUIb(uis[i], GMUI::VISIBILITY, true));
	}
}

#include "Gear.h"

void SpaceShooter2D::UpdateGearList()
{
	Gear::Load("data/Shields.csv");

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
		list->highlightOnHover = true;
		list->textureSource = "0x3344";
		list->sizeRatioY = 0.2f;
		list->padding = 0.02f;
		list->hoverable = true;
		list->onHover = "ShowGearDesc:"+gear.description;
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
				label = new UILabel("Max: "+String(gear.maxShield));
				label->hoverable = false;
				label->sizeRatioX = 0.1f;
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


void SpaceShooter2D::UpdatePlayerVelocity()
{
	Vector3f totalVec;
	for (int i = 0; i < movementDirections.Size(); ++i)
	{
		Vector3f vec = Direction::GetVector(movementDirections[i]);
		totalVec += vec;
	}
	totalVec.Normalize();
	totalVec *= playerShip.speed;
	totalVec += level.BaseVelocity();

	// Set player speed.
	if (playerShip.entity)
	{
		PhysicsMan.QueueMessage(new PMSetEntity(playerShip.entity, PT_VELOCITY, totalVec));
	}
}

void SpaceShooter2D::ResetCamera()
{
	levelCamera->projectionType = Camera::ORTHOGONAL;
	levelCamera->zoom = 15.f;
}

void SpaceShooter2D::SetPlayingFieldSize(Vector2f newSize)
{
	playingFieldSize = newSize;
	playingFieldHalfSize = newSize * .5f;
}
