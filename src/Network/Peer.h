/// Emil Hedemalm
/// 2014-01-24
/// Peer class used for various networking sessions.

#ifndef PEER_H
#define PEER_H

#include "SIP/SIPPacket.h"

class Socket;
class TcpSocket;
class Stream;
class Session;
class SessionData;

/// Abstraction for a network entity. Handled primarily by the NetworkManager.
class Peer {
public:
	/// Peer o/o
	Peer();
	virtual ~Peer();
	/// Name of the client! Might be good, yes.
	String name;
	/// Removes any references to target socket.
	void RemoveSocket(Socket * sock);
	/// o.o
	template<class T>
	T * GetSessionData();
	/// Returns target session data. For example SessionType::SIP as argument.
	SessionData * GetSessionData(int sessionType, int andSubType = -1);
	/// Networking sessions that this peer is taking part in.
	List<Session*> associatedSessions;
	/// Networking data related to the associated sessions.
	List<SessionData*> sessionData; 

	/// Closes and deletes all sockets bound to this peer.
	void DeleteSockets();
	/// Debug, prints name, ip and port.
	void Print();

	/// Flag that is set after name, IP and optionally port have been identified. Must be true before being used to send any packets.
	bool isValid;

	/// Main communication socket.
	Socket * primaryCommunicationSocket;
	/// List of additional sockets.
	List<Socket*> sockets;
	
	/// Hosting-port of target peer. If set to -1, will use the application default values in the 33000 to 33010 range. 
	int port;
	/// IP in the standard IPv4 format "#.#.#.#".
	String ipAddress;
	/// Our own address in reference to the peer. In localhost this is 127.0.0.1 for example, but might vary with network-types?
	String localAddress;

private:
	/// Checks if this peer is considered the same as another (name, ip, port)
	bool SameAs(Peer * otherPeer);
	/// Acts as a merger, extracting active tcpSockets and other data.
	void ExtractDataFrom(Peer * otherPeer);

};


#endif
