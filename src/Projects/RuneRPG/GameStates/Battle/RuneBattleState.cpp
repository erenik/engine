// Emil Hedemalm
// 2013-06-28

#include "RuneBattleState.h"

#include "OS/Sleep.h"
#include "Actions.h"
#include "Message/Message.h"
#include "../UI/UserInterface.h"
#include "UI/UIButtons.h"
#include "../Graphics/Messages/GMUI.h"
#include "../Physics/PhysicsProperty.h"
#include "Graphics/Render/RenderViewport.h"
#include "Physics/Messages/CollissionCallback.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "EntityStates/StateProperty.h"
#include "Graphics/Messages/GMParticleMessages.h"
#include <Util.h>
#include <iomanip>
#include "Battle/BattleManager.h"
#include "RuneRPG/Battle/RuneBattler.h"
#include "RuneRPG/Battle/RuneBattlerManager.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/Messages/GMSet.h"
#include "Battle/BattleAction.h"
#include "RuneRPG/Battle/RuneBattleAction.h"
#include "UI/UIList.h"
#include "../RuneGameStatesEnum.h"
#include "RuneRPG/RRPlayer.h"
#include "RuneRPG/Battle/RuneSpell.h"
#include <ctime>


#include "Graphics/GraphicsManager.h"
#include "TextureManager.h"
#include "Maps/MapManager.h"
#include "StateManager.h"
#include "Player/PlayerManager.h"

extern UserInterface * ui[GameStateID::MAX_GAME_STATES];

#define DEFAULT_UI_TEXTURE  "img/80gray50Alpha.png"

/// Stringify Integer
#define STRINT(i) String::ToString(i)

RuneBattleState::RuneBattleState()
{
	id = RUNE_GAME_STATE_BATTLE_STATE;
	camera = new Camera();
}

RuneBattleState::~RuneBattleState(){
	delete camera;
	camera = NULL;
}

/// Creates the user interface for this state
void RuneBattleState::CreateUserInterface(){
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/Battle.gui");
}

void RuneBattleState::OnEnter(GameState * previousState)
{
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

	// Randomize starting initiative.
	ResetInitiative();

	/// Update UI accordingly.
	OnBeginBattle();

	/// Toggle debug renders
#ifdef _DEBUG
	Graphics.EnableAllDebugRenders();
	Graphics.renderAI = false;
#else
	Graphics.EnableAllDebugRenders(false);
#endif

	/// Set editor selection as the renderable one!
	Graphics.selectionToRender = NULL;

	// And set it as active
	Graphics.cameraToTrack = camera;
	Graphics.cameraToTrack->SetRatio(Graphics.width, Graphics.height);
	Graphics.UpdateProjection();
	Graphics.EnableAllDebugRenders(false);
	Graphics.renderFPS = true;

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
}

void RuneBattleState::ResetInitiative()
{
    /// Set all battlers inititive
    List<Battler*> battlers = BattleMan.GetBattlers();
    for (int i = 0; i < battlers.Size(); ++i){
        RuneBattler * b = (RuneBattler*)battlers[i];
        b->initiative = 1500 + rand()%10000;
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
void RuneBattleState::OnBeginBattle()
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

#define SET_DEFAULT_TEXT_COLOR(hp); hp->textColor = Vector4f(1,1,1,1);

		/// Add a name to it too.
		UIElement * name = new UIElement();
		name->text = rb->name;
		name->textColor = Vector4f(1,1,1,1);
		SET_DEFAULT_TEXT_COLOR(name);
		name->sizeRatioX = 0.4f;
		ui->AddChild(name);

		/// Add HP/MP too
		UIElement * hp = new UIElement();
		hp->name = "Player" + STRINT(i) + "HP";
		SET_DEFAULT_TEXT_COLOR(hp);
		hp->text = "HP: "+String::ToString(rb->maxHP)+"/"+String::ToString(rb->hp);
		hp->sizeRatioX = 0.3f;
		ui->AddChild(hp);

		UIElement * mp = new UIElement();
		SET_DEFAULT_TEXT_COLOR(mp);
		mp->text = "MP: "+String::ToString(rb->maxMP)+"/"+String::ToString(rb->mp);
		mp->sizeRatioX = 0.3f;
		ui->AddChild(mp);

		Graphics.QueueMessage(new GMAddUI(ui, "PartyStatus"));
	}
}


void RuneBattleState::OnExit(GameState *nextState){
	/// Notify the input-manager to stop force-using menu-navigation.
	Input.ForceNavigateUI(false);

	// Load initial texture and set it to render over everything else
	Graphics.QueueMessage(new GMSet(OVERLAY_TEXTURE, TexMan.GetTexture("img/loadingData.png")));
	Graphics.QueueMessage(new GMSetViewports(NULL));

	Sleep(100);
	// Begin loading textures here for the UI
	Graphics.QueueMessage(new GraphicsMessage(GM_CLEAR_UI));
	Graphics.cameraToTrack->entityToTrack = NULL;

	std::cout<<"\nLeaving RuneBattleState.";
	// Set graphics manager to render UI, and remove the overlay-texture.
//	Graphics.SetOverlayTexture(NULL);
}





/// Returns idle player-controlled battler if available. NULL if not.
RuneBattler * RuneBattleState::GetIdlePlayer()
{
	List<RuneBattler*> playerBattlers = GetPlayerBattlers();
	for (int i = 0; i < playerBattlers.Size(); ++i){
		RuneBattler * playerBattler = playerBattlers[i];
		if (playerBattler->isAI)
			continue;
		if (playerBattler->IsIdle())
			return playerBattler;
	}
	return NULL;
}

void RuneBattleState::Process(float time){
	/// Process key input for navigating the 3D - Space
	Sleep(10);
    /// Clear is for killing children, yo.
   // Graphics.QueueMessage(new GMClearUI("Log"));
    List<Battler*> battlers = BattleMan.GetBattlers();
    List<Battler*> players = BattleMan.GetPlayers();
    String str = "Combatants: "+String::ToString(battlers.Size())+" of which players: "+String::ToString(players.Size());

    for (int i = 0; i < battlers.Size(); ++i){
        RuneBattler * b = (RuneBattler*)battlers[i];
        str += "\nBattler"+STRINT(i)+": "+b->name+" Initiative: " + STRINT(b->initiative);
    }

    Graphics.QueueMessage(new GMSetUIs("Log", GMUI::TEXT, str));


	/// If we got someone with initiative and the menu is down for some reason, open it again?
	if (GetIdlePlayer() && !commandsMenuOpen){
		/// Open menu..?
	}

    int newTime = timer.GetMs();
    int timeDiff = newTime - lastTime;
    lastTime = newTime;
	BattleMan.Process(timeDiff);
#ifdef USE_AUDIO
	AudioMan.Update();
#endif
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

void RuneBattleState::MouseWheel(float delta){
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
	switch(message->type){
		case MessageType::STRING: {
			String string = message->msg;
			String msg = message->msg;
			string.SetComparisonMode(String::NOT_CASE_SENSITIVE);
			if (string == "begin_commandline"){
				Input.EnterTextInputMode("INTERPRET_CONSOLE_COMMAND");
			}
			else if (string == "interpret_console_Command"){
				StateMan.ActiveState()->InputProcessor(INTERPRET_CONSOLE_COMMAND);
				return;
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
			else if (string == "ReloadBattle()" ||
				string == "PauseBattle()" ||
				string == "OpenBattlesList" ||
				string == "ReloadBattlers" ||
				string == "ReloadBattleActions" ||
				string == "ToggleLog")
			{
				Log("Not implemented yet, sorry!");	
			}
			else if (string == "BattlerReady"){
			    /// If the commands-menu is already open, don't do anything.
			    /// If it isn't, get the ready battler, open the menu and fill it with stuff!
                if (!commandsMenuOpen){
                    OpenCommandsMenu();
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
			else if (string.Contains("SetAction(")){
                BattleAction * ba;
                List<String> tokens = string.Tokenize("()");
                if (tokens.Size() >= 2){
                    ba = BALib.Get(tokens[1]);
                    if (ba){
                        selectedBattleAction = new RuneBattleAction(*ba);
                    }
                }
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
				/*
                activeTargets.Clear();
                String reqTargets = string.Tokenize("()")[1];
                List<Battler*> b = BattleMan.Get(reqTargets);
                for (int i = 0; i < b.Size(); ++i){
					activeTargets.Add((RuneBattler*)b[i]);
                }*/
			}
			else if (string.Contains("OnBattlerIncapacitated"))
			{
				/*
				/// Check if we won or died.
				bool playersDead = true, enemiesDead = true;
				for (int i = 0; i < playerBattlers.Size(); ++i)
				{
					if (playerBattlers[i]->state != INCAPACITATED)
						playersDead = false;
				}
				for (int i = 0; i < enemyBattlers.Size(); ++i){
					if (enemyBattlers[i]->state != INCAPACITATED)
						enemiesDead = false;
				}
				/// Fight is over. Run OnEnd stuff from the battle-template?
				if (playersDead || enemiesDead){
					EndBattle();
				}*/
				return;
			}
			else if (string == "ExecuteAction()"){
				
                List<Battler*> targets;
                for (int i = 0; i < activeTargets.Size(); ++i){
                    targets.Add((Battler*)activeTargets[i]);
                }
				if (selectedBattleAction){
					selectedBattleAction->subjects.Add(activePlayer);
//					selectedBattleAction->targets = activeTargets;
				}

                if (activeTargets.Size())
                    BattleMan.QueueAction(selectedBattleAction);
               // activePlayer->

                /// Execute the action by queueing it up in the battle-manager.

                /// Close all menus
                HideMenus();

                /// Reset booleans for that too.
                targetMode = 0;
                selectedBattleAction = NULL;
                commandsMenuOpen = false;
                activePlayer = NULL;
				return;
			}
		}
	}
	GameState::ProcessMessage(message);
}

/// Loads battle from source~! Returns false upon failure.
bool RuneBattleState::LoadBattle(String fromSource)
{
	if (!fromSource.Contains("data/")){
		fromSource = "data/Battles/" + fromSource;
	}
	if (!fromSource.Contains(".battle"))
		fromSource += ".battle";
	List<RuneBattler*> battlers;
	/// Initialize test battle!
	BattleMan.NewBattle();
	Battle b = RuneBattlers.GetBattleBySource(fromSource);
	if (!b.enemyNames.Size()){
		return false;
	}
	assert(b.enemyNames.Size() && "unable to laod test Battle");
	if (b.enemyNames.Size() == NULL)
		return false;
	for (int i = 0; i < b.playerNames.Size(); ++i){
		RuneBattler rb = RuneBattlers.GetBattlerBySource(b.playerNames[1]);
		std::cout<<"\nAdding RuneBattler player: "<<rb.name;
		RuneBattler * newPlayer = new RuneBattler(rb);
		AddPlayerBattler(newPlayer);
	}
	/// Add players based on the playerManager or RRPlayer thingy!
	List<Player*> players = PlayerMan.GetPlayers();
	for (int i = 0; i < players.Size(); ++i){
		RRPlayer * rp = (RRPlayer*)players[i];
		RuneBattler * newPlayerBattler = new RuneBattler(*rp->Battler());
		AddPlayerBattler(newPlayerBattler);
	}
	
	// If still no players, create a test one?
	if (!BattleMan.GetPlayers().Size()){
		RuneBattler * battler = new RuneBattler(RuneBattlers.GetBattlerBySource("Player"));
		battler->isAI = false;
		AddPlayerBattler(battler);
	}
	// Add enemies accordingly.
	for (int i = 0; i < b.enemyNames.Size(); ++i){
		String source = b.enemyNames[i];
		RuneBattler rb = RuneBattlers.GetBattlerBySource(source);
		RuneBattler * newEnemy =  new RuneBattler(rb);
		std::cout<<"\nAdding RuneBattler enemy: "<<rb.name;
		BattleMan.AddBattler(newEnemy);
	}
	for (int i = 0; i < battlers.Size(); ++i){
		battlers[i]->ResetStats();
	}
	BattleMan.Setup();
	return true;
}

/// Pokes the BattleManager to end the battle and then queues state-change to whereever we came from
void RuneBattleState::EndBattle(){
	std::cout<<"\n==========================================================";
	std::cout<<"\nBATTLE IS OVER!";
	std::cout<<"\n==========================================================";
	/// Hide all UI
	HideMenus();
	BattleMan.EndBattle();
	/// Wait for like.. 2 seconds?
	Sleep(2000);
	/// Go to previous state for now?
	StateMan.QueueState(StateMan.PreviousState());
}

/// Logs by printing both to std::cout and a visible graphical log (that can be toggled in-game).
void RuneBattleState::Log(String string){
	std::cout<<string;
	UIElement * newElement = new UILabel();
	newElement->text = string;
	newElement->sizeRatioY = 0.15f;
	newElement->textColor = ui->defaultTextColor;
	Graphics.QueueMessage(new GMAddUI(newElement, "BattleLog"));
}

/// Adds target battler as a player! o-o
bool RuneBattleState::AddPlayerBattler(RuneBattler * playerBattler)
{
	if (playerBattler->actionCategories.Size() == 0)
		std::cout<<"\nWARNING: Player lacks any action categories. Will not be able to do anything.";
	BattleMan.AddBattler(playerBattler, !playerBattler->isAI);
	return true;
}

void RuneBattleState::OpenCommandsMenu(){
    std::cout<<"\n\nOPEN COMMANDS MENU";
    /// Fill the UI with whatever the new battler is capable of?
    List<Battler*> battlers = BattleMan.GetIdlePlayerBattlers();
    /// Return if it was a false call.
    if (!battlers.Size())
        return;

    assert(battlers.Size());
    std::cout<<"\n\nOPEN COMMANDS MENU?!?!";
    commandsMenuOpen = true;


    /// First clear the commands-menu of any existing items.
	Graphics.QueueMessage(new GMClearUI("CommandsMenu"));

    Battler * battler = battlers[0];

    activePlayer = (RuneBattler*)battler;
    for (int i = 0; i < battler->actionCategories.Size(); ++i){
        BattleActionCategory * bac = battler->actionCategories[i];
        UIElement * categoryButton = new UIButton();
        categoryButton->sizeRatioY = 0.2f;
        std::cout<<"\nAdding category: "<<bac;
        std::cout<<"\n.."<<bac->name;
        categoryButton->text = bac->name;
        categoryButton->textColor = Vector4f(1,1,1,1);
        categoryButton->textureSource = DEFAULT_UI_TEXTURE;
        /// E.g. Attack is displayed straight away as an initial command.
        if (bac->isAction){
            /// Set the appropriate action too.
            BattleAction * ba = BALib.Get(bac->name);
            if (!ba){
                std::cout<<"\nERROR: No BattleAction with given category name "<<bac->name;
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
}


void RuneBattleState::OpenSubMenu(String whichMenu){

	/// Close and re-use the commands-menu as the user will most times not back away from it.
  //  Graphics.QueueMessage(new GMSetUIb("CommandsMenu", GMUI::VISIBILITY, false));

	std::cout<<"\n\nOPEN Sub menu: "<<whichMenu;
    /// Fill the UI with whatever the new battler is capable of?
    List<Battler*> battlers = BattleMan.GetIdlePlayerBattlers();
    /// Return if it was a false call.
    if (!battlers.Size())
        return;
    commandsMenuOpen = true;
#define MENU	String("SubCommandMenu")
    /// First clear the commands-menu of any existing items.
    Graphics.QueueMessage(new GMClearUI(MENU));

    Battler * battler = battlers[0];
	BattleActionCategory * bac = BALib.GetCategory(whichMenu);
    activePlayer = (RuneBattler*)battler;
    assert(battler->actions.Size() && "Battler has no actions at all. WTH?!");
    assert(battler->actionCategories.Size() && "Battler has no action categories.. fix this, yo?");
    for (int i = 0; i < battler->actions.Size(); ++i){
		BattleAction * ba = battler->actions[i];
		if (ba->category != bac)
			continue;
        UIElement * actionButton = new UIButton();
        actionButton->sizeRatioY = 0.2f;
        std::cout<<"\nAdding action: "<<ba->name;
        actionButton->text = ba->name;
        actionButton->textColor = Vector4f(1,1,1,1);
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
	SET_DEFAULT_TEXT_COLOR(cancelButton);
	cancelButton->textureSource = DEFAULT_UI_TEXTURE;
	cancelButton->activationMessage = "PopFromStack(" + MENU + ")";
	Graphics.QueueMessage(new GMAddUI(cancelButton, MENU));
	std::cout<<"\nAdding cancel-button.";

    /// Reveal the commands-menu when a party-member is ready for action, jRPG-style
    Graphics.QueueMessage(new GMPushUI(MENU, ui));
}

/// Opens menu for selecting target for action.
void RuneBattleState::OpenTargetMenu(){
    commandsMenuOpen = true;
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

    /// Sort the targets too, perhaps?
    /// TODO: Add sorting if needed.
    for (int i = 0; i < battlers.Size(); ++i){
        Battler * b = battlers[i];
        UIElement * targetButton = new UIButton();
        targetButton->sizeRatioY = 0.2f;
        targetButton->text = b->name;
        targetButton->textColor = Vector4f(1,1,1,1);
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
	SET_DEFAULT_TEXT_COLOR(cancelButton);
	cancelButton->textureSource = DEFAULT_UI_TEXTURE;
	cancelButton->activationMessage = "PopFromStack(TargetsMenu)";
	Graphics.QueueMessage(new GMAddUI(cancelButton, "TargetsMenu"));
	std::cout<<"\nAdding cancel-button.";

    Graphics.QueueMessage(new GMPushUI("TargetsMenu", ui));
}

void RuneBattleState::HideMenus()
{
	Graphics.QueueMessage(new GMPopUI("TargetsMenu", ui));
	Graphics.QueueMessage(new GMPopUI("SubCommandMenu", ui));
	Graphics.QueueMessage(new GMPopUI("CommandsMenu", ui, true));
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

