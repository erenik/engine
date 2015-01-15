/// Emil Hedemalm
/// 2014-01-24
/// Session handle for SIP. This includes handling of a Tcp Server, SIP peers etc.

#ifndef SIP_SESSION_H
#define SIP_SESSION_H

#include "Network/Session/Session.h"
#include "Network/SIP/SIPPacketParser.h"

/// Definition of Script-parameters for the SUBSCRIBE/NOTIFY mechanism
#define PEER_CONNECTED_EVENT    "PeerConnectedToNetwork"
#define MEDIA_SEARCH_EVENT      "MediaSearchEvent"
#define GAME_SEARCH_EVENT		"GameSearchEvent"

// Reserved pots for SIP, 10 ports.
#define DEFAULT_SIP_START_PORT	33000
#define DEFAULT_SIP_MAX_PORT	33009

class SIPSessionData;
class SIPRegisterPacket;
class SIPEvent;
class Game;

/// Session Initiation Protocol Session. Used to facilitate initiation of other sessions.
class SIPSession : public Session {
public:
	/// CC
	SIPSession();
	virtual ~SIPSession();
	/// Creates the SIPRegister packet using the data in the given Peer-structure for ourselves.
	void Initialize();

	/// Attempts to connect on all valid ports (33000 to 33010), until 1 works.
	virtual bool ConnectTo(String ipAddress);
	/// Connects to address/port.
	virtual bool ConnectTo(String ipAddress, int port);
	/// Connects to address/port in given range, until it works.
	virtual bool ConnectTo(String ipAddress, int startPort, int stopPort);
	/// Attempts to start hosting a session of this kind, using any available port in the 33000 to 33010 range. 
	virtual bool Host();
	/// Attempts to start hosting a session of this kind. 
	virtual bool Host(int port);
	/// Stops the session, disconnecting any connections and sockets the session might have.
	virtual void Stop();

	/// Called when the host disconnects.
	virtual void OnHostDisconnected(Peer * host);

	/// Function to process new incoming connections but also disbard old connections that are no longer active.
	virtual void EvaluateConnections();
	/// Returns a list of all peers that have connected since the last call to ReadPackets
//	List<Peer*> GetNewPeers();
	/// Returns a list of all peers that have disconnected since the last call to EvaluateConnections();
	List<Peer*> GetDisconnectedPeers();
    
	/// Sends target packet to all peers in this session using default targetsm, via host if possible.
	virtual void Send(Packet * packet);
	/// Constructs and sends a SIP INFO packet to all peers.
	virtual void SendInfo(String info);
	/// Reads packets, creating them and returning them for processing. Note that some packets will already be handled to some extent within the session (for exampling many SIP messages).
	virtual List<Packet*> ReadPackets();
	
	/// Returns a list of available games from all peers.
	List<Game*> GetAvailableGames();

	/// Uses the "name <name@ip>" from/to field to associate a peer.
	Peer * GetPeerByData(String data);

	/// Creates SIP Session Data structure and attaches it to the new peer as needed.
	void EnsureSIPSessionData(Peer * forPeer);
	/// Sends a BadRequest, complaining that the sender should register first before sending shit.
	void DemandRegistration(SIPPacket * packet);

	/// Handles packet received from target peer. Returns the validity of the peer after processing the packet. If false, the peer has been invalidated.
    bool HandlePacket(SIPPacket* packet, Peer* peer);

    /// Finds peers belonging to target packet and sets the appropriate pointers within (sender & recipient)
    void FindPeersForPacket(SIPPacket * packet, Peer * sender);
    /// Extracts the "tag"-Identifier for the peer, hopefully provided in the packet.
    bool ExtractPeerTag(Peer * peer, SIPPacket * packet);
    /// Ensures that all peer-parameters have been set apporpriately, using the given packet's data as needed. Returns false upon failure to extract peer data from packet.
    bool EnsurePeerData(Peer * peer, SIPPacket * packet);
	/// Builds and sends a SIP Register message to target peer.
	void RegisterWithPeer(Socket * sock);
	void RegisterWithPeer(Peer * peer);
	/// Number of peers reigsterd successfully.
	int NumRegisteredPeers();
	
	/// Builds and sends a SIP Subscribe message to target peer in order to automatically discover new connected peers to the network.
	void SubscribeToNewPeerConnections(Peer * peer);

	/// Send a NOTIFY message to all previously connected peers to inform of the new peer's arrival to the network
	void InformPeersOfNewConnectedPeer(Peer * newPeer);
	/** Contrary to the above method, this one informs a single newly connected peer of the entire list of connected peers. 
		This should be sent once at the start of a subscription!
	*/
	void InformPeerOfConnectedPeers(Peer * newPeer);

	
	/// Sets list of media to be made publicly available.
	void SetAvailableMedia(List<String> list);
	/// Informs a peer of the entire list of media available to it, conforming to it's requested search-string.
	void InformPeerOfAvailableSoughtMedia(Peer * peer);
	
	/// Returns sessionData associated with the peer.
	SIPSessionData * GetSessionData(Peer * forPeer);

	/// Returns a list of currently connected peers (by checking if the primaryCommunicationSocket is valid).
	List<Peer*> ConnectedPeers();

	/// Attempts to discover peers using the string format "ip port", e.g. "127.0.0.1 33000"
	void DiscoverPeers(List<String> peersList);

	/// Sets event state. If it didn't exist before it will be created.
	void SetEventState(String eventName, String eventState);

	/// List of peers that have recently joined/connected. <- Why?
//	List<Peer*> newPeers;
	/// List of peers that have recently disconnected.
	List<Peer*> disconnectedPeers;
protected:
	/// Called every time a socket is deleted. Any references to the socket should then be removed.
	virtual void OnSocketDeleted(Socket * sock);
	/// Informs peers currently subscribing of the new event state.
	void OnEventUpdated(SIPEvent * e);
private:
	/// List of events that can be subscribed upon, including current states.
	List<SIPEvent*> events;

	/// Parser for all packets. Will sent the ClientDisconnected signal if the TcpSocket starts returning -1 bytes read (clear indiciation of a d/c).
    SIPPacketParser sipPacketParser;
    
	/// Register message that we send to peers
	SIPRegisterPacket * registerPacket;

};

#endif