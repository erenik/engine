/// Emil Hedemalm
/// 2013-12-18

#include "Peer.h"
#include "Network/Session/SessionData.h"
#include "Stream.h"
#include "Network/Socket/TcpSocket.h"
#include <cassert>

Peer::Peer()
{
	name = "";
	port = -1;
	isValid = false;
	ipAddress = "";
	primaryCommunicationSocket = NULL;
}

Peer::~Peer()
{
	sessionData.ClearAndDelete();
}

/// Removes any references to target socket.
void Peer::RemoveSocket(Socket * sock){
	if (sock == this->primaryCommunicationSocket)
		primaryCommunicationSocket = NULL;
	sockets.Remove(sock);
}
	
/// Returns target session data. For example SessionType::SIP as argument.
SessionData * Peer::GetSessionData(int sessionType, int andSubType /* = -1 */){
	for (int i = 0; i < sessionData.Size(); ++i){
		SessionData * sd = sessionData[i];
		if (sd->type != sessionType)
			continue;
		if (andSubType == -1)
			return sd;
		if (andSubType == sd->subType)
			return sd;
	}
	return NULL;
}

/// Checks if this peer is considered the same as another (name, ip, port)
bool Peer::SameAs(Peer * otherPeer)
{	
	if (name == otherPeer->name &&
		ipAddress == otherPeer->ipAddress &&
		port == otherPeer->port)
		return true
;	return false;
}

/// Acts as a merger, extracting active tcpSockets and other data.
void Peer::ExtractDataFrom(Peer * otherPeer)
{
	// Assuming all contact information is already gathered, all we have to do now is gather up application-related material, and the tcpSockets!
	sockets += otherPeer->sockets;	
	isValid |= otherPeer->isValid;
}

/// Closes and deletes all sockets bound to this peer.
void Peer::DeleteSockets()
{
	std::cout<<"Peer::DeleteSockets";
	while(sockets.Size())
    {
        Socket * socket = sockets[0];
        socket->Close();
        sockets.Remove(socket);
        delete socket;
    }
}    

/// Debug, prints name, ip and port.
void Peer::Print()
{
	std::cout<<"Name: "<<name<<" IP: "<<ipAddress<<" port: "<<port;
}
