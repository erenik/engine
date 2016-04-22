// EWmil Hedemalm & Aron Wåhlberg
#ifndef NETWORKPACKETS_H
#define NETWORKPACKETS_H

#include "../NetworkSettings.h"

#ifdef USE_NETWORK

#include "NetworkIncludes.h"
#include <ctime>
#include "Globals.h"
#include "FtpManager.h"

/// Calculates all packet sizes
void CalculatePacketSize();

struct Packet{					// 24 byte
	/// Public packet size registration function.
	static void SetPacketSize(int packetType, int packetSize);

	unsigned char packetType;			// Type of packet, packet_types
	int packetSize;				// Size of packet
	char target;				// Target player
	char sender;				// Sender player
	long long timeCreated;		// Time since packet creation
	in_addr senderIP;			// Because IP is nice?
	Packet(char type, char target = -1);
};

//////////////////////////////////////////////////////////////////////////
// Custom Packets
/** A simple chat packet, contains a message up to 256 characters
	Could be sent by anyone to anyone
*/
struct PacketChat : Packet				// 280 byte
{
	PacketChat(const char *msg );
	char message[MAX_MESSAGE_LENGTH];	// Chat message
};

//////////////////////////////////////////////////////////////////////////
// Connection & Update packets
/** A packet for checking version of client
	NetworkClient should send to host who then verifies their info
	If info isn't accepted, a PacketVersionCheck is sent back
*/
struct PacketVersionCheck : Packet
{
	PacketVersionCheck();
	char clientName[NAME_LIMIT];		// Clients name
	float gameEngineVersion;			// Version on the Game Engine
	float gameVersion;					// Version on the Game
};
/** A packet telling the client that his connection is stable
	NetworkClient should send this to host before doing anything else with the network
*/
struct PacketEstablishConnection : Packet
{
	PacketEstablishConnection(int clientIndex);
	int clientIndex;					// In what the client is stored
};
/** A packet that send a clients information to the host
	NetworkClient should send this to the host which should reply with PacketBroadcastClientsInfo
*/
struct PacketUpdateMyClientInfo : Packet
{
	PacketUpdateMyClientInfo();

	int connectionStatus;
	char IP[NAME_LIMIT];
	char name[NAME_LIMIT];
	bool versionCheck;
};
/** A packet that contains information about the client
	Sent from host to all clients
*/
struct PacketBroadcastClientsInfo : Packet
{
	PacketBroadcastClientsInfo();

	int connectionStatus[MAX_CLIENTS];
	char IP[MAX_CLIENTS][NAME_LIMIT];
	char name[MAX_CLIENTS][NAME_LIMIT];
	bool versionCheck[MAX_CLIENTS];
};

//////////////////////////////////////////////////////////////////////////
// FTP
/** A FTP request.
	Contains information about files to be sent.
*/
struct FtpRequest;
struct PacketMakeFtpRequest : Packet
{
	PacketMakeFtpRequest(FtpRequest *request);

	int ftpID;											// To keep track of specific FTP request
	int files;											// Amount of files that are to be sent
	char filenames[MAX_FTP_FILES][MAX_FILENAME_LENGTH];	// Name of the files, with extension and all
	unsigned int filesizes[MAX_FTP_FILES];				// Size of the files in bytes
	int packets[MAX_FTP_FILES];							// Amount of packets each file will be split into
};
/** Answer of a FTP request
	Contains a bool, true or false.
*/
struct PacketAnswerFtpRequest : Packet
{
	PacketAnswerFtpRequest(int ftpID, bool answer);

	int ftpID;				// To keep track of what FTP request this is a answer to
	bool answer;			// The answer of course
};
/** A FTP data packet
	Contains the data of a part of a file in the FTP request.
*/
struct PacketFtpData : Packet
{
	PacketFtpData(int ftpID, int fileIndex, int packetNumber, char data[MAX_FTP_DATA], unsigned long sizeToRead);
	int ftpID;				// Id of ftp request
	int fileIndex;			// Index of file the data is belonging to
	int packetNumber;		// This specific packet's number
	char data[MAX_FTP_DATA];// The file data
	unsigned long sizeToRead;			// Tells the length of the data
};
/** Confirmation of a FTP data packet
	Confirms that a data part has been received.
*/
struct PacketFtpDataReply : Packet
{
	PacketFtpDataReply(int ftpID, int fileIndex, int packetNumber);
	int ftpID;				// Id of ftp request
	int fileIndex;			//ŘŘ Index of file the data is belonging to
	int packetNumber;		// This specific packet's number
};
/** FTP request finished
	Tells a receiver of a FTP request that it's finished.
*/
struct PacketFtpFinished : Packet
{
	PacketFtpFinished(int ftpID);
	int ftpID;				// Id of the specific ftp request/transfer
};

#endif // USE_NETWORK

#endif
