// Emil Hedemalm
// 2013-06-17

#ifndef RR_BATTLE_STATE_H
#define RR_BATTLE_STATE_H

#include "Game/GameConstants.h"
#include "../RRGameState.h"
#include "AppStates/AppStates.h"
#include "Selection.h"

class Camera;
class RuneBattler;
class Window;
class NavMesh;
class TileGrid2D;
class Waypoint;

/// cba place it elsewhere
extern List<String> queuedBattles;

class RuneBattleState : public RRGameState 
{
public:
	RuneBattleState();
	virtual ~RuneBattleState();
	/// Singleton it for access to all functions.
	static RuneBattleState * state;

	void OnEnter(AppState * previousState);
	/// Returns true upon success.
	bool StartQueuedBattle();

	void Process(int timeInMs);
	void OnExit(AppState * nextState);

	/// For rendering cursors or something?
	virtual void Render(GraphicsState & graphicsState);

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
	/// CBA to keep track of the BattleManager anymore.
	List<RuneBattler*> battlers;


	/// Gets target battlers
	RuneBattler * GetBattler(String byName);
	/// Gets all active battlers
	List<RuneBattler*> GetBattlers();
	List<RuneBattler*> GetPlayerBattlers();
	/// Gets all active battlers by filtering string. This can be comma-separated names of other kinds of specifiers.
	List<RuneBattler*> GetBattlers(String byFilter);

	/// Toggle visibility of the battle log (of previous executed actions)
	void ToggleLogVisibility(bool * newState = NULL);
	bool logVisibility;

	Window * battleTestWindow;

	// Randomize starting initiative.
	void ResetInitiative();
	/// Update UI accordingly.
	void CreatePartyUI();


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
    void OpenCommandsMenu(RuneBattler * forBattler);
	void OpenSubMenu(String whichMenu);
	/// Opens menu for selecting target for action.
    void OpenTargetMenu();
	void HideMenus();
	
	/// Chosen action. Name of it.
	String action;
	List<String> targets;

	/// Loads battle from source~! Returns false upon failure.
	bool LoadBattle(String fromSource);
	/// Loads the "map" to be used, creates the grid etc.
	void SetupBattleMap();
	/// Using known battle map, setup navmesh to use.
	void CreateNavMesh();
	/// Setup lighting to fit the location/time
	void SetupLighting();
	/// Create entities! 
	void CreateBattlerEntities();
	/// Place them on the grid.
	void PlaceBattlers();
	/// Setup camera
	void SetupCamera();

	/// Overall map and grid size. Used for camera placement. Note that the grid is not necessarily the same siz
	Vector2i mapSize;
	// Navigation and spawning.	Returns random free positions.
	Waypoint * GetFreeEnemyPosition();
	Waypoint * GetFreeAllyPosition();


	/// Grid.
	TileGrid2D * battleGrid;
	NavMesh * navMesh;

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
    List<RuneBattler *> targetBattlers;
	RuneBattler * activePlayerBattler;

	/** List of all active players! (human-controlled, either locally or via network).
		Index 0 will always be the primary local player.
		Refer to the MultiplayerProperty for organizing entities with different players (name/IP/etc).
	*/
	List<Entity*> playerEntities;
	/// p-p Re-created each new battle?
	List<Entity*> pointerEntities;

	/// The main camera
	Camera * camera;

	/// Default true when entering the state.
	bool paused;
};

#endif
