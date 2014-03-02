/// Emil Hedemalm
/// 2013-10-20
/// Packet sizes. Needed for some reason..

/// Packets of other sizes should be registerable via a public function 
/// so that game-specific packets aren't inserted straight into this file.

/*

#include "Packets.h"
#include "../Packet.h"

int packet_size[PACKET_TYPES];

#include "Network/NetworkManager.h"


void NetworkManager::CalculateDefaultPacketSizes()
{
	int normalPacketSize = sizeof(Packet);
	for(int i = 0; i < PACKET_TYPES; ++i)
		packet_size[i] = normalPacketSize;

	// Add custom sized packets here
	// packet_size[PACKET_TYPE] = sizeof(Packet_Custom);
	packet_size[PACKET_CHAT] = sizeof(PacketChat);
	packet_size[PACKET_VERSION_CHECK] = sizeof(PacketVersionCheck);
	packet_size[PACKET_ESTABLISH_CONNECTION] = sizeof(PacketEstablishConnection);
	packet_size[PACKET_UPDATE_MY_CLIENT_INFO] = sizeof(PacketUpdateMyClientInfo);
	packet_size[PACKET_BROADCAST_CLIENTS_INFO] = sizeof(PacketBroadcastClientsInfo);
	packet_size[PACKET_MAKE_FTP_REQUEST] = sizeof(PacketMakeFtpRequest);
	packet_size[PACKET_ANSWER_FTP_REQUEST] = sizeof(PacketAnswerFtpRequest);
	packet_size[PACKET_FTP_DATA] = sizeof(PacketFtpData);
	packet_size[PACKET_FTP_DATA_REPLY] = sizeof(PacketFtpDataReply);
	packet_size[PACKET_FTP_FINISHED] = sizeof(PacketFtpFinished);

}
*/
