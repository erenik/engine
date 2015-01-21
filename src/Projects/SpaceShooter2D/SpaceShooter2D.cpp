/// Emil Hedemalm
/// 2015-01-20
/// Space shooter.
/// For the Karl-Emil SpaceShooter project, mainly 2014-2015/

#include "SpaceShooter2D.h"

#include "Application/Application.h"
#include "StateManager.h"

#include "Input/InputManager.h"
#include "Input/Action.h"


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
	playerShip = NULL;
	levelCamera = NULL;
}
SpaceShooter2D::~SpaceShooter2D()
{
	SAFE_DELETE(playerShip);
}
/// Function when entering this state, providing a pointer to the previous StateMan.
void SpaceShooter2D::OnEnter(AppState * previousState)
{
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

/// Main processing function, using provided time since last frame.
void SpaceShooter2D::Process(int timeInMs)
{
	Sleep(10);
//	std::cout<<"\nSS2D entities: "<<shipEntities.Size() + projectileEntities.Size() + 1;
//	if (playerShip) std::cout<<"\nPlayer position: "<<playerShip->position;

	now = Time::Now();
	nowMs = (int) now.Milliseconds();
	timeElapsedMs = timeInMs;

	static int cpBarn = 0;
	cpBarn += timeElapsedMs;
	if (cpBarn > 1000)
	{
		std::cout<<"\nGraphics entities "<<GraphicsMan.RegisteredEntities()<<" physics "<<PhysicsMan.RegisteredEntities()
			<<" projectiles "<<projectileEntities.Size()<<" ships "<<shipEntities.Size();
		cpBarn = 0;
		int duplicates = shipEntities.Duplicates();
//		std::cout<<"\nDuplicates: "<<duplicates;
	}
	switch(mode)
	{
		case PLAYING_LEVEL:
		{
			// Check for game over.
			if (playerShip->hitPoints <= 0)
			{
				// Game OVER!
				GameOver();
				break;
			}

			/// Clearing the level
			if (playerShip->entity->position.x > level.goalPosition)
			{
				// Next level? Restart?
				LoadLevel(levelSource);
			}
			/// Remove projectiles which have been passed by.
			for (int i = 0; i < projectileEntities.Size(); ++i)
			{
				Entity * proj = projectileEntities[i];
				if (proj->position.x < levelCamera->position.x - 20.f ||
					proj->position.x > levelCamera->position.x + 40.f)
				{
					MapMan.DeleteEntity(proj);
					int occurances = 0;
					while(projectileEntities.Remove(proj))
						++occurances;
					if (occurances > 1)
					{
						std::cout<<"BAANANRANER";
					}
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
					// Check if it should de-spawn.
					if (ship.entity->position.x < levelCamera->position.x - 25.f)
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
				if (AbsoluteValue(levelCamera->position.x - ship.position.x) > 50.f)
					continue;
				Entity * entity = EntityMan.CreateEntity(ship.type, ModelMan.GetModel("sphere.obj"), TexMan.GetTextureByColor(Color(0,255,0,255)));
				entity->position = ship.position;
				entity->RecalculateMatrix();
				ShipProperty * sp = new ShipProperty(&ship, entity);
				entity->properties.Add(sp);
				ship.entity = entity;
				ship.spawned = true;
				shipEntities.Add(entity);
				MapMan.AddEntity(entity);
				// Only collide with the player.
				PhysicsMan.QueueMessage(new PMSetEntity(entity, PT_COLLISION_FILTER, CC_PLAYER));
				PhysicsMan.QueueMessage(new PMSetEntity(entity, PT_COLLISION_CATEGORY, CC_ENEMY));
			}
		}
	}
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void SpaceShooter2D::OnExit(AppState * nextState)
{
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
		case MessageType::STRING:
		{
			if (msg == "NewGame")
				NewGame();
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
}

/// Update UI
void SpaceShooter2D::UpdatePlayerHP()
{
	GraphicsMan.QueueMessage(new GMSetUIs("HP", GMUI::TEXT, String(playerShip->hitPoints)));
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
	if (!playerShip)
	{
		playerShip = new Ship();
		playerShip->ai = false;
		playerShip->allied = true;
		Weapon weapon;
		weapon.damage = 24;
		weapon.cooldownMs = 200;
		weapon.projectileSpeed = 24.f;
		playerShip->weapons.Add(weapon);
	}
	// Reset player stats.
	playerShip->hitPoints = playerShip->maxHitPoints = 500;

	LoadLevel("Levels/Stage 1/Level 1-1");
	mode = PLAYING_LEVEL;
	UpdateUI();
	// Play script for animation or whatever.
	ScriptMan.PlayScript("scripts/NewGame.txt");

}

/// Loads target level. The source and separate .txt description have the same name, just different file-endings, e.g. "Level 1.png" and "Level 1.txt"
void SpaceShooter2D::LoadLevel(String fromSource)
{
	this->levelSource = fromSource;
	// Delete all entities.
	MapMan.DeleteAllEntities();
	level.Load(fromSource);
	level.SetupCamera();
	Sleep(50);
	level.AddPlayer(playerShip);
}

void SpaceShooter2D::GameOver()
{
	mode = GAME_OVER;
	// Play script for animation or whatever.
	ScriptMan.PlayScript("scripts/GameOver.txt");
	// End script by going back to menu or playing a new game.
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
	if (playerShip && playerShip->entity)
	{
		PhysicsMan.QueueMessage(new PMSetEntity(playerShip->entity, PT_VELOCITY, totalVec + level.BaseVelocity()));
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
}
