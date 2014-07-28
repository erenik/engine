/// Emil Hedemalm
/// 2014-01-24
/// A general media- or network-session of any sort. Will mostly refer to a gaming session.


#include "GameSession.h"
#include "Game/Game.h"
#include "SessionTypes.h"
#include "Network/Peer.h"
#include "Network/Server/TcpServer.h"

/// Name of our specific session, the game's name, max amount of peers/clients, and your name for this session.
GameSession::GameSession(String sessionName, String gameName, int maxPeers)
: Session(sessionName, gameName, SessionType::GAME), maxPlayers(maxPlayers)
{
	currentPlayers = 0;
	udpSocket = NULL;
	isLocal = true;
}

/// Virtual destructor for proper deallocation.
GameSession::~GameSession()
{
	if (udpSocket){
		udpSocket->Close();
		delete udpSocket;
	}
	// Session or subclass should handle most?
}


/// Returns a Game-object containing all necessary info about this specific game instance.
Game * GameSession::GetGame()
{
	assert(false);
	/*
	assert(isHost);
	Game * game = new Game();
	game->name = name;
	game->type = typeName;
	assert(me->name.Length());
	game->host = me->name;
	game->port = this->tcpServer->Port();
	assert(game->port > 0);
	game->udpPort = this->udpSocket->port;
	game->currentPlayers = this->currentPlayers;
	game->maxPlayers = this->maxPlayers;
	return game;
	*/
	return NULL;
}

/// Performs regular tcp connection via Session, but also sets up our client UDP socket.
bool GameSession::ConnectTo(String ipAddress, int port, int clientUdpPort /* = -1 */)
{
	bool result = Session::ConnectTo(ipAddress, port);
	if (!result){
		/// Error string should have been set by Session::ConnectTo
		return false;
	}
	// Only do UDP stuff if it was requested to use a UDP port.
	if (clientUdpPort > 0)
	{
		if (!udpSocket)
			udpSocket = new UdpSocket();
		result = udpSocket->Bind(clientUdpPort);
		udpSocket->peerAddress = ipAddress;
		if (!result)
		{
			this->lastErrorString = udpSocket->GetLastErrorString();
			return false;
		}
	}
	isLocal = false;
	return true;
}

/// Calls host for Session but also creates and binds a UDP socket.
bool GameSession::Host(int tcpPort, int udpPort)
{
	bool result = Session::Host(tcpPort);
	if (!result)
	{	
		std::cout<<"\nGameSession::Host: Unable to start tcpServer.";
		return false;
	}
	/// Create 
	if (udpSocket){
		udpSocket->Close();
		delete udpSocket;
		udpSocket = NULL;
	}
	udpSocket = new UdpSocket();
	result = udpSocket->Bind(udpPort);
	if (!result)
	{
		std::cout<<"\nGameSession::Host: Unable to bind UdpSocket.";	
		return false;
	}
	/// Save away port numbers
	this->port = tcpPort;
	this->udpPort = udpPort;
	isLocal = false;
	return true;
}

/// Called when the host disconnects.
void GameSession::OnHostDisconnected(Peer * host)
{
	/// Woo.
}
