/// Emil Hedemalm
/// 2014-07-29
/** Reboot of the TIFS/Virtus project as was conducted during 10 weeks in the spring of 2013 with the following members:
	- Emil Hedemalm
	- Aksel Kornesj�
	- Cheng Wu
	- Andreas S�derberg
	- Michaela Sj�str�m
	- Fredric Lind

	Old dev blog:	http://focus.gscept.com/gp13-3/
	Old facebook page:	https://www.facebook.com/VirtusLTU
	Our final release for that first iteration of the project: http://svn.gscept.com/gp13-3/public/Virtus.zip
*/

#include "AppStates/AppState.h"

class Message;
class Packet;
class TIFS;
class TIFSMapEditor;
class Entity;

// Global pointers to the game states.
extern TIFS * tifs;
extern TIFSMapEditor * mapEditor;

/// The main/global Application state for the game.
class TIFS : public AppState 
{
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

	/// Creates the user interface for this state. Is called automatically when re-building the UI with the CTRL+R+U command.
	virtual void CreateUserInterface();

	/// Creates a turret!
	void CreateTurret(int ofSize, Vector3f atLocation);

	/// o-o
	List<Entity*> players,
		turrets,
		drones,
		motherships,
		groundEntities;

private:

};

