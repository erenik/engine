/// Emil Hedemalm
/// 2013-10-20
/// Enumerations for network packets.

#ifndef PACKET_TYPES_H
#define PACKET_TYPES_H

/// New packet types

namespace PacketType {
enum packetTypes{
	NULL_TYPE,
	TEXT, // Raw text packet for testing purposes.
	SIP, // General Session Initiation Protocol
	AUDIO_TRANSMISSION, // Audio transmission
	VIDEO, // Video transmission
	FTP, // File transfer protocol
	SYNCHRONIZATION, // Synchronization protocol for basic entity attributes.
	GAME_SPECIFIC, // Game specific protocol, this might require further thinking...
	SR_PACKET, // Space race packet
	GAME_SPECIFIC_START,
	GAME_SPECIFIC_END,
	PACKET_TYPES,
};};


namespace PacketTarget {
enum packet_targets{
	NONE,
	HOST,
	ALL,
	PLAYER,
	OTHERS,
	GROUP,
};};




/*

enum packet_types{
	NULL_PACKET,

	// General packets
	PACKET_CHAT,

	// Connection
	PACKET_VERSION_CHECK,
	PACKET_ESTABLISH_CONNECTION,
	PACKET_PING,
	PACKET_PING_REPLY,

	// Updating packets
	PACKET_UPDATE_MY_CLIENT_INFO,
	PACKET_BROADCAST_CLIENTS_INFO,
	PACKET_REQUEST_CLIENT_INFO,

	// FTP
	PACKET_MAKE_FTP_REQUEST,
	PACKET_ANSWER_FTP_REQUEST,
	PACKET_FTP_DATA,									// Contains data from a file that is included in a Ftp transfer
	PACKET_FTP_DATA_REPLY,							// A reply to the sender that a part of a file was successfully received
	PACKET_FTP_FINISHED,							// Tells that an FTP transfer was successful!

	// Allow some random game selection packets..?
    PACKET_GAME_SELECT = 50,
    PACKET_GAME_SELECT_END = PACKET_GAME_SELECT + 20,

	// Then start specifying packets for the specific game?
    PACKET_GAME_SPECIFIC_START = 100,
    PACKET_GAME_SPECIFIC_END = PACKET_GAME_SPECIFIC_START + 155,

	// Last one
	PACKET_TYPES
};
*/

/// Woo
//extern int packet_size[PACKET_TYPES];

#endif // PACKET_TYPES_H