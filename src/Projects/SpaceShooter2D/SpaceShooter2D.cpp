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


/// Particle system for sparks/explosion-ish effects.
Sparks * sparks = NULL;
Stars * stars = NULL;

ParticleEmitter * starEmitter = NULL;

static bool paused = false;

List<Weapon> Weapon::types;
List<Ship> Ship::types;

void SetApplicationDefaults()
{
	Application::name = "SpaceShooter2D";
	TextFont::defaultFontSource = "img/fonts/font3.png";
}


SpaceShooter2D * spaceShooter = NULL;

void RegisterStates()
{
	spaceShooter = new SpaceShooter2D();
	StateMan.RegisterState(spaceShooter);
	StateMan.QueueState(spaceShooter);
}



SpaceShooter2D::SpaceShooter2D()
{
	levelCamera = NULL;
}
SpaceShooter2D::~SpaceShooter2D()
{
}
/// Function when entering this state, providing a pointer to the previous StateMan.
void SpaceShooter2D::OnEnter(AppState * previousState)
{
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
	starEmitter->newType = true;
	starEmitter->positionEmitter.type = EmitterType::PLANE_XY;
	starEmitter->positionEmitter.Scale(40.f);
	starEmitter->velocityEmitter.type = EmitterType::CIRCLE_XY;
	starEmitter->SetEmissionVelocity(0.0001f);
	starEmitter->SetParticlesPerSecond(20);
	starEmitter->SetParticleLifeTime(50.f);
	starEmitter->SetScale(0.3f);
	Graphics.QueueMessage(new GMAttachParticleEmitter(starEmitter, stars));


	// Remove overlay.
	// Set up ui.
	if (!ui)
		CreateUserInterface();
	GraphicsMan.QueueMessage(new GMSetUI(ui));
	GraphicsMan.QueueMessage(new GMSetOverlay(NULL));

	mode = IN_MENU;

	// Load Space Race integrator
	integrator = new SSIntegrator(0.f);
	PhysicsMan.QueueMessage(new PMSet(integrator));
	cd = new SpaceShooterCD();
	PhysicsMan.QueueMessage(new PMSet(cd));
	cr = new SpaceShooterCR();
	PhysicsMan.QueueMessage(new PMSet(cr));

	// Run OnEnter.ini if such a file exists.
	Script * script = new Script();
	script->Load("OnEnter.ini");
	ScriptMan.PlayScript(script);

	PhysicsMan.checkType = AABB_SWEEP;
}


Time now;
int64 nowMs;
int timeElapsedMs;
Vector3f frustumMin, frustumMax;

/// Main processing function, using provided time since last frame.
void SpaceShooter2D::Process(int timeInMs)
{
	Sleep(10);
//	std::cout<<"\nSS2D entities: "<<shipEntities.Size() + projectileEntities.Size() + 1;
//	if (playerShip) std::cout<<"\nPlayer position: "<<playerShip.position;

	now = Time::Now();
	nowMs = (int) now.Milliseconds();
	timeElapsedMs = timeInMs;
	
	// Check camera bounding box.
	if (levelCamera)
	{
		Frustum frustum = levelCamera->GetFrustum();
		/// These will hopefully always be in AABB axes.
		frustumMin = frustum.hitherBottomLeft;
		frustumMax = frustum.fartherTopRight;
	}
	switch(mode)
	{
		case PLAYING_LEVEL:
		{
			// Check for game over.
			if (playerShip.hitPoints <= 0)
			{
				// Game OVER!
				GameOver();
				break;
			}

			float spawnPositionRight = levelCamera->position.x + 16.f;
			float despawnPositionLeft = levelCamera->position.x - 16.f;

			/// Clearing the level
			if (playerShip.entity->position.x > level.goalPosition)
			{
				LevelCleared();
			}
			else 
			{
				/// PRocess the player ship.
				Process(playerShip);
			}
			/// Remove projectiles which have been passed by.
			for (int i = 0; i < projectileEntities.Size(); ++i)
			{
				Entity * proj = projectileEntities[i];
				if ((proj->position - levelCamera->position).LengthSquared() > 2000)
				{
					MapMan.DeleteEntity(proj);
					projectileEntities.Remove(proj);
					--i;
				}
			}
			// Check if we want to spawn any enemy ships.
			// Add all enemy ships too?
			for (int i = 0; i < level.ships.Size(); ++i)
			{
				Ship & ship = level.ships[i];
				if (ship.spawned)
				{
					if (ship.entity == NULL)
						continue;
					if (ship.spawnInvulnerability)
					{
						if (ship.entity->position.x < spawnPositionRight)
						{
							// Change color.
							GraphicsMan.QueueMessage(new GMSetEntityTexture(ship.entity, DIFFUSE_MAP | SPECULAR_MAP, TexMan.GetTextureByColor(Color(255,255,255,255))));
							ship.spawnInvulnerability = false;
							continue;				
						}
					}
					// Check if it should de-spawn.
					if (ship.entity->position.x < despawnPositionLeft)
					{
						MapMan.DeleteEntity(ship.entity);
						shipEntities.Remove(ship.entity);
						ship.entity = NULL;
					}
					// If not, process it?
					else {
						Process(ship);
					}
					continue;
				}
				if (ship.position.x > spawnPositionRight + 5.f)
					continue;
				Entity * entity = EntityMan.CreateEntity(ship.type, ModelMan.GetModel("sphere.obj"), TexMan.GetTextureByColor(Color(0,255,0,255)));
				entity->position = ship.position;
				PhysicsProperty * pp = new PhysicsProperty();
				entity->physics = pp;
				// Setup physics.
				pp->type = PhysicsType::DYNAMIC;
				pp->collisionCategory = CC_ENEMY;
				pp->collisionFilter = CC_PLAYER | CC_PLAYER_PROJ;
				pp->collissionCallback = true;
				// By default, set invulerability on spawn.
				ship.spawnInvulnerability = true;
				entity->RecalculateMatrix();
				ShipProperty * sp = new ShipProperty(&ship, entity);
				entity->properties.Add(sp);
				ship.entity = entity;
				ship.spawned = true;
				shipEntities.Add(entity);
				MapMan.AddEntity(entity);
			}
		}
	}
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void SpaceShooter2D::OnExit(AppState * nextState)
{
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
		case MessageType::COLLISSION_CALLBACK:
		{

			CollisionCallback * cc = (CollisionCallback*) message;
			Entity * one = cc->one;
			Entity * two = cc->two;
#define SHIP 0
#define PROJ 1
//			std::cout<<"\nColCal: "<<cc->one->name<<" & "<<cc->two->name;

			Entity * shipEntity = NULL;
			Entity * other = NULL;
			int oneType = (one == playerShip.entity || shipEntities.Exists(one)) ? SHIP : PROJ;
			int twoType = (two == playerShip.entity || shipEntities.Exists(two)) ? SHIP : PROJ;
			int types[5] = {0,0,0,0,0};
			++types[oneType];
			++types[twoType];
			if (oneType == SHIP)
			{
				shipEntity = one;
				other = two;
			}
			else if (twoType == SHIP)
			{
				shipEntity = two;
				other = one;
			}
			if (shipEntity)
			{
				ShipProperty * shipProp = (ShipProperty*)shipEntity->GetProperty(ShipProperty::ID());
				shipProp->OnCollision(other);
			}
			break;
		}
		case MessageType::STRING:
		{
			if (msg == "NewGame")
				NewGame();
			else if (msg == "Pause/Break")
			{
				TogglePause();
			}
			else if (msg == "ListEntitiesAndRegistrations")
			{
				std::cout<<"\nGraphics entities "<<GraphicsMan.RegisteredEntities()<<" physics "<<PhysicsMan.RegisteredEntities()
					<<" projectiles "<<projectileEntities.Size()<<" ships "<<shipEntities.Size();
			}
			else if (msg == "NextLevel")
			{
				// Next level? Restart?
				LoadLevel(levelSource);
			}
			else if (msg == "ClearLevel")
			{
				level.ships.Clear();
				MapMan.DeleteEntities(shipEntities);
				PhysicsMan.QueueMessage(new PMSetEntity(playerShip.entity, PT_POSITION, Vector3f(level.goalPosition - 5.f, 10, 0)));
				GraphicsMan.QueueMessage(new GMSetCamera(levelCamera, CT_POSITION, Vector3f(level.goalPosition - 5.f, 10, 0)));
			}
			if (msg.StartsWith("DisplayCenterText"))
			{
				String text = msg;
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
	BINDING(Action::FromString("NewGame"), List<int>(2, KEY::N, KEY::G));
	BINDING(Action::FromString("ClearLevel"), List<int>(2, KEY::C, KEY::L));
	BINDING(Action::FromString("ListEntitiesAndRegistrations"), List<int>(2, KEY::L, KEY::E));
}

/// Update UI
void SpaceShooter2D::UpdatePlayerHP()
{
	GraphicsMan.QueueMessage(new GMSetUIs("HP", GMUI::TEXT, String(playerShip.hitPoints)));
}
void SpaceShooter2D::UpdatePlayerShield()
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




/// Starts a new game. Calls LoadLevel
void SpaceShooter2D::NewGame()
{
	// Load weapons.
	Weapon::LoadTypes("Ship Data/Alien/Weapons.csv");
	Weapon::LoadTypes("Ship Data/Human/Weapons.csv");
	// Load ship-types.
	Ship::LoadTypes("Ship Data/Alien/Ships.csv");
	Ship::LoadTypes("Ship Data/Human/Ships.csv");


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
	LoadLevel("Levels/Stage 1/Level 1-1");
	// Play script for animation or whatever.
	ScriptMan.PlayScript("scripts/NewGame.txt");

}

void SpaceShooter2D::TogglePause()
{
	paused = !paused;
	if (paused)
	{
		GraphicsMan.QueueMessage(new GraphicsMessage(GM_PAUSE_RENDERING));
		PhysicsMan.Pause();
	}
	else {
		GraphicsMan.QueueMessage(new GraphicsMessage(GM_RESUME_RENDERING));
		PhysicsMan.Resume();
	}
}

/// Loads target level. The source and separate .txt description have the same name, just different file-endings, e.g. "Level 1.png" and "Level 1.txt"
void SpaceShooter2D::LoadLevel(String fromSource)
{
	GraphicsMan.QueueMessage(new GMSetParticleEmitter(starEmitter, GT_EMITTER_ENTITY_TO_TRACK, NULL));
	this->levelSource = fromSource;
	// Delete all entities.
	MapMan.DeleteAllEntities();
	shipEntities.Clear();
	projectileEntities.Clear();
	GraphicsMan.PauseRendering();
	level.Load(fromSource);
	level.SetupCamera();
	level.AddPlayer(&playerShip);
	// Track player with effects.
	GraphicsMan.QueueMessage(new GMSetParticleEmitter(starEmitter, GT_EMITTER_ENTITY_TO_TRACK, playerShip.entity));
	GraphicsMan.QueueMessage(new GMSetParticleEmitter(starEmitter, GT_EMITTER_POSITION_OFFSET, Vector3f(50.f, 0, 0)));
	// Reset player stats.
	playerShip.hitPoints = playerShip.maxHitPoints;
	playerShip.shieldValue = playerShip.maxShieldValue;
	GraphicsMan.ResumeRendering();
	mode = PLAYING_LEVEL;
	UpdateUI();
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

/// Updates ui depending on mode.
void SpaceShooter2D::UpdateUI()
{
	switch(mode)
	{
		case GAME_OVER:
		case PLAYING_LEVEL:
			// Hide main menu
			GraphicsMan.QueueMessage(new GMSetUIb("MainMenu", GMUI::VISIBILITY, false, ui));
			GraphicsMan.QueueMessage(new GMSetUIb("HUD", GMUI::VISIBILITY, true, ui));
			UpdatePlayerHP();
			UpdatePlayerShield();
			break;
		default:
			GraphicsMan.QueueMessage(new GMSetUIb("MainMenu", GMUI::VISIBILITY, true, ui));
			GraphicsMan.QueueMessage(new GMSetUIb("HUD", GMUI::VISIBILITY, false, ui));
			break;
	}
}




void SpaceShooter2D::UpdatePlayerVelocity()
{
	Vector3f totalVec;
	for (int i = 0; i < movementDirections.Size(); ++i)
	{
		Vector3f vec = Direction::GetVector(movementDirections[i]);
		totalVec += vec;
	}
	totalVec *= 8.f;
	// Set player speed.
	if (playerShip.entity)
	{
		PhysicsMan.QueueMessage(new PMSetEntity(playerShip.entity, PT_VELOCITY, totalVec + level.BaseVelocity()));
	}
}

void SpaceShooter2D::ResetCamera()
{
	levelCamera->projectionType = Camera::ORTHOGONAL;
	levelCamera->zoom = 15.f;
}

/// Process target ship.
void SpaceShooter2D::Process(Ship & ship)
{
	if (ship.hasShield)
	{
		// Repair shield
		ship.shieldValue += timeElapsedMs * ship.shieldRegenRate;
		if (ship.shieldValue > ship.maxShieldValue)
			ship.shieldValue = ship.maxShieldValue;
		if (ship.allied)
			UpdatePlayerShield();
	}
}
