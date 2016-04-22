/// Emil Hedemalm
/// 2014-01-24
/// Udp socket class inspired by Qt Socket classes

#include "Network/NetworkIncludes.h"
#include "UdpSocket.h"

UdpSocket::UdpSocket()
: Socket(SocketType::UDP)
{

}

/// Virtual destructor for proper deallocation.
UdpSocket::~UdpSocket()
{
	std::cout<<"\nUdpSocket destructor";
}

/// Here just to overload so they are not used.
int UdpSocket::Write(const char * fromBuffer, int maxBytesToWrite)
{
	assert(false && "use WriteTo for udp sockets!");
	return 0;
}
/// Here just to overload so they are not used.
int UdpSocket::Read(char * intoBuffer, int maxBytesToRead)
{
	assert(false && "Use readFrom for udp sockets!");
	return 0;
}
/// Here just to overload so they are not used.
bool UdpSocket::ConnectTo(String hostAddress, int port)
{
	assert(false && "Do not Connect with Udp sockets.");
	return false;
}


/// Writes bytes from buffer into socket. Returns bytes written or -1 if the socket is closed.
int UdpSocket::WriteTo(String ip, String port, const char * fromBuffer, int maxBytesToWrite)
{
	/// Check that the socket is writable
	timeval t = {0, 1000};
	if (!ReadyToWrite(sockfd))
        return 0;

	int flags = 0;

	// Set up some settings on the connection
	memset(&hints, 0, sizeof(hints));		// Make sure the struct is empty
	hints.ai_family		= AF_INET;	// Use IPv4 or IPv6
	hints.ai_socktype	= SOCK_DGRAM;	// Use selected type of socket
	hints.ai_flags		= AI_PASSIVE;		// Fill in my IP for me
	hints.ai_protocol	= IPPROTO_UDP;			// Select correct Protocol for the connection
	int result = getaddrinfo(ip.c_str(), port.c_str(), &hints, &servInfo);
    if(result != 0)
	{
		std::cout<<"\nGetaddrinfo failed.";
		lastErrorString = "Getaddrinfo failed, errorCode: " + String::ToString(result);
		freeaddrinfo(servInfo);
		return false;
	}

    /// Try start the server
	bool boundSuccessfully = false;
	int bytesWritten = 0;
	for(server = servInfo; server != NULL; server = server->ai_next){
		// Get good address.
		bytesWritten += sendto(sockfd, fromBuffer, maxBytesToWrite, flags, server->ai_addr, server->ai_addrlen);
		if (bytesWritten == -1){
			// Add extra handling maybe.
			std::cout<<"Blugg";
		}
		if(bytesWritten){
			boundSuccessfully = true;
			break;	// Success!
		}
	}
	freeaddrinfo(servInfo);
	this->bytesSent += bytesWritten;
	return bytesWritten;
}

/// Reads bytes from the socket. If the socket has closed -1 will be returned.
int UdpSocket::ReadFrom(sockaddr * address, char * intoBuffer, int maxBytesToRead)
{
	timeval t = {0, 1000};
	if (!ReadyToRead(sockfd))
        return 0;
	int size = sizeof(sockaddr);
	int flags = 0;
	int bytesRead = recvfrom(sockfd, intoBuffer, maxBytesToRead, flags, address, (socklen_t*) &size);
	if (bytesRead == SOCKET_ERROR){
		int error = WSAGetLastError();
		std::cout<<"\nError reading socket from given address. ErrorCode: "<<error;
		deleteFlag = 1;
	}
	bytesReceived += bytesRead;
	return bytesRead;
}

/// Closes the socket.
void UdpSocket::Close()
{
	CloseSocket(sockfd);
}

/// Attempts to connect to target host.
bool UdpSocket::Bind(int port)
{
    std::cout<<"\nUdpSocket::ConnectTo";
	/// SOCK_STREAM for TCP, SOCK_DGRAM for UDP
	int connectionType = SOCK_DGRAM;
	int protocol = IPPROTO_UDP;
	int error;
	String portStr = String::ToString(port);
	bool connectedSuccessfully = false;

	// Set up some settings on the connection
	memset(&hints, 0, sizeof(hints));		// Make sure the struct is empty
	hints.ai_family		= AF_INET;	// Use IPv4 or IPv6
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
	bool boundSuccessfully = false;
	for(server = servInfo; server != NULL; server = server->ai_next){
		sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
		if(sockfd == INVALID_SOCKET)
			continue;
		if(bind(sockfd, server->ai_addr, server->ai_addrlen) != SOCKET_ERROR){
			boundSuccessfully = true;
			break;	// Success!
		}
		CloseSocket(sockfd);
	}
	/// Error handling
	if (!boundSuccessfully)
	{
		std::cout<<"\nCould not bind socket.";
		error = WSAGetLastError();
		lastErrorString = "Could not bind socket: " + String::ToString(error);
		return false;
	}
	// Disables internal buffering mechanism, speedier sending of packets
	int x = 1;
//	result = setsockopt(sockfd, SOL_SOCKET, SO_DONTLINGER, (char *)&x, sizeof(x));
//	assert(result == 0);
	/// Disable blocking
	SetNonBlocking();
	return true;
}

/*

	// If we have UDP, configure the host client
//	if(Network.connectionType == SOCK_DGRAM)
//	{
	//	clients[0].clientAddr.sin_family		= server->ai_family;
	//	clients[0].clientAddr.sin_port			= Network.port;
	//	clients[0].clientAddr.sin_addr.s_addr	= inet_addr(server->ai_addr->sa_data);
	//	clients[0].clientAddrSize				= sizeof(clients[0].clientAddr);
//	}

*/
