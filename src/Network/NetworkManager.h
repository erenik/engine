/// Emil Hedemalm
/// 2014-01-24
/// General network-manager that handles all peers and network-sessions.

#include "NetworkSettings.h"

#ifndef NETWORKMANAGER_H
#define	NETWORKMANAGER_H

/*
*/
#include "String/AEString.h"

class TcpServer;
class TcpSocket;
class Peer;
class PlayerWidget;
class Session;
class SIPSession;
class Game;

/**	NetworkManager
	Handles basic networking, peer-connections, etc.
	Interacts by sending most received packets to the MessageManager for further processing.
*/
#define NetworkMan (*NetworkManager::Instance())
class NetworkManager {   
private:
	NetworkManager();
	static NetworkManager * networkManager;
public:
	static void Allocate();
	static NetworkManager * Instance();
	static void Deallocate();
	~NetworkManager();

	/// Starts up the initial SIP server, identities host machine data, etc.
	void Initialize();
	/// Shuts down all active sessions.
	void Shutdown();

	/// Adds new peer!
	void AddPeer(Peer * newPeer);

	/// Returns object representing self. This should not be modified after sessions have been started of any kind.
	Peer * Me();

	/// Returns true if we are hosting a SIP session.
	bool IsHost();
	/// Attempts to retrieve target session by name
	Session * GetSessionByName(String byName);
	/// As defined in SessionTypes.h and specific sub type header file. E.g. GameSessionTypes.h. If no subtype is provided, the first of specified type will be returned.
	Session * GetSession(int byType, int andSubType = -1);
	/// Adds given session, adding it to the list of active sessions.
	bool AddSession(Session * session);

	/// Returns list of all available games. See Game/Game.h for structure definition.
	List<Game*> GetAvailableGames();

	/// Will retrieve the last error. Once called, the error string will be reset to an empty string.
	String GetLastErrorString();
	/// Initialized

	/// Query function for the SIP server.
	bool IsSIPServerRunning();
	/// Starts SIP server with previously set port via SetSIPServerPort. Default ports are 33000 through 33010, starting at 33000.
    bool StartSIPServer();
    /// Close the server socket.
    void StopSIPServer();
   
	/// Sets desired host port for the SIP server
    void SetSIPServerPort(int port);

    /// Attempts to discover peers using the string format "<ip> <port>", e.g. "127.0.0.1 33000"
    void DiscoverPeers(List<String> peerAddresses);
    /// Getter
    Peer * GetPeerByName(String name);
    /// Returns current list of active peers. Note that this list may be adjusted after any packet processing.
    List<Peer*> GetPeers();
   
/*	Media-streaming, add a new manager for this if it wants to be kept?
	/// Sets peer to be input media stream host, flagging all other peers to false. (Peer::streamingFrom)
    void SetStreamHost(Peer * peer);
    /// Returns the peer flagged to be the one that we're currently streaming media from (Peer::streamingFrom)
    Peer * GetStreamHost();
    /// Sets list of media to be made publicly available.
    void SetAvailableMedia(List<String> mediaList);
  */

	/// Attempts to establish a connection to the given server. Default will try ports DEFAULT_SIP_START_PORT through DEFAULT_SIP_MAX_PORT (33000-33009).
    bool ConnectTo(const String ipAddress, int port = -1);
    /// Disconnects this instance from the network, including all peers and our own tcpServer.
    void Disconnect();
    
	/// Removes target peer from all lists, and sends a OnPeerDisconnected before deleting it. 
    void RemovePeer(Peer * peer);
    
	/// Called when a new peer connects to us
    Peer * CreatePeer();
    
	/// Network main loop. This function should be called at a regular interval.
    void ProcessNetwork();

    /// Sets name of our "me"-peer.
    void SetName(String name);

	/// Target IP for conencting, can be set via command-line arg: ip=X.X.X.X
	String targetIP;
    /// Searches for media by sending a Subscribe message to all connected peers.
  //  void SearchForMedia(String searchString);

private:
	/// Stored for convenience
	List<Game*> availableGames;
	/// Machine ip
	char machineIP[128];
	/// General status. True if WSAStartup etc succeeded.
	bool initialized;
	/// For debugs
	String lastErrorString;
	/** Removes target peer from all lists (using given socket as reference), 
		and sends a OnPeerDisconnected before deleting it. Called once a peer's sockets stop responding.
    */
    void RemovePeerBySocket(TcpSocket * socket);
  
    /// Returns number of valid peers (OK name/IP/port).
    int NumValidPeers();
    /// Returns number of registered peers (currently registered).
    int NumRegisteredPeers();
	/// Function called every time new peers have connected to the basic communication layer
	void OnNewPeersConnected(List<Peer*> newPeers);
	/// Called when peers have been disconnected.
	void OnPeersDisconnected(List<Peer*>  dcPeers);
    /// Builds and sends a SIP Subscribe message to target peer in order to automatically discover new connected peers to the network.
    void SubscribeToNewPeerConnections(Peer * peer);
    /// Builds and sends a SIP Register message to target peer.
    void RegisterWithPeer(Peer * peer);
    /** Internal function that handles post-register events, like updating UI, 
        ensuring both are mutually registered, sending subscribe-messages, etc.
        If peer is a duplicate, a merge will be performed and the mergee peer will be returned. If not, the same peer as the argument should be returned.
    */
    Peer * OnPeerRegistered(Peer * peer);
    /// Fort hostin'
    int sipPort;
	Peer* GetPeerBySocket(TcpSocket* socket);
    
    /// Uses the "name <name@ip>" from/to field to associate a peer.
    Peer * GetPeerByData(String data);
    /// Send a NOTIFY message to all previously connected peers to inform of the new peer's arrival to the network
    void InformPeersOfNewConnectedPeer(Peer * newPeer);
    /** Contrary to the above method, this one informs a single newly connected peer of the entire list of connected peers. 
        This should be sent once at the start of a subscription!
    */
    void InformPeerOfConnectedPeers(Peer * newPeer);
    /** Merges duplicated peers (issue caused by how we handle the connections & sockets). 
        If the suspectedPeer has been merged/deleted, it's mergee will be returned instead of itself.
    */
    Peer * MergeDuplicatePeers(Peer * suspectedPeer = NULL);
    /// Informs a peer of the entire list of media available to it, conforming to it's requested search-string.
    void InformPeerOfAvailableSoughtMedia(Peer * peer);

    /// For debugging
    void PrintPeerList();

    // This instance as a peer
    Peer * me;

    // List of our currently available media. Needs to be set via slot SetAvailableMedia(List<String>)
  //  List<String> availableMedia;

	/// Used for primary communication, stored here too for eased access.
	SIPSession * sipSession;

    // Connected peers
    List<Peer*> peers;
	// Networking sessions. These will probably do most work later on.
	List<Session*> sessions;
	// All servers.
	List<TcpServer*> tcpServers;
	// List of all sockets. This list might be redundant since they should be stored within the Peer-objects.
	List<TcpSocket*> sockets;
};


#endif	/* P2PMANAGER_H */

