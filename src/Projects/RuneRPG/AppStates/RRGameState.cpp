/// Emil Hedemalm
/// 2014-04-19
/** Base class for all game-states belonging to the RuneRPG game.
	Provides pointers and accessing functions to the game session,
	players, etc.
*/

#include "RRGameState.h"
#include "Network/NetworkManager.h"
#include "Graphics/Messages/GMUI.h"
#include "Graphics/GraphicsManager.h"
#include "Message/MessageManager.h"
#include "StateManager.h"

// Static variables.
RRSession * RRGameState::session;
String RRGameState::playerName = DEFAULT_NAME;

RRGameState::RRGameState()
{
	// Initialization, set pointers to 0.
	session = NULL;
}

// If not created, create and add the session to be handled by the network-manager.
void RRGameState::CreateSessionIfNeeded()
{
	if (!session){
		session = new RRSession(playerName);
		// Add the session
		NetworkMan.AddSession(session);
	}
}

/// For starting it from anywhere.
void RRGameState::StartBattle(String battleRef)
{
//	RuneBattleState.
//	BattleMan.QueueBattle(battleRef);
//	RuneBattleState * battleState = ;
	// Enter battle state.
	StateMan.QueueState(StateMan.GetStateByID(RUNE_GAME_STATE_BATTLE_STATE));
}


/// Hosts a game
bool RRGameState::Host(int port /*= 33010*/)
{
	// If not created, create and add the session to be handled by the network-manager.
	CreateSessionIfNeeded();
	bool success = session->Host(port);
	if (success)
	{
		// Show lobby gui!
		Graphics.QueueMessage(new GMPushUI("gui/Lobby.gui", GetUI()));
		MesMan.QueueMessages("OnPlayersUpdated");
		// Since host, push ui to select if game type: new or load a saved game.
		Graphics.QueueMessage(new GMPushUI("gui/GameType.gui", GetUI()));
	}
	return success;
}

// Stop hosting this game. 
bool RRGameState::CancelGame()
{
	assert(session);
	if (!session)
		return false;
	session->Stop();
	NetworkLog("Game canceled");
	// Remove lobby if it is up.
	Graphics.QueueMessage(new GMPopUI("gui/Lobby.gui", GetUI(), true));
	return true;
}

bool RRGameState::Join(String ip, int port /*= 33010*/)
{
	/// Start SIP session if needed?
	bool success = NetworkMan.StartSIPServer();
	if (!success)
		return false;
	// If not created, create and add the session to be handled by the network-manager.
	CreateSessionIfNeeded();
	/// Check if already connected.
	if (session->IsConnected())
	{
		NetworkLog("Already connected.");
		// Show lobby?
		Graphics.QueueMessage(new GMPushUI("gui/Lobby.gui", GetUI()));
		MesMan.QueueMessages("OnPlayersUpdated");
		return false;
	}
	/// Connect with our session.
	success = session->ConnectTo(ip, port);
	if (success)
	{
		NetworkLog("Connected successfully.");
		// Show lobby?
		Graphics.QueueMessage(new GMPushUI("gui/Lobby.gui", GetUI()));
		MesMan.QueueMessages("OnPlayersUpdated");
	}
	return success;
}

/// Fetches players from the active session.
List<RRPlayer*> RRGameState::GetPlayers()
{
	return session->GetPlayers();
}

/// Fetches our main player entity!
Entity * RRGameState::GetMainPlayerEntity()
{
	List<RRPlayer*> players = session->GetPlayers();
	for (int i = 0; i < players.Size(); ++i)
	{
		RRPlayer * player = players[i];
		return player->mapEntity;
	}
	return NULL;
}

RREntityState * RRGameState::GetMainPlayerState()
{
	Entity * mainPlayerEntity = GetMainPlayerEntity();
	RREntityState * rrState = (RREntityState *)mainPlayerEntity->GetProperty("RREntityState");
	return rrState;
}	


/// Called to log network-related messages, like clients joining or failures to host. Display appropriately.
void RRGameState::NetworkLog(String message)
{
	Graphics.QueueMessage(new GMSetUIs("NetworkLog", GMUI::TEXT, message));
}
