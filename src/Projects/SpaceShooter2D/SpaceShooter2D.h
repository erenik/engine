/// Emil Hedemalm
/// 2015-01-20
/// Space shooter.
/// For the Karl-Emil SpaceShooter project, mainly 2014-2015/

#include "AppStates/AppState.h"

#include "Ship.h"
#include "ShipProperty.h"
#include "Weapon.h"
#include "Level.h"
#include "SSIntegrator.h"

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

#include "Game/SpaceShooter/SpaceShooterCD.h"
#include "Game/SpaceShooter/SpaceShooterCR.h"

#include "ShipProperty.h"
#include "ProjectileProperty.h"

// Collision categories.
#define CC_PLAYER		1 
#define CC_ENEMY		2
#define CC_PLAYER_PROJ	4
#define CC_ENEMY_PROJ	8

enum {
	SHIP_PROP,
	PROJ_PROP,
};

class SpaceShooterCR;
class SpaceShooterCD;

extern int64 nowMs;
extern int timeElapsedMs;
/// Particle system for sparks/explosion-ish effects.
extern Sparks * sparks;

/// These will hopefully always be in AABB axes.
extern Vector3f frustumMin, frustumMax;

extern Ship playerShip;
/// The level entity, around which the playing field and camera are based upon.
extern Entity * levelEntity;
extern Vector2f playingFieldSize;
extern Vector2f playingFieldHalfSize;
extern float playingFieldPadding;
/// All ships, including player.
extern List<Entity*> shipEntities;
extern List<Entity*> projectileEntities;
extern String playerName;

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

	/// Creates the user interface for this state
	virtual void CreateUserInterface();

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();

	/// Called from the render-thread for every viewport/window, after the main rendering-pipeline has done its job.
	virtual void Render(GraphicsState * graphicsState);

	void UpdateUI();
	/// Update UI parts
	void UpdateUIPlayerHP();
	void UpdateUIPlayerShield();
	/// Update ui
	void OnScoreUpdated();
	/// o.o
	Entity * OnShipDestroyed(Ship * ship);

	/// o.o
	int score;


// private:

	/// Starts a new game. Calls LoadLevel
	void NewGame();
	void Pause();
	void Resume();
	void TogglePause();
	
	/// Loads target level. The source and separate .txt description have the same name, just different file-endings, e.g. "Level 1.png" and "Level 1.txt"
	void LoadLevel(String levelSource = "CurrentStageLevel");
	void GameOver();
	void LevelCleared();

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
		IN_WORKSHOP,
		LOAD_SAVES,
		PLAYING_LEVEL,
		GAME_OVER,
		LEVEL_CLEARED,
	};
	int mode;
	SSIntegrator * integrator;
	SpaceShooterCR * cr;
	SpaceShooterCD * cd;
	Level level;
	List<int> movementDirections;

	String levelSource;

	/// o.o
	int currentLevel;
	int currentStage;

	/// Default 30x20
	void SetPlayingFieldSize(Vector2f newSize);

private:
	/// Called each app frame to remove projectiles and ships outside the relevant area.
	void Cleanup();
	void OnPauseStateUpdated();

	/// For display.
	String lastError;
};


extern SpaceShooter2D * spaceShooter;
