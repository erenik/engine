/// Emil Hedemalm
/// 2014-01-28
/// Session responsible for synchronizing all data related to the Space Race game, including lobby interaction. Server-list and global chat should be handled by the SIP Session or similar.

#ifndef SPACE_RACE_SESSION_H
#define SPACE_RACE_SESSION_H

#include "Network/Session/GameSession.h"
#include "SpaceRace/SRPlayer.h"

class SRPacket;
class SRSessionData;

class SRSession : public GameSession {
public:
	SRSession(String gameName);
	/// Virtual destructor for proper deallocation.
	virtual ~SRSession();
	/// Connects to address/port.
	virtual bool ConnectTo(String ipAddress, int port, int clientUdpPort);
	/// For disconnecting from a game we are connected to (as a client/peer)
	virtual bool Disconnect();
	/// Attempts to start hosting a session of this kind. Default hosts 1 TcpServer on target port. 
	virtual bool Host(int tcpPort, int udpPort);
	/// Starts a local game.
	virtual bool HostLocalGame();

	/// Called when the host disconnects.
	virtual void OnHostDisconnected(Peer * host);

	/// For analysis, queries the same functions in all available sockets.
	virtual int BytesSentPerSecond();
	virtual int BytesReceivedPerSecond();

	int RequestedProtocol(){return requestedProtocol;};

	/// Function to process new incoming connections but also disbard old connections that are no longer active.
	virtual void EvaluateConnections();

	/// Lists all active peers in this session, including the host, except yourself!
	List<Peer*> GetPeers();

	/// Sends target packet to all peers in this session using default primary sockets, via host if possible. Will try to send via UDP for certain packet types.
	virtual void Send(Packet * packet);
	/// Reads packets, creating them and returning them for processing. Note that some packets will already be handled to some extent within the session (for exampling many SIP messages).
	virtual List<Packet*> ReadPackets();
	
	/// Queues up the packet to be sent later on.
	virtual void QueuePacket(Packet * packet);
	
	/// Posts a chat message, to be delivered both to the ChatManager and to all peers.
	void PostChatMessage(String text);

	/// Sets requested transmission protocol. 0 = TCP, 1 = UDP.
	void SetRequestedProtocol(int protocol);

	/// Sets players in this game. To be deprecated. Only call from host for now. The SRSession will take over handling of peers overall soon. TODO: REPLACE
//	virtual void SetPlayers(List<Player*> player);
	
	/// Returns target player index
	SRPlayer * GetPlayer(int index);
	/// Returns player by ID. NOTE: IDs are not synced over network yet, and thus names will be used there until further notice.
	SRPlayer * GetPlayerByID(int id);
	SRPlayer * GetPlayerByName(String name);
	/// Returns current list of active players.
	List<SRPlayer*> GetPlayers();
	/// Returns current list of local players.
	List<SRPlayer*> GetLocalPlayers();
	/// Returns current list of AI players.
	List<SRPlayer*> GetAIPlayers();
	
	/// Creates and adds a new local player with given name. If no name is given a default name will be created using the Me object and current amount of local players.
	bool AddLocalPlayer(String * namePtr = NULL);
	/// Adds a new AI player.
	bool AddAIPlayer();
	/// Removes local player of target index. If -1 the last added local player will be removed.
	bool RemoveLocalPlayer(int index = -1);
	/// Removes target AI player, if -1 the last added AI player will be removed.
	bool RemoveAIPlayer(int index = -1);
	/// Removes player by index. Only the host may invoke this function. Returns false if unable.
	bool RemovePlayerByIndex(int index);
	
	/// Sets amount of checkpoints this race will use.
	void SetCheckpoints(int amount);
	/// Returns total amount of checkpoints on this map. Default is 3 if nothing has been set (should be set by host).
	int Checkpoints();
	/// Sets amount of laps this race will use.
	void SetLaps(int amount);
	/// Returns total amount of laps for the race. Default is 3 if nothing has been set (should be set by host).
	int Laps();

	/// Sends a request packet to select this ship for our player.
	void SelectShip(int forPlayerID, String andShipName);

	/// Sets us as ready or not! Will began race once all players are readied. All settings will also be locked then.
	void SetReady(bool readyState);

	/// Set map to race on. Checks have to be made by the host before setting this to ensure it is a valid name.
	void SetMap(String name);
	String GetMap() { return mapName; };
	/// TODO: Add a function to linking references that are needed to play this map.
	
	/// Fetches session data for target peer.
	SRSessionData * GetSessionData(Peer * forPeer);
	
protected:
	/// Called once a game is joined successfully (upon reciving an OK to the SRRegister packet). Also called exactly ONCE upon successfully hosting a session.
	void OnGameJoined();

	/// For resetting appropriate variables when he disconnects, like UDP-test-statistics.
	void OnPeerDisconnected(Peer * peer);
	/// Adjusts peer ready-state.
	void OnPeerReady(Peer * peer, bool readyState);

	/// Adds a new player for target peer, checking that the maximum hasn't already been reached, in which case a false will be returned.
	bool AddPlayer(Peer * forPeer, String playerName = "NewPlayer");
	/// Removes a player for target peer. Returns false if it failed or the peer already has 0 players.
	bool RemovePlayer(Peer * forPeer);
	/// Sets amount of players to be used. Will delete and create a new array if needed. Should be called by host.
	void SetAmountOfPlayers(int amount);
	/// Called every time a socket is deleted. Any references to the socket should then be removed.
	virtual void OnSocketDeleted(Socket * sock);
	/// Called every time the players are updated, at which new packets will be sent to all clients.
	void OnPlayersUpdated();
	/// Fetches sender for target packet, using name within to look it up.
	Peer * GetSender(SRPacket * );
	/// Opens main udp socket to peer, storing it in the peer's SRSessionData.
	bool OpenUdpSocketToPeer(Peer * peer);
	/// For handling udp-packets.
	Peer * GetSenderForPacket(SRPacket * pack);

	/// Locks game settings, informing peers of the change so that they may adjust any UI as well.
	void LockGameSettings(bool lock);

private:
	/// If true (default), will try and request 1 local player automatically when joining a game.
	bool autoAddLocalPlayer;

	/// Default port to send to for udp. This will be either 33002 for sending to a host or 33003 for sending to a client/peer.
	int targetUdpPort;

	/// Will be set to NULL when not set.
	long long timeToEnterRacingState;
	long long timeToStartGame;

	/// Set when all peers are readied. No more players may be entered after this, nor other settings changed. Will be reset after the host leaves to the lobby again.
	bool gameSettingsLocked;

	/// Synchronize time if client.
	bool timeSynchronized;
	/// If local only
	bool localGameHosted;
	/// Level!
	String mapName;
	/// Checkpoints in race level.
	int checkpoints;
	int laps;
	/// For internal processing and data gathering before sending it off to the main game for handling.
	void ProcessPacket(SRPacket * p);

	/// For regulating on a per-peer basis. Default should be 4?
	int maxPlayersPerPeer;

	/// Woo.
	List<Packet*> packetQueue;
	List<SRPlayer*> players;

	/// For initial udp test to host.
	bool udpTested;
	/// Client side requested protocol. 0 = TCP, 1 = UDP.
	int requestedProtocol;
};

#endif