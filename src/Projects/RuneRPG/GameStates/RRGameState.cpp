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

/// Hosts a game
bool RRGameState::Host(int port /*= 33010*/)
{
	// If not created, create and add the session to be handled by the network-manager.
	CreateSessionIfNeeded();
	bool success = session->Host(port);
	if (success)
	{
		// Show lobby gui!
		Graphics.QueueMessage(new GMPushUI("gui/Lobby.gui"));
		MesMan.QueueMessages("OnPlayersUpdated");
	}
	return success;
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
		Graphics.QueueMessage(new GMPushUI("gui/Lobby.gui"));
		MesMan.QueueMessages("OnPlayersUpdated");
		return false;
	}
	/// Connect with our session.
	success = session->ConnectTo(ip, port);
	if (success)
	{
		NetworkLog("Connected successfully.");
		// Show lobby?
		Graphics.QueueMessage(new GMPushUI("gui/Lobby.gui"));
		MesMan.QueueMessages("OnPlayersUpdated");
	}
	return success;
}

/// Fetches players from the active session.
List<RRPlayer*> RRGameState::Players()
{
	return session->GetPlayers();
}

/// Called to log network-related messages, like clients joining or failures to host. Display appropriately.
void RRGameState::NetworkLog(String message)
{
	Graphics.QueueMessage(new GMSetUIs("NetworkLog", GMUI::TEXT, message));
}
