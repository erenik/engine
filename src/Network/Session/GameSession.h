/// Emil Hedemalm
/// 2014-01-24
/// A general media- or network-session of any sort. Will mostly refer to a gaming session.

#ifndef GAME_SESSION_H
#define GAME_SESSION_H

#include "Session.h"
#include "Network/Socket/UdpSocket.h"

class Game;

class GameSession : public Session {
public:
	GameSession(String sessionName, String gameName, int maxPeers);
	/// Virtual destructor for proper deallocation.
	virtual ~GameSession();

	/// Sends a packet to all peers in the session, via the host if possible.
//	void Send(Packet * packet);

	/// Performs regular tcp connection via Session, but also sets up our client UDP socket.
	virtual bool ConnectTo(String ipAddress, int port, int clientUdpPort);
	/// Calls host for Session but also creates and binds a UDP socket.
	virtual bool Host(int tcpPort, int udpPort);

	/// Called when the host disconnects.
	virtual void OnHostDisconnected(Peer * host);

	/// Returns a Game-object containing all necessary info about this specific game instance.
	Game * GetGame();

	/// For fast-paced data.
	UdpSocket * udpSocket;
//	UdpSocket * clientUdpSocket;

	/// Maximum amount of peers for this game session.
	int maxPlayers;
	int currentPlayers;

	/// Main udp port in use when hosting this session.
	int udpPort;

};

#endif