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

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMParticles.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Camera/Camera.h"
#include "Graphics/Particles/Sparks.h"
#include "Graphics/Particles/SparksEmitter.h"

#include "Game/SpaceShooter/SpaceShooterCD.h"
#include "Game/SpaceShooter/SpaceShooterCR.h"

#include "ShipProperty.h"
#include "ProjectileProperty.h"

// Collision categories.
#define CC_PLAYER	1 
#define CC_ENEMY	2

enum {
	SHIP_PROP,
	PROJ_PROP,
};

class SpaceShooterCR;
class SpaceShooterCD;

extern int64 nowMs;
/// Particle system for sparks/explosion-ish effects.
extern Sparks * sparks;

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

	void UpdateUI();
	/// Update UI
	void UpdatePlayerHP();
	/// Update ui
	void OnScoreUpdated();
	/// o.o
	Entity * OnShipDestroyed(Ship * ship);

	/// o.o
	int score;


// private:

	/// Starts a new game. Calls LoadLevel
	void NewGame();
	/// Loads target level. The source and separate .txt description have the same name, just different file-endings, e.g. "Level 1.png" and "Level 1.txt"
	void LoadLevel(String levelSource);
	void GameOver();

	void UpdatePlayerVelocity();
	void ResetCamera();

	/// Process target ship.
	void Process(Ship & ship);
	

	enum {
		IN_MENU,
		PLAYING_LEVEL,
		GAME_OVER,
	};
	int mode;
	SSIntegrator * integrator;
	SpaceShooterCR * cr;
	SpaceShooterCD * cd;
	Level level;
	Ship * playerShip;
	List<int> movementDirections;
	/// All ships, including player.
	List<Entity*> shipEntities;
	List<Entity*> projectileEntities;

	String levelSource;
};


extern SpaceShooter2D * spaceShooter;

