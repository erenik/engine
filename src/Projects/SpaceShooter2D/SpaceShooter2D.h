/// Emil Hedemalm
/// 2015-01-20
/// Space shooter.
/// For the Karl-Emil SpaceShooter project, mainly 2014-2015/

#include "AppStates/AppState.h"

#include "Base/Ship.h"
#include "Base/Weapon.h"
#include "Properties/ShipProperty.h"
#include "Level/Level.h"
#include "Physics/SSIntegrator.h"

#include "Entity/EntityManager.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Model/Model.h"

#include "Texture.h"
#include "TextureManager.h"

#include "Model/ModelManager.h"

#include "StateManager.h"

#include "UI/UserInterface.h"

#include "String/StringUtil.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"

#include "File/File.h"
#include "File/FileUtil.h"

#include "Maps/MapManager.h"

#include "Network/NetworkManager.h"

#include "Script/Script.h"
#include "Script/ScriptManager.h"

#include "Audio/AudioManager.h"
#include "Audio/Messages/AudioMessage.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMParticles.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/Particles/Sparks.h"
#include "Graphics/Particles/Stars.h"
#include "Graphics/Particles/SparksEmitter.h"

#include "Physics/SpaceShooterCD.h"
#include "Physics/SpaceShooterCR.h"
#include "Game/GameVariableManager.h"

#include "SpaceShooter2D/Properties/ShipProperty.h"
#include "SpaceShooter2D/Properties/ProjectileProperty.h"

// Collision categories.
#define CC_PLAYER		1 
#define CC_ENEMY		2
#define CC_PLAYER_PROJ	4
#define CC_ENEMY_PROJ	8
#define CC_PLAYER_EXPL	16
#define CC_ENEMY_EXPL	32

#define CC_ALL_PLAYER (CC_PLAYER | CC_PLAYER_PROJ | CC_PLAYER_EXPL)
#define CC_ALL_ENEMY (CC_ENEMY | CC_ENEMY_PROJ | CC_ENEMY_EXPL)

enum {
	SHIP_PROP,
	PROJ_PROP,
	EXPL_PROP,
};

class SpaceShooterCR;
class SpaceShooterCD;
class GameVariable;

/// Time in current level, from 0 when starting. Measured in milliseconds.
extern Time levelTime, flyTime;
// extern int64 nowMs;
extern int timeElapsedMs;
/// Particle system for sparks/explosion-ish effects.
extern Sparks * sparks;

/// These will hopefully always be in AABB axes.
extern Vector3f frustumMin, frustumMax;

extern Ship * playerShip;
/// The level entity, around which the playing field and camera are based upon.
extern Entity * levelEntity;
extern Vector2f playingFieldSize;
extern Vector2f playingFieldHalfSize;
extern float playingFieldPadding;
/// All ships, including player.
extern List<Entity*> shipEntities;
extern List<Entity*> projectileEntities;
extern String playerName;
extern bool paused;
extern String onDeath; // What happens when the player dies?

// Macros
#define leftEdge (levelEntity->worldPosition.x - playingFieldHalfSize.x)
#define rightEdge (levelEntity->worldPosition.x + playingFieldHalfSize.x)

Vector2i WeaponTypeLevelFromString(String str); // For shop/UI-interaction.
String WeaponTypeLevelToString(int type, int level);
int DiffCost(String toUpgrade);

/// In WeaponScriptEditing.cpp
void ProcessMessageWSS(Message * message);

class SpaceShooter2D : public AppState 
{
public:
	SpaceShooter2D();
	virtual ~SpaceShooter2D();
	/// Function when entering this state, providing a pointer to the previous StateMan.
	void OnEnter(AppState * previousState);
	/// Main processing function, using provided time since last frame.
	void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	void OnExit(AppState * nextState);
	
	/// Searches among actively spawned ships.
	Ship * GetShip(Entity * forEntity);
	Ship * GetShipByID(int id);

	/// Creates the user interface for this state
	virtual void CreateUserInterface();

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);
	/// Callback from the Input-manager, query it for additional information as needed.
	virtual void KeyPressed(int keyCode, bool downBefore);

	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();

	/// Yerrr.
	virtual void UpdateRenderArrows();
	/// Called from the render-thread for every viewport/AppWindow, after the main rendering-pipeline has done its job.
	virtual void Render(GraphicsState * graphicsState);

	/// Loads .csv's and data so as to know default gear and ships.
	void LoadShipData();
	/// UI stuffs. All implemented in UIHandling.cpp
	void UpdateUI();
	void UpdateGearList();
	/// Update UI parts
	void UpdateHUDGearedWeapons();
	void UpdateUIPlayerHP(bool force);
	void UpdateUIPlayerShield(bool force);
	void UpdateHUDSkill();
	void UpdateCooldowns();
	void UpdateUpgradesLists();
	void UpdateUpgradeStatesInList(); // Updates colors n stuff based on level
	void UpdateHoverUpgrade(String upgrade, bool force = false);
	void UpdateActiveUpgrade(String upgrade);
	void UpdateUpgradesMoney();

	/// Shop handling...
	void BuySellToUpgrade(String upgrade);

	void OpenJumpDialog();
	/// Update ui
	void OnScoreUpdated();
	void ShowLevelStats();
	void LoadDefaultName();
	/// o.o
	Entity * OnShipDestroyed(Ship * ship);


	String GetLevelVarName(String level, String name);
	/// Level score.
	GameVariable * LevelScore(String level = "current");
	/// Level kills. If -1, returns current.
	GameVariable * LevelKills(String level = "current");
	/// Level max kills possible.
	GameVariable * LevelPossibleKills(String level = "current");
	/// Resets all the above.
	void ResetLevelStats();
// private:

	/// Starts a new game. Calls LoadLevel
	void NewGame();
	void NewPlayer();
	void Pause();
	void Resume();
	void TogglePause();
	
	/// Loads target level. The source and separate .txt description have the same name, just different file-endings, e.g. "Level 1.png" and "Level 1.txt"
	void LoadLevel(String levelSource = "CurrentStageLevel");
	void GameOver();
	void OnLevelCleared();

	/// Opens main menu.
	void OpenMainMenu();
	/// Where the ship will be re-fitted and new gear bought.
	void EnterShipWorkshop();
	/// Returns a list of save-files.
	void OpenLoadScreen();
	// Bring up the in-game menu.
	void OpenInGameMenu();
	
	
	/// Saves current progress.
	bool SaveGame();
	/// Loads progress from target save.
	bool LoadGame(String save);

	void UpdatePlayerVelocity();
	void ResetCamera();	

	enum {
		MAIN_MENU,
		EDITING_OPTIONS,
		NEW_GAME,
		IN_LOBBY,
		IN_HANGAR,
		IN_WORKSHOP,
		EDIT_WEAPON_SWITCH_SCRIPTS,
		BUYING_GEAR,
		LOAD_SAVES,
		PLAYING_LEVEL,
		GAME_OVER,
		LEVEL_CLEARED,
		SHOWING_LEVEL_STATS,
	};
	int mode;
	SSIntegrator * integrator;
	SpaceShooterCR * cr;
	SpaceShooterCD * cd;
	Level level;
	List<int> movementDirections;

	String levelSource;

	/// o.o
	GameVariable * currentLevel,
		* currentStage,
		* score,
		* money,
		* playTime,
		* playerName,
		* gameStartDate,
		* difficulty;

	/// Default 30x20
	void SetPlayingFieldSize(Vector2f newSize);
	
	/// Saves previousMode
	void SetMode(int newMode, bool updateUI = true);
	/// o.o
	int previousMode;
	/// 0 by default.
	int gearCategory;
private:
	
	void RenderInLevel(GraphicsState * graphicsState);

	/// Called each app frame to remove projectiles and ships outside the relevant area.
//	void Cleanup();
	void OnPauseStateUpdated();

	/// For display.
	String lastError;

	/// For jumping in time... o.o
	Time lastTimeInput;
	Time targetTime;
};


extern SpaceShooter2D * spaceShooter;
