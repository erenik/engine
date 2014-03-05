// Emil Hedemalm
// 2013-09-02

#ifndef SR_PACKETS_H
#define SR_PACKETS_H

#include "Network/Packet/PacketTypes.h"

namespace SRPacketType {
enum SRPacketTypes {
    PACKET_NULL,
	UNIDENTIFIED, // Used when reading from socket, before identification.
	REGISTER, // For hand-shake procedure.
	OK,	// Formal accept response
	DECLINE,	// Formal decline response
	CHAT, // Chat-packet, for this game's lobby primarily.
	
	/// As defined in SRPackets.txt on dropbox.
	REQUEST_PLAYERS,
	PLAYERS,
	PLAYER_POSITION,
	PLAYER_MOVE,
	PAUSE,
	RACE,
	READY,
	
	/// Add packets for controling race-track and ships here later on prob.
	PACKETS_IN_USE,

	

	LOCAL_PLAYERS_INCREASED,
    SET_NAME,
    START_GAME,
    GO_TO_LOBBY,
    THRUST,
    STOP_THRUSTING,
    POSITION,
	TRANSFORM, // Pos + Orientation
    PACKET_TYPES,
};};

void SRSetPacketSize();

#endif



