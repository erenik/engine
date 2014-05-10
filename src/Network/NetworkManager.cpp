/// Emil Hedemalm
/// 2014-01-24
/// General network-manager that handles all peers and network-sessions.

#include "NetworkManager.h"

#include "Message/MessageManager.h"
#include <Network/Server/TcpServer.h>
#include <Network/Socket/TcpSocket.h>
#include <Timer/Timer.h>
#include "Network/Session/SessionTypes.h"
#include "Network/Session/GameSession.h"
#include "Network/SIP/SIPSession.h"
#include "Peer.h"
#include "Game/Game.h"
#include <cassert>


#define print(a) {std::cout<< a ; }
void PrintNetworkError(int i_error = -1)
{
    int error = i_error;
//	if(error == -1)
//		error = sockerrno;
    #ifdef WINDOWS
	switch(error){
		case WSANOTINITIALISED:
			print("A successful WSAStartup call must occur before using this function.");
			break;
		case WSAENETDOWN:
			print("The network subsystem has failed.");
			break;
		case WSAEACCES:
			print("An attempt was made to access a socket in a way forbidden by its access permissions. This error is returned if nn attempt to bind a datagram socket to the broadcast address failed because the setsockopt option SO_BROADCAST is not enabled.");
			break;
		case WSAEADDRINUSE:
			print("Only one usage of each socket address (protocol/network address/port) is normally permitted. This error is returned if a process on the computer is already bound to the same fully qualified address and the socket has not been marked to allow address reuse with SO_REUSEADDR. For example, the IP address and port specified in the name parameter are already bound to another socket being used by another application. For more information, see the SO_REUSEADDR socket option in the SOL_SOCKET Socket Options reference, Using SO_REUSEADDR and SO_EXCLUSIVEADDRUSE, and SO_EXCLUSIVEADDRUSE.");
			break;
		case WSAECONNRESET:
			print("An incoming connection was indicated, but was subsequently terminated by the remote peer prior to accepting the call.");
			break;
		case WSAECONNABORTED:
			print("The virtual circuit was terminated due to a time-out or other failure. The application should close the socket as it is no longer usable.");
			break;
		case WSAETIMEDOUT:
			print("The connection has been dropped because of a network failure or because the peer system failed to respond.");
			break;
		case WSAEADDRNOTAVAIL:
			print("The requested address is not valid in its context. This error is returned if the specified address pointed to by the name parameter is not a valid local IP address on this computer.");
			break;
		case WSAEAFNOSUPPORT:
			print("The specified address family is not supported. For example, an application tried to create a socket for the AF_IRDA address family but an infrared adapter and device driver is not installed on the local computer.");
			break;
		case WSAEFAULT:
			print("The system detected an invalid pointer address in attempting to use a pointer argument in a call. This error is returned if the name parameter is NULL, the name or namelen parameter is not a valid part of the user address space, the namelen parameter is too small, the name parameter contains an incorrect address format for the associated address family, or the first two bytes of the memory block specified by name do not match the address family associated with the socket descriptor s.");
			break;
		case WSAENOTCONN:
			print("The socket is not connected.");
			break;
		case WSAEINPROGRESS:
			print("A blocking Windows Sockets 1.1 call is in progress, or the service provider is still processing a callback function.");
			break;
		case WSAEINTR:
			print("A blocking Windows Sockets 1.1 call was canceled through WSACancelBlockingCall.");
			break;
		case WSAEINVAL:
			print("An invalid argument was supplied. This error is returned of the socket s is already bound to an address.");
			break;
		case WSAEMFILE: // 10024
			print("Too many open files.");
			print("No more socket descriptors are available.");
			break;
		case WSAENETRESET: // 10052
			print("Network dropped connection on reset.");
			print("For a connection-oriented socket, this error indicates that the connection has been broken due to keep-alive activity that detected a failure while the operation was in progress. For a datagram socket, this error indicates that the time to live has expired.");
			break;
		case WSAENOBUFS: // 10055
			print("No buffer space available.");
			print("An operation on a socket could not be performed because the system lacked sufficient buffer space or because a queue was full. This error is returned of not enough buffers are available or there are too many connections.");
			break;
		case WSAENOTSOCK: // 10038
			print("Socket operation on nonsocket.");
			print("An operation was attempted on something that is not a socket. This error is returned if the descriptor in the s parameter is not a socket.");
			break;
		case WSAEOPNOTSUPP: // 10045
			print("Operation not supported.");
			print("MSG_OOB was specified, but the socket is not stream-style such as type SOCK_STREAM, OOB data is not supported in the communication domain associated with this socket, or the socket is unidirectional and supports only send operations.");
			break;
		case WSAESHUTDOWN: // 10058
			print("Cannot send after socket shutdown.");
			print("The socket has been shut down; it is not possible to send or receive on a socket after shutdown has been invoked with how set to SD_RECEIVE or SD_BOTH.");
			break;
		case WSAEPROTONOSUPPORT: // 10043
			print("Protocol not supported.");
			print("The specified protocol is not supported.");
			break;
		case WSAEPROTOTYPE: // 10041
			print("Protocol wrong type for socket.");
			print("The specified protocol is the wrong type for this socket.");
			break;
		case WSAEPROVIDERFAILEDINIT: // 10106
			print("Service provider failed to initialize.");
			print("The service provider failed to initialize. This error is returned if a layered service provider (LSP) or namespace provider was improperly installed or the provider fails to operate correctly.");
			break;
		case WSAESOCKTNOSUPPORT: // 10044
			print("Socket type not supported.");
			print("The specified socket type is not supported in this address family.");
			break;
		case WSAEWOULDBLOCK: // 10035
			print("Resource temporarily unavailable.");
			print("The socket is marked as nonblocking and no connections are present to be accepted.");
			break;
		case WSAEMSGSIZE: // 10040
			print("Message too long.");
			print("A message sent on a datagram socket was larger than the internal message buffer or some other network limit, or the buffer used to receive a datagram was smaller than the datagram itself.");
			break;
		case WSAECONNREFUSED: // 10061
			print("Connection refused.");
			print("No connection could be made because the target computer actively refused it. This usually results from trying to connect to a service that is inactive on the foreign host-that is, one with no server application running.");
			break;
		default:
			char buff[8]; _itoa(error, buff, 10);
			print ("Unknown error: " << buff);
	}
	#else
	std::cout<<"NetworkError: " << error;
	#endif
}


/// Windows start up data structure for network.
#ifdef WINDOWS
WSADATA wsaData;
#endif
NetworkManager * NetworkManager::networkManager = NULL;

NetworkManager::NetworkManager()
{
	initialized = false;
	this->targetIP = "109.225.69.146";
}

NetworkManager::~NetworkManager()
{
#ifdef WINDOWS
	int errorCode = WSACleanup();
	if(errorCode == SOCKET_ERROR){
		std::cout<<"\nWSACleanup failed: ";
		PrintNetworkError(errorCode);
	}
#endif
    delete this->me;
	sessions.ClearAndDelete();
}

void NetworkManager::Allocate(){
	assert(networkManager == NULL);
	networkManager = new NetworkManager();
}
NetworkManager * NetworkManager::Instance(){
	assert(networkManager);
	return networkManager;
}
void NetworkManager::Deallocate(){
	assert(networkManager);
	delete(networkManager);
	networkManager = NULL;
}

/// Starts up the initial SIP server, identities host machine data, etc.
void NetworkManager::Initialize()
{
	/// Set default-sender now, as name etc. should have been fixed before-hand.
    this->me = new Peer();
	/// Set default statistics
	me->port = 33000;
	me->name = "Localhost";
	me->ipAddress = "127.0.0.1";

	// Run specific network-initialization tasks.
#ifdef WINDOWS
#ifdef _MSC_VER
	// Required to run WSAStartup before any socket actions later on!
	int errorCode = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if(errorCode != 0){
		char buff[8]; _itoa(errorCode, buff, 10);
		print("\nWSAStartup failure, error code: " << buff);
		WSACleanup();
		lastErrorString = "WSAStartup failed, error code: "+ String(buff);
		return;
	}
#endif

	// Get the local IP
	char ac[80];
	if(gethostname(ac, sizeof(ac)) == SOCKET_ERROR){
		print("\nGet host name failed: ");
	}
	else{
		/// Save computer name as our name.
		me->name = ac;
		struct hostent *phe = gethostbyname(ac);
		if(phe == 0){
			print("\nBad host!");
		}
		/// Save address too if possible
		for(int i = 0; phe->h_addr_list[i] != 0; ++i){
			struct in_addr addr;
			memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
			// Save the IP-Address
			strncpy(machineIP, inet_ntoa(addr), 32);
			me->ipAddress = machineIP;
		}
	}
#endif
	initialized = true;
	print("\nNetworkManager Initialized.");

	SIPPacket::SetDefaultSender(this->me);
    // Set a requested tag for us that we try to stick to when registering with other peers
	srand((int)Timer::GetCurrentTimeMs());
	// Default 33000
    sipPort = 33000;
	sipSession = new SIPSession();
	sipSession->Initialize();
	initialized = true;
}
/// Shuts down all active sessions.
void NetworkManager::Shutdown(){
	while(sessions.Size()){
		Session * s = sessions[0];
		s->Stop();
		sessions.Remove(s);
		delete s;
	}
}

/// Adds new peer!
void NetworkManager::AddPeer(Peer * newPeer)
{
	peers.Add(newPeer);
}

/// Returns object representing self. This should not be modified after sessions have been started of any kind.
Peer * NetworkManager::Me(){
	return me;
}

/// Returns true if we are hosting a SIP session.
bool NetworkManager::IsHost(){
	return sipSession->IsHost();
}

//==================================================
//  PUBLIC
//==================================================
/// Attempts to retrieve target session by name
Session * NetworkManager::GetSessionByName(String name){
	for (int i = 0; i < sessions.Size(); ++i){
		Session * s = sessions[i];
		if (s->name == name)
			return s;
	}
	return NULL;
}

/// As defined in SessionTypes.h and specific sub type header file. E.g. GameSessionTypes.h
Session * NetworkManager::GetSession(int byType, int andSubType /* = -1 */)
{
	for (int i = 0; i < sessions.Size(); ++i){
		Session * s = sessions[i];
		if (s->type != byType)
			continue;
		if (andSubType == -1)
			return s;
		if (s->subType == andSubType)
			return s;
	}
	return NULL;
}

/// Starts given session, adding it to the list of active sessions.
bool NetworkManager::AddSession(Session * session){
	sessions.Add(session);
	return true;
}

/// Returns list of all available games. See Game/Game.h for structure definition.
List<Game*> NetworkManager::GetAvailableGames(){
	availableGames.ClearAndDelete();
	availableGames = this->sipSession->GetAvailableGames();
	/// Get games that we are hosting too.
	for (int i = 0; i < sessions.Size(); ++i){
		Session * s = sessions[i];
		if (!s->IsHost() || s->localOnly)
			continue;
		if (s->type == SessionType::GAME){
			GameSession * gs = (GameSession*)s;
			/// Check if the game-session is local only, if so skip it.
			if (gs->isLocal)
				continue;
			availableGames += gs->GetGame();
		}
	}
	return availableGames;
}


/// Will retrieve the last error. Once called, the error string will be reset to an empty string.
String NetworkManager::GetLastErrorString()
{
	String err = lastErrorString;
	lastErrorString = "";
	return err;
}

/// Sets desired host port for the SIP server
void NetworkManager::SetSIPServerPort(int port){
    sipPort = port;
    std::cout << "SIP server port set to: "<<sipPort;
}


/// Query function for the SIP server.
bool NetworkManager::IsSIPServerRunning()
{
    if (!initialized)
        return false;
	assert(sipSession);
	if (sipSession->IsHost())
		return true;
	return false;
}

bool NetworkManager::StartSIPServer()
{
    if (!initialized)
        return false;
    std::cout<<"\nStarting SIP server.";
    assert(sipSession);
	// Return if already hosted.
	if (sipSession->IsHost())
		return true;
	bool hosted;
	for (int i = DEFAULT_SIP_START_PORT; i <= DEFAULT_SIP_MAX_PORT; ++i)
	{
		hosted = sipSession->Host(i);
		if (hosted)
		{
			std::cout<<"\nSIP server hosted on port: "<<i;
			break;
		}
		else {
			std::cout<<"\nUnable to host SIP Server on port: "<<i;	
		}
	}
	if (!hosted)
	{
		std::cout<<"\nUnable to host SIP Server on any of the 10 default ports!";
		lastErrorString = sipSession->GetLastErrorString();
		return false;
	}
	return true;
}

void NetworkManager::StopSIPServer()
{
	SIPSession * session = (SIPSession*)GetSession(SessionType::SIP);
	session->Stop();
	/*
    std::cout<<"NetworkManager::StopSIPServer";
    int disconnectedPeers = 0;
    while (peers.Size())
    {
		std::cout << "Disconnecting peer " << ++disconnectedPeers << "...";
		Peer* p = peers[0];
		this->RemovePeer(p);
	}
    // Shutdown the TCP server.
	tcpServer.close();
    this->processNetworkTimer.stop();
	*/
}

/// Getter
Peer * NetworkManager::GetPeerByName(String name)
{
    for (int i = 0; i < peers.Size(); ++i)
    {
        Peer * peer = peers[i];
        if (peer->name == name)
            return peer;
    }
    return NULL;
}

/// Returns current list of active peers. Note that this list may be adjusted after any packet processing.
List<Peer*> NetworkManager::GetPeers()
{
    return peers;
}
/**
 * @param ipAddress The IP address of the server
 * @param port Port for the server
 * @return True on success
 */
bool NetworkManager::ConnectTo(const String ipAddress, int port /* = -1 */)
{
	// Default settings, recursive with default ports.
	if (port == -1)
	{
		for (int i = DEFAULT_SIP_START_PORT; i <= DEFAULT_SIP_MAX_PORT; ++i)
		{
			bool result = ConnectTo(ipAddress, i);
			if (result)
				return result;
		} 
		lastErrorString = "Default SIP ports not working.";
		return false;
	}

    std::cout << "NetworkManager::ConnectTo called with ip: "<< ipAddress <<" and port: "<<port;
	if (sipSession->IsHost() && port == sipPort && ipAddress == "127.0.0.1"){
        std::cout<<"Trying to connect to self, aborting.";
		lastErrorString = "Trying to connect to self, aborting.";
        return false;
    }
	/// Try to connect to target ip/port.
	bool result = sipSession->ConnectTo(ipAddress,port);
	lastErrorString = sipSession->GetLastErrorString();
	/// Set target ip address to the same as the one we connect to for SIP.
	targetIP = ipAddress;
	return result;
}


/// Removes target peer from all lists, and sends a OnPeerDisconnected before deleting it.
void NetworkManager::RemovePeer(Peer * peer)
{
}

// Disconnects this instance from the network, including all peers and our own tcpServer.
void NetworkManager::Disconnect()
{
}

//==================================================
//  PUBLIC SLOTS
//==================================================
void NetworkManager::ProcessNetwork()
{
	if (!this->initialized)
		return;
	List<Packet*> packets;

	/// Process the main SIP session.
	this->sipSession->EvaluateConnections();
	packets += this->sipSession->ReadPackets();
	this->sipSession->SendPackets();
	List<Peer*> disconnectedPeers = this->sipSession->GetDisconnectedPeers();
	OnPeersDisconnected(disconnectedPeers);

	/// Add new peers from sip-session list to our total list.
	List<Peer*> newPeers = sipSession->GetNewPeers();
	this->peers += newPeers;
	OnNewPeersConnected(newPeers);

	// Process active sessions.
	for (int i = 0; i < sessions.Size(); ++i){
		Session * session = sessions[i];
		// Also evaluates expiring connections.
		session->EvaluateConnections();
		// Process received packets.
		packets += session->ReadPackets();
		session->SendPackets();
	}

	/// Queue the packets to be handled by the message manager.
	MesMan.QueuePackets(packets);
}

/** Merges duplicated peers (issue caused by how we handle the connections & sockets).
    If the suspectedPeer has been merged/deleted, it's mergee will be returned instead of itself.
*/
Peer * NetworkManager::MergeDuplicatePeers(Peer * suspectedPeer)
{
	Peer * peerToReturn = suspectedPeer;

	/*
    std::cout<<"NetworkManager::MergeDuplicatePeers for peer: "<<suspectedPeer->name;
    // Make sure this is performed relatively early?
    for (int i = 0; i < peers.Size(); ++i)
    {
        Peer * p = peers[i];
        // Skip those peers who have not a clear IP/Name to use as reference...
        if (!p->isValid)
            continue;
        for (int j = i+1; j < peers.Size(); ++j){
            Peer * p2 = peers.at(j);
            // Skip those peers who have not a clear IP/Name to use as reference...
            if (!p2->isValid)
                continue;
            // If two peers are considered the same (same IP, port and name), merge them.
            if (p->SameAs(p2)){

                String peerListPreMerge = "Peers pre merge: "+String::ToString(peers.Size())+": ";
                for (int i = 0; i < peers.Size(); ++i)
                    peerListPreMerge.Add(peers[i]->name+" ");
                Q_EMIT ConnectionEvent(peerListPreMerge);


                p->Print();
                p2->Print();
                assert(p->name == p2->name);
                Q_EMIT ConnectionEvent("Peers merged: "+p->name+" & "+p2->name);
                p->ExtractDataFrom(p2);
                assert(p->isValid);
                assert(p2->isValid);
                bool result = peers.removeOne(p2);
                assert(result);
                if (p2 == suspectedPeer)
                    peerToReturn = p;
                // Switch all occurences of the peer-pointer before deleting now.
                SIPPacket::ReplacePeer(p2, p);
                delete p2;
                p2 = NULL;
                // TODO: Consider calling merge again, but with a NULL-argument, until no more merges will be done?
                // MergeDuplicatePeers();
                PrintPeerList();
                // Notify UI-components of the new peer-list.
                Q_EMIT PeerListChanged(peers);
                String peerListPostMerge = "  Peers post merge: "+String::ToString(peers.Size())+": ";
                for (int i = 0; i < peers.Size(); ++i)
                    peerListPostMerge.Add(peers[i]->name+" ");
                Q_EMIT ConnectionEvent(peerListPostMerge);
                std::cout<<"Peer to return: "<<peerToReturn->name;
                return peerToReturn;
            }
        }
    }
	*/
    return peerToReturn;
}

/// For debugging
void NetworkManager::PrintPeerList()
{
    std::cout<<"Connected peers: "<<peers.Size();
    for (int i = 0; i < peers.Size(); ++i)
    {
        Peer * peer = peers[i];
        std::cout<<"- Peer "<<i<<": "<<peer->name<<" active sockets: "<<peer->sockets.Size();
    }
}

/// Sets name of our "me"-peer.
void NetworkManager::SetName(String name){
    this->me->name = name;
}

/*
/// Searches for media by sending a Subscribe message to all connected peers.
void NetworkManager::SearchForMedia(String searchString){
    // Set search string for "me" for future reference! (like when new peers are connected, to subscribe to them too).
    int updateDuration = 60;
    me->mediaSearchString = searchString;
    SIPSubscribePacket subscribePacket(MEDIA_SEARCH_EVENT, updateDuration, MEDIA_SEARCH_EVENT_CSEQ);
    subscribePacket.body.Add(searchString);
    for (int i = 0; i < peers.Size(); ++i){
        Peer * peer = peers[i];
        subscribePacket.Send(peer);
    }
}
*/
//==================================================
//  PRIVATE
//==================================================

/// Returns number of valid peers (currently registered).
int NetworkManager::NumValidPeers()
{
    int n = 0;
    for (int i = 0; i < peers.Size(); ++i)
        if (peers[i]->isValid)
            ++n;
    return n;
}
/// Returns number of registered peers (currently registered).
int NetworkManager::NumRegisteredPeers()
{
	return sipSession->NumRegisteredPeers();
}



/// Function called every time new peers have connected to the basic communication layer
void NetworkManager::OnNewPeersConnected(List<Peer*> newPeers){
	for (int i = 0; i < newPeers.Size(); ++i){
		Peer * peer = newPeers[i];
		/// Notify that a peer has registered successfully.
		MesMan.QueueMessages("OnPeerConnected("+peer->name+")");
	}
}

/// Called when peers have been disconnected.
void NetworkManager::OnPeersDisconnected(List<Peer*>  dcPeers){
	/// Notify that a peer has registered successfully.
	for (int i = 0; i < dcPeers.Size(); ++i){
		Peer * peer = dcPeers[i];
		/// Notify that a peer has registered successfully.
		MesMan.QueueMessages("OnPeerDisconnected("+peer->name+")");
	}
}

/** Internal function that handles post-register events, like updating UI,
    ensuring both are mutually registered, sending subscribe-messages, etc.
    If peer is a duplicate, a merge will be performed and the mergee peer will be returned. If not, the same peer as the argument should be returned.
*/
Peer * NetworkManager::OnPeerRegistered(Peer * peer)
{
    assert(false);
    return peer;
}

/*
Peer* NetworkManager::GetPeerBySocket(TcpSocket* socket)
{
    Peer* p;
    for (int i = 0, size = peers.Size(); i < size; ++i)
    {
        p = peers[i];
        if (p->HasTcpSocket(socket))
            return p;
    }

    return NULL;
}
*/
