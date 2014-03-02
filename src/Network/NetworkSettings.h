// Emil Hedemalm & Aron Wåhlberg
// 2013-03-29

#ifndef NETWORK_SETTINGS_H
#define NETWORK_SETTINGS_H

// Enables/disables the whole network solution and usage

// Enables/disables the FTP possibility of the network, of course FTP wont work without network(!)
// #define USE_FTP
// Enables Ping
//#define USE_PING

#define USE_NETWORK

#include "OS/OS.h"

/// Disable network for OSX for the time being!
#ifdef OSX
#undef USE_NETWORK
#endif // OSX

/// General buffer used for receiving data. A similar size is used as max in the String class for Tokenizing among other things.
#define NETWORK_BUFFER_SIZE (4096*4)
#define DEFAULT_FAMILY				AF_INET			// Address family, Internet.
#define MAX_PENDING_CONNECTIONS		SOMAXCONN		// Set to 5 or very large for winsock 2

/* Not used.
#define RECV_BUFFER_SIZE			4096
//#define RECV_FTP_BUFFER_SIZE		103424			// 101KB, Used for receiving FTP packets
#define DEFAULT_PROTOCOL			IPPROTO_TCP
#define DEFAULT_PORT				33000
#define MAX_PASSWORD				20
#define MAX_CLIENTS					8
#define MAX_MESSAGE_LENGTH			256
#define MAX_FTP_FILES				10				// Amount of files that can be sent with a single FTP request
#define MAX_FILENAME_LENGTH			64				// Max length of the name of a file to be sent
#define MAX_FTP_DATA				4000 /*102400*/	// Maximum data to be sent per FTP packet
/*
#define MAX_FTP_REQUESTS			128				// Maximum amount of Ftp requests to be active
#define MAX_FTP_TRANSFERS			(MAX_FTP_REQUESTS*MAX_CLIENTS)	// Maximum active FTP transfers
#define FTP_RESEND_TIME				300				// Time before resending a part of a file that have not yet been confirmed
#define FTP_DELAY_TIME				40				// Time to wait before even checking if new ftp data is needed to be sent
#define SHUTDOWN_SLEEP				100
#define PACKETLISTENER_SLEEP		20
#define NO_CLIENTS_SLEEP			2000
#define PING_CHECK_DELAY			200				// Time between we check each client if PING_DELAY have passed
#define PING_DELAY					2000			// Time between ping
#define PING_RESENDS				3				// Amount of times a ping packet should be sent and lost before declaring target disconnected
*/

// If more than 64 sockets are needed to be listen/select at the same time
// one must remember to redefine FD_SETSIZE larger than 64
// #define FD_SETSIZE 128; But remember, this must be defined before including WinSock2 !!!!!!!!!

/*

enum networkUserTypes {
	USER_TYPE_NULL,
	HOST,
	CLIENT
};

enum clientNetworkConnectionStatus {
	CONNECTION_STATUS_NULL,
	CONNECTION_STATUS_PENDING,
	CONNECTION_STATUS_CONNECTING,
	CONNECTION_STATUS_CONNECTED,
	CONNECTION_STATUS_DISCONNECTING,
	CONNECTION_STATUS_ERROR
};
*/

#endif	// NETWORK_SETTINGS_H
