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
#include "TextureManager.h"
#include "Input/Action.h"

class Camera;
class Message;
class Packet;
class TIFS;
class TIFSMapEditor;
class Entity;
class TIFSPlayerProperty;
class ToolParticleSystem;

// Global pointers to the game states.
extern TIFS * tifs;
extern TIFSMapEditor * mapEditor;

/// The main/global Application state for the game.
class TIFS : public AppState 
{
	friend class TIFSPlayerProperty;
public:
	TIFS();
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
	void SpawnDrones();
	void SpawnDrone(ConstVec3fr atLocation);

	/// Randomly!!!! o-=o
	void CreateTurrets();
	/// Creates a turret!
	void CreateTurret(int ofSize, ConstVec3fr atLocation);

	// Spawn player
	void SpawnPlayer();

	/// o-o
	List<Entity*> players,
		turrets,
		drones,
		motherships,
		groundEntities;

	List<Camera*> cameras;

	// Player property for steering.
	TIFSPlayerProperty * playerProp;

private:
	// Creates a new game with standard stuff.
	void NewGame();

	void HideMainMenu();
	void ShowMainMenu();
	void HideTitle();
	void ShowTitle();
	void ShowHUD();
	void HideHUD();

	/// o-o
	ToolParticleSystem * toolParticles;

};

