/// Emil Hedemalm
/// 2014-01-24
/// General socket class inspired by Qt Socket classes

#include "Socket.h"
#include "Network/Peer.h"
#include "Timer/Timer.h"

/// Closes target socket.
void CloseSocket(SOCKET sock){
#ifdef WINDOWS
	if(sock != -1 && sock)
		closesocket(sock);
#else
	if(sock != -1 && sock)
		close(sock);
#endif
	sock = NULL;
}


/// Returns state of a socket, if it is ready to be read or not!
bool ReadyToRead(SOCKET sock)
{
    int millisecondsToWait = 10;
	timeval checkTime;
	checkTime.tv_sec = 0;
	checkTime.tv_usec = 1000;
	int result;
#ifdef WINDOWS
	struct fd_set readfds;
	readfds.fd_array[0] = sock;
	readfds.fd_count = 1;
	result = select(0, &readfds, NULL, NULL, &checkTime);
#elif defined LINUX
    struct pollfd pollfds;
    memset(&pollfds, 0, sizeof(pollfd));
    pollfds.fd = sock;
    pollfds.events = POLLIN;
    int timeout = millisecondsToWait;
    result = poll(&pollfds, 1, timeout);
#endif
	/// Error
	if (result == SOCKET_ERROR){
        return false;
	}
	/// None ready
	else if (result == 0)
		return false;
    return true;
}

/// Returns state of a socket, if we can write to it.
bool ReadyToWrite(SOCKET sock)
{
//    std::cout<<"\nPolling socket in ReadyToWrite...";
    int millisecondsToWait = 10;
	timeval checkTime;
	checkTime.tv_sec = 0;
	checkTime.tv_usec = 1000;
	int result;
#ifdef WINDOWS
	struct fd_set writefds;
	writefds.fd_array[0] = sock;
	writefds.fd_count = 1;
	result = select(0, NULL, &writefds, NULL, &checkTime);
#elif defined LINUX
    struct pollfd pollfds;
    memset(&pollfds, 0, sizeof(pollfd));
    pollfds.fd = sock;
    pollfds.events = POLLOUT;
    int timeout = millisecondsToWait;
    result = poll(&pollfds, 1, timeout);
#endif
	/// Error
	if (result == SOCKET_ERROR ||
     result == 0
     )
	{
        std::cout<<"\nUnable to write to socket!";
        return false;
	}
	return true;
}

Socket::Socket(int type)
: type(type)
{
	peer = NULL;
	deleteFlag = 0;
	nonBlocking = false;
	messagesSent = 0;

	/// Transmission data for later analysis.
	bytesSent = bytesReceived = 0;
	lastBytesReceived = lastBytesSent = 0;
	lastSentCheckTime = Timer::GetCurrentTimeMs();
	lastReceivedCheckTime = Timer::GetCurrentTimeMs();
}

/// Destructor that closes sockets if required.
Socket::~Socket(){
	if (sockfd != INVALID_SOCKET)
		Close();
	std::cout<<"\nSocket destructor";
}

/// Writes bytes from buffer into socket. Returns bytes written or -1 if the socket is closed.
int Socket::Write(const char * fromBuffer, int maxBytesToWrite)
{
	timeval t = {0, 1000};
	if (!ReadyToWrite(sockfd))
        return 0;
	int bytesWritten = send(sockfd, fromBuffer, maxBytesToWrite, NULL);
	++messagesSent;
	bytesSent += bytesWritten;
	return bytesWritten;
}

/// Reads bytes from the socket. If the socket has closed -1 will be returned.
int Socket::Read(char * intoBuffer, int maxBytesToRead)
{
	timeval t = {0, 1000};
	if (!ReadyToRead(sockfd))
        return 0;

	int bytesRead = recv(sockfd, intoBuffer, maxBytesToRead, NULL);
	if (bytesRead == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		std::cout<<"\nError receiving/reading socket, errorCode: "<<error<<" Setting delete flag.";
		switch(error)
		{
			case 10053:// WSAECONNABORTED:
			{
				std::cout<<"\nInternal software error? D:";
				break;
			}
			default:
			{
				/// http://msdn.microsoft.com/en-us/library/windows/desktop/ms740668%28v=vs.85%29.aspx
				std::cout<<"\nSome other error occurred. Look it up?";
			}
		}
		deleteFlag = 1;
	}
	bytesReceived += bytesRead;
	return bytesRead;
}

/// If the delete flag has been set.
bool Socket::ShouldDelete(){
	return deleteFlag == 1;
}

/// Closes the socket. Returns false if any error occured.
bool Socket::Close(){
	if (sockfd == INVALID_SOCKET)
		return true;
	int result = closesocket(sockfd);
	if (result == SOCKET_ERROR){
		std::cout<<"\nSocket::Close Error closing socket: "<<result;
		return false;
	}
	sockfd = INVALID_SOCKET;
	return true;
}

/// Attempts to connect to target host.
bool Socket::ConnectTo(String hostAddress, int port)
{
    std::cout<<"\nSocket::JoinServer";
	/// SOCK_STREAM for TCP, SOCK_DGRAM for UDP
	int connectionType = SOCK_STREAM;
	int protocol = (connectionType == SOCK_DGRAM)? IPPROTO_UDP : IPPROTO_TCP;

	memset(&hints, 0, sizeof(hints));		// Make sure the struct is empty
	hints.ai_family		= AF_UNSPEC;		// Find both IPv4 and IPv6 servers
	hints.ai_socktype	= connectionType;	// Find TCP or UDP servers
	hints.ai_flags		= AI_PASSIVE;		// Fill in my IP for me
	hints.ai_protocol	= protocol;	// Select correct Protocol for the connection

	String portStr = String::ToString(port);

	int status = getaddrinfo(hostAddress.c_str(), portStr.c_str(), &hints, &servInfo);
	if(status != 0)
	{
		std::cout<<"\nSocket::ConnectTo Getaddrinfo error: " << status<<" for hostAddress: "<<hostAddress<<" and port: "<<portStr;
		return false;
	}

	bool connectedSuccessfully = false;
	int error = 0;
	// Try all until we successfully connect
	for(server = servInfo; server != NULL; server = server->ai_next)
	{
		sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
		if(sockfd == INVALID_SOCKET)
			continue;
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
	// If no address succeeded
	if(!connectedSuccessfully)
	{
		std::cout<<"\nCould not bind socket.";
		freeaddrinfo(servInfo);
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
	assert(result == 0);
	/// Disable blocking
	SetNonBlocking();
	// Free the addr-info, no longer needed
	freeaddrinfo(servInfo);
	return true;
}

/// Performs a select check to see if it currently has any errors.
bool Socket::HasError(){
	fd_set efds = {1, sockfd};
	struct timeval timeout = {0, 1000};
	int result = select(0, NULL, NULL, &efds, &timeout);
	return result > 0;
}

/// Enables non-blocking mode.
void Socket::SetNonBlocking(bool nonBlockingEnabled){
    /*
	u_long nonblocking = 1;
	int result = ioctlsocket(sockfd, FIONBIO, &nonblocking);
	assert(result == 0);
	nonBlocking = true;
*/
    bool blocking = !nonBlockingEnabled;
#ifdef WIN32
    unsigned long mode = blocking ? 0 : 1;
    ioctlsocket(sockfd, FIONBIO, &mode);
#else
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags < 0)
        return;
    flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
    bool success = fcntl(sockfd, F_SETFL, flags);
#endif
    return;
}

/// Attempts to connect to target host.
//bool ConnectTo(String hostAddress, int port);


/// Retrieves the last error string, resetting it.
String Socket::GetLastErrorString(){
	String s = lastErrorString;
	lastErrorString = "";
	return s;
}

/// For analysis, based on the amount of data sent since the last call to the same function.
int Socket::BytesSentPerSecond(){
	long long currentTime = Timer::GetCurrentTimeMs();
	float timeDiff = currentTime - lastSentCheckTime;
	timeDiff /= 1000.0f;
	if (timeDiff < 1.0f)
		return bsps;
	assert(timeDiff);
	int byteDiff = bytesSent - lastBytesSent;
	float bps = byteDiff / (float) timeDiff;
	lastBytesSent = bytesSent;
	lastSentCheckTime = currentTime;
	bsps = bps;
	return bps;
}
int Socket::BytesReceivedPerSecond(){
	long long currentTime = Timer::GetCurrentTimeMs();
	float timeDiff = currentTime - lastReceivedCheckTime;
	timeDiff /= 1000.0f;
	if (timeDiff < 1.0f)
		return brps;
	int byteDiff = bytesReceived - lastBytesReceived;
	float bps = byteDiff / (float) timeDiff;
	lastBytesReceived = bytesReceived;
	lastReceivedCheckTime = currentTime;
	brps = bps;
	return bps;
}

/// Total statistics
int Socket::TotalBytesSent(){
	return bytesSent;
}
int Socket::TotalBytesReceived(){
	return bytesReceived;
}
