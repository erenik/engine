/// Emil Hedemalm
/// 2014-01-24
/// A general media- or network-session of any sort. Will mostly refer to a gaming session.

#define SAFE_DELETE(p) {if (p) delete p; p = NULL;}

#include "Session.h"
#include "SessionTypes.h"
#include "Network/Peer.h"
#include "Network/Socket/TcpSocket.h"
#include "Network/Server/TcpServer.h"

#include "Network/NetworkManager.h"

/*namespace SessionType {
enum sessionTypes {
	NULL_TYPE,
	GAME, // Session type that handles both a chat and a game, including spectators.
	VOIP, // Voice over IP.
	VIDEO, // For transmitting a live-stream, camera, or similar.
};};
*/
Session::Session(String name, String typeName, int type)
: name(name), typeName(typeName), type(type)
{
	/// Assign 'me' via network manager.
	me = NetworkMan.Me();

	isHost = false;
	isConnected = false;
	host = NULL;
	tcpServer = NULL;
	hostSocket = NULL;
	maxPeers = 32;
}

Session::~Session()
{
	std::cout<<"\nSession destructor.";
	SAFE_DELETE(tcpServer);
	SAFE_DELETE(hostSocket);
}


/// Query if this session is running/active and successfully has bound it's designated listen ports.
bool Session::IsHost() const
{
	return isHost;
}

/// If we are currently connected as a client in this session.
bool Session::IsConnected() const
{
	return isConnected;
}

/// Connects to address/port.
bool Session::ConnectTo(String ipAddress, int port)
{
//	assert(!isHost);
//	assert(hostSocket == NULL);
	// Set host flag to false if we try to connect, making us act as clients instead from now on!
	isHost = false;
	if (hostSocket)
	{
		hostSocket->Close();
		delete hostSocket;
	}
	hostSocket = new TcpSocket();
	bool success = hostSocket->ConnectTo(ipAddress, port);
	if (success)
	{
		isConnected = true;
		return true;
	}
	lastErrorString = hostSocket->GetLastErrorString();
	std::cout<<"\nSession::ConnectTo failed to connect to host.";
	delete hostSocket;
	hostSocket = NULL;
	return false;
}

/// Attempts to start hosting a session of this kind. Default hosts 1 TcpServer on target port. 
bool Session::Host(int port){
	/// if already hosting, clean stuff maybe
	std::cout<<"Already hosting ,clean maybe.";
	if (tcpServer){
		tcpServer->Stop();
		delete tcpServer;
		tcpServer = NULL;
	}

	assert(tcpServer == NULL);
	tcpServer = new TcpServer();
	tcpServer->SetServerPort(port);
	bool success = tcpServer->Start();
	if (!success)
	{
		lastErrorString = tcpServer->GetLastErrorString();
		delete tcpServer;
		tcpServer = NULL;
		return false;
	}
	this->port = port;
	this->host = me;
	this->hostName = me->name;
	isHost = true;
	return true;
}

bool Session::IsHost()
{
	return isHost;
}

/// Stops the session, disconnecting any connections and sockets the session might have.
void Session::Stop(){
	/// Stop all server sockets.
	if (tcpServer){
		tcpServer->Stop();
	}
	/// Disconenct all sockets.
	while(sockets.Size()){
		Socket * s = sockets[0];
		this->OnSocketDeleted(s);
		sockets.Remove(s);
		s->Close();
		delete s;
	}
	/// Remove flags.
	isHost = false;
	isLocal = true;
}

/// Called when the host disconnects.
void Session::OnHostDisconnected(Peer * host)
{
	assert(false && "Subclass");
	isConnected = false;
}


/// For analysis, queries the same functions in all available sockets.
int Session::BytesSentPerSecond(){
	int total = 0;
	if (hostSocket)
		total += hostSocket->BytesSentPerSecond();
	for (int i = 0; i < sockets.Size(); ++i){
		total += sockets[i]->BytesSentPerSecond();
	}
	return total;
}
int Session::BytesReceivedPerSecond(){
	int total = 0;
	if (hostSocket)
		total += hostSocket->BytesReceivedPerSecond();
	for (int i = 0; i < sockets.Size(); ++i){
		total += sockets[i]->BytesReceivedPerSecond();
	}
	return total;
}


/// Function to process new incoming connections but also disbard old connections that are no longer active.
void Session::EvaluateConnections(){
	// If hosting, check for new connections
	if (this->tcpServer){
		assert(this->tcpServer);
		Socket * newSocket = this->tcpServer->NextPendingConnection();
		if (newSocket)
			sockets.Add(newSocket);
	}

	/// Deletes those sockets that either have errors or have been flagged for deletion (other errors)
	DeleteFlaggedSockets();	
}

/// Sends target packet to all peers in this session using default targetsm, via host if possible.
void Session::Send(Packet * packet)
{
	if (isHost)
	{
		/*
		for (int i = 0; i < peers.Size(); ++i){
			Peer * p = peers[i];
			packet->Send(p);
		}
		*/
		/// Send to all sockets for now.
		for (int i = 0; i < sockets.Size(); ++i){
			Socket * s = sockets[i];
			packet->Send(s);
		}
	}
	else {
		Socket * socket = hostSocket;
		packet->Send(socket);
	}
}

/// Sends text as a packet.
void Session::SendText(String text)
{
	Packet textPack = Packet(PacketType::TEXT);
	textPack.data.Push(text);
	// Include terminating NULL-sign.
	textPack.size = text.Length() + 1;
	Send(&textPack);
}
	

/** Reads packets, creating them and returning them for processing. 
	Each batch of received bytes is considered one "packet". Subclass to override.
*/
List<Packet*> Session::ReadPackets()
{
	List<Packet*> packetsReceived;
	const int packetBufferSize = 5000;
	char packetBuffer[packetBufferSize];

	/// Gather relevant sockets to check for received data.
	List<Socket*> socketsToCheck;
	if (isHost)
	{
		socketsToCheck = sockets;
	}
	else {
		if (hostSocket)
			socketsToCheck = hostSocket;
	}	

	/// Check all sockets now.
	for (int i = 0; i < socketsToCheck.Size(); ++i)
	{
		Socket * s = socketsToCheck[i];
		int bytesRead = s->Read(packetBuffer, packetBufferSize);
		// Read one packet each frame?
		if (bytesRead > 0)
		{
			Packet * pack = new Packet(PacketType::NULL_TYPE);
			pack->data.PushBytes((uchar *)packetBuffer, bytesRead);
			packetsReceived.Add(pack);
		}
	}
	return packetsReceived;
}

/// Returns type name of this session.
String Session::TypeName(){
	return TypeName(type);
}

/// See SessionType above.
String Session::TypeName(int id){
	switch(id){
		case SessionType::SIP:
			return "SIP";
		case SessionType::GAME:
			return "Game";
		case SessionType::VOIP:
			return "VoiceOverIP";
		case SessionType::VIDEO:
			return "Video transmission";
		default:
			return "Undefined";
	}
}


/// Returns the last error string, nullifying it too.
String Session::GetLastErrorString(){
	String s = lastErrorString;
	lastErrorString = "";
	return s;
}


/// Pokes at all associated sockets, prompting them to send their queued packets. Also sends all general queued packets to all peers.
void Session::SendPackets(){
	/// For testing purposes, create text-packets.
	/*
	if (!packetQueue.Size()){
		Packet * packet = new Packet(PacketType::GT_TEXT);
		packet->data = "Testing packet 123";
		packetQueue.Add(packet);
	}*/

	/// Send until the packet queue is empty.
	while(this->packetQueue.Size()){
		Packet * packet = this->packetQueue[0];
		/// And send each packet to every peer.
		for (int i = 0; i < this->sockets.Size(); ++i){
			Socket * sock = this->sockets[i];
			packet->Send(sock);
		}
		/// Delete the packet after it's been sent too.
		packetQueue.Remove(packet);
		delete packet;
	}
}

/// Called every time a socket is deleted. Any references to the socket should then be removed.
void Session::OnSocketDeleted(Socket * sock){
	std::cout<<"\nSession::OnSocketDeleted called. should be handled in a subclass of Session!";
	/// Clear any relevant connections between this socket and the peers.
	if (sock->peer){
		sock->peer->RemoveSocket(sock);
	}
}

/// Deletes those sockets that either have errors or have been flagged for deletion (other errors)
void Session::DeleteFlaggedSockets(){
	/// Check if any connections have gone down, if so remove them?
	for (int i = 0; i < sockets.Size(); ++i){
		Socket * s = sockets[i];
		if (s->HasError() || s->ShouldDelete()){
			OnSocketDeleted(s);
			sockets.Remove(s);
			s->Close();
			delete s;
			--i;
			continue;
		}
	}
	/// If host disconnects, disconnect us too.
	if (hostSocket)
	{	
		if (hostSocket->HasError() || hostSocket->ShouldDelete()){
			OnSocketDeleted(hostSocket);
			hostSocket->Close();
			delete hostSocket;
			hostSocket = NULL;
			OnHostDisconnected(host);
		}
	}
}
