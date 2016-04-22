/// Emil Hedemalm
/// 2014-01-24
/// General socket class inspired by Qt Socket classes

#include "Network/NetworkIncludes.h"
#include "TcpSocket.h"

TcpSocket::TcpSocket()
: Socket(SocketType::TCP)
{

}

/// Writes bytes from buffer into socket. Returns bytes written or -1 if the socket is closed.
int TcpSocket::Write(const char * fromBuffer, int maxBytesToWrite)
{
	int result = Socket::Write(fromBuffer, maxBytesToWrite);
	return result;
}

/// Reads bytes from the socket. If the socket has closed -1 will be returned.
int TcpSocket::Read(char * intoBuffer, int maxBytesToRead)
{
	int result = Socket::Read(intoBuffer, maxBytesToRead);
	return result;
}

/// Closes the socket.
void TcpSocket::Close()
{

}
/// Attempts to connect to target host.
bool TcpSocket::ConnectTo(String hostAddress, int port)
{
    std::cout<<"\nTcpSocket::ConnectTo called with address: "<<hostAddress<<" and port: "<<port;
	/// SOCK_STREAM for TCP, SOCK_DGRAM for UDP
	int connectionType = SOCK_STREAM;
	int protocol = IPPROTO_TCP;

	memset(&hints, 0, sizeof(hints));		// Make sure the struct is empty
	hints.ai_family		= AF_UNSPEC;		// Find both IPv4 and IPv6 servers
	hints.ai_socktype	= connectionType;	// Find TCP or UDP servers
	hints.ai_flags		= AI_PASSIVE;		// Fill in my IP for me
	hints.ai_protocol	= protocol;	// Select correct Protocol for the connection

	String portStr = String::ToString(port);

	int status = getaddrinfo(hostAddress.c_str(), portStr.c_str(), &hints, &servInfo);
	if(status != 0)
	{
		std::cout<<"\nTcpSocket::ConnectTo Getaddrinfo error: " << status<<" for hostAddress: "<<hostAddress<<" and port: "<<portStr;
		return false;
	}

	bool connectedSuccessfully = false;
	int error = 0;
	// Try all until we successfully connect
	for(server = servInfo; server != NULL; server = server->ai_next)
	{
		sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
		/// Check server protocol family (similar to address family)
		String protocolFamily = "Unknown";
		switch(server->ai_family){
			case AF_INET:
				protocolFamily = "PF_INET";
				break;
			case AF_INET6:
				protocolFamily = "AF_INET6";
				break;
			default:
				break;
		}

		if(sockfd == INVALID_SOCKET)
			continue;
		std::cout<<"\nTrying socket with family: "<<protocolFamily<<" socktype: "<<server->ai_socktype<<" and protocol: "<<server->ai_protocol;
		/// Skip IPv6 for now, since it isn't supported much yet anyway?
		if (server->ai_family == AF_INET6){
			std::cout<<"\nSkipping IPv6 until further notice.";
			continue;
		}
		// TCP
		if(connectionType == SOCK_STREAM)
		{
			int result = connect(sockfd, server->ai_addr, server->ai_addrlen);
			if(result == SOCKET_ERROR)
			{
			    /// Try all server types first, so do nothing here.
				std::cout<<"\nUnable to bind socket.";
				error = WSAGetLastError();
				std::cout<<"\nWrrrr: "<<error;
			}
			// No error here
			else {
				std::cout<<"\nSuccessfully bound socket.";
				connectedSuccessfully = true;
				break;	// Success!
			}
		}
		CloseSocket(sockfd);
	}
	// Free the addr-info, no longer needed
	freeaddrinfo(servInfo);
	// If no address succeeded
	if(!connectedSuccessfully)
	{
		std::cout<<"\nCould not bind socket.";
		lastErrorString = "Could not bind socket: " + String::ToString(error);
		return false;
	}
	int x = 1;
	// If TCP, setup socket
	// Maybe has to be set before hosting?
	// Disables internal buffering mechanism, speedier sending of packets
	int result = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&x, sizeof(x));
	assert(result == 0);
	result = setsockopt(sockfd, SOL_SOCKET, SO_DONTLINGER, (char *)&x, sizeof(x));
	if (!result){
        std::cout<<"\nWARNING: Unable to set SO_DONTLINGER option to socket.";
	}
	/// Disable blocking
	SetNonBlocking();
	return true;
}
