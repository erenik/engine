/// Emil Hedemalm
/// 2015-01-20
/// Space shooter.
/// For the Karl-Emil SpaceShooter project, mainly 2014-2015/

#include "AppStates/AppState.h"

#include "Texture.h"
#include "TextureManager.h"

#include "Model/Model.h"
#include "Model/ModelManager.h"

#include "UI/UserInterface.h"

#include "String/StringUtil.h"

#include "File/File.h"
#include "File/FileUtil.h"

#include "Maps/MapManager.h"

#include "Script/Script.h"
#include "Script/ScriptManager.h"

#include "Audio/AudioManager.h"
#include "Audio/Messages/AudioMessage.h"

#include "Game/SpaceShooter/SpaceShooterCD.h"
#include "Game/SpaceShooter/SpaceShooterCR.h"
#include "Game/GameVariableManager.h"

#include "Application/Application.h"

#include "Random/Random.h"

#include "Physics/Messages/CollisionCallback.h"

#include "Window/AppWindow.h"

#include "Entity/EntityProperty.h"
#include "Entity/EntityManager.h"

#include "File/SaveFile.h"

#include "Graphics/Messages/GMAnimate.h"
#include "Graphics/Messages/GMRenderPass.h"
#include "Graphics/GraphicsProperty.h"
#include "Graphics/Animation/AnimationSet.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMParticles.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Camera/Camera.h"

#include "Render/RenderPass.h"

#include "Message/MathMessage.h"
#include "Message/MessageManager.h"
#include "Message/Message.h"

#include "Input/InputManager.h"

#include "Physics/Integrators/FirstPersonIntegrator.h"
#include "Physics/CollisionDetectors/FirstPersonCD.h"
#include "Physics/CollisionResolvers/FirstPersonCR.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "OS/OSUtil.h"
#include "OS/Sleep.h"

#include "Sphere.h"
#include "StateManager.h"
#include "Viewport.h"

#include "Luchador.h"

#define CC_ENVIRONMENT	1
#define CC_PLAYER		(1 << 1)
#define CC_PESO			(1 << 2)
#define CC_OBSTACLE		(1 << 3)

#define EP_PESO 0
#define EP_LUCHA 1

class GameVariable;

/// Time in current level, from 0 when starting. Measured in milliseconds.
extern Time levelTime;
// extern int64 nowMs;
extern int timeElapsedMs;

extern float distance;
extern int attempts;
extern Time now;

/// Particle system for sparks/explosion-ish effects.
//extern Sparks * sparks;

/// These will hopefully always be in AABB axes.
extern Vector3f frustumMin, frustumMax;

extern bool paused;

class SideScroller : public AppState 
{
public:
	SideScroller();
	virtual ~SideScroller();
	/// Function when entering this state, providing a pointer to the previous StateMan.
	void OnEnter(AppState * previousState);
	/// Main processing function, using provided time since last frame.
	void Process(int timeInMs);
	void ProcessLevel(int timeInMs); // o.o
	/// Returns true if it should create more. So use with while(CreateNextLevelParts());
	bool CreateNextLevelParts();
	/// Function when leaving this state, providing a pointer to the next StateMan.
	void OnExit(AppState * nextState);

	/// Creates the user interface for this state
	virtual void CreateUserInterface();

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();

	/// Called from the render-thread for every viewport/AppWindow, after the main rendering-pipeline has done its job.
	virtual void Render(GraphicsState * graphicsState);

	/// UI stuffs. All implemented in UIHandling.cpp
	void UpdateUI();
	void UpdateMunny();
	void UpdateAttempts();
	void UpdateDistance();
	void UpdateGearList();
	/// Update UI parts
//	void UpdateUIPlayerHP();
//	void UpdateUIPlayerShield();
	/// Update ui
	void OnScoreUpdated();
	void ShowLevelStats();
	void LoadDefaultName();

	/// Level score. If -1, returns current.
	GameVariable * LevelScore(int stage = -1, int level = -1);
	/// Level score. If -1, returns current.
	GameVariable * LevelKills(int stage = -1, int level = -1);

// private:

	/// Starts a new game. Calls LoadLevel
	void NewGame();
	void Jump();
	void NewPlayer();
	void Pause();
	void Resume();
	void TogglePause();
	
	/// Loads target level. The source and separate .txt description have the same name, just different file-endings, e.g. "Level 1.png" and "Level 1.txt"
	void LoadLevel(String levelSource = "CurrentStageLevel");
	void GameOver();
	void LevelCleared();

	/// Opens main menu.
	void OpenMainMenu();
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
		IN_WORKSHOP,
		BUYING_GEAR,
		LOAD_SAVES,
		PLAYING_LEVEL,
		GAME_OVER,
		LEVEL_CLEARED,
		SHOWING_LEVEL_STATS,
	};
	int state;

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
	void SetState(int newState, bool updateUI = true);
	/// o.o
	int previousState;
	/// 0 by default.
	int gearCategory;
private:
	
//	void RenderInLevel(GraphicsState * graphicsState);

	/// Called each app frame to remove projectiles and ships outside the relevant area.
//	void Cleanup();
	void OnPauseStateUpdated();

	/// Place da man!
	void PacoTaco();
	/// Creates the 5 meter wide 'breather'-block. Increments the level-length.
	void BreatherBlock(float width = 5.f);
	/// Creates a 20+ meters level-part.
	void AddLevelPart();
	void CleanupOldBlocks();

	/// Defaults.
//	void Block(); // Appends a block. Default size 2.
//	void Hole(); // Appends a hole, default side 2.

	/// Various parts o.o
	void FlatPart(); // Just flat, 10 pieces.
	void LinearHoles(int numHoles); // With a number of holes at varying positions, always with 1 block in between. Max 5 holes.
	void AddDBLPart(); // Difficulty-By-Length, randomly generated. Used in initial test

	/// For display.
	String lastError;
};


extern SideScroller * sideScroller;
