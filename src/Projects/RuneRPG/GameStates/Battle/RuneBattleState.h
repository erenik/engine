// Emil Hedemalm
// 2013-06-17

#ifndef RR_BATTLE_STATE_H
#define RR_BATTLE_STATE_H

#include "Game/GameConstants.h"
#include "../RRGameState.h"
#include "GameStates/GameStates.h"
#include "Selection.h"

class Camera;
class RuneBattler;

class RuneBattleState : public RRGameState {
public:
	RuneBattleState();
	virtual ~RuneBattleState();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);
	/// Input functions for the various states
	void MouseClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	void MouseRightClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	virtual void MouseMove(int x, int y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
	void MouseWheel(float delta);
	void CreateDefaultBindings();
	void CreateUserInterface();
	void InputProcessor(int action, int inputDevice = 0);
	void ProcessMessage(Message * message);

private:

	// Randomize starting initiative.
	void ResetInitiative();
	/// Update UI accordingly.
	void OnBeginBattle();


	/// For special battles
	enum {
		NORMAL_BATTLE,
		PRACTICE_DUMMY_BATTLE,
	};
	int battleType;

	/// Returns idle player-controlled battler if available. NULL if not.
	RuneBattler * GetIdlePlayer();
	/// Adds target battler as a player! o-o
	bool AddPlayerBattler(RuneBattler * playerBattler);
    /// Timing...!
    Timer timer;
    int lastTime;

	/// Source file of the battle we are currently fighting.
	String battleSource;

    /// Stuff.
    class RuneBattleAction * selectedBattleAction;
    int targetMode;
    void OpenCommandsMenu();
	void OpenSubMenu(String whichMenu);
	/// Opens menu for selecting target for action.
    void OpenTargetMenu();
	void HideMenus();

	/// Loads battle from source~! Returns false upon failure.
	bool LoadBattle(String fromSource);
	/// Pokes the BattleManager to end the battle and then queues state-change to whereever we came from
	void EndBattle();

	/// Logs by printing both to std::cout and a visible graphical log (that can be toggled in-game).
	void Log(String string);

	// UI Updaters
	void UpdatePlayerHPUI();

	/// Amount of requested players to create upon entry.
	int requestedPlayers;

    /// Sup. For basic interaction.
	bool commandsMenuOpen;

    /// For ze targetting!
    List<Entity*> activeTargets;
	RuneBattler * activePlayer;

	/** List of all active players! (human-controlled, either locally or via network).
		Index 0 will always be the primary local player.
		Refer to the MultiplayerProperty for organizing entities with different players (name/IP/etc).
	*/
	List<Entity*> playerEntities;

	/// The cameras for the 4 primary players and spectators...!
	Camera * camera;
	/// Actively manipulated entities
	Selection editorSelection;
};

#endif
