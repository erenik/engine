/// Emil Hedemalm
/// 2014-01-24
/// A server handling incoming connections.

#include "TcpServer.h"
#include "Network/NetworkSettings.h"
#include "Network/Socket/TcpSocket.h"
#include <cassert>
#include <iostream>
#include <cstring>
#include "Network/NetworkIncludes.h"

TcpServer::TcpServer(){
	port = 33000;
	status = ServerStatus::NOT_STARTED;
}


void TcpServer::SetServerPort(int i_port){
	port = i_port;
}

/// Starts listening. Returns false if the network interfaces are busy/inaccessible.
bool TcpServer::Start()
{
	assert(this->status == ServerStatus::NOT_STARTED);
	std::cout<<"\nNetworkManager::StartServer called...";

	String portStr = String::ToString(port);

	/// SOCK_DGRAM for Datagram (UDP), SOCK_STREAM for TCP
	int connectionType = SOCK_STREAM;
	int protocol = (connectionType == SOCK_DGRAM)? IPPROTO_UDP : IPPROTO_TCP;

	// Set up some settings on the connection
	memset(&hints, 0, sizeof(hints));		// Make sure the struct is empty
	hints.ai_family		= DEFAULT_FAMILY;	// Use IPv4 or IPv6
	hints.ai_socktype	= connectionType;	// Use selected type of socket
	hints.ai_flags		= AI_PASSIVE;		// Fill in my IP for me
	hints.ai_protocol	= protocol;			// Select correct Protocol for the connection
	int result = getaddrinfo(NULL, portStr.c_str(), &hints, &servInfo);
    if(result != 0)
	{
		std::cout<<"\nGetaddrinfo failed.";
		lastErrorString = "Getaddrinfo failed, errorCode: " + String::ToString(result);
		return false;
	}

    /// Try start the server
	bool hostedSuccessfully = false;
	for(server = servInfo; server != NULL; server = server->ai_next){
		listenSocket = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
		if(listenSocket == INVALID_SOCKET)
			continue;
		if(bind(listenSocket, server->ai_addr, server->ai_addrlen) != SOCKET_ERROR){
			hostedSuccessfully = true;
			break;	// Success!
		}
		CloseSocket(listenSocket);
	}
	// If no address succeeded
	if(!hostedSuccessfully){
		std::cout<<"\nCould not bind socket.";
		lastErrorString = "Could not bind socket.";
		return false;
	}

	int x = 1;

	// If TCP, setup socket

	// Disables internal buffering mechanism, speedier sending of packets
	setsockopt(listenSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&x, sizeof(x));
	// Sets something
	setsockopt(listenSocket, SOL_SOCKET, SO_DONTLINGER, (char *)&x, sizeof(x));

	// Free info about the found servers
	freeaddrinfo(servInfo);

	/// Start listening for new connections on the socket.
	std::cout<<"\nBegin checking for pending connections.";
	int maxPendingConnections = MAX_PENDING_CONNECTIONS;
	if(listen(listenSocket, maxPendingConnections) == SOCKET_ERROR)
	{
		std::cout<<"\nFailed to start listening from socket!";
		lastErrorString = "Error listening to socket";
		return false;
	}
	std::cout<<"\nServer up and running, port: " << port;
	return true;
}

/// Stops listening, unbinding the listen socket.
void TcpServer::Stop(){
	/// Just close the socket.
	this->CloseSocket(listenSocket);

	/// Reset some status maybe?
	status = ServerStatus::NOT_STARTED;
}

/// Returns the next pending connection, if any available. If not, returns NULL.
Socket * TcpServer::NextPendingConnection(){
	/// Check if there is any pending connection.
	fd_set readfds;

    if (!ReadyToRead(listenSocket))
        return NULL;

    std::cout<<"\nPending connection found.";
	/// Accept pending connection.
	sockaddr socketAddress;
	int size = sizeof(sockaddr);
	SOCKET newSock = accept(listenSocket, &socketAddress, (socklen_t *) &size);
	if (newSock == INVALID_SOCKET){
		std::cout<<"\nGetaddrinfo failed.";
		int result = GetLastError();
		lastErrorString = "Getaddrinfo failed, errorCode: " + String::ToString(result);
		assert(false && "Handle");

	}
	/// Identify new socket stats as far as possible.
	Socket * sock = new Socket(SocketType::NULL_TYPE);
	sock->sockfd = newSock;
	/// Check connecting address and try to parse it?
	if (socketAddress.sa_family == AF_INET){
		sockaddr_in * socketAddressInternet = (sockaddr_in*) &socketAddress;
		std::cout<<"\nConnection from/to port: "<<socketAddressInternet->sin_port;
		char * address = inet_ntoa(socketAddressInternet->sin_addr);
		String ipAddress = address;
		sock->peerAddress = ipAddress;
		std::cout<<"\nConnection from/to address: "<<ipAddress;
		sock->socketAddress = socketAddress;
	}
	else {
		assert(false && "implement for ipv6 or whatever this address type is");
		std::cout<<"\n.sa_family: "<<socketAddress.sa_family;
	}
	return sock;
}

String TcpServer::GetLastErrorString(){
	String s = lastErrorString;
	lastErrorString = "";
	return s;
}


void TcpServer::CloseSocket(SOCKET & sock)
{
#ifdef WINDOWS
	if(sock != -1 && sock)
		closesocket(sock);
#else
	if(sock != -1 && sock)
		close(sock);
#endif
	sock = NULL;
}

