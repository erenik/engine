/// Emil Hedemalm
/// 2014-01-28
/// Session responsible for synchronizing all data related to the Space Race game, including lobby interaction. Server-list and global chat should be handled by the SIP Session or similar.

#include "RRSession.h"
#include "Network/NetworkSettings.h"
#include "RRPacket.h"
#include "RRPacketTypes.h"
#include "Network/Server/TcpServer.h"
#include "Network/NetworkManager.h"
#include "Network/Peer.h"
#include "Network/Socket/Socket.h"
#include "Message/MessageManager.h"
#include "Message/Message.h"
#include "Message/MessageTypes.h"
#include "RRSessionData.h"
#include "Network/Session/SessionTypes.h"
#include "Network/Session/GameSessionTypes.h"
#include "Chat/ChatManager.h"
#include "Input/InputDevices.h"
#include "Input/InputManager.h"
#include "Network/Socket/UdpSocket.h"
#include "String/StringUtil.h"

RRSession::RRSession(String playerName, String sessionName)
: GameSession(sessionName, RR_DEFAULT_SESSION_NAME, 4, playerName)
{
	RRPacket::defaultSender = me;
	this->subType = GameSessionType::RUNE_RPG;
	maxPlayersPerPeer = 1;
	maxPlayers = 4;
	localOnly = false;
	timeSynchronized = false;
	udpTested = false;
	requestedProtocol = TransmissionProtocol::TCP;
	gameSettingsLocked = false;
	timeToStartGame = 0;
	autoAddLocalPlayer = true;
	targetUdpPort = 0;
}

/// Virtual destructor for proper deallocation.
RRSession::~RRSession()
{
	// Delete stuff... please?
}

/// Connects to address/port.
bool RRSession::ConnectTo(String ipAddress, int port, int clientUdpPort /* = -1*/)
{
	timeSynchronized = false;
	bool success = GameSession::ConnectTo(ipAddress, port, clientUdpPort);
	if (success)
	{
		localOnly = false;
		// Send a new register-message so that they may associate our peer-counterpart.
		SRRegister reg(me);
		reg.Send(hostSocket);
		targetUdpPort = 33002;
		return true;
	}
	std::cout<<"\nRRSession::ConnectTo failed to connect to host.";
	return false;
}

/// For disconnecting from a game we are connected to (as a client/peer)
bool RRSession::Disconnect()
{
	/// Close socket to host.
	if (this->hostSocket){
		hostSocket->Close();
		delete hostSocket;
		hostSocket = NULL;
	}
	/// Close UDP socket.
	if (udpSocket)
	{
		udpSocket->Close();
		delete udpSocket;
		udpSocket = NULL;
	}
	/// Reset values.
	isLocal = true;
	isHost = true;
	return true;
}

/// Attempts to start hosting a session of this kind.
bool RRSession::Host(int port, int udpPort)
{
	bool result;
	/// Just use a standard tcpServer for now.
	result = GameSession::Host(port, udpPort);
	if (result){
		localGameHosted = false;
		/// host successful, process initial stuff like adding 1 local player
		OnGameJoined();
		targetUdpPort = 33003;
	}
	/// Create and bind the udp socket.
	return result;
}

/// Starts a local game.
bool RRSession::HostLocalGame()
{
	// If currently in a networked game, stop it first.
	if (!isLocal)
	{

	}
	MesMan.QueueMessages("LocalGameHosted");
	isHost = true;
	localGameHosted = true;
	isLocal = true;
	/// host successful, process initial stuff like adding 1 local player
	OnGameJoined();
	return true;
}

/// Returns tcp port used to host this session, or default target port if the session has not hosted yet.
int RRSession::TcpPort() 
{ 
	if (tcpServer)
		return this->tcpServer->Port(); 
	return RR_DEFAULT_PORT;
};


/// Called when the host disconnects.
void RRSession::OnHostDisconnected(Peer * host)
{
	ChatMan.PostGeneral("Host Disconnected");
	MesMan.QueueMessages("OnHostDisconnected");
}

/// For analysis, queries the same functions in all available sockets.
int RRSession::BytesSentPerSecond()
{
	int total = Session::BytesSentPerSecond();
	if (udpSocket)
		total += udpSocket->BytesSentPerSecond();
	return total;
}
int RRSession::BytesReceivedPerSecond()
{
	int total = Session::BytesReceivedPerSecond();
	if (udpSocket)
		total += udpSocket->BytesReceivedPerSecond();
	return total;
}

/// Function to process new incoming connections but also disbard old connections that are no longer active.
void RRSession::EvaluateConnections(){
	// If hosting, check for new connections
	if (this->tcpServer){
		assert(this->tcpServer);
		Socket * newSocket = this->tcpServer->NextPendingConnection();
		/// Check ip of the new socket, does it coincide with any existing peer?
		if (newSocket)
		{
			sockets.Add(newSocket);
			std::cout<<"New connection in RRSession.";
	    }
	}

	/// Deletes those sockets that either have errors or have been flagged for deletion (other errors)
	DeleteFlaggedSockets();

	/// Test udp
	List<Peer*> totalPeers = peers;
	/// Add host to the list of peers we want to test udp with.
	if (host)
		totalPeers.Add(host);
	/// Remove ourself if we're in there.
	totalPeers.Remove(me);
	for (int i = 0; i < peers.Size(); ++i){
		Peer * peer = peers[i];
		RRSessionData * srsd = GetSessionData(peers[i]);
		/// Don't process the peer if their connection is down.
		if (isHost && srsd->connectionState != CONNECTED)
			continue;
		/// If we doesn't have a UdpSocket, don't handle it further.
		if (!udpSocket)
		{
			continue;
		}
		// Skip if already working.
		if (srsd->udpWorking)
			continue;
		/// Set it to true if it's working for us both!
		if (srsd->udpWorkingForUs && srsd->udpWorkingForPeer){
			srsd->udpWorking = true;
			ChatMan.PostGeneral("Udp tested and working for peer: "+peer->name);
		}
		/// If not, test more.
		else {
			/// Once Udp is working for us, take note of it and inform our peer of it.
			if (srsd->udpTestPacketsSent > 0 && srsd->udpTestPacketsReceived > 0
				&& srsd->udpWorkingPacketsSent  < 10000){
				// Ok, ok, its working yaow?
				srsd->udpWorkingForUs = true;
				srsd->udpWorkingPacketsSent++;
				RRGeneralPacket udpTest("UdpWorking");
				udpTest.SendUdp(peer, udpSocket, targetUdpPort);
			}
			// Check if working for peer.
			if (srsd->udpWorkingPacketsReceived > 0){
				srsd->udpWorkingForPeer = true;
			}
			/// If not, spam him some more.
			else if (srsd->udpTestPacketsSent < 10000){
				/// Send packets to host with it.
				RRGeneralPacket udpTest("UdpTest");
				/// Ports: 33000 SIP, 33001 SR TCP, 33002, SR Host UDP, 33003 SR Client UDP
				udpTest.SendUdp(peer, udpSocket, targetUdpPort);
				/// Take note of how many packets we send with "UdpTest" to host.
				srsd->udpTestPacketsSent++;
			}
		}

	}

}

/// Lists all active peers in this session.
List<Peer*> RRSession::GetPeers()
{
	List<Peer*> peerList = peers;
	peerList.Remove(me);
	return peerList;
}

/// Sends target packet to all peers in this session using default targetsm, via host if possible.
void RRSession::Send(Packet * basePacket)
{
	RRPacket * packet = (RRPacket *) basePacket;
	bool udpPreferred = false;
	switch(packet->srPacketType){
		case RRPacketType::PLAYER_MOVE:
		case RRPacketType::PLAYER_POSITION:
			udpPreferred = true;
	}
	/// If host
	if (isHost){
		for (int i = 0; i < peers.Size(); ++i){
			Peer * peer = peers[i];
			RRSessionData * srsd = GetSessionData(peer);
			if (srsd->connectionState == DISCONNECTED)
				continue;
			/// Check if udp is enabled in the session. If so use it.
			if (udpPreferred && srsd->requestedProtocol == TransmissionProtocol::UDP && srsd->udpWorking){
				packet->SendUdp(peer, udpSocket, 33003);
			}
			else {
				packet->Send(peer);
			}
		}
	}
	/// If client
	else {
		if (udpPreferred && requestedProtocol == TransmissionProtocol::UDP && this->udpTested)
			packet->SendUdp(host, udpSocket, 33002);
		else
			packet->Send(host);
	}
/*		else {
			Socket * socket = hostSocket;
			packet->Send(socket);
		}
	}
*/
}

/// Reads packets, creating them and returning them for processing. Note that some packets will already be handled to some extent within the session (for exampling many SIP messages).
List<Packet*> RRSession::ReadPackets()
{
	List<Packet*> packets;
	List<RRPacket*> srPackets;
	/// Read packets from host
	if (this->hostSocket){
		int resu = RRPacket::ReadPackets(hostSocket, srPackets);
		/// Process some straight away?
		for (int j = 0; j < srPackets.Size(); ++j){
			RRPacket * srp = srPackets[j];
			ProcessPacket(srp);
			packets.Add(srp);
		}
	}

	/// Read packets from peers/clients
	for (int i = 0; i < sockets.Size(); ++i){
		Socket * sock = sockets[i];
		/// Parse the data straight away.
		srPackets.Clear();
		int resu = RRPacket::ReadPackets(sock, srPackets);
		/// Process some straight away?
		for (int j = 0; j < srPackets.Size(); ++j){
			RRPacket * srp = srPackets[j];
			ProcessPacket(srp);
			packets.Add(srp);
		}
	}
	return packets;
}

/// Queues up the packet to be sent later on.
void RRSession::QueuePacket(Packet * packet){
	assert(false);
	return;
}


void RRSession::PostChatMessage(String message)
{
	ChatMan.AddMessage(new ChatMessage(NetworkMan.Me(), message));
	/// Send a chat message to the game session if open.
	RRChatPacket * srcp = new RRChatPacket(name, message);
	Send(srcp);
	delete srcp;
}

/// Sets requested transmission protocol. 0 = TCP, 1 = UDP.
void RRSession::SetRequestedProtocol(int protocol)
{
	if (isHost)
		return;
	requestedProtocol = protocol;
	RRGeneralPacket pack("SetProtocol", String::ToString(protocol));
	this->Send(&pack);
}

/// Returns target player index
RRPlayer * RRSession::GetPlayer(int index){
	assert(players.Size());
	if (players.Size())
		return players[index];
	return NULL;
}

/// Returns player by ID. NOTE: IDs are not synced over network yet, and thus names will be used there until further notice.
RRPlayer * RRSession::GetPlayerByID(int id)
{
	for (int i = 0; i < players.Size(); ++i){
		RRPlayer * player = players[i];
		if (player->ID() == id)
			return player;
	}
	return NULL;
}

RRPlayer * RRSession::GetPlayerByName(String name){
	for (int i = 0; i < players.Size(); ++i){
		RRPlayer * player = players[i];
		if (player->name == name)
			return player;
	}
	return NULL;
}

/// Returns current list of active players.
List<RRPlayer*> RRSession::GetPlayers()
{
	return players;
}

/// Returns current list of local players.
List<RRPlayer*> RRSession::GetLocalPlayers()
{
	List<RRPlayer*> localPlayerList;
	for (int i = 0; i < players.Size(); ++i){
		RRPlayer * srp = players[i];
		if (srp->isLocal && !srp->isAI)
			localPlayerList.Add(srp);
	}
	return localPlayerList;
}

/// Returns current list of AI players.
List<RRPlayer*> RRSession::GetAIPlayers()
{
	List<RRPlayer*> localPlayerList;
	for (int i = 0; i < players.Size(); ++i){
		RRPlayer * srp = players[i];
		if (srp->isAI)
			localPlayerList.Add(srp);
	}
	return localPlayerList;
}


/// Creates and adds a new local player with given name. If no name is given a default name will be created using the Me object and current amount of local players.
bool RRSession::AddLocalPlayer(String * namePtr /*= NULL */)
{
	if (gameSettingsLocked)
		return false;
    if (players.Size() >= maxPlayers)
        return false;
	String name;
	if (namePtr)
		name = *namePtr;
	else {
		name = me->name + String::ToString(GetLocalPlayers().Size());
	}
	/// If host, just create it straight away.
	if (isHost){
		// Check first that we got a valid input-device for this player.
		int inputDevice = Input.GetNextAvailableInputDevice();
		if (inputDevice == InputDevice::INVALID_DEVICE)
			return false;
		int currentlyControlledPlayers = GetSessionData(me)->players.Size();
		if (currentlyControlledPlayers >= maxPlayersPerPeer)
			return false;
		RRPlayer * p = new RRPlayer();
		p->name = name;
		p->owner = this->me;
		p->isLocal = true;
		p->isAI = false;
		p->inputDevice = inputDevice;
		Input.SetInputDeviceAvailability(inputDevice, true);
		/// Add it to the list too...
		this->players.Add(p);
		GetSessionData(me)->players.Add(p);
		// Notify of the update
		this->OnPlayersUpdated();
	}
	/// If client, send request to host.
	else {
		SRRequestPlayersPacket srp(SRRequestPlayersPacket::ADD, 1, name);
		this->Send(&srp);
	}
}

/// Adds a new AI player.
bool RRSession::AddAIPlayer()
{
	if (gameSettingsLocked)
		return false;
    if (players.Size() >= maxPlayers)
        return false;
	String name;
//	if (namePtr)
//		name = *namePtr;
//	else {
		name = "AI" + String::ToString(GetAIPlayers().Size());
//	}
	/// If host, just create it straight away.
	if (isHost){
		// Check first that we got a valid input-device for this player.
		RRPlayer * p = new RRPlayer();
		p->name = name;
		p->owner = this->me;
		p->isLocal = true;
		p->isAI = true;
		/// Disable input for AIs
		p->inputDevice = InputDevice::INVALID_DEVICE;
		/// Add it to the list too...
		this->players.Add(p);
		// Notify of the update
		this->OnPlayersUpdated();
	}
	/// If client, send request to host.
	else {
		assert(false && "Disable adding AI for clients? Ignore this assertion if you encounter it.");
		return false;
	}
	return true;
}

/// Removes local player of target index. If -1 the last added local player will be removed.
bool RRSession::RemoveLocalPlayer(int index /* = -1 */)
{
	if (gameSettingsLocked)
		return false;
	/// If host, just create it straight away.
	if (isHost){
		RRPlayer * playerToRemove = NULL;
		/// Remove last local one we find.
		if (index == -1){
			for (int i = players.Size()-1; i >= 0; --i){
				RRPlayer * p = players[i];
				if (!p->isLocal)
					continue;
				if (p->isAI)
					continue;
				playerToRemove = p;
				break;
			}
		}
		else
			playerToRemove = players[index];
		/// Woo o-o
		if (playerToRemove){
			/// Remove references from both players-list and session data of owner peer.
			players.Remove(playerToRemove);
			RRSessionData * sd = GetSessionData(playerToRemove->owner);
			if (sd)
				sd->players.Remove(playerToRemove);
			/// Delete it.
			Input.SetInputDeviceAvailability(playerToRemove->inputDevice, false);
			delete playerToRemove;
			/// Inform of changes
			OnPlayersUpdated();
		}
	}
	/// If client, send request to host.
	else {
		SRRequestPlayersPacket srp(SRRequestPlayersPacket::REMOVE, 1, name);
		this->Send(&srp);
	}
	return true;
}

/// Removes target AI player, if -1 the last added AI player will be removed.
bool RRSession::RemoveAIPlayer(int index /*= -1*/)
{
	if (gameSettingsLocked)
		return false;
	/// If host, just create it straight away.
	if (isHost){
		RRPlayer * playerToRemove = NULL;
		List<RRPlayer*> aiPlayers = GetAIPlayers();
		if (aiPlayers.Size()){
			/// Remove last local one we find.
			if (index == -1){
				playerToRemove = aiPlayers.Last();
			}
			else
				playerToRemove = players[index];
		}
		/// Woo o-o
		if (playerToRemove){
			/// Remove references from both players-list and session data of owner peer.
			players.Remove(playerToRemove);
			RRSessionData * sd = GetSessionData(playerToRemove->owner);
			if (sd)
				sd->players.Remove(playerToRemove);
			/// Delete it.
			Input.SetInputDeviceAvailability(playerToRemove->inputDevice, false);
			delete playerToRemove;
			/// Inform of changes
			OnPlayersUpdated();
		}
	}
	/// If client, send request to host?
	else {
	}
	return true;
}

/// Removes player by index. Only the host may invoke this function.
bool RRSession::RemovePlayerByIndex(int index)
{
	if (gameSettingsLocked)
		return false;
	if (!isHost)
		return false;
	if (index >= players.Size() || index < 0)
	{
		std::cout<<"\nUnable to remove player, index invalid.";
		return false;
	}
	RRPlayer * player = players[index];
	/// If local, make input device free again.
	if (player->isLocal){
		Input.SetInputDeviceAvailability(player->inputDevice, false);
	}
	players.Remove(player);
	RRSessionData * srsd = GetSessionData(player->owner);
	if (srsd){
		srsd->players.Remove(player);
	}
	/// Notify relevant parties of the change.
	OnPlayersUpdated();
	return true;
}

// Sends a message to the peers of the new state. 
void RRSession::ToggleReady()
{
	/// If client o-o
	if (!isHost)
	{
		RRGeneralPacket readyPack("ToggleReadyState");
		Send(&readyPack);
		return;
	}

	RRSessionData * rr = this->GetSessionData(me);
	if (!rr->players.Size())
		return;
	RRPlayer * player = rr->players[0];
	bool readyState = player->isReady = !player->isReady;
	/// Send ready state notification to host.
	/// Just set it.
	{
		OnPeerReady(me, readyState);
	}
}

/// Sets us as ready or not! Will began race once all players are readied. All settings will also be locked then.
void RRSession::SetReady(bool readyState)
{
	/// If not host, just stuff.
	if (!isHost){
		RRGeneralPacket readyPack("SetReadyState", readyState? "true" : "false");
		Send(&readyPack);
	}
	else {
		OnPeerReady(me, readyState);
	}
	/// Request to set my ready state.
	MesMan.QueueMessages(String("ReadyStateSetTo(")+(readyState? "true" : "false")+")");
}

/// Called once a game is joined successfully (upon reciving an OK to the SRRegister packet).
void RRSession::OnGameJoined(){
	if (this->autoAddLocalPlayer)
	{
		this->AddLocalPlayer();
	}
}

/// For resetting appropriate variables when he disconnects, like UDP-test-statistics.
void RRSession::OnPeerDisconnected(Peer * peer)
{
	/// Set stuff for the peer.. if relevant.
	RRSessionData * srsd = GetSessionData(peer);
	if (!srsd)
		return;
	srsd->udpWorking = false;
	srsd->udpTestPacketsReceived = 0;
	srsd->udpTestPacketsSent = 0;
	srsd->udpTestReceivedPacketsReceived = 0;
	srsd->udpWorkingForPeer = false;
	srsd->udpWorkingForUs = false;
	srsd->udpWorkingPacketsSent = 0;
	srsd->udpWorkingPacketsReceived = 0;
}

/// Adjusts peer ready-state.
void RRSession::OnPeerReady(Peer * readyPeer, bool readyState)
{
	if (!isHost)
		return;
	RRSessionData * srsd = GetSessionData(readyPeer);
	if (!srsd)
		return;
	srsd->ready = readyState;

	/// Set ready flag for all players controlled by the peer.
	for (int i = 0; i < srsd->players.Size(); ++i)
	{
		RRPlayer * player = srsd->players[i];
		player->isReady = readyState;
	}

	// Notify peers of current ready-states.
	List<String> playersReady;
	for (int i = 0; i < players.Size(); ++i)
	{
		RRPlayer * player = players[i];
		if (player->isReady)
			playersReady.Add(String::ToString(i));
	}
	RRGeneralPacket readyPack("PlayersReady", MergeLines(playersReady,","));
	Send(&readyPack);

	List<String> peersNotReady;

	/// Check if all players are ready.
	for (int i = 0; i < peers.Size(); ++i){
		Peer * peer = peers[i];
		srsd = GetSessionData(peer);
		/// Skip peers with no players.
		if (!srsd->players.Size())
			continue;
		if (srsd->ready){
			continue;
		}
		peersNotReady.Add(peer->name);
	}
	/// Include ourselves?
	srsd = GetSessionData(me);
	if (!srsd->ready)
		peersNotReady.Add(me->name);

	// Update ui.
	MesMan.QueueMessages("PeerReadyStatesUpdated");

	/// Notify how many peers are still not ready.
	if (peersNotReady.Size()){
		this->LockGameSettings(false);
		String msg = "PeersStillNotReady";
		String peerListString = MergeLines(peersNotReady,";");
		RRGeneralPacket info(msg, peerListString);
		Send(&info);

		/// Inform host as well.
		MesMan.QueueMessages(msg+"("+peerListString+")");
		return;
	}
	/// Lock and prepare for game start!
	this->LockGameSettings(true);
	/// And move to racing state in 3 seconds unless noted otherwise.
	String msg = "MoveToRacingState";
	String timeString = "3";
	RRGeneralPacket start(msg, timeString);
	Send(&start);
	MesMan.QueueMessages(msg+"("+timeString+")");
}

/// Adds a new player for target peer, checking that the maximum hasn't already been reached, in which case a false will be returned.
bool RRSession::AddPlayer(Peer * forPeer, String playerName /*= "NewPlayer" */)
{
	if (gameSettingsLocked)
		return false;
    if (players.Size() >= maxPlayers)
        return false;
	RRSessionData * srsd = GetSessionData(forPeer);
	assert(srsd);
	/// Check if number of players belonging to this peer already has reached maximum per peer.
	if (srsd->players.Size() >= maxPlayersPerPeer)
		return false;

	String baseName = playerName;
	int enumerator = 0;
	/// If necessary, adjust name.
	while(GetPlayerByName(playerName))
	{
		playerName = baseName + String::ToString(enumerator);
		enumerator++;
	}

	RRPlayer * p = new RRPlayer();
	p->owner = forPeer;
	p->name = playerName;
	p->isLocal = false;
	p->isAI = false;
	players.Add(p);
	/// Add the player to the list of players this peer controls.
	srsd->players.Add(p);
	/// Call updater function to notify peers and GUI et al.
	OnPlayersUpdated();
	return true;
}

/// Removes a player for target peer. Returns false if it failed or the peer already has 0 players.
bool RRSession::RemovePlayer(Peer * forPeer)
{
	if (gameSettingsLocked)
		return false;
	RRSessionData * srsd = (RRSessionData*)forPeer->GetSessionData(SessionType::GAME);
	/// Check if number of players belonging to this peer already has reached maximum per peer.
	if (srsd->players.Size() == 0)
		return false;
	/// Find last index player
	RRPlayer * srp = srsd->players[srsd->players.Size()-1];
	/// Remove both from session-data list as well as session player list.
	srsd->players.Remove(srp);
	players.Remove(srp);
	/// Delete it.
	delete srp;
	/// Call updater function to notify peers and GUI et al.
	OnPlayersUpdated();
	return true;

}


/// Sets amount of players to be used. Will delete and create a new array if needed. Should be called by host.
void RRSession::SetAmountOfPlayers(int amount)
{
	if (gameSettingsLocked)
		return;
	this->players.ClearAndDelete();
	for (int i = 0; i < amount; ++i){
		RRPlayer * newP = new RRPlayer();
		players.Add(newP);
	}
}

/// Called every time a socket is deleted. Any references to the socket should then be removed.
void RRSession::OnSocketDeleted(Socket * sock)
{
	for (int i = 0; i < peers.Size(); ++i)
	{
		Peer * p = peers[i];
		RRSessionData * srsd = GetSessionData(p);
		/// Peer TCP socket was deleted. Mark peer as currently disconnected.
		if (srsd->srSocket == sock){
			srsd->srSocket = NULL;
			srsd->connectionState = DISCONNECTED;
			OnPeerDisconnected(p);
		}
	}
	/// Remove it from any references in peers here.
	Session::OnSocketDeleted(sock);
}

/// Called every time the players are updated, at which new packets will be sent to all clients.
void RRSession::OnPlayersUpdated()
{
	/// If host, notify clients of the new setup
	if (isHost){
		/// Get number
		int numPlayers = players.Size();
		/// Get names
		List<String> playerNames;
		List<String> playerShips;
		for (int i = 0; i < players.Size(); ++i){
			RRPlayer * player = players[i];
			// Player name and ship-name.
			playerNames.Add(player->name);
		}
		/// For each peer
		for (int i = 0; i < this->peers.Size(); ++i){
			Peer * peer = peers[i];
			/// Find which players belong to this peer.
			List<int> playerIndices;
			for (int j = 0; j < players.Size(); ++j){
				Player * p = players[j];
				if (p->owner == peer)
					playerIndices.Add(j);
			}
			/// And send a packet to it.
			RRPlayersPacket playersPacket(players.Size(), playerNames, playerIndices);
			/// Send it to all clients of the game.
			playersPacket.Send(peer);
		}
	}
	/// Notify relevant game states that the players-list has been updated too.
	MesMan.QueueMessages("OnPlayersUpdated");
}


/// Fetches sender for target packet, using name within to look it up.
Peer * RRSession::GetSender(RRPacket * packet)
{
	/// Check for a peer with similar name/stuff.
	String sender = packet->GetSender();
	Peer * peer = NetworkMan.GetPeerByName(sender);
	return peer;
}

/// Fetches session data for target peer.
RRSessionData * RRSession::GetSessionData(Peer * forPeer)
{
	RRSessionData * srSessionData = (RRSessionData*)forPeer->GetSessionData(SessionType::GAME, GameSessionType::SPACE_RACE);
	if (!srSessionData){
		srSessionData = new RRSessionData();
		forPeer->sessionData.Add(srSessionData);
		srSessionData = (RRSessionData*)forPeer->GetSessionData(SessionType::GAME, GameSessionType::SPACE_RACE);
		assert(srSessionData);
	}
	return srSessionData;
}

/// Opens main udp socket to peer, storing it in the peer's RRSessionData.
bool RRSession::OpenUdpSocketToPeer(Peer * peer)
{

/*	RRSessionData * srSessionData = GetSessionData(peer);
	UdpSocket * udpSocket = new UdpSocket();
	udpSocket->ConnectTo(peer->ipAddress, 33002);
	srSessionData->udpSocket = udpSocket;
	*/
	return false;
}

/// For handling udp-packets.
Peer * RRSession::GetSenderForPacket(RRPacket * pack)
{
	for (int i = 0; i < peers.Size(); ++i){
		String senderName = pack->GetSender();
		Peer * peer = peers[i];
		if (senderName == peer->name){
			pack->sender = peer;
			return peer;
		}
	}
	return NULL;
}

/// Locks game settings, informing peers of the change so that they may adjust any UI as well.
void RRSession::LockGameSettings(bool lock)
{
	if (!isHost)
		return;
	/// If already has the desired setting, don't do same update procedure again.
	if (gameSettingsLocked == lock)
		return;
	gameSettingsLocked = lock;

	String msg;
	if (gameSettingsLocked){
		msg = "GameSettingsLocked";
	}
	else {
		msg = "GameSettingsUnlocked";
	}
	/// Send a packet.
	RRGeneralPacket srPack(msg);
	Send(&srPack);
	// Notify UI and game state.
	MesMan.QueueMessages(msg);
}

Peer * RRSession::GetPeerByName(String name)
{
	for (int i = 0;i < peers.Size(); ++i)
	{
		Peer * p= peers[i];
		if (p->name == name)
			return p;
	}
	return NULL;
}

Peer * RRSession::CreatePeerForPacket(RRPacket * rr)
{
	Peer * peer = new Peer();
	peer->sockets.Add(rr->socket);
	String baseName = rr->GetSender();
	peer->name = baseName;
	// Set unique peer-name.
	int i = 1;
	while(GetPeerByName(peer->name))
	{
		++i;
		peer->name = baseName + String::ToString(i);
	}
	peer->primaryCommunicationSocket = rr->socket;
	NetworkMan.AddPeer(peer);
	return peer;
}


/// For internal processing and data gathering before sending it off to the main game for handling.
void RRSession::ProcessPacket(RRPacket * packet)
{
	/// Find which peer sent it if possible
	if (packet->sender == NULL)
		packet->sender = packet->socket->peer;

	/// Upon receiving the first few packets, synchronize time (if we are clients)
	if (!IsHost() && !timeSynchronized){
		/// Fetch time stamp
		long long serverTime = packet->timeCreated;
		/// Check difference with local time.
		long long cTime = Timer::GetCurrentTimeMs(true);
		long long timeDiff = serverTime - cTime;
		Timer::SetAdjustment(timeDiff);
		long long newCTime = Timer::GetCurrentTimeMs();
		timeSynchronized = true;
	}


//	std::cout<<"\nRRSession::ProcessPacket for packet: "<<packet->GetPacketName(packet->srPacketType);
	switch(packet->srPacketType)
	{
		case RRPacketType::CHAT:
		{
			RRChatPacket * chatPack = (RRChatPacket*) packet;
			/// Got chat packet, allright. So if we are host, we want to re-distribute this to all other peers except the one that sent it.
			for (int i = 0; i < peers.Size(); ++i)
			{
				/// Check if sender
				Peer * peer = peers[i];
				if (packet->sender == peer)
					continue;
				packet->Send(peer);
			}
			// Add it to the chat-manager too.
			ChatMan.AddMessage(new ChatMessage(packet->sender, chatPack->GetText()));
			break;
		}
		/// For race-specific stuff
		case RRPacketType::RACE:
		{
			String msg, additionalData;
			RRGeneralPacket * rp = (RRGeneralPacket*) packet;
			rp->Parse(msg, additionalData);
			if (msg == "SetCheckpoints"){
				checkpoints = additionalData.ParseInt();
				MesMan.QueueMessages("OnCheckpointsUpdated");
			}
			else if (msg == "SetProtocol"){
				int protocol = additionalData.ParseInt();
				RRSessionData * srsd = GetSessionData(packet->sender);
				srsd->requestedProtocol = protocol;
			}
			else if (msg == "SetLaps"){
				laps = additionalData.ParseInt();
				MesMan.QueueMessages("OnLapsUpdated");
			}
			else if (msg == "UdpTest"){
				/// Take not of the retrieval.
				RRSessionData * srsd = GetSessionData(packet->sender);
				srsd->udpTestPacketsReceived++;
			}
			// Client confirms it received our packet too.
			else if (msg == "UdpWorking"){
				RRSessionData * srsd = GetSessionData(packet->sender);
				srsd->udpWorkingPacketsReceived++;
			}
			/// Host setting map to play on.
			else if (msg == "SetMap"){
				/// Failsafe so clients can't set map.
				if (isHost)
					return;
				this->mapName = additionalData;
				MesMan.QueueMessages("MapSetTo("+additionalData+")");
			}
			// Client requesting a ship!
			else if (msg == "ToggleReadyState")
			{
				// Previous state
				bool wasReady = GetSessionData(packet->sender)->players[0]->isReady;
				this->OnPeerReady(packet->sender, !wasReady);
			}
			else if (msg == "SetReadyState"){
				bool readyState = additionalData == "true";
				this->OnPeerReady(packet->sender, readyState);
			}
			else if (msg == "PlayersReady")
			{
				List<String> playersReady = additionalData.Tokenize(",");
				for (int i = 0; i < players.Size(); ++i)
				{
					RRPlayer * player = players[i];
					player->isReady = false;
				}
				for (int i = 0; i < playersReady.Size(); ++i)
				{
					int index = playersReady[i].ParseInt();
					players[index]->isReady = true;
				}
				MesMan.QueueMessages("PeerReadyStatesUpdated");
			}
			else if (msg == "MoveToRacingState"){
				MesMan.QueueMessages("MoveToRacingState("+additionalData+")");
			}
			else if (msg == "PeersStillNotReady"){
				MesMan.QueueMessages(msg+"("+additionalData+")");
			}
			else if (msg == "GameSettingsLocked"){
				this->gameSettingsLocked = true;
				MesMan.QueueMessages(msg);
			}
			else if (msg == "GameSettingsUnlocked"){
				this->gameSettingsLocked = false;
				MesMan.QueueMessages(msg);
			}
			else {
				std::cout<<"\nUnhandled RRGeneralPacket with header: "<<msg;
				std::cout<<"\nSuggest breakpoint and invhestigate!";
			}
			break;
		}
		/// For when clients request adjustments to the amount of players they want to be in control over.
		case RRPacketType::REQUEST_PLAYERS:
		{
			assert(isHost && "Only host should receive this packet");
			SRRequestPlayersPacket * rpp = (SRRequestPlayersPacket*)packet;
			int action, amount;
			List<String> names;
			rpp->Parse(action, amount, names);
			/// Find peer for packet..
			Peer * sender = packet->sender;
			assert(sender);
			// Check players already controlled by the peer.
			int playersAlreadyControlledByPeer = GetSessionData(sender)->players.Size();
			if (playersAlreadyControlledByPeer >= maxPlayersPerPeer)
				return;
			/// Check type of action peer requested.
			if (action == SRRequestPlayersPacket::ADD){
				if (names.Size())
					AddPlayer(sender, names[0]);
				else
					AddPlayer(sender, sender->name);
			}
			else if (action == SRRequestPlayersPacket::REMOVE)
				RemovePlayer(sender);

			break;
		}
		/// For handling new player list. On client-side we parse the players and set names here automatically, informing the relevant states via a message afterward once the players have changed.
		case RRPacketType::PLAYERS:
		{
			/// Only handle this if received as a client.
			if (this->isHost){
				std::cout<<"\nReceiving RRPlayersPacket despite being host. What gives?";
				return;
			}
			RRPlayersPacket * playersPacket = (RRPlayersPacket*)packet;
			int numPlayers;
			List<String> playerNames;
			List<String> playerShips;
			List<int> clientPlayerIndices;
			playersPacket->Parse(numPlayers, playerNames, clientPlayerIndices);

			/// Check if we lack any ships, and if so request the necessary data from the host?
			/// TODO:

			SetAmountOfPlayers(numPlayers);
			/// Set names and default locality -> not local
			for (int i = 0; i < playerNames.Size(); ++i){
				players[i]->name = playerNames[i];
				players[i]->isLocal = false;
			}
			/// Set which players are to be local for us.
			for (int i = 0; i < clientPlayerIndices.Size(); ++i){
				players[clientPlayerIndices[i]]->isLocal = true;
			}
			/// Notify stuff
			OnPlayersUpdated();
			break;
		}
		case RRPacketType::OK:
		{
			SROKPacket * ok = (SROKPacket*)packet;
			String request = ok->GetRequest();
			Message * message = new Message(MessageType::STRING);
			if (request == RRPacket::GetPacketName(RRPacketType::REGISTER)){
				/// Upon registration, try and identify the peer behind the server, make sure it's the right one as the one in SIP earlier?
				String sender = ok->GetSender();
				Peer * peer = NetworkMan.GetPeerByName(sender);
				// Unknown peer? Create it.
				if (!peer)
				{
					peer = CreatePeerForPacket(ok);
				}
				if (peer){
					/// Set peer of the socket.
					packet->socket->peer = peer;
					/// Add the peer to this session too, then.
					if (!peers.Exists(peer))
						peers.Add(peer);
					message->msg = "JoinRequestAccepted("+RR_DEFAULT_SESSION_NAME+")";
					/// Save it in the host-variable too, then!
					host = peer;
					host->ipAddress = NetworkMan.targetIP;
					std::cout<<"\nHost IP: "<<host->ipAddress;
					assert(host->ipAddress.Length());
					/// Generate RRSessionData if not available.
					RRSessionData * srsd = GetSessionData(host);
					srsd->srSocket = packet->socket;
				}
				else {
					assert(false && "Host of game not found among registered peers? Could be another person hosting the game than was previously specified.");
					message->msg = "JoinRequestDoubting";
				}
				/// After RegisterOK also update player list and inform new peers.
				OnPlayersUpdated();
				// Alright! We are registerd, do post-join-acceptal thingies, like requesting an initial local player.
				OnGameJoined();
				ChatMan.PostGeneral("Game joined. ");
			}
			message->data = (char*)this;
			MesMan.QueueMessage(message);
			break;
		}
		case RRPacketType::DECLINE:
		{
			SRDeclinePacket * dec = (SRDeclinePacket*)packet;
			String request = dec->GetRequest();
			String reason = dec->GetReason();
			Message * message = new Message(MessageType::STRING);
			if (request == RRPacket::GetPacketName(RRPacketType::REGISTER))
			{
				message->msg = "JoinRequestRefused("+RR_DEFAULT_SESSION_NAME+");Reason: "+reason;
				this->PostChatMessage(message->msg);
				/// TODO: Add more disconnection stuffs here?
			}
			message->data = (char*)this;
			MesMan.QueueMessage(message);
			break;
		}
		case RRPacketType::REGISTER:
		{
			SRRegister * r = (SRRegister*)packet;
			/// Check for a peer with similar name/stuff.
			String sender = r->GetSender();
			Peer * peer = NetworkMan.GetPeerByName(sender);
			// Add peer if not existing for this name.
			if (!peer)
			{
				std::cout<<"\nPeer not existing, creating new.";
				peer = CreatePeerForPacket(packet);
			}
			if (peer)
			{
				if (peers.Size() >= maxPeers){
					// Send a register declined packet.
					String reason = "Max peers reached: "+String::ToString(maxPeers);
					SRDeclinePacket decline(packet, reason);
					decline.Send(packet->socket);
					return;
				}
				/// Add the peer to this session too, then.
				if (!peers.Exists(peer))
						peers.Add(peer);
				/// If ok, add stuff.
				RRSessionData * srSessionData = GetSessionData(peer);
				/// If reconnecting, only reset certain variables
				if (srSessionData)
				{
					/// Wooo?
					srSessionData->connectionState = CONNECTED;
				}
				/// If new connection, create it
				else {
					srSessionData = new RRSessionData();
				}
				srSessionData->srSocket = packet->socket;
				peer->sessionData.Add(srSessionData);
				r->socket->peer = peer;
				r->sender = peer;
				// Send a Register OK packet.
				SROKPacket ok(packet);
				ok.Send(peer);
				/// Inform the new peer of our current player setup... TODO: create separate function that doesn't fload the network
				if (isHost)
					OnPlayersUpdated();

				ChatMan.PostGeneral("Client joined: "+peer->name);
			}
			else {
				// Send a register declined packet.
				String reason = "Unable to find peer in previously registered SIP peers.";
				SRDeclinePacket decline(packet, reason);
				decline.Send(packet->socket);
			}
			break;
		}

	}
}
