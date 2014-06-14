/// Emil Hedemalm
/// 2014-04-19
/** Base class for all game-states belonging to the RuneRPG game.
	Provides pointers and accessing functions to the game session,
	players, etc.
*/

#ifndef RR_GAMESTATE_H
#define RR_GAMESTATE_H

#include "GameStates/GameState.h"
#include "RuneRPG/Network/RRSession.h"
#include "RuneRPG/GameStates/RuneGameStatesEnum.h"



#define DEFAULT_NAME "No-name"

class RRGameState : public GameState {
public:
	RRGameState();

	/// Hosts a game
	bool Host(int port = RR_DEFAULT_PORT);
	// Stop hosting this game. 
	bool CancelGame();
	bool Join(String ip, int port = RR_DEFAULT_PORT);

	/// Fetches players from the active session.
	static List<RRPlayer*> Players();

	/// The main-session.
	static RRSession * session;
protected:
	// If not created, create and add the session to be handled by the network-manager.
	void CreateSessionIfNeeded();
	
	/// For starting it from anywhere.
	void StartBattle(String battleRef);


	// Name our player on this device.
	static String playerName;
	/// Called to log network-related messages, like clients joining or failures to host. Display appropriately.
	virtual void NetworkLog(String message);
};

#endif