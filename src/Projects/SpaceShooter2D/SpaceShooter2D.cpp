/// Emil Hedemalm
/// 2015-01-20
/// Space shooter.
/// For the Karl-Emil SpaceShooter project, mainly 2014-2015/

#include "Message/FileEvent.h"
#include "MovementPattern.h"

#include "SpaceShooter2D/SpaceShooter2D.h"
#include "Base/WeaponScript.h"

#include "Application/Application.h"
#include "StateManager.h"

#include "Physics/Messages/CollisionCallback.h"
#include "Window/AppWindow.h"
#include "Viewport.h"

#include "OS/OSUtil.h"
#include "OS/Sleep.h"
#include "File/SaveFile.h"
#include "UI/UIUtil.h"

#include "Text/TextManager.h"
#include "Graphics/Messages/GMRenderPass.h"
#include "Render/RenderPass.h"

#include "Message/MathMessage.h"
#include "SpaceShooter2D/Base/Gear.h"

#include "Input/InputManager.h"

/// Particle system for sparks/explosion-ish effects.
Sparks * sparks = NULL;
Stars * stars = NULL;

ParticleEmitter * starEmitter = NULL;

/// Global pause, used for pause/Break, etc. Most calculations should stop/halt while paused.
bool paused = false;

List<Weapon> Weapon::types;
List<Ship*> Ship::types;

/// 4 entities constitude the blackness.
List<Entity*> blacknessEntities;

bool shipDataLoaded = false;

/// If true, queues up messages so the player automatically starts a new game with the default name and difficulty.
bool introTest = false;

void SetApplicationDefaults()
{
	Application::name = "SpaceShooter2D";
	TextFont::defaultFontSource = "img/fonts/font3.png";
	PhysicsProperty::defaultUseQuaternions = false;
	UIElement::defaultTextureSource = "0x22AA";
}

// Global variables.
SpaceShooter2D * spaceShooter = 0;
Ship * playerShip = 0;
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
String onDeath; // What happens when the player dies?
String levelToLoad;
String previousActiveUpgrade;

void RegisterStates()
{
	spaceShooter = new SpaceShooter2D();
	StateMan.RegisterState(spaceShooter);
	StateMan.QueueState(spaceShooter);
}

SpaceShooter2D::SpaceShooter2D()
{
//	playerShip = new Ship();
	levelCamera = NULL;
	SetPlayingFieldSize(Vector2f(30,20));
	levelEntity = NULL;
	playingFieldPadding = 1.f;
	gearCategory = 0;
	previousMode = mode = 0;
	// Default vel smoothing.
	PhysicsProperty::defaultVelocitySmoothing = 0.02f;
	PhysicsProperty::defaultLinearDamping = 1.f;
}

SpaceShooter2D::~SpaceShooter2D()
{
	Ship::types.ClearAndDelete();
	delete playerShip;
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void SpaceShooter2D::OnEnter(AppState * previousState)
{

	MovementPattern::Load();
	QueuePhysics(new PMSeti(PT_AABB_SWEEPER_DIVISIONS, 1));// subdivisionsZ

	WeaponScript::CreateDefault();
	/// Enable Input-UI navigation via arrow-keys and Enter/Esc.
	InputMan.ForceNavigateUI(true);

	/// Create game variables.
	currentLevel = GameVars.CreateInt("currentLevel", 1);
	currentStage = GameVars.CreateInt("currentStage", 1);
	playerName = GameVars.CreateString("playerName", "Cytine");
	score = GameVars.CreateInt("score", 0);
	money = GameVars.CreateInt("money", 200);
	playTime = GameVars.CreateInt("playTime", 0);
	gameStartDate = GameVars.CreateTime("gameStartDate");
	difficulty = GameVars.CreateInt("difficulty", 1);

	AppWindow * w = MainWindow();
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
	starEmitter = new StarEmitter(Vector3f());
	Graphics.QueueMessage(new GMAttachParticleEmitter(starEmitter, stars));


	// Remove overlay.
	// Set up ui.
//	if (!ui)
//		CreateUserInterface();
	// Set UI without delay.
//	GraphicsMan.ProcessMessage(new GMSetUI(ui));
	GraphicsMan.ProcessMessage(new GMSetOverlay(NULL));

	// Load Space Race integrator
	integrator = new SSIntegrator(0.f);
	PhysicsMan.QueueMessage(new PMSet(integrator));
	cd = new SpaceShooterCD();
	PhysicsMan.QueueMessage(new PMSet(cd));
	cr = new SpaceShooterCR();
	PhysicsMan.QueueMessage(new PMSet(cr));

	PhysicsMan.checkType = AABB_SWEEP;

	// Run OnStartApp script.
	ScriptMan.PlayScript("scripts/OnStartApp.txt");

	/// Enter main menu
//	OpenMainMenu();

	TextMan.LoadFromDir();
	TextMan.SetLanguage("English");

	NewPlayer();

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
void SpaceShooter2D::Process(int timeInMs)
{
	SleepThread(10);
//	std::cout<<"\nSS2D entities: "<<shipEntities.Size() + projectileEntities.Size() + 1;
//	if (playerShip) std::cout<<"\nPlayer position: "<<playerShip->position;

	now = Time::Now();
	timeElapsedMs = timeInMs;
	
	level.Cleanup();

	switch(mode)
	{
		case PLAYING_LEVEL:
		{
			if (paused)
				return;
			level.Process(timeInMs);
			UpdateCooldowns();
			UpdateRenderArrows();
		}
	}
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void SpaceShooter2D::OnExit(AppState * nextState)
{
	levelEntity = NULL;
	SleepThread(50);
	// Register it for rendering.
	Graphics.QueueMessage(new GMUnregisterParticleSystem(sparks, true));
	Graphics.QueueMessage(new GMUnregisterParticleSystem(stars, true));
	MapMan.DeleteAllEntities();
	SleepThread(100);
}

/// Searches among actively spawned ships.
Ship * SpaceShooter2D::GetShip(Entity * forEntity)
{
	for (int i = 0; i < level.ships.Size(); ++i)
	{
		Ship * ship = level.ships[i];
		if (ship->entity == forEntity)
			return ship;
	}
	return 0;
}

Ship * SpaceShooter2D::GetShipByID(int id)
{
	for (int i = 0; i < level.ships.Size(); ++i)
	{
		Ship * ship = level.ships[i];
		if (ship->ID() == id)
			return ship;
	}
	if (id == playerShip->ID())
		return playerShip;
	return 0;
}

/// Creates the user interface for this state
void SpaceShooter2D::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->CreateRoot();
//	ui->Load("gui/MainMenu.gui");
}

bool playerInvulnerability = false;
void OnPlayerInvulnerabilityUpdated()
{
	
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void SpaceShooter2D::ProcessMessage(Message * message)
{
	String msg = message->msg;
	if (mode == SpaceShooter2D::EDIT_WEAPON_SWITCH_SCRIPTS)
		ProcessMessageWSS(message);
	level.ProcessMessage(message);
	switch(message->type)
	{
		case MessageType::FILE_EVENT:
		{
			FileEvent * fv = (FileEvent *) message;
			for (int i = 0; i < fv->files.Size(); ++i)
			{
				String file = fv->files[i];
				if (file.Contains(".srl"))
				{
					level.Load(file);
					level.OnEnter();
				}
				if (file.Contains(".csv"))
				{
					// Load stuff?
				}
			}
			break;
		}
		case MessageType::SET_STRING:
		{
			SetStringMessage * strMes = (SetStringMessage *) message;
			if (msg == "lall")
			{
				playerName->strValue = strMes->value;
			}
			if (msg == "JumpToTime")
			{
				level.JumpToTime(strMes->value);
			}

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
			else if (msg == "SetActiveWeapon")
			{
				playerShip->SwitchToWeapon(im->value);
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
			int oneType = (one == playerShip->entity || shipEntities.Exists(one)) ? SHIP : PROJ;
			int twoType = (two == playerShip->entity || shipEntities.Exists(two)) ? SHIP : PROJ;
			int types[5] = {0,0,0,0,0};
			++types[oneType];
			++types[twoType];
		//	std::cout<<"\nCollision between "<<one->name<<" and "<<two->name;
			if (oneType == SHIP)
			{
				ShipProperty * shipProp = (ShipProperty*)one->GetProperty(ShipProperty::ID());
				if (shipProp)
					shipProp->OnCollision(two);
			}
			else if (twoType == SHIP)
			{
				ShipProperty * shipProp = (ShipProperty*)two->GetProperty(ShipProperty::ID());
				if (shipProp)
					shipProp->OnCollision(one);
			}
			break;
		}
		case MessageType::ON_UI_ELEMENT_HOVER:
		{
			if (msg.StartsWith("SetHoverUpgrade:"))
			{
				String upgrade = msg.Tokenize(":")[1];
				UpdateHoverUpgrade(upgrade);
			}
			break;
		}
		case MessageType::STRING:
		{
			msg.RemoveSurroundingWhitespaces();
			int found = msg.Find("//");
			if (found > 0)
				msg = msg.Part(0,found);
			if (msg == "ProceedMessage")
			{
				level.ProceedMessage();
			}
			if (msg == "NewGame")
				NewGame();
			if (msg == "TogglePlayerInvulnerability")
			{
				playerInvulnerability = !playerInvulnerability;
				OnPlayerInvulnerabilityUpdated();
			}
			if (msg == "Continue")
			{
				if (levelToLoad.Length())
				{
					LoadLevel(levelToLoad);
					return;
				}
				// Set stage n level
				if (currentStage->iValue == 0)
				{
					currentStage->iValue = 1; 
					currentLevel->iValue = 1;
				}
				else {
					currentLevel->iValue += 1;
					if (currentLevel->iValue == 4)
					{
						currentStage->iValue += 1;
						currentLevel->iValue = 1;
					}
				}
				// Load weapons?
				
				// And load it.
				LoadLevel();
				// Play script for animation or whatever.
				ScriptMan.PlayScript("scripts/NewGame.txt");
				// Resume physics/graphics if paused.
				Resume();
			}
			if (msg == "GoToHangar")
			{
				SetMode(IN_HANGAR);
			}
			if (msg == "GoToEditWeaponSwitchScripts")
			{
				SetMode(EDIT_WEAPON_SWITCH_SCRIPTS);
			}
			if (msg == "GoToWorkshop")
			{
				SetMode(IN_WORKSHOP);
			}
			if (msg == "ActivateSkill")
			{
				playerShip->ActivateSkill();
			}
//			std::cout<<"\n"<<msg;
			if (msg == "TutorialBaseGun")
			{
				playerShip->weapons.Clear(); // Clear old wepaons.
				playerShip->SetWeaponLevel(WeaponType::TYPE_0, 1);
				playerShip->activeWeapon = playerShip->weapons[0];
				UpdateHUDGearedWeapons();
			}
			if (msg == "TutorialLevel1Weapons")
			{
				playerShip->SetWeaponLevel(WeaponType::TYPE_0, 1);
				playerShip->SetWeaponLevel(WeaponType::TYPE_1, 1);
				playerShip->SetWeaponLevel(WeaponType::TYPE_2, 1);
				UpdateHUDGearedWeapons();
			}
			if (msg == "TutorialLevel3Weapons")
			{
				playerShip->SetWeaponLevel(WeaponType::TYPE_0, 3);
				playerShip->SetWeaponLevel(WeaponType::TYPE_1, 3);
				playerShip->SetWeaponLevel(WeaponType::TYPE_2, 3);			
				UpdateHUDGearedWeapons();
			}
			if (msg.StartsWith("DecreaseWeaponLevel:"))
			{
				List<String> parts = msg.Tokenize(":");
				int weaponIndex = parts[1].ParseInt();
				Weapon * weap = playerShip->GetWeapon(WeaponType::TYPE_0 + weaponIndex);
				int currLevel = weap->level;
				playerShip->SetWeaponLevel(WeaponType::TYPE_0 + weaponIndex, currLevel-1);				
				std::cout<<"\nWeapon "<<weap->type<<" set to level "<<weap->level<<": "<<weap->name;
			}
			if (msg.StartsWith("IncreaseWeaponLevel:"))
			{
				List<String> parts = msg.Tokenize(":");
				int weaponIndex = parts[1].ParseInt();
				Weapon * weap = playerShip->GetWeapon(WeaponType::TYPE_0 + weaponIndex);
				int currLevel = weap->level;
				playerShip->SetWeaponLevel(WeaponType::TYPE_0 + weaponIndex, currLevel+1);
				std::cout<<"\nWeapon "<<weap->type<<" set to level "<<weap->level<<": "<<weap->name;
			}
			else if (msg == "AllTheWeapons")
			{
				for (int i = 0; i < WeaponType::MAX_TYPES; ++i)
				{
					if (playerShip->weapons[i]->level <= 0)
						playerShip->SetWeaponLevel(i, 1);
					UpdateHUDGearedWeapons();
				}
			}
			if (msg == "ToggleWeaponScript")
			{
				if (playerShip->weaponScript == 0)
					playerShip->weaponScript = WeaponScript::LastEdited();
				playerShip->weaponScriptActive = !playerShip->weaponScriptActive;
			}
			if (msg == "ActivateWeaponScript")
			{
				playerShip->weaponScriptActive = true;
				if (playerShip->weaponScript == 0)
					playerShip->weaponScript = WeaponScript::LastEdited();
			}
			if (msg == "DeactivateWeaponScript")
			{
				playerShip->weaponScriptActive = false;
				if (!InputMan.KeyPressed(KEY::SPACE))
					playerShip->shoot = false;
			}
			if (msg.StartsWith("SetSkill"))
			{
				String skill = msg.Tokenize(":")[1];
				if (skill == "AttackFrenzy")
					playerShip->skill = ATTACK_FRENZY;
				if (skill == "SpeedBoost")
					playerShip->skill = SPEED_BOOST;
				if (skill == "PowerShield")
					playerShip->skill = POWER_SHIELD;
				playerShip->skillName = skill;
				UpdateHUDSkill();
			}
			if (msg == "TutorialSkillCooldowns")
			{
				playerShip->skillCooldownMultiplier = 0.1f;
			}
			if (msg == "DisablePlayerMovement")
			{
				playerShip->movementDisabled = true;
				UpdatePlayerVelocity();
			}
			if (msg == "EnablePlayerMovement")
			{
				playerShip->movementDisabled = false;
				UpdatePlayerVelocity();
			}
			if (msg.StartsWith("SetOnDeath:"))
			{
				onDeath = msg - "SetOnDeath:";
			}
			if (msg.StartsWith("ActiveUpgrade:"))
			{
				String upgrade = msg.Tokenize(":")[1];
			//	if (previousActiveUpgrade == upgrade)
				BuySellToUpgrade(upgrade);
				UpdateHoverUpgrade(upgrade, true);
			//	UpdateActiveUpgrade(upgrade);
				previousActiveUpgrade = upgrade;
			}
			if (msg == "OpenJumpDialog")
			{
				Pause();
				OpenJumpDialog();
			}
			if (msg == "SpawnTutorialBomb")
			{
				// Shoot.
				Color color = Vector4f(0.8f,0.7f,0.1f,1.f);
				Texture * tex = TexMan.GetTextureByColor(color);
				Entity * projectileEntity = EntityMan.CreateEntity(name + " Projectile", ModelMan.GetModel("sphere.obj"), tex);
				Weapon weapon;
				weapon.damage = 750;
				ProjectileProperty * projProp = new ProjectileProperty(weapon, projectileEntity, true);
				projectileEntity->properties.Add(projProp);
				// Set scale and position.
				projectileEntity->localPosition = playerShip->entity->worldPosition + Vector3f(30,0,0);
				projectileEntity->SetScale(Vector3f(1,1,1) * 0.5f);
				projProp->color = color;
				projectileEntity->RecalculateMatrix();
				projProp->onCollisionMessage = "ResumeGameTime";
				// pew
				Vector3f dir(-1.f,0,0);
				Vector3f vel = dir * 5.f;
				PhysicsProperty * pp = projectileEntity->physics = new PhysicsProperty();
				pp->type = PhysicsType::DYNAMIC;
				pp->velocity = vel;
				pp->collisionCallback = true;	
				pp->maxCallbacks = 1;
				// Set collision category and filter.
				pp->collisionCategory = CC_ENEMY_PROJ;
				pp->collisionFilter = CC_PLAYER;
				// Add to map.
				MapMan.AddEntity(projectileEntity);
				projectileEntities.Add(projectileEntity);
			}
			if (msg == "UpdateHUDGearedWeapons")
				UpdateHUDGearedWeapons();
			else if (msg.StartsWith("Weapon:"))
			{
				int weaponIndex = msg.Tokenize(":")[1].ParseInt();
				weaponIndex -= 1;
				if (weaponIndex < 0)
					weaponIndex = 9;
				playerShip->SwitchToWeapon(weaponIndex);
			}
			else if (msg == "StartShooting")
			{
				playerShip->shoot = true;
			}
			else if (msg == "Reload OnEnter")
			{
				// Run OnEnter.ini start script if such a file exists.
				Script * script = new Script();
				script->Load("OnEnter.ini");
				ScriptMan.PlayScript(script);
			}
			else if (msg == "StopShooting")
			{
				playerShip->shoot = false;
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
						SetMode(IN_LOBBY);
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
			else if (msg == "GoToPreviousMode")
			{
				SetMode(previousMode);
			}
			else if (msg == "OpenOptionsScreen")
			{
				SetMode(EDITING_OPTIONS);
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
				ShowLevelStats();
			}
			else if (msg.StartsWith("HideLevelStats"))
			{
				showLevelStats = false;
				UpdateUI();
			}
			else if (msg == "Pause/Break" || msg == "TogglePause")
			{
				TogglePause();
			}
			else if (msg == "ResumeGame")
				Resume();
			else if (msg == "ListEntitiesAndRegistrations")
			{
				std::cout<<"\nGraphics entities "<<GraphicsMan.RegisteredEntities()<<" physics "<<PhysicsMan.RegisteredEntities()
					<<" projectiles "<<projectileEntities.Size()<<" ships "<<shipEntities.Size();
			}
			else if (msg.StartsWith("LevelToLoad:"))
			{
				String source = msg;
				source.Remove("LevelToLoad:");
				source.RemoveSurroundingWhitespaces();
				levelToLoad = source;
			}
			else if (msg.StartsWith("LoadLevel:"))
			{
				String source = msg;
				source.Remove("LoadLevel:");
				source.RemoveSurroundingWhitespaces();
				if (!source.Contains("Levels"))
					source = "Levels/" + source;
				if (!source.Contains(".srl"))
					source += ".srl";
				LoadLevel(source);
			}
			else if (msg == "ReloadLevel")
			{
				LoadLevel(level.source);
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
				for (int i = 0; i < level.ships.Size(); ++i)
				{
					level.ships[i]->spawned = true;
					level.ships[i]->destroyed = true;
				}
				MapMan.DeleteEntities(shipEntities);
				// Move the level-entity, the player will follow.
				PhysicsMan.QueueMessage(new PMSetEntity(levelEntity, PT_POSITION, Vector3f(level.goalPosition - 5.f, 10, 0)));
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
				SetMode(IN_LOBBY);
			}
			else if (msg.StartsWith("ShowGearDesc:"))
			{
				String text = msg;
				text.Remove("ShowGearDesc:");
				GraphicsMan.QueueMessage(new GMSetUIs("GearInfo", GMUI::TEXT, text));
			}
			else if (msg.StartsWith("BuyGear:"))
			{
				String name = msg;
				name.Remove("BuyGear:");
				Gear gear = Gear::Get(name);
				switch(gear.type)
				{
					case Gear::SHIELD_GENERATOR:
						playerShip->shield = gear;
						break;
					case Gear::ARMOR:
						playerShip->armor = gear;
						break;
				}
				// Update stats.
				playerShip->UpdateStatsFromGear();
				/// Reduce munny
				money->iValue -= gear.price;
				/// Update UI to reflect reduced funds.
				UpdateGearList();
				// Play some SFX too?
				
				// Auto-save.
				MesMan.QueueMessages("AutoSave(silent)");
			}
			else if (msg.Contains("ExitToMainMenu"))
			{
				SetMode(MAIN_MENU);
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

/// Callback from the Input-manager, query it for additional information as needed.
void SpaceShooter2D::KeyPressed(int keyCode, bool downBefore)
{

}


/// Called from the render-thread for every viewport/AppWindow, after the main rendering-pipeline has done its job.
void SpaceShooter2D::Render(GraphicsState * graphicsState)
{
	switch(mode)
	{
		case PLAYING_LEVEL:	
			if (!levelEntity)
				return;
			RenderInLevel(graphicsState);
			break;
		default:
			return;
	}
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

String SpaceShooter2D::GetLevelVarName(String levelPath, String name)
{
	if (levelPath == "current")
		levelPath = level.source;
	return "Level_"+ levelPath +"_"+ name;
}

#define GetVar(varName)\
	String name = GetLevelVarName(level, varName);\
	GameVar * gv = GameVars.Get(name);\
	if (!gv)\
		gv = GameVars.CreateInt(name, 0);\
	return gv;\

/// Level score. If -1, returns current.
GameVariable * SpaceShooter2D::LevelScore(String level)
{
	GetVar("score");
}

/// Level score. If -1, returns current.
GameVariable * SpaceShooter2D::LevelKills(String level)
{
	GetVar("kills");
}

GameVariable * SpaceShooter2D::LevelPossibleKills(String level)
{
	GetVar("possibleKills");
}

/// Resets all the above.
void SpaceShooter2D::ResetLevelStats()
{
	LevelScore()->iValue = 0;
	LevelKills()->iValue = 0;
	LevelPossibleKills()->iValue = 0;
}


void SpaceShooter2D::LoadShipData()
{

	/// Fetch file which dictates where to load weapons and ships from.
	List<String> lines = File::GetLines("ToLoad.txt");
	enum {
		SHIPS, WEAPONS
	};
	int parseMode = SHIPS;
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		if (line.StartsWith("//"))
			continue;
		if (line.Contains("Ships:"))
			parseMode = SHIPS;
		else if (line.Contains("Weapons:"))
			parseMode = WEAPONS;
		else if (parseMode == SHIPS)
			Ship::LoadTypes(line);	
		else if (parseMode == WEAPONS)
			Weapon::LoadTypes(line);	
	}
	// Load shop-data.
	Gear::Load("data/ShopWeapons.csv");
	Gear::Load("data/ShopArmors.csv");
	Gear::Load("data/ShopShields.csv");
	shipDataLoaded = true;
}


/// Starts a new game. Calls LoadLevel
void SpaceShooter2D::NewGame()
{
	PopUI("NewGame");
	PopUI("MainMenu");
		
	// Create player.
	NewPlayer();
	startDate = Time::Now();

	// Reset scores.
	score->iValue = 0;
	// Set stage n level
	currentStage->iValue = 0;
	currentLevel->iValue = 0;
	// And load it.
	LoadLevel();
	// Play script for animation or whatever.
	ScriptMan.PlayScript("scripts/NewGame.txt");
	// Resume physics/graphics if paused.
	Resume();

	TextMan.LoadFromDir();
	TextMan.SetLanguage("English");
}

/// o.o
void SpaceShooter2D::NewPlayer()
{
	if (!shipDataLoaded)
		LoadShipData();

	SAFE_DELETE(playerShip);
	// Reset player-ship.
	if (playerShip == 0)
	{
		playerShip = Ship::New("Default");
		playerShip->enemy = false;
		playerShip->allied = true;
	}
	playerShip->weapon = Gear::StartingWeapon();
	playerShip->armor = Gear::StartingArmor();
	playerShip->shield = Gear::StartingShield();
	playerShip->UpdateStatsFromGear();

	for (int i = 0; i < WeaponType::MAX_TYPES; ++i)
	{
		playerShip->SetWeaponLevel(i, 0);
	}
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
		QueueAudio(new AMGlobal(AM_PAUSE_PLAYBACK));
	}
	else {
		GraphicsMan.QueueMessage(new GraphicsMessage(GM_RESUME_PROCESSING));
		PhysicsMan.Resume();
		QueueAudio(new AMGlobal(AM_RESUME_PLAYBACK));
	}
}


/// Loads target level. The source and separate .txt description have the same name, just different file-endings, e.g. "Level 1.png" and "Level 1.txt"
void SpaceShooter2D::LoadLevel(String fromSource)
{
	levelTime = flyTime = Time(TimeType::MILLISECONDS_NO_CALENDER);
	bool nonStandardLevel = false;
	if (fromSource != "CurrentStageLevel")
	{
		nonStandardLevel = true;
		currentStage->SetInt(999);
		currentLevel->SetInt(0);
	}
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
	if (currentStage->GetInt() == 0)
		fromSource = "Levels/Tutorial";
	this->levelSource = fromSource;
	// Delete all entities.
	MapMan.DeleteAllEntities();
	shipEntities.Clear();
	projectileEntities.Clear();
	
	GraphicsMan.PauseRendering();
	SleepThread(50);
	PhysicsMan.Pause();

	level.Load(fromSource);
	level.SetupCamera();
	if (!playerShip)
		NewPlayer();


	/// Clear old stars?
	QueueGraphics(new GMClearParticles(stars));

	/// Add emitter for stars at player start.
	float emissionSpeed = level.starSpeed.Length();
	Vector3f starDir = level.starSpeed.NormalizedCopy();
	float starScale = 0.2f;

	ParticleEmitter * startEmitter = new ParticleEmitter();
	startEmitter->newType = true;
	startEmitter->instantaneous = true;
	startEmitter->constantEmission = 1400;
	startEmitter->positionEmitter.type = EmitterType::PLANE_XY;
	startEmitter->positionEmitter.SetScale(100.f);
	startEmitter->velocityEmitter.type = EmitterType::VECTOR;
	startEmitter->velocityEmitter.vec = starDir;
	startEmitter->SetEmissionVelocity(emissionSpeed);
	startEmitter->SetParticleLifeTime(60.f);
	startEmitter->SetScale(starScale);
	startEmitter->SetColor(level.starColor);
	QueueGraphics(new GMAttachParticleEmitter(startEmitter, stars));

	/// Update base emitter emitting all the time.
	starEmitter->newType = true;
	starEmitter->direction = starDir;
	starEmitter->SetEmissionVelocity(emissionSpeed);
	starEmitter->SetParticlesPerSecond(40);
	starEmitter->positionEmitter.type = EmitterType::PLANE_XY;
	starEmitter->positionEmitter.SetScale(30.f);
	starEmitter->velocityEmitter.type = EmitterType::VECTOR;
	starEmitter->velocityEmitter.vec = starDir;
	starEmitter->SetParticleLifeTime(60.f);
	starEmitter->SetColor(level.starColor);
	starEmitter->SetScale(starScale);


	/// Add entity to track for both the camera, blackness and player playing field.
	Vector3f initialPosition = Vector3f(0,10,0);
	if (!levelEntity)
	{
		levelEntity = EntityMan.CreateEntity("LevelEntity", NULL, NULL);
		levelEntity->localPosition = initialPosition;
		PhysicsProperty * pp = levelEntity->physics = new PhysicsProperty();
		pp->collisionsEnabled = false;
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
			blackness->localPosition = position;
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
	starEmitter->entityToTrack = levelEntity;
	starEmitter->positionOffset = Vector3f(playingFieldSize.x + 10.f,0,0);
//	GraphicsMan.QueueMessage(new GMSetParticleEmitter(starEmitter, GT_EMITTER_ENTITY_TO_TRACK, playerShip->entity));
//	GraphicsMan.QueueMessage(new GMSetParticleEmitter(starEmitter, GT_EMITTER_POSITION_OFFSET, Vector3f(70.f, 0, 0)));
	// Reset position of level entity if already created.
	levelEntity->localPosition = initialPosition;
	levelEntity->physics->velocity = level.BaseVelocity();
//	PhysicsMan.QueueMessage(new PMSetEntity(levelEntity, PT_POSITION, initialPosition));
	// Set velocity of the game.
//	PhysicsMan.QueueMessage(new PMSetEntity(levelEntity, PT_VELOCITY, level.BaseVelocity()));
	// Reset position of player!
//	PhysicsMan.QueueMessage(new PMSetEntity(playerShip->entity, PT_POSITION, initialPosition));

	level.AddPlayer(playerShip);
	// Reset player stats.
	playerShip->hp = playerShip->maxHP;
	playerShip->shieldValue = playerShip->maxShieldValue;
	playerShip->entity->localPosition = initialPosition + Vector3f(-50,0,0);


	sparks->SetAlphaDecay(DecayType::QUADRATIC);

	GraphicsMan.ResumeRendering();
	PhysicsMan.Resume();
	// Set mode! UI updated from within.
	SetMode(PLAYING_LEVEL);
	level.OnEnter();
	// Run start script.
	ScriptMan.PlayScript("scripts/OnLevelStart.txt");

	// o.o
	this->Resume();
}

void SpaceShooter2D::GameOver()
{
	if (mode != GAME_OVER)
	{
		SetMode(GAME_OVER);
		// Play script for animation or whatever.
		ScriptMan.PlayScript("scripts/GameOver.txt");
		// End script by going back to menu or playing a new game.
	}
}

void SpaceShooter2D::OnLevelCleared()
{
	if (mode != LEVEL_CLEARED)
	{
		SetMode(LEVEL_CLEARED);
		ScriptMan.PlayScript("scripts/LevelComplete.txt");
	}
}

/// Opens main menu.
void SpaceShooter2D::OpenMainMenu()
{
	SetMode(MAIN_MENU);
	/// Proceed straight away if test.
	if (introTest)
	{
		NewGame();
	}
}

/// Where the ship will be re-fitted and new gear bought.
void SpaceShooter2D::EnterShipWorkshop()
{
	SetMode(IN_WORKSHOP);
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



void SpaceShooter2D::UpdatePlayerVelocity()
{
	Vector3f totalVec;
	for (int i = 0; i < movementDirections.Size(); ++i)
	{
		Vector3f vec = Direction::GetVector(movementDirections[i]);
		totalVec += vec;
	}
	totalVec.Normalize();
	totalVec *= playerShip->Speed();
	totalVec *= playerShip->movementDisabled? 0 : 1;
	totalVec += level.BaseVelocity();

	// Set player speed.
	if (playerShip->entity)
	{
		PhysicsMan.QueueMessage(new PMSetEntity(playerShip->entity, PT_VELOCITY, totalVec));
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

List<int> previousModes;
/// Saves previousMode
void SpaceShooter2D::SetMode(int newMode, bool updateUI)
{
	if (previousMode != mode)
	{
		previousMode = mode;
		previousModes.AddItem(mode); // List of modes for backing much?
	}
	mode = newMode;
	// Update UI automagically?
	if (updateUI)
		UpdateUI();
}

