/// Emil Hedemalm
/// 2014-01-24
/// A general media- or network-session of any sort. Will mostly refer to a gaming session.

#ifndef NETWORK_SESSION_H
#define NETWORK_SESSION_H

#include "String/AEString.h"
class Peer;
class Packet;
class TcpServer;
class Socket;

class Session {
	friend class NetworkManager;
public:
	Session(String name, String typeName, int type);
	/// Virtual destructor for proper deallocation.
	virtual ~Session();

	/// Query if this session is running/active and successfully has bound it's designated listen ports.
	virtual bool IsHost() const;
	/// If we are currently connected as a client in this session.
	virtual bool IsConnected() const;

	/// Connects to address/port.
	virtual bool ConnectTo(String ipAddress, int port);
	/// Attempts to start hosting a session of this kind. Default hosts 1 TcpServer on target port. 
	virtual bool Host(int port);
	bool IsHost();
	/// Stops the session, disconnecting any connections and sockets the session might have.
	virtual void Stop();

	/// Called when the host disconnects.
	virtual void OnHostDisconnected(Peer * host);

	/// For analysis, queries the same functions in all available sockets.
	virtual int BytesSentPerSecond();
	virtual int BytesReceivedPerSecond();
	
	/// Function to process new incoming connections but also disbard old connections that are no longer active.
	virtual void EvaluateConnections();
	
	/// Sends target packet to all peers in this session using default primary sockets, via host if possible.
	virtual void Send(Packet * packet);
	/// Sends text as a packet.
	virtual void SendText(String text);
	/** Reads packets, creating them and returning them for processing. 
		Each batch of received bytes is considered one "packet". Subclass to override.
	*/
	virtual List<Packet*> ReadPackets();

	/// Name of this session, could be "Emil's Game" or "Lobby Chat" for example.
	String name;
	/// Name of this type of session.
	String typeName;
	/// See SessionTypes.h
	int type;
	/// See specific header file. For example GameSessionTypes.h
	int subType;
	/// Returns type name of this session.
	String TypeName();
	/// See SessionType of target session ID.
	static String TypeName(int id);
	/// Host of this session (if specified)
	Peer * host;
	/// Peers in this session (excluding me)
	List<Peer*> peers;
	/// Maximum amount of peers that we will allow to be active in this session before refusing new registrations.
	int maxPeers;
	/// Returns the last error string, nullifying it too.
	String GetLastErrorString();
protected:
	/// Called every time a socket is deleted. Any references to the socket should then be removed.
	virtual void OnSocketDeleted(Socket * sock);
	
	/// Deletes those sockets that either have errors or have been flagged for deletion (other errors)
	void DeleteFlaggedSockets();

	/// Packet queue for all packets that are outbound to all connected peers.
	List<Packet*> packetQueue;
	/// Pokes at all associated sockets, prompting them to send their queued packets. Also sends all general queued packets to all peers.
	void SendPackets(); 

	String lastErrorString;
	/// Socket for communicating with "host"
	Socket * hostSocket;
	/// List of sockets not currently associated with a single peer.
	List<Socket*> sockets;
	/// If true, we are the host to this session. This will be true also when playing single-player.
	bool isHost;
	/// If true, we are currently connected to a peer/host.
	bool isConnected;
	/// For hosting localHost-only sessions.
	bool localOnly;
	
	/// As a session most likely includes a tcp server, this will be stored here if so.
	TcpServer * tcpServer;
	/// Pointer reference to the me-structure found in the NetworkManager.
	Peer * me;
	/// Main ipAddress of this session.
	String hostName;
	/// Main port we've hosted this session on?
	int port;
	
	/// If the game session is local. This is set to false if using ConnectTo or Host.
	bool isLocal;

};

#endif