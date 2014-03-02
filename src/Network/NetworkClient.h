/// Emil Hedemalm
/// 2013-10-20

#ifndef NETWORK_CLIENT_H
#define NETWORK_CLIENT_H

/*
#include "NetworkIncludes.h"


/** Contains information about a connected network user (client). */
/*
class NetworkClient{
public:
	NetworkClient();

	// NetworkClient Information
	int connectionStatus;		// Tells whether client is active or not
	char IP[NAME_LIMIT];		// Nice formated IP address
	char name[NAME_LIMIT];		// The name the client is using
	int index;					// On what position in the clients array client is located
	bool versionCheck;			// Tells that a client is running right version of the game and have sent all data needed
	long long lastMessage;		// Time in MS since Jan 1, 1970 the last packet was received

	// Connection information
	SOCKET clientSock;			// Socket used for TCP connections
	/// Internet address in number-style formats. Ref: http://msdn.microsoft.com/en-us/library/aa922011.aspx
	sockaddr_in clientAddr;		// Address information used for UDP
	int clientAddrSize;			// size of the sockaddr_in structure
	Timer lastPingTime;			// Ping timer, last ping message
	int pingRetries;			// Amount of ping retries client have left
};
*/

#endif
