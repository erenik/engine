// Aron Wåhlberg & Emil Hedemalm
// 2013-03-17

/*
#include "Network/NetworkSettings.h"
#include <cstring>

#define print(arg) std::cout << "\n" << arg ;

#ifdef USE_NETWORK

#include "NetworkStatus.h"
#include "Packet.h"

// ------------------- Managers -------------------
#include "NetworkManager.h"
#include "Message/MessageManager.h"
#include "FtpManager.h"
NetworkManager * NetworkManager::networkManager = NULL;			// Global Manager

#include "String/AEString.h"
#include "Network/Packet/Packets.h"
#include "OS/Sleep.h"
#include "Player/PlayerManager.h"
#include "Chat/ChatManager.h"
#include "Message/Message.h"
#include "NetworkClient.h"

extern int packet_size[PACKET_TYPES];

// ------------------- Functions -------------------
NetworkClient NetworkManager::clients[MAX_CLIENTS];
SOCKET NetworkManager::sockfd = NULL;

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

NetworkManager::NetworkManager(){
	std::cout<<"\nNetworkManager constructor...";
	protocol					= DEFAULT_PROTOCOL;
	userType					= USER_TYPE_NULL;
	networkStatus				= NETWORK_STATUS_NULL;

	sockfd						= NULL;
	initialized					= false;

	clientListenerActive		= false;
	packetSenderActive			= false;
	packetListenerActive		= false;
	connectingActive			= false;

	clientListenerThread        = NULL;
	packetSenderThread          = NULL;
	packetListenerThread        = NULL;
	connectingThread            = NULL;

	CalculateDefaultPacketSizes();
}

NetworkManager::~NetworkManager(){
#ifdef WINDOWS
	int errorCode = WSACleanup();
	if(errorCode == SOCKET_ERROR){
		print("\nWSACleanup failed: ");
		PrintNetworkError();
	}
#endif
	Shutdown();

	initialized = false;
	networkStatus = NETWORK_STATUS_NULL;
}

void NetworkManager::Initialize(){
	Network.networkStatus = NETWORK_STATUS_INITIALIZING;
#ifdef WINDOWS
#ifdef _MSC_VER
	// For doing something with WinSoc library
	int errorCode = WSAStartup(MAKEWORD(2, 2), &Network.wsaData);
	if(errorCode != 0){
		networkStatus = NETWORK_STATUS_ERROR;
		char buff[8]; _itoa(errorCode, buff, 10);
		print("\nWSAStartup failure, error code: " << buff);
		WSACleanup();
		lastErrorString = "WSAStartup failed, error code: "+ String(buff);
	}
#endif
	// Get the local IP
	char ac[80];
	if(gethostname(ac, sizeof(ac)) == SOCKET_ERROR){
		print("\nGet host name failed: ");
		PrintNetworkError();
	}
	else{
		struct hostent *phe = gethostbyname(ac);
		if(phe == 0){
			print("\nBad host!");
		}

		for(int i = 0; phe->h_addr_list[i] != 0; ++i){
			struct in_addr addr;
			memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
			// Save the IP-Address
			strncpy(Network.machineIP, inet_ntoa(addr), 32);
		}
	}
	#endif
	networkStatus = NETWORK_STATUS_IDLE;
	initialized = true;
	print("\nNetworkManager Initialized.");
}

/// Checks whether the manager is initialized with no errors.
bool NetworkManager::IsInitialized(){
    if(!Network.initialized || networkStatus == NETWORK_STATUS_ERROR){
		print("\nNetworkManager not successfully initialized.");
		lastErrorString = "NetworkManager not initialized.";
		return false;
	}
    return true;
}

/// If busy trying to do something critical (like hosting/joining)
bool NetworkManager::IsBusy(){
    if (networkStatus == NETWORK_STATUS_HOSTING ){
        lastErrorString = "Server busy trying to host game.";
        return true;
    }
    else if (networkStatus == NETWORK_STATUS_JOINING){
        lastErrorString = "Server busy trying to join game.";
        return true;
    }
    return false;
}

/// Checks if the user is already in a game and sets appropriate lastErrorString if so.
bool NetworkManager::IsAlreadyInAGame(){
    if (networkStatus == NETWORK_STATUS_HOSTED){
        print("This machine is already running a server!");
		print("Stop it before staring a new one");
        lastErrorString = "Server already running. Stop it before starting a new one.";
        return true;
    }
    else if (networkStatus == NETWORK_STATUS_JOINING){
        print("This machine is connected to another server!");
		print("Disconnect before joining a new one");
		lastErrorString = "This machine is conntected to another server! Disconnect before joining a new one.";
		return true;
    }
    return false;
}

/// Starts the packetListener- and packetSender-threads.
bool NetworkManager::StartPacketThreads(){
#ifdef WINDOWS
	// Listen to incoming packets
	if(!packetListenerActive){
		packetListenerThread = _beginthread(&NetworkManager::PacketReceiver, NULL, NULL);
		packetListenerActive = true;
	}
	// Send outgoing packets
	if(!packetSenderActive){
		packetSenderThread = _beginthread(&NetworkManager::PacketSender, NULL, NULL);
		packetSenderActive = true;
	}
#else /// pThreads
	// Listen to incoming packets
	if(!packetListenerActive){
		int rc = pthread_create(&packetListenerThread, NULL, NetworkManager::PacketReceiver, NULL);
        // Check if create thread failed
        if(rc){
            print("Failed to start packet receiver thread!");
            Shutdown();
            lastErrorString = "Failed to start packetListenerThread.";
            return false;
        }
		packetListenerActive = true;
	}
	// Send outgoing packets
	if(!packetSenderActive){
		 int rc = pthread_create(&packetSenderThread, NULL, NetworkManager::PacketSender, NULL);
        // Check if create thread failed
        if(rc){
            print("Failed to start packet sender thread!");
            lastErrorString = "Failed to start packetSenderThread.";
            Shutdown();
            return false;
        }
		packetSenderActive = true;
	}
#endif
    return true;
}

/// Returns my IP as a string!
String NetworkManager::MyIP(){
    return machineIP;
}

bool NetworkManager::StartServer(int i_port, char *i_password, int i_connectionType){
	print("\nNetworkManager::StartServer called...");
	if (!IsInitialized())
        return false;
    if (IsBusy())
        return false;
    if (IsAlreadyInAGame())
        return false;
    networkStatus = NETWORK_STATUS_HOSTING;

    assert(userType == USER_TYPE_NULL);

	// If a server already is up and running correctly
	if(Network.userType == HOST){
		if(Network.clientListenerActive && Network.packetSenderActive && Network.packetListenerActive
    // Try and check the threads only via the above bools if possible? // Emil
    //  &&	Network.clientListenerThread && Network.packetSenderThread && Network.packetListenerThread
        ){
			print("This machine is already running a server!");
			print("Stop it before staring a new one");
			lastErrorString = "Server already running. Stop it before starting a new one.";
			return false;
		}
		else {
		    print("\nServer already running, resetting it.");
		    lastErrorString = "Server already running, resetting it.";
			ResetServer();
		}
	}
	// Else check so we aren't connected to another server
	else if(Network.userType == CLIENT)
	{
		if(Network.packetSenderActive && Network.packetListenerActive
     // Same here. // Emil
	//	&&	Network.packetSenderThread && Network.packetListenerThread
	)
		{
			print("This machine is connected to another server!");
			print("Disconnect before joining a new one");
			lastErrorString = "This machine is conntected to another server! Disconnect before joining a new one.";
			return false;
		}
		else {
		    lastErrorString = "Server already running, resetting server? Wat. I don't even.";
		    print("\nServer already running, resetting it.");
			ResetServer();
        }
	}

	port = i_port;
	String portStr = String::ToString(port);
	password = i_password;
	connectionType = i_connectionType;

	protocol = (connectionType == SOCK_DGRAM)? IPPROTO_UDP : IPPROTO_TCP;

	Network.userType = HOST;

	// Set up some settings on the connection
	memset(&hints, 0, sizeof(hints));		// Make sure the struct is empty
	hints.ai_family		= DEFAULT_FAMILY;	// Use IPv4 or IPv6
	hints.ai_socktype	= connectionType;	// Use selected type of socket
	hints.ai_flags		= AI_PASSIVE;		// Fill in my IP for me
	hints.ai_protocol	= protocol;			// Select correct Protocol for the connection
    if(getaddrinfo(NULL, portStr.c_str(), &hints, &servInfo) != 0)
	{
		print("Getaddrinfo failed.");
		lastErrorString = "Getaddrinfo failed.";
		return false;
	}

    /// Try start the server
	for(server = servInfo; server != NULL; server = server->ai_next){
		sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
		if(sockfd == INVALID_SOCKET)
			continue;
		if(bind(sockfd, server->ai_addr, server->ai_addrlen) != SOCKET_ERROR)
			break;	// Success!
		CloseSocket(sockfd);
	}
	// If no address succeeded
	if(sockfd <= 0){
		print("Could not bind socket.");
		lastErrorString = "Could not bind socket.";
		PrintNetworkError();
		Network.ResetServer();
		return false;
	}

	int x = 1;
	// If we have UDP, configure the host client
	if(Network.connectionType == SOCK_DGRAM)
	{
		clients[0].clientAddr.sin_family		= server->ai_family;
		clients[0].clientAddr.sin_port			= Network.port;
		clients[0].clientAddr.sin_addr.s_addr	= inet_addr(server->ai_addr->sa_data);
		clients[0].clientAddrSize				= sizeof(clients[0].clientAddr);
	}
	// If TCP, setup socket
	else
	{
		// Disables internal buffering mechanism, speedier sending of packets
		setsockopt(Network.sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&x, sizeof(x));
	}
	setsockopt(Network.sockfd, SOL_SOCKET, SO_DONTLINGER, (char *)&x, sizeof(x));

	// Free info about the found servers
	freeaddrinfo(servInfo);

	print("\nServer up and running, port: " << port);
	if(password && String(password) != "")
	{
		print("Password: " << password);
	}

	Network.networkStatus = NETWORK_STATUS_HOSTED;
	strncpy(Network.serverIP, Network.machineIP, NAME_LIMIT);

	// Set ourself as number 0 in clients
	Network.info.yourClientIndex = 0;
	Network.info.connectionStatus = CONNECTION_STATUS_CONNECTED;
	NetworkClient * client = &clients[0];
	strncpy(client->IP, Network.serverIP, NAME_LIMIT);
	strncpy(client->name, Network.info.name, NAME_LIMIT);
	client->clientSock = sockfd;
	client->connectionStatus = CONNECTION_STATUS_CONNECTED;
	client->versionCheck = true;

	// If the connection type is TCP
	if(Network.connectionType == SOCK_STREAM)
	{
		// Start listening to incoming connections from clients
		#ifdef WINDOWS
		if(!clientListenerActive){
			clientListenerThread = _beginthread(&NetworkManager::AcceptClients, NULL, NULL);
			clientListenerActive = true;
		}
		#else
		if(!clientListenerActive){
            int rc = pthread_create(&clientListenerThread, NULL, &NetworkManager::AcceptClients, NULL);
            // Check if create thread failed
            if(rc){
                print("Failed to start NetworkClient listener thread!");
                lastErrorString = "Failed to start clientListenerThread.";
                Shutdown();
                return false;
            }
            clientListenerActive = true;
		}
		#endif
	}
    /// Start packet-threads!
    bool result = StartPacketThreads();

	return result;
}

bool NetworkManager::JoinServer( const char* IP, int port, char* password, int connectionType )
{
    std::cout<<"\nNetworkManager::JoinServer";

	if (!IsInitialized())
        return false;
    if (IsBusy())
        return false;
    if (IsAlreadyInAGame())
        return false;

	if (userType != USER_TYPE_NULL){
		std::cout<<"\nWARNiNG: User type not NULL when queueing to join server! Resetting variables before attempting new connection.";
		userType = USER_TYPE_NULL;
	}
	if (networkStatus != NETWORK_STATUS_IDLE){
		std::cout<<"\nWARNING: Network status not idle. Attempting to close current connections before attemping to join server.";
		ResetServer();
		switch(networkStatus){
			default:
				std::cout<<"\nNetwork status before reset: "<<networkStatus;
		}
		networkStatus = NETWORK_STATUS_IDLE;
	}
//    assert(userType == USER_TYPE_NULL);
//    assert(networkStatus == NETWORK_STATUS_IDLE);

    networkStatus = NETWORK_STATUS_JOINING;

    std::cout<<"\nNetworkManager::JoinServer IP: "<<IP<<" port: "<<port;
    if (password)
        std::cout<<" passwordLength: "<<strlen(password);
	// If a server already is up and running correctly
	if(Network.userType == HOST)
	{
		if(Network.clientListenerActive && Network.packetSenderActive && Network.packetListenerActive &&
			Network.clientListenerThread && Network.packetSenderThread && Network.packetListenerThread)
		{
			print("This machine is already running a server!");
			print("Stop it before staring a new one");
			return false;
		}
		else {
		    ResetServer();
        }
	}
	// Else check so we aren't connected to another server
	else if(Network.userType == CLIENT)
	{
		if(Network.packetSenderActive && Network.packetListenerActive &&
			Network.packetSenderThread && Network.packetListenerThread)
		{
			print("This machine is connected to another server!");
			print("Disconnect before joining a new one");
			return false;
		}
		else
			ResetServer();
	}

	this->connectionType = connectionType;
	Network.protocol = (connectionType == SOCK_DGRAM)? IPPROTO_UDP : IPPROTO_TCP;

	Network.userType = CLIENT;

	memset(&hints, 0, sizeof(hints));		// Make sure the struct is empty
	hints.ai_family		= AF_UNSPEC;		// Find both IPv4 and IPv6 servers
	hints.ai_socktype	= connectionType;	// Find TCP or UDP servers
	hints.ai_flags		= AI_PASSIVE;		// Fill in my IP for me
	hints.ai_protocol	= Network.protocol;	// Select correct Protocol for the connection


	// Get address info of the server
	ChatMan.AddMessage(new ChatMessage(NULL, "Getting address info of the server..."));
	int status = getaddrinfo(IP, String::ToString(port), &hints, &servInfo);
	if(status != 0)
	{
		print("Getaddrinfo error: " << status);
		return false;
	}

	// Try all until we successfully connect
	for(server = servInfo; server != NULL; server = server->ai_next)
	{
	    String debugs = "Trying to connect using socket type:";
	    // " "+String::ToString(server->ai_socktype)+" canonname: "+String(server->ai_canonname);
	    std::cout<<"\n" + debugs;
        ChatMan.AddMessage(new ChatMessage(NULL, debugs));

		sockfd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
		if(sockfd == INVALID_SOCKET)
			continue;

		// TCP
		if(connectionType == SOCK_STREAM)
		{
			if(connect(sockfd, server->ai_addr, server->ai_addrlen) == SOCKET_ERROR)
			{
			    String fail = "Failed to connect socket.";
			    lastErrorString = fail;
				print(fail);
				PrintNetworkError();
				freeaddrinfo(servInfo);
				Network.ResetServer();
				ChatMan.AddMessage(new ChatMessage(NULL, fail));
				return false;
			}
			print("Socket connected.");
			ChatMan.AddMessage(new ChatMessage(NULL, "Socket connected."));
			break;	// Success!
		}
		// UDP
		else if(bind(sockfd, server->ai_addr, server->ai_addrlen) != SOCKET_ERROR)
		{
		    ChatMan.AddMessage(new ChatMessage(NULL, "Socket connected."));
			break;	// Success!
		}

		CloseSocket(sockfd);
	}

	// If no address succeeded
	if(sockfd <= 0)
	{
	    ChatMan.AddMessage(new ChatMessage(NULL, "ERROR: Could not bind socket."));
		print("Could not bind socket.");
		PrintNetworkError();
		freeaddrinfo(servInfo);
		Network.ResetServer();
		networkStatus = NETWORK_STATUS_ERROR;
		return false;
	}
	ChatMan.AddMessage(new ChatMessage(NULL, "Socket bound successfully."));

	int x = 1;
	// Else if we have UDP, configure the host client
	if(connectionType == SOCK_DGRAM)
	{
		clients[0].clientAddr.sin_family		= server->ai_family;
		clients[0].clientAddr.sin_port			= Network.port;
		clients[0].clientAddr.sin_addr.s_addr	= inet_addr(server->ai_addr->sa_data);
		clients[0].clientAddrSize				= sizeof(clients[0].clientAddr);
	}
	// If TCP, setup socket
	else
	{
		// Disables internal buffering mechanism, speedier sending of packets
		setsockopt(Network.sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&x, sizeof(x));
	}
	setsockopt(Network.sockfd, SOL_SOCKET, SO_DONTLINGER, (char *)&x, sizeof(x));

	// Do some more host configuration
	clients[0].connectionStatus = CONNECTION_STATUS_CONNECTED;
	clients[0].clientSock = sockfd;

	// Free the addr-info, no longer needed
	freeaddrinfo(servInfo);



    ChatMan.AddMessage(new ChatMessage(NULL, "Joined server on port: "+String::ToString(port)));
	print("\nJoined server, port: " << port);
	if(password && String(password) != "")
	{
		print("Password: " << password);
	}

	Network.networkStatus = NETWORK_STATUS_JOINED;
	strncpy(Network.serverIP, IP, NAME_LIMIT);
	Network.info.connectionStatus = CONNECTION_STATUS_PENDING;

    /// Start packet threads
    bool result = StartPacketThreads();
    if (!result){
        ChatMan.AddMessage(new ChatMessage(NULL, "Failed to start packet threads!"));
        return false;
    }


    /// And start another connecting thread...?
#ifdef WINDOWS
	// Start establishing the connection
	if(!connectingActive){
		connectingThread = _beginthread(&NetworkManager::Connecting, NULL, NULL);
		connectingActive = true;
	}
#else
	if(!connectingActive){
		int rc = pthread_create(&connectingThread, NULL, NetworkManager::Connecting, NULL);
        // Check if create thread failed
        if(rc){
            ChatMan.AddMessage(new ChatMessage(NULL, "Failed to start the connection establisher thread!"));
            print("Failed to start the connection establisher thread!");
            Shutdown();
            return false;
        }
		connectingActive = true;
	}
#endif
    ChatMan.AddMessage(new ChatMessage(NULL, "Connection establisher initialized, waiting for server handshake procedure..."));
	return true;
}

/// Stop running server and deallocate and stop stuff
void NetworkManager::Shutdown()
{
	print("\nShutting down Networking..");

    /// Remove all players (except self?)
    PlayerMan.RemoveAllNetworkPlayers();


	ResetServer();
	print("\nThe server has been shut down.");
}

void NetworkManager::QueuePacket( Packet* packet, char target )
{
    /// TODO: Add mutex here. Could be needed assuming it's used in another thread, which I bet it is.
	// Check if we're trying to set a new target for the packet
	if(target != -1)
		packet->target = target;

	packetQueue.Push(packet);
}

void NetworkManager::Ping()
{
	// First check if we should ping
	if(Network.pingTimer.GetMs() >= PING_CHECK_DELAY)
	{
		Network.pingTimer.Start();
		for(int i = ((Network.userType == HOST)?1:0); i < ((Network.userType == HOST)?MAX_CLIENTS:1); ++i){
			NetworkClient *client = &Network.clients[i];
			// Check if client is connected
			if(client->connectionStatus == CONNECTION_STATUS_NULL || !client->versionCheck)
				continue;

			long long lastPkt = Timer::GetCurrentTimeMs() - client->lastMessage;
			int pingTime = client->lastPingTime.GetMs();
			if(lastPkt >= PING_DELAY && (pingTime) >= PING_DELAY){
				if(clients[i].pingRetries-- < 0){
					if(Network.userType == HOST){
						print("NetworkClient [" << client->name << "] disconnected through timeout!");
						Network.ResetClient(client);
					}
					else{
						print("Lost connection to host!");
						Network.Shutdown();
					}
					return;
				}
				Network.QueuePacket(new Packet(PACKET_PING, i));
				clients[i].lastPingTime.Start();
			}
		}
	}
}

void NetworkManager::ResetServer(){
    std::cout<<"\nResetting server";
#ifdef USE_FTP
	Ftp.ClearFtp();
#endif // End USE_FTP

	// Reset variables
	userType = USER_TYPE_NULL;
	networkStatus = NETWORK_STATUS_IDLE;

	std::cout<<"\nRequesting network threads to stop...";
	// Start with making trying to stop all threads automatically
	Network.clientListenerActive	= false;
	Network.packetSenderActive		= false;
	Network.packetListenerActive	= false;
	Network.connectingActive		= false;

	// Give threads some time to stop working
	CloseThread(Network.clientListenerThread);
	CloseThread(Network.packetSenderThread);
	CloseThread(Network.packetListenerThread);
	CloseThread(Network.connectingThread);

	CloseSocket(Network.sockfd);
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		ResetClient(&clients[i]);
	}
	Network.info.yourClientIndex = -1;
	Network.info.connectionStatus = CONNECTION_STATUS_NULL;
	Network.userType			= USER_TYPE_NULL;
	if(Network.initialized)
		Network.networkStatus	= NETWORK_STATUS_IDLE;
	else
		Network.networkStatus	= NETWORK_STATUS_NULL;
}

void NetworkManager::PrintNetworkError(int i_error)
{
    int error = i_error;
	if(error == -1)
		error = sockerrno;
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
	print("Error: " << strerror(error));
	#endif
}

#ifdef WINDOWS
bool NetworkManager::CloseThread(uintptr_t &threadHandle){

	if (threadHandle){
		print("Closing thread: " << threadHandle);
		int result = TerminateThread( (HANDLE) threadHandle, 0);
		if (result == 0){
			int error = GetLastError();
			print("Error closing thread: " << error);
			return false;
		}
	}
	else
		print("Thread already terminated");
	threadHandle = NULL;
	return true;
}
#else
bool NetworkManager::CloseThread(pthread_t &threadHandle){

	if (threadHandle){
		print("Closing thread: " << threadHandle);
		int result = pthread_cancel(threadHandle);
		if (result != 0){
			print("Error closing thread: " << strerror(sockerrno));
			threadHandle = NULL;
			return false;
		}

		result = pthread_join(threadHandle, NULL);
		if (result != 0){
            print("Thread join error: " << strerror(sockerrno))
		}
	}
	else
		print("Thread already terminated");
	threadHandle = NULL;
	return true;
}
#endif

void NetworkManager::CloseSocket( int sock )
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

int NetworkManager::ActiveClients()
{
	int activeClients = 0;
	for(int i = 0; i < MAX_CLIENTS; ++i)
	{
		if(clients[i].connectionStatus != CONNECTION_STATUS_NULL)
			++activeClients;
	}
	return activeClients;
}

void NetworkManager::ResetClient( NetworkClient *client )
{
	if(!client)
		return;
	// If using FTP, make sure there are no transfers going on to this player!
	#ifdef USE_FTP
	// Get client index
	Ftp.ClearClientFromFtp(client->index);
	#endif
	strncpy(client->IP, "0.0.0.0", NAME_LIMIT+1);
	strncpy(client->name, "No Name", NAME_LIMIT+1);
	client->index = -1;
	client->connectionStatus = CONNECTION_STATUS_NULL;
	CloseSocket(client->clientSock);
	client->versionCheck = false;
	client->lastMessage = NULL;
	client->pingRetries = PING_RESENDS;
}

int NetworkManager::GetUserType()
{
	return userType;
}

/// Returns true if we are the current host.
bool NetworkManager::IsHost(){
	return userType == HOST;
}

/// Returns true if you are either a host or a client.
bool NetworkManager::IsActive(){
	if (userType == CLIENT || userType == HOST)
		return true;
	return false;
}

NetworkClient* NetworkManager::GetClient( int clientIndex )
{
	return &clients[clientIndex];
}

/// IP should be in the form of aggregate x.y.z.w -> (x*255^3 + y * 255^2 + z * 255 + w) = IP
NetworkClient * NetworkManager::GetClientByIP(in_addr IP){
	for (int i = 0; i < MAX_CLIENTS; ++i){
		NetworkClient * c = &clients[i];
		if (c->clientAddr.sin_addr.s_addr == IP.s_addr)
			return c;
	}
	return NULL;
}

int NetworkManager::GetConnectionType()
{
	return Network.connectionType;
}

/// Woshi. Returns the last error string. Resets the errorString to a null-string too!
String NetworkManager::GetLastErrorString(){
     String les = lastErrorString;
     lastErrorString = String();
     return les;
}

NetworkClient::NetworkClient()
{
	strncpy(IP, "0.0.0.0", NAME_LIMIT+1);
	strncpy(name, "No Name", NAME_LIMIT+1);
	index = -1;
	connectionStatus = CONNECTION_STATUS_NULL;
	clientSock = NULL;
	versionCheck = false;
	lastMessage = NULL;
	pingRetries = PING_RESENDS;
	lastPingTime.Start();
}
MachineInfo::MachineInfo()
{
	/// Get random seed
	srand((unsigned int)time(NULL));
	// Add some random names for the phun :P
	char *names[] = { "Puny NetworkClient", "Clienster", "Tha User", "Dr4g0nki113r_1337", "Unknown", "John Smith", "iamjustsendingthisleter", "root", "guest", "ghost", "test", "feest!", "Linux guy", "Windows guy", "h4x0r", "hacker", "The Guy", "Mr. Andersson", "Sven Svenson", "Din Mamma", "Plushie", "Captain Obvious", "Star Wars Kid", "Homie", "Bloke", "The Collector", "Reaper", "Matrix", "Vector", "Vertex"};
	int nameAmount = sizeof(names) / sizeof(names[0]);

	strncpy(name, names[rand()%nameAmount], NAME_LIMIT);
	yourClientIndex = -1;
	connectionStatus = CONNECTION_STATUS_NULL;
}


#endif // USE_NETWORK

*/