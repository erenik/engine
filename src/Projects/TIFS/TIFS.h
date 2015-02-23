/// Emil Hedemalm
/// 2014-07-29
/** Reboot of the TIFS/Virtus project as was conducted during 10 weeks in the spring of 2013 with the following members:
	- Emil Hedemalm
	- Aksel Kornesjö
	- Cheng Wu
	- Andreas Söderberg
	- Michaela Sjöström
	- Fredric Lind

	Old dev blog:	http://focus.gscept.com/gp13-3/
	Old facebook page:	https://www.facebook.com/VirtusLTU
	Our final release for that first iteration of the project: http://svn.gscept.com/gp13-3/public/Virtus.zip
*/

#include "AppStates/AppState.h"

#include "Maps/MapManager.h"

#include "Model/ModelManager.h"
#include "Model/Model.h"

#include "TextureManager.h"
#include "Input/Action.h"
#include "Graphics/GraphicsProperty.h"

#include "File/LogFile.h"
#include "StateManager.h"

#include "TIFSBuilding.h"
#include "TIFSGrid.h"
#include "TIFSMapEditor.h"
#include "TIFS/Physics/TIFSIntegrator.h"
#include "TIFS/Physics/TIFSCD.h"
#include "TIFS/Physics/TIFSCR.h"
#include "TIFS/Properties/TIFSPlayerProperty.h"
#include "TIFS/Properties/TIFSProjectile.h"
#include "TIFS/Properties/TIFSTurretProperty.h"
#include "TIFS/Properties/TIFSDroneProperty.h"
#include "TIFS/Graphics/ToolParticles.h"

#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMParticles.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Camera/Camera.h"

#include "Message/Message.h"

#include "Application/Application.h"
#include "StateManager.h"
#include "Audio/Messages/AudioMessage.h"

#include "Entity/EntityManager.h"
#include "Input/InputManager.h"

#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"
#include "Physics/Messages/CollisionCallback.h"
#include "Physics/PhysicsManager.h"
#include "Physics/PhysicsProperty.h"

#include "Random/Random.h"

#include "Script/ScriptManager.h"

#include "Weather/WeatherSystem.h"

class Camera;
class Message;
class Packet;
class TIFS;
class TIFSMapEditor;
class Entity;
class TIFSPlayerProperty;
class ToolParticleSystem;

#define CC_ENVIRON	1
#define CC_LASER	(1 << 1)
#define CC_DRONE	(1 << 2)
#define CC_TURRET	(1 << 3)
#define CC_PLAYER	(1 << 4)

// Global pointers to the game states.
extern TIFS * tifs;
extern TIFSMapEditor * mapEditor;

enum 
{
	BAD_SIZE,
	SMALL, // 1
	MEDIUM, // 2
	LARGE, // 3
	SIZES, // 4?
};

extern float timeDiffS;
extern int64 timeNowMs;

// Default true?
extern bool tifsInstancingEnabled;

/// The main/global Application state for the game.
class TIFS : public AppState 
{
	friend class TIFSPlayerProperty;
public:
	TIFS();
	virtual ~TIFS();
	/// Function when entering this state, providing a pointer to the previous StateMan.
	virtual void OnEnter(AppState * previousState);
	/// Main processing function, using provided time since last frame.
	virtual void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	virtual void OnExit(AppState * nextState);

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessPacket(Packet * packet);
	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();
	/// Creates the user interface for this state. Is called automatically when re-building the UI with the CTRL+R+U command.
	virtual void CreateUserInterface();

	void ResetCamera();

	/// Randomly!!!! o-=o
	void SpawnDrones(int num);
	void SpawnDrone(ConstVec3fr atLocation);

	/// Randomly!!!! o-=o
	void CreateTurrets(int num);
	/// Creates a turret! Sizes Small, Medium and Large accepted.
	void CreateTurret(int ofSize, ConstVec3fr atLocation);

	// Spawn player
	void SpawnPlayer();

	Entity * GetClosestDefender(ConstVec3fr toPosition);
	Turret * GetClosestTurret(ConstVec3fr toPosition);
	Turret * GetClosestActiveTurret(ConstVec3fr toPosition);

	/// o-o
	List<Entity*> players,
		turretEntities,
		drones,
		motherships,
		groundEntities;

	List<TIFSTurretProperty*> turrets;

	List<Camera*> cameras;

	// Player property for steering.
	TIFSPlayerProperty * playerProp;
	
	void TogglePause();
private:
	bool paused;
	// Creates a new game with standard stuff.
	void NewGame();
	void CreateField();
	void AddBuildings(int num = 5);

	void HideMainMenu();
	void ShowMainMenu();
	void HideTitle();
	void ShowTitle();
	void ShowHUD();
	void HideHUD();

	/// o-o
	ToolParticleSystem * toolParticles;
	TIFSGrid * grid;

	Vector3i gridSize;
	// o.o
	float fieldSize, halfFieldSize;
};

