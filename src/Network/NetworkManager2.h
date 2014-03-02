// Aron Wåhlberg & Emil Hedemalm
// 2013-03-17
/** The NetworkManager is a singleton that should be used to handle all network-traffic 
	and base connection/ping/heartbeat packets. Optionally the FTPManager can be hooked 
	in to help transfer relevant files.
	
	Every time a notable action occurs, a message will be generated and sent to the MessageManager
	for custom behaviour in the game. Additionally, packets can be intercepted as well, 
	but this is only recommended for game-specific packets.

	Several messages exist that will be dispatched (see NetworkMessage.h for full descriptions)
	when connections are modified.
*/

#include "Network/NetworkSettings.h"

// If not using network, uncomment all rows using the network-manager... if possible.
#ifndef USE_NETWORK
#define Network		NULL //Network disabled.
#endif

// IF using network, do awesome stuff! o-o
#ifdef USE_NETWORK
#define Network		(*NetworkManager::Instance())

#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include "NetworkIncludes.h"

#include "Globals.h"
#include <iostream>
#include <string>
#include <errno.h>
#include <String/AEString.h>
#include "Util/Queue/Queue.h"
#include "Util/Timer/Timer.h"

struct Packet;

/** MachineInfo
	Some information about the user. */
struct MachineInfo
{
	MachineInfo();
	char name[NAME_LIMIT];		// Your name, will also be the name of your client
	int yourClientIndex;		// Index of your client in NetworkManager::clients
	int connectionStatus;		// Like client, but used for checking when to send version check, etc.
};

enum versionCheckReturnValues {
	VERSION_CHECK_INVALID_VERSION,
	VERSION_CHECK_CONNECTING,
	VERSION_CHECK_OK,
};

// Forward declarations
class NetworkClient;

/**	NetworkManager
	Handles the network traffic.*/
#define NetworkMan (*NetworkManager::Instance())
class NetworkManager{
private:
	NetworkManager();
	static NetworkManager * networkManager;
public:
	static void Allocate();
	static NetworkManager * Instance();
	static void Deallocate();
	~NetworkManager();

	/// Initialized
	void Initialize();
	/// Initializes server, start listening on incoming connections
	bool StartServer(int port = DEFAULT_PORT, char* password = NULL, int connectionType = SOCK_STREAM);
	/// Initializes the process of joining a server
	bool JoinServer(const char* IP, int port = DEFAULT_PORT, char* password = NULL, int connectionType = SOCK_STREAM);
	/// Stop running server and deallocate and stop stuff
	void Shutdown();
	/// Wtf is the default value for target doing? Sets the target if not -1? See Packets.h for targets.
	void QueuePacket(Packet* packet, char target = -1);
	/// Returns amount of active clients
	int ActiveClients();
	/// Returns usertype
	int GetUserType();
	
	/// Returns true if we are the current host.
	bool IsHost();
	/// Returns true if you are either a host or a client.
	bool IsActive();

	/// Return client
	NetworkClient * GetClient(int clientIndex);
	/// IP should be in the form of aggregate x.y.z.w -> (x*255^3 + y * 255^2 + z * 255 + w) = IP
	NetworkClient * GetClientByIP(in_addr IP);

	/// Returns what type of connections is active
	int GetConnectionType();

	/// Public packet size registration function.
	void SetPacketSize(int packetType, int packetSize);

#ifdef WINDOWS
	static void AcceptClients(void *vArgs);
	static void PacketReceiver(void *vArgs);
	static void PacketSender(void *vArgs);
	static void Connecting(void *vArgs);
#else
	static void * AcceptClients(void *vArgs);
	static void * PacketReceiver(void *vArgs);
	static void * PacketSender(void *vArgs);
	static void * Connecting(void *vArgs);
#endif

    /// Returns my IP as a string!
    String MyIP();

	/// Your own machine's info.
	MachineInfo info;

    /// Returns the last error string. Resets the errorString to a null-string upon usage.
    String GetLastErrorString();

private:
	/// See Packet/PacketSizes.cpp
	void CalculateDefaultPacketSizes();
	/// Handles networks managed by the network manager. Returns true if it processed the packet.
	void ProcessPacket(Packet * packet);
	
	/// Checks whether the manager is initialized with no errors.
    bool IsInitialized();
    /// If busy trying to do something critical (like hosting/joining)
    bool IsBusy();
    /// Checks if the user is already in a game and sets appropriate lastErrorString if so.
    bool IsAlreadyInAGame();
    /// Starts the packetListener- and packetSender-threads.
    bool StartPacketThreads();

    /// Update every time an error occurs. Switch it out then or catenate maybe?
    String lastErrorString;


	bool initialized;
	/// Define if current program will act as a Server or NetworkClient
	char userType;
	/// Status of this machine/network
	char networkStatus;
	/// Port to open up the server on
	int port;
	/// Server password
	String password;
	/// IP-Address of server/host
	char serverIP[NAME_LIMIT];
	/// IP-Address of this machine
	char machineIP[NAME_LIMIT];
	/// Type of connection, TCP or UDP
	int connectionType;
//#ifdef WINDOWS
	/// Info about server to search for, found servers and selected server
	struct addrinfo hints, *servInfo, *server;
	/// Main socket
	static SOCKET sockfd;
#ifdef WINDOWS
	/// WSA data
	WSAData wsaData;
#endif
	/// What protocol to be used, TCP or UDP
	int protocol;
	/// All network users
	static NetworkClient clients[MAX_CLIENTS];
	/// Network information about the host (if we ourselves are a client, that is).
	NetworkClient * host;
	/// New list for all connected clients.
	List<NetworkClient*> clientList;
	/// Packet queue
	Queue<Packet *> packetQueue;
	/// Timer for when checking if ping should be sent
	Timer pingTimer;

	bool clientListenerActive;
	bool packetSenderActive;
	bool packetListenerActive;
	bool connectingActive;
#ifdef WINDOWS
	uintptr_t clientListenerThread;
	uintptr_t packetSenderThread;
	uintptr_t packetListenerThread;
	uintptr_t connectingThread;
#else
	pthread_t clientListenerThread;
	pthread_t packetSenderThread;
	pthread_t packetListenerThread;
	pthread_t connectingThread;
#endif

	/// Pings required connections if needed
	void Ping();
	/// Resets variables from the creation of server
	void ResetServer();
    /// Print network error
	static void PrintNetworkError(int error = -1);
	/// Terminates a thread and NULLifies it
#ifdef WINDOWS
	bool CloseThread(uintptr_t &threadHandle);
#else
    bool CloseThread(pthread_t &threadHandle);
#endif
	/// Closes a socket correctly
	static void CloseSocket(int sock);
	/// Resets data in a client, does what?
	void ResetClient(NetworkClient * client);
};

#endif

#endif // USE_NETWORK
