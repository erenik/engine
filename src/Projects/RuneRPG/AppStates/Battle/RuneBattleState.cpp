// Emil Hedemalm
// 2013-06-28
// Handles battles 

#include "RuneBattleState.h"

#include "RuneRPG/RRPlayer.h"
#include "RuneRPG/Battle.h"

#include "Physics/PhysicsProperty.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/Messages/CollisionCallback.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Random/Random.h"

#include "Pathfinding/NavMesh.h"
#include "Pathfinding/WaypointManager.h"

#include "Graphics/Camera/Camera.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Messages/GMLight.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMParticles.h"

#include "Message/FileEvent.h"
#include "Message/Message.h"

#include "OS/Sleep.h"
#include "Actions.h"

#include "UI/UserInterface.h"
#include "UI/UIButtons.h"
#include "UI/UIFileBrowser.h"
#include "UI/UIList.h"
#include "UI/UIInputs.h"

#include "Entity/EntityProperty.h"
#include "Player/PlayerManager.h"
#include "Window/AppWindowManager.h"
#include "Window/AppWindowManager.h"

#include "Maps/MapManager.h"
#include "Maps/Grids/TileGrid.h"

#include "Viewport.h"
#include <Util.h>
#include <iomanip>
#include "../RuneGameStatesEnum.h"
#include <ctime>

#include "TextureManager.h"
#include "StateManager.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "Color.h"

#include "RuneRPG/Battle/BattleStats.h"
#include "Message/MathMessage.h"
#include "Audio/AudioManager.h"
#include "Audio/Messages/AudioMessage.h"

extern UserInterface * ui[GameStateID::MAX_GAME_STATES];

#define DEFAULT_UI_TEXTURE  "img/80gray50Alpha.png"

Random spawnRandom;

List<String> queuedBattles;

RuneBattleState * RuneBattleState::state = NULL;

/// Stringify Integer
#define STRINT(i) String::ToString(i)

RuneBattleState::RuneBattleState()
{
	id = RUNE_GAME_STATE_BATTLE_STATE;
	battleTestWindow = NULL;
	camera = NULL;
	navMesh = NULL;
	battleGrid = NULL;
	selectedBattleAction = NULL;
	paused = false;
	state = this;
	addBattlerMode = ADD_BATTLER_TO_ENEMY_SIDE;
}

RuneBattleState::~RuneBattleState()
{
	state = NULL;
}

/// Creates the user interface for this state
void RuneBattleState::CreateUserInterface()
{
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/RuneBattle.gui");
}

void RuneBattleState::OnEnter(AppState * previousState)
{
	if (!ui)
		CreateUserInterface();
	paused = false;
	// Load initial texture and set it to render over everything else
	Graphics.QueueMessage(new GMSetOverlay("img/loadingData.png", 100));

	battleType = NORMAL_BATTLE;

	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSetUI(ui));

    /// Hide the commands-menu until a party-member is ready for action, jRPG-style
    commandsMenuOpen = false;
    Graphics.QueueMessage(new GMSetUIb("CommandsMenu", GMUI::VISIBILITY, false));
    Graphics.QueueMessage(new GMSetUIb("TargetsMenu", GMUI::VISIBILITY, false));
	Graphics.QueueMessage(new GMSetUIs("Narrator", GMUI::TEXT, "An enemy appears!"));

	/// Starts the queued battle if any. If none is queued, don't do any more?
	if (!StartQueuedBattle())
		; // return;


	/// Set editor selection as the renderable one!
	Graphics.selectionToRender = NULL;

	// And set it as active
	Graphics.cameraToTrack = camera;

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));

	/// Start the battle-timer
    timer.Start();
 	/// Notify the input-manager to use menu-navigation.
	Input.ForceNavigateUI(true);

	// Open AppWindow with extra UI for tweaking stuff in real-time on the battle?
	if (!battleTestWindow)
	{
		battleTestWindow = WindowMan.NewWindow("BattleTest");
		battleTestWindow->ui = new UserInterface();
		battleTestWindow->ui->Load("gui/BattleTest.gui");
//		battleTestWindow->renderViewports = false;
		battleTestWindow->CreateGlobalUI();
		battleTestWindow->requestedSize = Vector2i(400, 600);
		battleTestWindow->requestedRelativePosition = Vector2i(-400,0);
		battleTestWindow->Create();
		battleTestWindow->DisableAllRenders();
		battleTestWindow->renderUI = true;
		battleTestWindow->hideOnEsc = false;
	}
	battleTestWindow->Show();


	logVisibility = false;
	// Update it. <- What? Seriously.
	ToggleLogVisibility();


	/// Set up a dedicated viewport for the battle as we are not interested in rendering stuff behind the UI in the bottom part of the screen?
	float viewportSizeX = 1.f;
	Viewport * battleViewport = new Viewport();
	battleViewport->EnableAllDebugRenders(false);
	battleViewport->backgroundColor = Vector4f(0.2f, 0.2f, 0.2f, 1);
	battleViewport->SetRelative(Vector2f(0, 0.2f), Vector2f(viewportSizeX, 0.8f));
	Graphics.QueueMessage(new GMSetViewports(battleViewport, MainWindow()));

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GMSetOverlay(NULL));
}

/// Returns true upon success.
bool RuneBattleState::StartQueuedBattle()
{
	// Check for data (battlers)
	bool gotData = false;
	if (!queuedBattles.Size())
	{
		std::cout<<"\nNo queued battle.";
		return false;
	}
	String requestedBattle = queuedBattles[0];
	battleSource = requestedBattle;
	gotData = requestedBattle.Length() != 0;
	if (!gotData){
		requestedBattle = "data/Battles/testBattle.txt";
	}
	/// Load it!
	bool result = LoadBattle(requestedBattle);
	/// Failed to load battle, try a new one or return?
	if (!result){
		EndBattle();
		return false;
	}
	if (requestedBattle.Contains("Practice")){
		battleType = PRACTICE_DUMMY_BATTLE;
	}
	// Start applicable music..


	return true;
}


void RuneBattleState::OnExit(AppState *nextState)
{
	if (battleTestWindow)
		battleTestWindow->Hide();
	/// Notify the input-manager to stop force-using menu-navigation.
	Input.ForceNavigateUI(false);

	// Load initial texture and set it to render over everything else
	Graphics.QueueMessage(new GMSetOverlay(TexMan.GetTexture("img/loadingData.png")));

	SleepThread(100);
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	Graphics.QueueMessage(new GMSetCamera(camera, CT_ENTITY_TO_TRACK, NULL));
//	Graphics.cameraToTrack->entityToTrack = NULL;

	std::cout<<"\nLeaving RuneBattleState.";
	// Set graphics manager to render UI, and remove the overlay-texture.
//	Graphics.SetOverlayTexture(NULL);
}


/// For rendering cursors or something?
void RuneBattleState::Render(GraphicsState & graphicsState)
{
}


/// Gets target battlers
RuneBattler * RuneBattleState::GetBattler(String byName)
{
	RuneBattler * battler = NULL;
	List<RuneBattler*> battlers = GetBattlers();
	for (int i = 0; i < battlers.Size(); ++i)
	{
		RuneBattler * rb = battlers[i];
		if (rb->name == byName)
			battler = rb;
	}
	return battler;
}

/// Gets all active battlers
List<RuneBattler*> RuneBattleState::GetBattlers()
{
	return battlers;
}

/// Gets all active battlers by filtering string. This can be comma-separated names of other kinds of specifiers.
List<RuneBattler*> RuneBattleState::GetBattlers(String byFilter)
{
	List<RuneBattler*> runeBattlers;
	for (int i = 0; i < battlers.Size(); ++i)
	{
		RuneBattler * battler = battlers[i];
		if (battler->name == byFilter)
			runeBattlers.Add(battlers[i]);
	}
	return runeBattlers;
	/*
	if (byFilter == "none" ||
		byFilter == "NULL")
		return runeBattlers;
	List<Battler*> battlers = BattleMan.GetBattlers();
	for (int i = 0; i < battlers.Size(); ++i)
	{
		RuneBattler * battler = (RuneBattler *) battlers[i];
		if (battler->name == byFilter)
			runeBattlers.Add((RuneBattler*) battlers[i]);
	}
	return runeBattlers;
	*/
}


/// Toggle visibility of the battle log (of previous executed actions)
void RuneBattleState::ToggleLogVisibility(bool * newState)
{
	if (newState)
	{
		logVisibility = *newState;
	}
	Graphics.QueueMessage(new GMSetUIb("Log", GMUI::VISIBILITY, logVisibility, ui));
}


void RuneBattleState::ResetInitiative()
{
    /// Set all battlers inititive
    for (int i = 0; i < battlers.Size(); ++i){
        RuneBattler * b = (RuneBattler*)battlers[i];
		b->actionPoints = rand() % 25;
 //       b->initiative = 1500 + rand()%10000;
    }
}

/// !
List<RuneBattler*> RuneBattleState::GetPlayerBattlers()
{
	List<RuneBattler*> players;
	for (int i = 0; i < battlers.Size(); ++i){
		RuneBattler * battler = battlers[i];
		if (!battler->isEnemy)
			players.Add(battler);
	}
	return players;
}

/// Update UI accordingly.
void RuneBattleState::CreatePartyUI()
{
	/// Fill the player's UI!
	/// But first clear it.
	Graphics.QueueMessage(new GMClearUI("PartyStatus"));

	List<RuneBattler*> playerBattlers = GetPlayerBattlers();

	/// Fill it with new UI's per player.
	for (int i = 0; i < playerBattlers.Size(); ++i)
	{
		RuneBattler * rb = playerBattlers[i];
		UIElement * ui = new UIColumnList();
		ui->name = "Player"+String::ToString(i);
		ui->textureSource = "img/80gray50Alpha.png";
		/// Assume max 4 players.
		ui->sizeRatioY = 1.f;

		Vector4f textColor = Vector4f(1,1,1,1);

		/// Add a name to it too.
		UIElement * name = new UIElement();
		name->text = rb->name;
		name->textColor = Vector4f(1,1,1,1);
		name->sizeRatioX = 0.3f;
		ui->AddChild(name);

		/// Add HP/MP too
		UIElement * hp = new UIElement();
		hp->name = "Player" + STRINT(i) + "HP";
		hp->textColor = textColor;
		hp->text = "HP: "+String::ToString(rb->HP())+"/"+String::ToString(rb->MaxHP());
		hp->sizeRatioX = 0.2f;
		ui->AddChild(hp);

		UIElement * mp = new UIElement();
		mp->name = "Player" + STRINT(i) + "MP";
		mp->textColor = textColor;
		mp->text = "MP: "+String::ToString(rb->MP())+"/"+String::ToString(rb->MaxMP());
		mp->sizeRatioX = 0.2f;
		ui->AddChild(mp);

		// And initiative
		UIElement * initiative = new UIElement();
		initiative->name = rb->name+"Ini";
		initiative->textColor = textColor;
		initiative->text = "Ini: "+String::ToString(rb->actionPoints);
		initiative->sizeRatioX = 0.2f;
		ui->AddChild(initiative);


		Graphics.QueueMessage(new GMAddUI(ui, "PartyStatus"));
	}
}






/// Returns idle player-controlled battler if available. NULL if not.
RuneBattler * RuneBattleState::GetIdlePlayer()
{
	List<RuneBattler*> playerBattlers = GetPlayerBattlers();
	for (int i = 0; i < playerBattlers.Size(); ++i)
	{
		RuneBattler * playerBattler = playerBattlers[i];
		if (playerBattler->isAI)
			continue;
		if (playerBattler->IsIdle())
			return playerBattler;
	}
	return NULL;
}

void RuneBattleState::Process(int timeInMs)
{
	/// Process key input <- ?
	SleepThread(10);
 
	if (paused)
		return;

	RBattleState bs;
	bs.battlers = battlers;
	bs.timeInMs = timeInMs;

	/// Update UI, yo.
	UpdatePlayerHPUI();

	// Process all battlers, as well as their attached effects and active actions.
	for (int i = 0; i < battlers.Size(); ++i)
	{
		RuneBattler * rb = battlers[i];
		rb->Process(bs);
		/// Narrate as needed.
		if (bs.log.Length())
			Graphics.QueueMessage(new GMSetUIs("Narrator", GMUI::TEXT, bs.log));
	}

	/// Check if commands AppWindow is opened.
	if (!commandsMenuOpen)
	{
		List<RuneBattler*> playerBattlers = GetPlayerBattlers();
		if (playerBattlers.Size())
		{
			RuneBattler * player = playerBattlers[0];
			// Open menu if no queued actions!
			if (player->queuedActions.Size() == 0)
				OpenCommandsMenu(player);
		}
	}

	// Should it be opened?


	/// 
	UIElement * targetWindow = this->ui->GetElementByName("TargetsMenu");
	// When reloading UI just wait.
	if (!targetWindow)
		return;
	bool targetWindowOpen = targetWindow->IsVisible();

	// Control target-rendering using some entities, yes? Might be good to keep it away from old GL stuff... should improve animation of them as well!
	if (this->targetBattlers.Size())
	{
		for (int i = 0; i < targetBattlers.Size(); ++i)
		{
			while(pointerEntities.Size() < targetBattlers.Size())
			{
				// Create entities for handling this target-rendering...
				Entity * entity = MapMan.CreateEntity("Targeting Pointer", ModelMan.GetModel("obj/3DPointer.obj"), TexMan.GetTexture("Grey"));
				pointerEntities.Add(entity);
				Physics.QueueMessage(new PMSetEntity(entity, PT_SET_SCALE, 0.5f));
			} 
			RuneBattler * targetBattler = targetBattlers[i];
			Entity * battlerEntity = targetBattler->entity;
			if (!battlerEntity)
				continue;
			Entity * pointerEntity = pointerEntities[i];
			// Reveal the targetting arrows and move them info place.
			Physics.QueueMessage(new PMSetEntity(pointerEntities[i], PT_POSITION, battlerEntity->position + Vector3f(0,battlerEntity->scale.y,0)));
			Graphics.QueueMessage(new GMSetEntityb(pointerEntity, GT_VISIBILITY, true));
		}
	}
	else 
	{
		// Hide all entity-pointers!
		for (int i = 0; i < pointerEntities.Size(); ++i)
		{
			Entity * pointerEntity = pointerEntities[i];
			Graphics.QueueMessage(new GMSetEntityb(pointerEntity, GT_VISIBILITY, false));
		}
	}

	
	/// Clear is for killing children, yo.
   // Graphics.QueueMessage(new GMClearUI("Log"));
	List<RuneBattler*> players = GetPlayerBattlers();
    String str = "Combatants: "+String::ToString(battlers.Size())+" of which players: "+String::ToString(players.Size());

	List<RuneBattler*> playerBattlers = GetPlayerBattlers();
    for (int i = 0; i < players.Size(); ++i)
	{
        RuneBattler * b = (RuneBattler*)playerBattlers[i];
		int actionPoints = b->ActionPoints();
		String strAP = STRINT(actionPoints);
		str += "\nBattler"+STRINT(i)+": "+b->name+" Initiative: " + strAP;
		if (!b->isEnemy)
		{
			Graphics.QueueMessage(new GMSetUIs(b->name+"Ini", GMUI::TEXT, strAP));
		}
	}
	
//    Graphics.QueueMessage(new GMSetUIs("Log", GMUI::TEXT, str));


	/// If we got someone with initiative and the menu is down for some reason, open it again?
	RuneBattler * idlePlayer = GetIdlePlayer();
	if (idlePlayer && !commandsMenuOpen){
		/// Open menu..?
		this->OpenCommandsMenu(idlePlayer);
	}
};

enum mouseCameraStates {
	NULL_STATE,
	ROTATING,
	PANNING
};


/// Input functions for the various states
void RuneBattleState::MouseClick(bool down, int x, int y, UIElement * elementClicked){
}
void RuneBattleState::MouseRightClick(bool down, int x, int y, UIElement * elementClicked){
}

#define PAN_SPEED_MULTIPLIER (abs(camera->distanceFromCentreOfMovement)/2.0f + 1)
void RuneBattleState::MouseMove(int x, int y, bool lDown, bool rDown, UIElement * elementOver){
}

void RuneBattleState::MouseWheel(float delta)
{
	Camera * camera = Graphics.cameraToTrack;
	camera->distanceFromCentreOfMovement += delta / 100.0f;
	if (delta > 0){
		camera->distanceFromCentreOfMovement *= 0.8f;
	}
	else if (delta < 0){
		camera->distanceFromCentreOfMovement *= 1.25f;
	}
	if (camera->distanceFromCentreOfMovement > 0)
		camera->distanceFromCentreOfMovement = 0;
}


void RuneBattleState::ProcessMessage(Message * message)
{

//	std::cout<<"\nRacing::ProcessMessage: ";
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::INTEGER_MESSAGE:
		{
			IntegerMessage * setInt = (IntegerMessage*) message;
			if (msg == "SetAddBattlerSide")
			{
				addBattlerMode = setInt->value;
			}
			else if (msg.Contains("SetActiveBattlerStat"))
			{
				String statName = msg.Tokenize(":")[1];
				RuneBattler * battler = GetBattler(activeEditBattlerName);
				if (!battler)
					return;
				int index = GetStatByString(statName);
				if (index >= 0)
				{
					Variable * stat = &battler->baseStats[index];
					stat->iValue = setInt->value;
				}
				/// Update current stats.
				battler->UpdateCurrentStats();
			}
			break;
		}
		case MessageType::ON_UI_ELEMENT_HOVER:
		{
			UIElement * element = message->element;
			if (msg == "SetTargetBattlers(this)")
			{
				String targetString = element->text;
				List<RuneBattler*> battlers = GetBattlers(targetString);
				targetBattlers = battlers;
			}
			break;
		}
		case MessageType::FILE_EVENT:
		{
			FileEvent * fe = (FileEvent*)message;
			List<String> files = fe->files;
			String primaryFile = files[0];
			if (msg == "LoadBattle")
			{
				// From file..?
				LoadBattle(primaryFile);
			}
		}
		case MessageType::STRING: 
		{
			String string = message->msg;
			string.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (string == "begin_commandline"){
				Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			}
			else if (string == "interpret_console_Command"){
				StateMan.ActiveState()->InputProcessor(INTERPRET_CONSOLE_COMMAND);
				return;
			}
			/// Sent when pressing Pause/Break or optionally ALT+P if that is defined in this game state?
			else if (string == "Pause/Break")
			{
				paused = !paused;
			}
			else if (msg.Contains("UpdateBattlersList("))
			{

				String targetUIList = msg.Tokenize("()")[1];
				// Clear the target ui first.
				UserInterface * inUI = message->element->ui;
				Graphics.QueueMessage(new GMClearUI(targetUIList, inUI));
				List<RuneBattler*> battlers = RuneBattlers.GetBattlers();
				for (int i = 0; i < battlers.Size(); ++i)
				{
					RuneBattler * battler = battlers[i];
					if (battler->name.Length() == 0)
						continue;
					UIButton * addBattlerButton = new UIButton(battler->name);
					addBattlerButton->activationMessage = "AddBattler:"+battler->name;
					addBattlerButton->sizeRatioY = 0.1f;
					Graphics.QueueMessage(new GMAddUI(addBattlerButton, targetUIList, inUI));
				}
			}
			else if (msg.Contains("AddBattler"))
			{
				String battlerName = msg.Tokenize(":")[1];
				const RuneBattler * battlerReference = RuneBattlers.GetBattlerByName(battlerName);
				if (!battlerReference)
				{
					std::cout<<"\nError: No Battler found with given name \'"<<battlerName<<"\'";
					return;
				}
				RuneBattler * newBattler = new RuneBattler(*battlerReference);
				/// Create a copy of it?
				switch(addBattlerMode)
				{
					case ADD_BATTLER_TO_ENEMY_SIDE:
						newBattler->isAI = true;
						newBattler->isEnemy = true;
						battlers.Add(newBattler);
						break;
					case ADD_BATTLER_REPLACE_PLAYER:
					{
						// Remove the old player?
						if (activePlayerBattler){
							battlers.Remove(activePlayerBattler);
							delete activePlayerBattler;						
							activePlayerBattler = newBattler;
						}
						battlers.Add(newBattler);
						newBattler->isAI = false;
						newBattler->isEnemy = false;
						
						/// Update menus if it was a player. Just hide and they should refresh...
						HideMenus();
						this->CreatePartyUI();
						break;
					}
				}
				// Recalculate stats!
				newBattler->UpdateCurrentStats();
				/// o.o
				newBattler->name = GetNewSpawnName(newBattler->name);
				/// Create entity for it.
				newBattler->CreateEntity();
				/// Place it.
				PlaceBattler(newBattler);
				break;
			}
			else if (msg.Contains("LoadBattle("))
			{
				String b = msg.Tokenize("()")[1];
				LoadBattle(b);
			}
			else if (msg == "RenderNavMesh")
			{
				Viewport * viewport = MainWindow()->MainViewport();
				viewport->renderNavMesh = !viewport->renderNavMesh;
			}
			else if (msg == "OnReloadUI")
			{
				CreatePartyUI();
				this->commandsMenuOpen = false;
			}
			else if (msg == "EndBattle")
			{
				// End the battle prematurely.
				this->EndBattle();
			}
			else if (msg == "ReloadBattlers")
			{
				// Reload battlers and reload the battle itself too
				RuneBattlers.ReloadBattlers();
				// Reload the current battlers based on the reloaded templates..?
			}
			else if (msg == "ReloadBattleActions")
			{
				RBALib.Reload();
				// Reload the current battlers' actions based on the reloaded templates..?
			}
			else if (msg == "OpenBattle")
			{
				// Open a file-browser.
				UIFileBrowser * browser = new UIFileBrowser("Open battle", "LoadBattle", ".battle");
				browser->SetPath("./data/Battles/", false);
				// Finally create all UI children.
				Graphics.QueueMessage(new GMAddUI(browser, "root", this->battleTestWindow->ui));
				Graphics.QueueMessage(new GMPushUI(browser, battleTestWindow->ui));
			}
			else if (msg == "ToggleBattleLog")
			{
				logVisibility = !logVisibility;
				ToggleLogVisibility();
			}
			else if (msg == "ReloadBattle()")
			{
				// Hide all UI
				HideMenus();
				// Reload it.
				this->LoadBattle(this->battleSource);
			}
			else if (string == "PauseBattle()")
			{
				this->paused = !this->paused;
			}
			else if (string == "ReloadBattle()" ||
				string == "PauseBattle()" ||
				string == "OpenBattlesList" ||
				string == "ReloadBattlers" ||
				string == "ReloadBattleActions" ||
				string == "ToggleLog")
			{
				Log("Not implemented yet, sorry!");	
			}
			else if (msg.Contains("BattlerReady("))
			{
				String battlerName = msg.Tokenize("()")[1];
				RuneBattler * battler = GetBattler(battlerName);
				assert(battler);
			    /// If the commands-menu is already open, don't do anything.
			    /// If it isn't, get the ready battler, open the menu and fill it with stuff!
                if (!commandsMenuOpen){
                    OpenCommandsMenu(battler);
                }
			}
			else if (string == "AbortPractice()"){
				StateMan.QueueState(StateMan.PreviousState());
				return;
			}
			else if (string.Contains("OpenSubMenu(")){
				String categoryName = string.Tokenize("()")[1];
				OpenSubMenu(categoryName);
			}
			else if (string.Contains("SetAction("))
			{
                action = string.Tokenize("()")[1];
			}
			else if (string.Contains("OnAttackPlayers")){
				/// Update the HP gui.
				UpdatePlayerHPUI();
			}
			else if (string.Contains("OpenTargetMenu("))
			{
			    targetMode = string.Tokenize("()")[1].ParseInt();
			    OpenTargetMenu();
			}
			else if (string.Contains("SetTarget"))
			{
				this->targets = string.Tokenize("(,)");
				targets.RemoveIndex(0);
                targetBattlers.Clear();
                for (int i = 0; i < targets.Size(); ++i)
				{
					RuneBattler * b = (RuneBattler *) GetBattler(targets[i]);
					if (!b)
						continue;
					targetBattlers.Add(b);
                }
			}
			else if (string.Contains("OnBattlerIncapacitated"))
			{	

				// Should be handled by the battle manager.
				/*
				/// Check if we won or died.
				bool playersDead = true, enemiesDead = true;
				List<RuneBattler*> playerBattlers = this->GetPlayerBattlers();
				for (int i = 0; i < playerBattlers.Size(); ++i)
				{
					if (playerBattlers[i]->state != INCAPACITATED)
						playersDead = false;
				}
				List<RuneBattler*> enemyBattlers = this->GetEnemyBattlers();
				for (int i = 0; i < enemyBattlers.Size(); ++i){
					if (enemyBattlers[i]->state != INCAPACITATED)
						enemiesDead = false;
				}
				/// Fight is over. Run OnEnd stuff from the battle-template?
				if (playersDead || enemiesDead){
					EndBattle();
				}
				*/
				return;
			}
			else if (string == "ExecuteAction()")
			{
				if (!activePlayerBattler)
					return;
				/// Fetch the action.
				if (!selectedBattleAction)
				{
					RuneBattleAction * referenceAction = NULL;
					for (int i = 0; i < activePlayerBattler->actions.Size(); ++i)
					{
						RuneBattleAction * rba = activePlayerBattler->actions[i];
						if (rba->name == action)
						{
							referenceAction = rba;
							break;
						}
					}
					assert(referenceAction);
					selectedBattleAction = new RuneBattleAction(*referenceAction);
				}
				
				// Set targets
          		selectedBattleAction->targets = targetBattlers;
          		// Set subject
				selectedBattleAction->subjects.Add(activePlayerBattler);

				/// Add it to player queue so we know what's up.
				activePlayerBattler->QueueAction(selectedBattleAction);
				
                /// Close all menus
                HideMenus();

                /// Reset booleans for that too.
                targetMode = 0;
				targetBattlers.Clear();
                selectedBattleAction = NULL;
                commandsMenuOpen = false;
                activePlayerBattler = NULL;
				return;
			}
			else if (msg.Contains("UpdateActiveBattlersList("))
			{
				String targetUIList = msg.Tokenize("()")[1];
				UserInterface * inUI = message->element->ui;

				// Clear the target ui first.
				Graphics.QueueMessage(new GMClearUI(targetUIList, inUI));
				for (int i = 0; i < battlers.Size(); ++i)
				{
					RuneBattler * battler = battlers[i];
					if (battler->name.Length() == 0)
						continue;
					UIButton * addBattlerButton = new UIButton(battler->name);
					addBattlerButton->activationMessage = "SetActiveBattler("+battler->name+")&PushUI(ActiveBattlerStats)";
					addBattlerButton->sizeRatioY = 0.1f;
					Graphics.QueueMessage(new GMAddUI(addBattlerButton, targetUIList, inUI));
				}
			}
			else if (msg.Contains("SetActiveBattler("))
			{
				String name = msg.Tokenize("()")[1];
				activeEditBattlerName = name;
				/// Fetch the actual battler when editing the ui... which I guess we should do now.
				RuneBattler * battler = NULL;
				for (int i = 0; i < battlers.Size(); ++i)
				{
					RuneBattler * rb = battlers[i];
					if (rb->name == name)
					{
						battler = rb;
					}
				}
				String targetUIList = "ActiveBattlerStats";
				UserInterface * inUI = message->element->ui;
				Graphics.QueueMessage(new GMClearUI(targetUIList, inUI));
				/// Leave empty list if there is some error.
				if (!battler)
					break;
				/// Add label with name.
				UILabel * nameLabel = new UILabel(name);
				nameLabel->sizeRatioY = 0.1f;
				Graphics.QueueMessage(new GMAddUI(nameLabel, targetUIList, inUI));
				/// List all base stats.
				for (int i = 0; i < battler->baseStats.Size(); ++i)
				{
					Variable * stat = &battler->baseStats[i];
					UIColumnList * box = new UIColumnList("ActiveBattlerStatCL:"+stat->name);
					box->sizeRatioY = 0.1f;
					UIIntegerInput * statInput = new UIIntegerInput(stat->name, "SetActiveBattlerStat:"+stat->name);
					statInput->CreateChildren();
					statInput->SetValue(stat->iValue);
					statInput->sizeRatioX = 0.8f;
					/// Check current value. Is it differing from the base value?
					Variable * current = &battler->currentStats[i];
					int diff = current->iValue - stat->iValue;
					UILabel * statModifierLabel = new UILabel();
					if (diff != 0)
					{
						// Add label with the diff.
						String diffText = String(diff);
						if (diff > 0)
							diffText = "+" + diffText;
						// Add a plus sign if positive.
						statModifierLabel->SetText(diffText, true);
						if (diff > 0)
							statModifierLabel->textColor = Color::ColorByHex24(0x22FF22);
						else 
							statModifierLabel->textColor = Color::ColorByHex24(0xFF2222);
					}
					else {
						statModifierLabel->SetText("+0");
						statModifierLabel->textColor = Color::ColorByHex16(0xFF11);
					}					
					box->AddChild(statInput);
					box->AddChild(statModifierLabel);
					/// Add label with name.
					Graphics.QueueMessage(new GMAddUI(box, targetUIList, inUI));
				}
				/// List other stats not stored in that list?
			}
		}
	}
	AppState::ProcessMessage(message);
}

/// Loads battle from source~! Returns false upon failure.
bool RuneBattleState::LoadBattle(String fromSource)
{
	// Delete battlers.
	battlers.ClearAndDelete();

	if (!fromSource.Contains("data/Battles/")){
		fromSource = "data/Battles/" + fromSource;
	}
	if (!fromSource.Contains(".battle"))
		fromSource += ".battle";
	/// Initialize test battle!
	RuneBattle b = RuneBattlers.GetBattleBySource(fromSource);
	if (!b.enemyNames.Size())
	{
		std::cout<<"\nUnable to load battle! No enemies present.";
		return false;
	}
	assert(b.enemyNames.Size() && "unable to laod test RuneBattle");
	if (b.enemyNames.Size() == NULL)
		return false;

	// Save battle source for when reloading later
	battleSource = fromSource;

	/// Load custom player-battlers for specific/unique battles
	for (int i = 0; i < b.playerNames.Size(); ++i)
	{
		RuneBattler rb = RuneBattler(*RuneBattlers.GetBattlerBySource(b.playerNames[1]));
		std::cout<<"\nAdding RuneBattler player: "<<rb.name;
		RuneBattler * newPlayer = new RuneBattler(rb);
		battlers.Add(newPlayer);
	}
	/// Add players based on the playerManager or RRPlayer thingy!
	List<Player*> players = PlayerMan.GetPlayers();
	for (int i = 0; i < players.Size(); ++i)
	{
		RRPlayer * rp = (RRPlayer*)players[i];
		RuneBattler * newPlayerBattler = new RuneBattler(*rp->Battler());
		battlers.Add(newPlayerBattler);
	}
	
	// If still no players, create a test one?
	if (!GetPlayerBattlers().Size())
	{
		const RuneBattler * base = RuneBattlers.GetBattlerBySource("Player");
		assert(base);
		RuneBattler * battler = new RuneBattler(*base);
		battler->isAI = false;
		battlers.Add(battler);
	}
	// Add enemies accordingly.
	for (int i = 0; i < b.enemyNames.Size(); ++i){
		String source = b.enemyNames[i];
		const RuneBattler * runeBattlerBase = RuneBattlers.GetBattlerBySource(source);
		if (!runeBattlerBase)
		{
			std::cout<<"\nUnable to load battler by source: "<<source;
			continue;
		}
		RuneBattler * newEnemy =  new RuneBattler(*runeBattlerBase);
		newEnemy->isEnemy = true;
		newEnemy->isAI = true;
		std::cout<<"\nAdding RuneBattler enemy: "<<newEnemy->name;
		battlers.Add(newEnemy);
	}
	/// Fetch all battlers.
	for (int i = 0; i < battlers.Size(); ++i){
		battlers[i]->ResetStats();
	}	
	/// For all battlers, count the names, and adjust them if there are ANY multiples at all out there.
	for (int i = 0; i < battlers.Size(); ++i)
	{
		RuneBattler * battler = battlers[i];
		battler->name = GetNewSpawnName(battler->name);
	}

	Log("RuneBattle loaded from file: "+fromSource);

	/// Setup the map.
	SetupBattleMap();
	CreateNavMesh();
	/// Setup lighting to fit the location/time
	SetupLighting();
	/// Create entities!
	CreateBattlerEntities();
	/// Place them on the grid.
	PlaceBattlers();

	/// Setup camera
	SetupCamera();

	/// Create UI
	CreatePartyUI();
	
	// Randomize starting initiative.
	ResetInitiative();

	// Start the music! :D
	AudioMan.QueueMessage(new AMPlay(AudioType::BGM, "Somewhere.ogg", 0.5f));

	// Reload ui.
	commandsMenuOpen = false;

	// Delete the targetting entities.
	this->pointerEntities.Clear();

	return true;
}

/// Loads the "map" to be used, creates the grid etc.
void RuneBattleState::SetupBattleMap()
{
	bool result = MapMan.MakeActive("BattleMap");
	if (!result)
	{
		Map * map = MapMan.CreateMap("BattleMap");
		assert(MapMan.MakeActive(map));
	}
	// Clear old entities
	MapMan.DeleteAllEntities();

	/// Set map size..
	mapSize = Vector2i(10,5);

	// Set a background image or something for the time being..?
	// Generate tile grid and stuff to render it?
}

/// Using known battle map, setup navmesh to use.
void RuneBattleState::CreateNavMesh()
{
	if (!battleGrid)
		battleGrid = new TileGrid2D();
	Vector2i gridSize = mapSize;
	battleGrid->SetType(GridType::HEXAGONS);
	battleGrid->Resize(gridSize);
	// Create a default grid.
	if (!navMesh)
		navMesh = WaypointMan.CreateNavMesh("BattleGrid");
	navMesh->Clear();
	battleGrid->GenerateWaypoints(navMesh, 1.1f);
}


/// Setup lighting to fit the location/time
void RuneBattleState::SetupLighting()
{
	float intensity = 5.f;
	Graphics.QueueMessage(new GMSetAmbience(Vector4f(intensity, intensity, intensity, 1)));
}

/// Create entities! 
void RuneBattleState::CreateBattlerEntities()
{
	// Delete previous entities?

	List<RuneBattler*> battlers = this->GetBattlers();
	for (int i = 0; i < battlers.Size(); ++i)
	{
		RuneBattler * battler = battlers[i];
		battler->CreateEntity();
	}
}

/// Place them on the grid.
void RuneBattleState::PlaceBattlers()
{
	List<RuneBattler*> battlers = this->GetBattlers();
	for (int i = 0; i < battlers.Size(); ++i)
	{
		RuneBattler * battler = battlers[i];
		PlaceBattler(battler);
	}
}

/// Places the battler somewhere on the map, where fitting.
void RuneBattleState::PlaceBattler(RuneBattler * battler)
{
	// Place them and their entities on the grid.
	Entity * entity = battler->entity;
	assert(battler->entity && "Create the entity first!");
	Waypoint * wp;
	if (battler->isEnemy)
	{
		wp = GetFreeEnemyPosition();
	}
	else 
		wp = GetFreeAllyPosition();

	Vector3f position = wp->position;
	position.z = 1 - position.y * 0.1f; 
	Physics.QueueMessage(new PMSetEntity(entity, PT_POSITION, position));
	/// Require depth-sorting so the alpha-blending will work.
	Graphics.QueueMessage(new GMSetEntityb(entity, GT_REQUIRE_DEPTH_SORTING, true));
	/// Update render-offset while at it, so that all sprites assume 0.25f of the sprite to be the bottom/center?
	Graphics.QueueMessage(new GMSetEntityVec4f(entity, GT_RENDER_OFFSET, Vector3f(0, 0.75f, 0)));
	/// Set waypoint via physics thread.
	Physics.QueueMessage(new PMSetEntity(entity, PT_CURRENT_WAYPOINT, wp));
//	wp->entity = entity;
}

/// Setup camera
void RuneBattleState::SetupCamera()
{
	if (!camera)
	{
		camera = CameraMan.NewCamera();
		camera->name = "RuneBattle camera";
	}
	Graphics.QueueMessage(new GMSetCamera(camera, MainWindow()));
	camera->Nullify();
	camera->projectionType = Camera::ORTHOGONAL;
	camera->rotation = Vector3f();
	camera->position = Vector3f(mapSize.x * 0.5, mapSize.y * 0.5 + 1.f, 20.f);
	/// Somehow calculate appropriate zoom...
	camera->zoom = 5.f;
	camera->distanceFromCentreOfMovement = -10.f;
}

// Navigation and spawning.	
Waypoint * RuneBattleState::GetFreeEnemyPosition()
{
	List<Waypoint*> validWaypoints;
	// Just use the left half of the grid?
	for (int i = 0; i < navMesh->waypoints.Size(); ++i)
	{
		Waypoint * waypoint = navMesh->waypoints[i];
		if (waypoint->position.x < mapSize.x * 0.5)
			validWaypoints.Add(waypoint);
	}
	if (!validWaypoints.Size())
		return NULL;
	Waypoint * randomWaypoint = validWaypoints[int(spawnRandom.Randf() * validWaypoints.Size()) % validWaypoints.Size()];
	return randomWaypoint;
}
Waypoint * RuneBattleState::GetFreeAllyPosition()
{
	assert(navMesh);
	List<Waypoint*> validWaypoints;
	// Just use the left half of the grid?
	for (int i = 0; i < navMesh->waypoints.Size(); ++i)
	{
		Waypoint * waypoint = navMesh->waypoints[i];
		if (waypoint->position.x > mapSize.x * 0.5)
			validWaypoints.Add(waypoint);
	}
	if (!validWaypoints.Size())
		return NULL;
	Waypoint * randomWaypoint = validWaypoints[int(spawnRandom.Randf() * validWaypoints.Size()) % validWaypoints.Size()];
	return randomWaypoint;
}


/// Pokes the BattleManager to end the battle and then queues state-change to whereever we came from
void RuneBattleState::EndBattle()
{
	std::cout<<"\n==========================================================";
	std::cout<<"\nBATTLE IS OVER!";
	std::cout<<"\n==========================================================";
	/// Hide all UI
	HideMenus();
	/// Go to previous state for now?
	StateMan.QueueState(StateMan.PreviousState());
}

/// Logs by printing both to std::cout and a visible graphical log (that can be toggled in-game).
void RuneBattleState::Log(String string)
{
	std::cout<<string;
	UIElement * newElement = new UILabel();
	newElement->text = string;
	newElement->sizeRatioY = 0.15f;
	assert(ui);
	newElement->textColor = ui->defaultTextColor;
	Graphics.QueueMessage(new GMAddUI(newElement, "Log"));
}

/// Adds target battler as a player! o-o
bool RuneBattleState::AddPlayerBattler(RuneBattler * playerBattler)
{
	assert(false);
	if (playerBattler->actions.Size() == 0)
		std::cout<<"\nWARNING: Player lacks any action categories. Will not be able to do anything.";
//	BattleMan.AddBattler(playerBattler, !playerBattler->isAI);
	return true;
}

void RuneBattleState::OpenCommandsMenu(RuneBattler * battler)
{
    std::cout<<"\n\nOPEN COMMANDS MENU";
    /// First clear the commands-menu of any existing items.
	Graphics.QueueMessage(new GMClearUI("CommandsMenu"));
	if (battler->actions.Size() == 0)
	{
		battler->UpdateActions();
	}
	if (battler->actionCategories.Size() == 0)
	{
		int sortingScheme = 1;
		battler->UpdateActionCategories(sortingScheme);
	}  
	activePlayerBattler = battler;
    for (int i = 0; i < battler->actionCategories.Size(); ++i)
	{
        RuneBattleActionCategory * rbac = battler->actionCategories[i];
        UIElement * categoryButton = new UIButton();
        categoryButton->sizeRatioY = 0.2f;
     //   std::cout<<"\nAdding category: "<<rbac;
     //   std::cout<<"\n.."<<rbac->name;
        categoryButton->text = rbac->name;
        categoryButton->textColor = Vector4f(1,1,1,1);
        categoryButton->textureSource = DEFAULT_UI_TEXTURE;
        /// E.g. Attack is displayed straight away as an initial command.
        if (rbac->isAction)
		{
            /// Set the appropriate action too.
			RuneBattleAction * rba = rbac->isAction;
            assert(rba->targetFilter != 0);
            categoryButton->activationMessage = "SetAction("+rba->name+")&&OpenTargetMenu("+rba->targetFilter+")";
        }
        /// E.g. Magic, opens a sub-menu.
        else {
            categoryButton->activationMessage = "OpenSubMenu("+rbac->name+")";
        }
        Graphics.QueueMessage(new GMAddUI(categoryButton, "CommandsMenu"));
//        std::cout<<"\nAdding category-button "<<categoryButton->text;
    }
	// Add extra commans for special battles?
	if (battleType == PRACTICE_DUMMY_BATTLE){
	    UIElement * categoryButton = new UIButton("Abort practice");
        categoryButton->sizeRatioY = 0.2f;
        categoryButton->textColor = Vector4f(1,1,1,1);
        categoryButton->textureSource = DEFAULT_UI_TEXTURE;
		categoryButton->activationMessage = "AbortPractice()";
        Graphics.QueueMessage(new GMAddUI(categoryButton, "CommandsMenu"));
  //      std::cout<<"\nAdding category-button "<<categoryButton->text;
	}

    /// Reveal the commands-menu when a party-member is ready for action, jRPG-style
    Graphics.QueueMessage(new GMPushUI("CommandsMenu", ui));
	commandsMenuOpen = true;
}


void RuneBattleState::OpenSubMenu(String whichMenu)
{
	/// Close and re-use the commands-menu as the user will most times not back away from it.
  //  Graphics.QueueMessage(new GMSetUIb("CommandsMenu", GMUI::VISIBILITY, false));
//	std::cout<<"\n\nOPEN Sub menu: "<<whichMenu;
    /// Return if it was a false call.
    commandsMenuOpen = true;
#define MENU	String("SubCommandMenu")
    /// First clear the commands-menu of any existing items.
    Graphics.QueueMessage(new GMClearUI(MENU));

	Vector4f textColor(1,1,1,1);

	if (activePlayerBattler->actions.Size() == 0)
		activePlayerBattler->UpdateActions();

	RuneBattleActionCategory * rbac = NULL;
	for (int i = 0; i < activePlayerBattler->actionCategories.Size(); ++i)
	{
		RuneBattleActionCategory * rbac2 = activePlayerBattler->actionCategories[i];
		if (rbac2->name == whichMenu)
			rbac = rbac2;
	}
	assert(rbac);
    for (int i = 0; i < rbac->actions.Size(); ++i)
	{
		RuneBattleAction * ba = rbac->actions[i];
		UIElement * actionButton = new UIButton();
        actionButton->sizeRatioY = 0.2f;
//        std::cout<<"\nAdding action: "<<ba->name;
        actionButton->text = ba->name;
        actionButton->textColor = textColor;
        actionButton->textureSource = DEFAULT_UI_TEXTURE;
        actionButton->activationMessage = "SetAction("+ba->name+")&&OpenTargetMenu("+ba->targetFilter+")";
        Graphics.QueueMessage(new GMAddUI(actionButton, MENU));
  //      std::cout<<"\nAdding action-button "<<actionButton->text;
    }

	/// Add a cancel-button.
	UIElement * cancelButton = new UIButton();
	cancelButton->name = "CancelActionSelect";
	cancelButton->sizeRatioY = 0.2f;
	cancelButton->text = "Back";
	cancelButton->textColor = textColor;
	cancelButton->textureSource = DEFAULT_UI_TEXTURE;
	cancelButton->activationMessage = "PopFromStack(" + MENU + ")";
	Graphics.QueueMessage(new GMAddUI(cancelButton, MENU));
//	std::cout<<"\nAdding cancel-button.";

    /// Reveal the commands-menu when a party-member is ready for action, jRPG-style
    Graphics.QueueMessage(new GMPushUI(MENU, ui));
}

/// Opens menu for selecting target for action.
void RuneBattleState::OpenTargetMenu()
{
    /// Fill the UI with whatever the new battler is capable of?
    /// Return if it was a false call.
    if (!battlers.Size())
        return;

    /// First clear the commands-menu of any existing items.
    Graphics.QueueMessage(new GMClearUI("TargetsMenu"));
	List<RuneBattler*> relevantBattlersToTarget;
	List<String> otherTargets;
    for (int i = 0; i < battlers.Size(); ++i)
	{
        RuneBattler * b = battlers[i];
		std::cout<<"\nBattler "<<i<<": "<<b->name<<" ai: "<<b->isAI;
		/// Skip dead targets.
        if (!b->IsARelevantTarget())
			continue;
        /// Remove battlers depending on what criteria they meet.
        switch(targetMode)
		{
			case TargetFilter::ALLY:
                /// Any non-AI
                if (!b->isAI)
					relevantBattlersToTarget.Add(b);
                break;
            case TargetFilter::ENEMY:
                /// Any AI
                if (b->isAI)
					relevantBattlersToTarget.Add(b);
                break;
			case TargetFilter::SELF:
				if (b == activePlayerBattler)
					relevantBattlersToTarget.Add(b);
				break;
			case TargetFilter::ALL:
			case TargetFilter::POINT:
				break;
            default:
				std::cout<<"unsupported target mode ("<<targetMode<<")for now, I'm afraid!";
                assert(false && "unsupported target mode for now, I'm afraid!");
        }
    }

	/// Non-battler specific targets.
	switch(targetMode)
	{	
		case TargetFilter::ALL:
			// Add 'ALL' as an option. o.o
			otherTargets.Add("All battlers");
			break;
		case TargetFilter::POINT:
			/// Disable UI-navigation so that the point may be navigated to!
			Input.ForceNavigateUI(false);
			otherTargets.Add("Point");
			break;
	}

	Vector4f textColor(1,1,1,1);

    /// Sort the targets too, perhaps?
    /// TODO: Add sorting if needed.
    for (int i = 0; i < relevantBattlersToTarget.Size(); ++i)
	{
        RuneBattler * b = relevantBattlersToTarget[i];
        UIElement * targetButton = new UIButton();
        targetButton->sizeRatioY = 0.2f;
        targetButton->text = b->name;
        targetButton->textColor = textColor;
		targetButton->onHover = "SetTargetBattlers(this)";
        targetButton->activationMessage = "SetTarget("+b->name+")&&ExecuteAction()";
        targetButton->textureSource = DEFAULT_UI_TEXTURE;
        Graphics.QueueMessage(new GMAddUI(targetButton, "TargetsMenu"));
        std::cout<<"\nAdding category-button "<<targetButton->text;
    }
	for (int i = 0; i < otherTargets.Size(); ++i)
	{
		String otherTarget = otherTargets[i];
        UIElement * targetButton = new UIButton();
        targetButton->sizeRatioY = 0.2f;
        targetButton->text = otherTarget;
        targetButton->textColor = textColor;
		targetButton->onHover = "SetTargetBattlers(this)";
        targetButton->activationMessage = "SetTarget("+otherTarget+")&&ExecuteAction()";
        targetButton->textureSource = DEFAULT_UI_TEXTURE;
        Graphics.QueueMessage(new GMAddUI(targetButton, "TargetsMenu"));
        std::cout<<"\nAdding category-button "<<targetButton->text;
	}

	/// Add a cancel-button.
	UIElement * cancelButton = new UIButton();
	cancelButton->name = "CancelTargetting";
	cancelButton->sizeRatioY = 0.2f;
	cancelButton->text = "Cancel";
	cancelButton->textColor = textColor;
	cancelButton->textureSource = DEFAULT_UI_TEXTURE;
	cancelButton->activationMessage = "PopFromStack(TargetsMenu)";
	Graphics.QueueMessage(new GMAddUI(cancelButton, "TargetsMenu"));
	std::cout<<"\nAdding cancel-button.";

    Graphics.QueueMessage(new GMPushUI("TargetsMenu", ui));
//	targetsMenuOpen = true;
}

void RuneBattleState::HideMenus()
{
	Graphics.QueueMessage(new GMPopUI("TargetsMenu", ui));
	Graphics.QueueMessage(new GMPopUI("SubCommandMenu", ui));
	Graphics.QueueMessage(new GMPopUI("CommandsMenu", ui, true));
	commandsMenuOpen = false;
}

/// Update the HP gui.
void RuneBattleState::UpdatePlayerHPUI()
{
	List<RuneBattler*> playerBattlers = GetPlayerBattlers();
	for (int i = 0; i < playerBattlers.Size(); ++i)
	{
		RuneBattler * playerBattler = playerBattlers[i];
		int hp = playerBattler->HP();
		int maxHP = playerBattler->MaxHP();
		float ratio = hp / (float) maxHP;
		String uiName = "Player"+STRINT(i)+"HP";
		Graphics.QueueMessage(new GMSetUIs(uiName, GMUI::TEXT, "HP: "+STRINT(hp)+"/"+STRINT(maxHP)));
		if (ratio < 0.25f)
			Graphics.QueueMessage(new GMSetUIv3f(uiName, GMUI::TEXT_COLOR, Vector3f(1.f, 0.2f, 0.2f)));
		else if (ratio < 0.5f)
			Graphics.QueueMessage(new GMSetUIv3f(uiName, GMUI::TEXT_COLOR, Vector3f(1.f, 1.f, 0.f)));
		else if (ratio < 0.75f)
			Graphics.QueueMessage(new GMSetUIv3f(uiName, GMUI::TEXT_COLOR, Vector3f(1.f, 1.f, 0.5f)));
		// Good hp.
		else 
			Graphics.QueueMessage(new GMSetUIv3f(uiName, GMUI::TEXT_COLOR, Vector3f(1.f, 1.f, 1.f)));


		/// MP!
		int mp = playerBattler->MP();
		int maxMP = playerBattler->MaxMP();
		ratio = mp / (float) maxMP;
		uiName = "Player"+STRINT(i)+"MP";
		Graphics.QueueMessage(new GMSetUIs(uiName, GMUI::TEXT, "MP: "+STRINT(mp)+"/"+STRINT(maxMP)));
		if (ratio < 0.25f)
			Graphics.QueueMessage(new GMSetUIv3f(uiName, GMUI::TEXT_COLOR, Vector3f(0.4f, 0.4f, 1.0f)));
		else if (ratio < 0.5f)
			Graphics.QueueMessage(new GMSetUIv3f(uiName, GMUI::TEXT_COLOR, Vector3f(0.f, 1.f, 1.f)));
		else if (ratio < 0.75f)
			Graphics.QueueMessage(new GMSetUIv3f(uiName, GMUI::TEXT_COLOR, Vector3f(0.5f, 1.f, 1.f)));
		// Good hp.
		else 
			Graphics.QueueMessage(new GMSetUIv3f(uiName, GMUI::TEXT_COLOR, Vector3f(1.f, 1.f, 1.f)));
	}
}




/// Creates bindings that are used for debugging purposes only
void RuneBattleState::CreateDefaultBindings(){

	std::cout<<"\nRacing::CreateDefaultBindings() called";

/// (int action, int * inputCombinationArray, int inputs, const char * name = NULL);
	/// Get pointer to this mapping
	InputMapping * mapping = &this->inputMapping;
	/// Create default bindings

	mapping->CreateBinding("ToggleBattleLog", KEY::L);

	// old shit
    mapping->CreateBinding(PRINT_BATTLER_ACTIONS, KEY::P, KEY::B, "Print battler actions");
	mapping->CreateBinding(PRINT_FRAME_TIME, KEY::CTRL, KEY::T);

};


/// Creates an appropriate name for target battler based on the amount of battlers of that type that has been spawned this fight.
String RuneBattleState::GetNewSpawnName(String byBaseName)
{
	for (int i = 0; i < spawnedThisFight.Size(); ++i)
	{
		SpawnHistory & hist = spawnedThisFight[i];
		if (hist.name == byBaseName)
		{
			++hist.number;
			return byBaseName + " " + String((char)('A' + hist.number));
		}
	}
	// Create a new spawn-history for this name.
	SpawnHistory newHist;
	newHist.name = byBaseName;
	newHist.number = 0;
	spawnedThisFight.Add(newHist);
	return byBaseName;
}
