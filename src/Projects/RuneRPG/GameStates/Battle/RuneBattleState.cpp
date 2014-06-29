// Emil Hedemalm
// 2013-06-28

#include "RuneBattleState.h"

#include "Battle/BattleManager.h"
#include "Battle/BattleAction.h"

#include "RuneRPG/RRPlayer.h"
#include "RuneRPG/Battle.h"

#include "Physics/PhysicsProperty.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/Messages/CollissionCallback.h"
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
#include "Graphics/Messages/GMParticleMessages.h"

#include "Message/FileEvent.h"
#include "Message/Message.h"

#include "OS/Sleep.h"
#include "Actions.h"

#include "UI/UserInterface.h"
#include "UI/UIButtons.h"
#include "UI/UIFileBrowser.h"
#include "UI/UIList.h"

#include "Entity/EntityProperty.h"
#include "Player/PlayerManager.h"
#include "Window/WindowManager.h"
#include "Window/WindowManager.h"

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

extern UserInterface * ui[GameStateID::MAX_GAME_STATES];

#define DEFAULT_UI_TEXTURE  "img/80gray50Alpha.png"

Random spawnRandom;

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
}

RuneBattleState::~RuneBattleState()
{
}

/// Creates the user interface for this state
void RuneBattleState::CreateUserInterface(){
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/RuneBattle.gui");
}

void RuneBattleState::OnEnter(GameState * previousState)
{
	paused = false;
	// Load initial texture and set it to render over everything else
	Graphics.QueueMessage(new GMSetOverlay("img/loadingData.png", 100));

	battleType = NORMAL_BATTLE;
	Sleep(100);

	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GMSetUI(ui));

    /// Hide the commands-menu until a party-member is ready for action, jRPG-style
    commandsMenuOpen = false;
    Graphics.QueueMessage(new GMSetUIb("CommandsMenu", GMUI::VISIBILITY, false));
    Graphics.QueueMessage(new GMSetUIb("TargetsMenu", GMUI::VISIBILITY, false));
	Graphics.QueueMessage(new GMSetUIs("Narrator", GMUI::TEXT, "An enemy appears!"));

	// Check for data (battlers)
	bool gotData = false;
	String requestedBattle = BattleMan.QueuedBattle();
	battleSource = requestedBattle;
	BattleMan.StartBattle();
	gotData = requestedBattle.Length() != 0;
	if (!gotData){
		requestedBattle = "data/Battles/testBattle.txt";
	}
	/// Load it!
	bool result = LoadBattle(requestedBattle);
	/// Failed to load battle, try a new one or return?
	if (!result){
		EndBattle();
		return;
	}
	if (requestedBattle.Contains("Practice")){
		battleType = PRACTICE_DUMMY_BATTLE;
	}

	/// Set editor selection as the renderable one!
	Graphics.selectionToRender = NULL;

	// And set it as active
	Graphics.cameraToTrack = camera;
//	Graphics.cameraToTrack->SetRatio(Graphics.width, Graphics.height);
//	Graphics.UpdateProjection();
//	Graphics.EnableAllDebugRenders(false);
	MainWindow()->MainViewport()->EnableAllDebugRenders(false);
	MainWindow()->renderFPS = true;

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));

	// And start tha music..
#ifdef USE_AUDIO
	AudioMan.Play(BGM, "2013-02-21 Impacto.ogg", true);
#endif
    /// Start the battle-timer
    timer.Start();
    lastTime = timer.GetMs();
	/// Notify the input-manager to use menu-navigation.
	Input.ForceNavigateUI(true);

	// Open window with extra UI for tweaking stuff in real-time on the battle?
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
	}
	battleTestWindow->Show();

	logVisibility = false;
	// Update it.
	ToggleLogVisibility();


	/// TODO: Adjust the viewport to cover only the active (non-UI) region? 
	List<Viewport*> vps;
	int viewports = 1;
	for (int i = 0; i < viewports; ++i)
	{
		float viewportSizeX = 1.f / viewports;
		Viewport * battleViewport = new Viewport();
		battleViewport->backgroundColor = Vector4f(0, 0 + 0.1f * i, 0, 1);
		battleViewport->SetRelative(Vector2f(0 + viewportSizeX * i, 0.2f), Vector2f(viewportSizeX, 0.8f));
		vps.Add(battleViewport);
	}
	Graphics.QueueMessage(new GMSetViewports(vps, MainWindow()));

	// Set graphics manager to render UI, and remove the overlay-texture.
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_OVERLAY_TEXTURE));
}


void RuneBattleState::OnExit(GameState *nextState)
{
	if (battleTestWindow)
		battleTestWindow->Hide();
	/// Notify the input-manager to stop force-using menu-navigation.
	Input.ForceNavigateUI(false);

	// Load initial texture and set it to render over everything else
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, TexMan.GetTexture("img/loadingData.png")));

	Sleep(100);
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	Graphics.cameraToTrack->entityToTrack = NULL;

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
	List<RuneBattler*> runeBattlers;
	List<Battler*> battlers = BattleMan.GetBattlers();
	for (int i = 0; i < battlers.Size(); ++i)
	{
		runeBattlers.Add((RuneBattler*) battlers[i]);
	}
	return runeBattlers;
}

/// Gets all active battlers by filtering string. This can be comma-separated names of other kinds of specifiers.
List<RuneBattler*> RuneBattleState::GetBattlers(String byFilter)
{
	List<RuneBattler*> runeBattlers;
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
    List<Battler*> battlers = BattleMan.GetBattlers();
    for (int i = 0; i < battlers.Size(); ++i){
        RuneBattler * b = (RuneBattler*)battlers[i];
		b->actionPoints = rand() % 25;
 //       b->initiative = 1500 + rand()%10000;
    }
}

/// !
List<RuneBattler*> GetPlayerBattlers()
{
	List<Battler*> battlers = BattleMan.GetPlayers();
	List<RuneBattler*> runeBattlers;
	for (int i = 0; i < battlers.Size(); ++i){
		Battler * b = battlers[i];
		runeBattlers.Add((RuneBattler*)b);
	}
	return runeBattlers;
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
		hp->text = "HP: "+String::ToString(rb->maxHP)+"/"+String::ToString(rb->hp);
		hp->sizeRatioX = 0.2f;
		ui->AddChild(hp);

		UIElement * mp = new UIElement();
		mp->textColor = textColor;
		mp->text = "MP: "+String::ToString(rb->maxMP)+"/"+String::ToString(rb->mp);
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
	/// Process key input for navigating the 3D - Space
	Sleep(10);
 
	if (paused)
		return;
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
				Entity * entity = MapMan.CreateEntity(ModelMan.GetModel("obj/3DPointer.obj"), TexMan.GetTexture("Grey"));
				pointerEntities.Add(entity);
				Physics.QueueMessage(new PMSetEntity(SET_SCALE, entity, 0.5f));
			} 
			RuneBattler * targetBattler = targetBattlers[i];
			Entity * battlerEntity = targetBattler->entity;
			if (!battlerEntity)
				continue;
			Entity * pointerEntity = pointerEntities[i];
			// Reveal the targetting arrows and move them info place.
			Physics.QueueMessage(new PMSetEntity(POSITION, pointerEntities[i], battlerEntity->position + Vector3f(0,battlerEntity->scale.y,0)));
			Graphics.QueueMessage(new GMSetEntityb(pointerEntity, VISIBILITY, true));
		}
	}
	else 
	{
		// Hide all entity-pointers!
		for (int i = 0; i < pointerEntities.Size(); ++i)
		{
			Entity * pointerEntity = pointerEntities[i];
			Graphics.QueueMessage(new GMSetEntityb(pointerEntity, VISIBILITY, false));
		}
	}

	
	/// Clear is for killing children, yo.
   // Graphics.QueueMessage(new GMClearUI("Log"));
    List<Battler*> battlers = BattleMan.GetBattlers();
    List<Battler*> players = BattleMan.GetPlayers();
    String str = "Combatants: "+String::ToString(battlers.Size())+" of which players: "+String::ToString(players.Size());

    for (int i = 0; i < battlers.Size(); ++i){
        RuneBattler * b = (RuneBattler*)battlers[i];
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

	BattleMan.Process(timeInMs);
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
				// Clear battlers.
				BattleMan.EndBattle();
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
			else if (string.Contains("OpenTargetMenu(")){
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
					RuneBattler * b = (RuneBattler *) BattleMan.GetBattlerByName(targets[i]);
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
				/// Fetch the action.
				if (!selectedBattleAction)
				{
					selectedBattleAction = new RuneBattleAction(*RBALib.GetBattleAction(action));
				}
				
				// Set targets
          		selectedBattleAction->targets = targetBattlers;
          		// Set subject
				selectedBattleAction->subjects.Add(activePlayerBattler);

				/// Add it to player queue so we know what's up.
				activePlayerBattler->actionsQueued.Add(selectedBattleAction);
				/// Set state to preparing the action
				activePlayerBattler->SetState(BattlerState::PREPARING_ACTION);
				
                /// Execute the action by queueing it up in the battle-manager.
				BattleMan.QueueAction(selectedBattleAction);
            
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
		}
	}
	GameState::ProcessMessage(message);
}

/// Loads battle from source~! Returns false upon failure.
bool RuneBattleState::LoadBattle(String fromSource)
{
	if (!fromSource.Contains("data/Battles/")){
		fromSource = "data/Battles/" + fromSource;
	}
	if (!fromSource.Contains(".battle"))
		fromSource += ".battle";
	/// Initialize test battle!
	BattleMan.NewBattle();
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
		AddPlayerBattler(newPlayer);
	}
	/// Add players based on the playerManager or RRPlayer thingy!
	List<Player*> players = PlayerMan.GetPlayers();
	for (int i = 0; i < players.Size(); ++i)
	{
		RRPlayer * rp = (RRPlayer*)players[i];
		RuneBattler * newPlayerBattler = new RuneBattler(*rp->Battler());
		AddPlayerBattler(newPlayerBattler);
	}
	
	// If still no players, create a test one?
	if (!BattleMan.GetPlayers().Size()){
		const RuneBattler * base = RuneBattlers.GetBattlerBySource("Player");
		assert(base);
		RuneBattler * battler = new RuneBattler(*base);
		battler->isAI = false;
		AddPlayerBattler(battler);
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
		std::cout<<"\nAdding RuneBattler enemy: "<<newEnemy->name;
		BattleMan.AddBattler(newEnemy);
	}
	/// Fetch all battlers.
	List<RuneBattler*> battlers = this->GetBattlers();
	for (int i = 0; i < battlers.Size(); ++i){
		battlers[i]->ResetStats();
	}
	
	/// For all battlers, count the names, and adjust them if there are ANY multiples at all out there.
	struct NameCount 
	{
		String name;
		int count;
		List<RuneBattler*> battlers;
	};
	List<NameCount> names;
	for (int i = 0; i < battlers.Size(); ++i)
	{
		RuneBattler * battler = battlers[i];
		String name = battler->name;
		bool occurredInNameCount = false;
		for (int j = 0; j < names.Size(); ++j)
		{
			NameCount & nc = names[j];
			if (nc.name == name)
			{
				occurredInNameCount = true;
				nc.battlers.Add(battler);
				nc.count++;
			}
		}
		if (!occurredInNameCount)
		{
			NameCount nc;
			nc.name = name;
			nc.count = 1;
			nc.battlers.Add(battler);
			names.Add(nc);
		}
	}
	/// Adjust names as needed.
	for (int i = 0; i < names.Size(); ++i)
	{
		NameCount & nc = names[i];
		if (nc.count <= 1)
			continue;
		for (int j = 0; j < nc.count; ++j)
		{
			RuneBattler * battler = nc.battlers[j];
			// Enumerate them .. A .. B .. C etc.
			battler->name = battler->name + " " + (char)('A' + j);
		}
	}



	BattleMan.Setup();
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
	MapMan.DeleteEntities();

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
		// Place them and their entities on the grid.
		Model * model = ModelMan.GetModel("obj/Sprite.obj");
		Texture * tex;
		if (battler->isEnemy)
			tex = TexMan.GetTexture("White");
		else 
			tex = TexMan.GetTexture("Red");
		Entity * entity = MapMan.CreateEntity(model, tex);
		battler->entity = entity;
		/// Give it animation set if applicable.
		if (battler->animationSet.Length())
			Graphics.QueueMessage(new GMSetEntity(entity, ANIMATION_SET, battler->animationSet));
		/// Scale it.
		Physics.QueueMessage(new PMSetEntity(SET_SCALE, entity, 2.f));
		/// Set relative position due to the animation's form.

	}
}

/// Place them on the grid.
void RuneBattleState::PlaceBattlers()
{
	List<RuneBattler*> battlers = this->GetBattlers();
	for (int i = 0; i < battlers.Size(); ++i)
	{
		RuneBattler * battler = battlers[i];
		// Place them and their entities on the grid.
		Entity * entity = battler->entity;
		Waypoint * wp;
		if (battler->isEnemy)
		{
			wp = GetFreeEnemyPosition();
		}
		else 
			wp = GetFreeAllyPosition();

		Vector3f position = wp->position;
		position.z = 1 - position.y * 0.1f; 
		Physics.QueueMessage(new PMSetEntity(POSITION, entity, position));
		/// Require depth-sorting so the alpha-blending will work.
		Graphics.QueueMessage(new GMSetEntityb(entity, REQUIRE_DEPTH_SORTING, true));
		/// Update render-offset while at it, so that all sprites assume 0.25f of the sprite to be the bottom/center?
		Graphics.QueueMessage(new GMSetEntityVec4f(entity, RENDER_OFFSET, Vector3f(0, 0.75f, 0)));
		wp->entity = entity;
	}
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
	camera->position = Vector3f(-mapSize.x * 0.5, -mapSize.y * 0.5 + 1.f, -20.f);
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
	BattleMan.EndBattle();
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
	newElement->textColor = ui->defaultTextColor;
	Graphics.QueueMessage(new GMAddUI(newElement, "Log"));
}

/// Adds target battler as a player! o-o
bool RuneBattleState::AddPlayerBattler(RuneBattler * playerBattler)
{
	if (playerBattler->actionCategories.Size() == 0)
		std::cout<<"\nWARNING: Player lacks any action categories. Will not be able to do anything.";
	BattleMan.AddBattler(playerBattler, !playerBattler->isAI);
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
    activePlayerBattler = battler;
    for (int i = 0; i < battler->actionCategories.Size(); ++i)
	{
        BattleActionCategory * bac = battler->actionCategories[i];
        UIElement * categoryButton = new UIButton();
        categoryButton->sizeRatioY = 0.2f;
        std::cout<<"\nAdding category: "<<bac;
        std::cout<<"\n.."<<bac->name;
        categoryButton->text = bac->name;
        categoryButton->textColor = Vector4f(1,1,1,1);
        categoryButton->textureSource = DEFAULT_UI_TEXTURE;
        /// E.g. Attack is displayed straight away as an initial command.
        if (bac->isAction)
		{
            /// Set the appropriate action too.
			assert(bac->actions.Size());
			BattleAction * ba = (BattleAction*) bac->actions[0];
            if (!ba){
                std::cout<<"\nERROR: No BattleAction with given category name "<<ba->name;
				delete categoryButton;
                continue;
            }
            assert(ba->targetFilter != 0);
            categoryButton->activationMessage = "SetAction("+ba->name+")&&OpenTargetMenu("+ba->targetFilter+")";
        }
        /// E.g. Magic, opens a sub-menu.
        else {
            categoryButton->activationMessage = "OpenSubMenu("+bac->name+")";
        }
        Graphics.QueueMessage(new GMAddUI(categoryButton, "CommandsMenu"));
        std::cout<<"\nAdding category-button "<<categoryButton->text;
    }
	// Add extra commans for special battles?
	if (battleType == PRACTICE_DUMMY_BATTLE){
	    UIElement * categoryButton = new UIButton("Abort practice");
        categoryButton->sizeRatioY = 0.2f;
        categoryButton->textColor = Vector4f(1,1,1,1);
        categoryButton->textureSource = DEFAULT_UI_TEXTURE;
		categoryButton->activationMessage = "AbortPractice()";
        Graphics.QueueMessage(new GMAddUI(categoryButton, "CommandsMenu"));
        std::cout<<"\nAdding category-button "<<categoryButton->text;
	}

    /// Reveal the commands-menu when a party-member is ready for action, jRPG-style
    Graphics.QueueMessage(new GMPushUI("CommandsMenu", ui));
	commandsMenuOpen = true;
}


void RuneBattleState::OpenSubMenu(String whichMenu)
{
	/// Close and re-use the commands-menu as the user will most times not back away from it.
  //  Graphics.QueueMessage(new GMSetUIb("CommandsMenu", GMUI::VISIBILITY, false));
	std::cout<<"\n\nOPEN Sub menu: "<<whichMenu;
    /// Return if it was a false call.
    commandsMenuOpen = true;
#define MENU	String("SubCommandMenu")
    /// First clear the commands-menu of any existing items.
    Graphics.QueueMessage(new GMClearUI(MENU));

	Vector4f textColor(1,1,1,1);

	if (activePlayerBattler->actions.Size() == 0)
		activePlayerBattler->UpdateActions();

	BattleActionCategory * bac = activePlayerBattler->GetCategory(whichMenu);
	assert(bac);
    for (int i = 0; i < bac->actions.Size(); ++i)
	{
		BattleAction * ba = bac->actions[i];
		UIElement * actionButton = new UIButton();
        actionButton->sizeRatioY = 0.2f;
        std::cout<<"\nAdding action: "<<ba->name;
        actionButton->text = ba->name;
        actionButton->textColor = textColor;
        actionButton->textureSource = DEFAULT_UI_TEXTURE;
        actionButton->activationMessage = "SetAction("+ba->name+")&&OpenTargetMenu("+ba->targetFilter+")";
        Graphics.QueueMessage(new GMAddUI(actionButton, MENU));
        std::cout<<"\nAdding action-button "<<actionButton->text;
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
	std::cout<<"\nAdding cancel-button.";

    /// Reveal the commands-menu when a party-member is ready for action, jRPG-style
    Graphics.QueueMessage(new GMPushUI(MENU, ui));
}

/// Opens menu for selecting target for action.
void RuneBattleState::OpenTargetMenu()
{
    /// Fill the UI with whatever the new battler is capable of?
    List<Battler*> battlers = BattleMan.GetBattlers();
    /// Return if it was a false call.
    if (!battlers.Size())
        return;
    assert(battlers.Size());
    /// First clear the commands-menu of any existing items.
    Graphics.QueueMessage(new GMClearUI("TargetsMenu"));
    for (int i = 0; i < battlers.Size(); ++i){
        Battler * b = battlers[i];
		bool remove = false;
		/// Skip dead targets.
        if (!b->IsARelevantTarget())
			remove = true;
        /// Remove battlers depending on what criteria they meet.
        switch(targetMode){
			case TargetFilter::ALLY:
                /// Any non-AI
                if (b->isAI)
                    remove = true;
                break;
            case TargetFilter::ENEMY:
                /// Any AI
                if (!b->isAI)
                    remove = true;
                break;
            default:
            std::cout<<"unsupported target mode ("<<targetMode<<")for now, I'm afraid!";
                assert(false && "unsupported target mode for now, I'm afraid!");
    /*
namespace BattleTargets {
    enum {
        NULL_TARGET,
        PLAYER,
        ENEMY,
        RANDOM,
        ALL_PLAYERS,
        ALL_ENEMIES,
        ALL,
        MAX_BATTLE_TARGETS
    };
};*/
        }
        if (remove){
            battlers.Remove(b);
            --i;
            continue;
        }
    }

	Vector4f textColor(1,1,1,1);

    /// Sort the targets too, perhaps?
    /// TODO: Add sorting if needed.
    for (int i = 0; i < battlers.Size(); ++i)
	{
        Battler * b = battlers[i];
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
	for (int i = 0; i < playerBattlers.Size(); ++i){
		RuneBattler * playerBattler = playerBattlers[i];
		Graphics.QueueMessage(new GMSetUIs("Player"+STRINT(i)+"HP", GMUI::TEXT, "HP: "+STRINT(playerBattler->hp)+"/"+STRINT(playerBattler->maxHP)));
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
