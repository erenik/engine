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

class Message;
class Packet;
class TIFSMapEditor;

/// Map editor, including script-editors and random map generator tools.
class TIFSMapEditor : public AppState 
{
public:
	TIFSMapEditor();
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

private:
	void GenerateGround();

	void OnGroundSizeUpdated();

	// Scale in x/y
	Vector2f groundSize;
};

