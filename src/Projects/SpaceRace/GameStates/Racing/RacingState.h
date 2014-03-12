// Emil Hedemalm
// 2013-06-17

#ifndef RACING_H
#define RACING_H

#include "../SpaceRaceGameState.h"
#include "Selection.h"
#include "Graphics/Camera/Camera.h"

class GameSession;
class RenderViewport;
class SRPlayer;

#define LAP_START_TIME_STR	"LapStartTime"

class Racing : public SpaceRaceGameState {
	static Racing * racing;
public:
	Racing();
	/// Virtual destructor to discard everything appropriately.
	virtual ~Racing();
	void OnEnter(GameState * previousState);
	void Process(float time);
	void OnExit(GameState * nextState);
	/// Input functions for the various states
	void MouseClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	void MouseRightClick(bool down, int x = -1, int y = -1, UIElement * elementClicked = NULL);
	virtual void MouseMove(float x, float y, bool lDown = false, bool rDown = false, UIElement * elementOver = NULL);
	void MouseWheel(float delta);
	void CreateDefaultBindings();
	void CreateUserInterface();
	void InputProcessor(int action, int inputDevice);
	void ProcessMessage(Message * message);
	void ProcessPacket(Packet * packet);
	void ToggleMenu();

	// For resetting position
	static Entity * GetCheckpoint(int index);

protected:
	/// Creates player entities!
	bool CreatePlayers(List<SRPlayer*> players);

	/// Sets message to be displayed in-game, for example when host disconnects or returns to the lobby.
	void SetGeneralMessage(String str);

private:
	/// Debugging variables
	int stateUpdatesSent;
	long long lastStateGUIUpdated;
	/// Last time we (as host) sent data to clients. Used to regulate how often it is sent out.
	long long lastUpdateToClients;
	/// Used to regulate how often we notify clients of everyones positions. delay in ms
	int minimumHostUpdateDelay;
	/// Estimation type used by clients
	int estimationMode;
	/// Estimation delay used by clients
	int estimationDelay;
	/// Smoothing duration used by clients for extrapolation
	int smoothingDuration;

	/// Networked gaming session.
	GameSession * gameSession;

	/// Creates ship-entity for target player.
	Entity * CreateShipForPlayer(SRPlayer * player, Vector3f startPosition);

	/// Wosh.
	bool timerStarted;
	///!
	void OnOpenMenu();
	/// ?
	void OnCloseMenu();

	/// Increments checkpoints passed and posts updates to the session manager. Also calls OnPlayerCheckpointsPassedUpdated() to update UI.
	void PlayerPassCheckpoint(SRPlayer * player, int checkpointsPassed = -1);
	/// Updates GUI if local player
	void OnPlayerCheckpointsPassedUpdated(SRPlayer * player);
	void OnPlayerPositionUpdated(SRPlayer * player);
	void OnPlayerLapsUpdated(SRPlayer * player);

	/// Formats the Results-div of the swhoshi for when the players finish the race!
	void FormatResults();

	/// Wosh!
	bool menuOpen;
	String level;
	int previousStateID;
	/// Amount of requested players to create upon entry.
	int requestedPlayers;

	/** List of all active players! (human-controlled, either locally or via network).
		Index 0 will always be the primary local player.
		Refer to the MultiplayerProperty for organizing entities with different players (name/IP/etc).
	*/
	List<Entity*> playerEntities;
	/// Set upon loading map!
	List<Entity*> checkpoints;

#define MAX_RACING_CAMERAS	(MAX_PLAYERS + 1)
	/// The cameras for the 4 primary players and spectators...!
	List<Camera*> cameras;
	List<RenderViewport*> viewports;
	/// Actively manipulated entities
	Selection editorSelection;
};

#endif

// #endif // SPACE_RACE
