/// Emil Hedemalm
/// 2015-01-15
/// Host-specific interaction: World creation and such.

#include "AppStates/AppState.h"

class MORPGSession;
class Zone;
class MORPGCharacterProperty;

extern MORPGSession * session;

class MHost : public AppState 
{
	friend class MORPG;
public:
	MHost();
	virtual ~MHost();
	/// Function when entering this state, providing a pointer to the previous StateMan.
	void OnEnter(AppState * previousState);
	/// Main processing function, using provided time since last frame.
	void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	void OnExit(AppState * nextState);


	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();
	void HandleCameraMessages(String msg);
private:

	void EnterWorldCreation();
	void GenerateWorld(bool newRandomSeed = false);
	void GenerateSettlements(bool newRandomSeed = false);
	void OnWorldUpdated();

	enum {
		WORLD_CREATION, WORLD_EDITOR = WORLD_CREATION,
		SETTLEMENT_EDITOR,
	};
	int enterMode;
	int mode;
	int settlementIndex;

};
