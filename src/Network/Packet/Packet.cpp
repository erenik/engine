/// Emil Hedemalm
/// 2013-11-28
/// Packet encapsulation

#include "Packet.h"
#include "Network/Peer.h"
#include "Network/Socket/Socket.h"
#include "Network/Packet/PacketTypes.h"
#include <cassert>

/// List of packet size types.
int Packet::packet_size[PacketType::PACKET_TYPES]; 

/// Public packet size registration function.
void Packet::SetPacketSize(int packetType, int packetSize){
	assert(packetType >= 0 && packetType < PacketType::PACKET_TYPES);
	assert(packetSize >= 0);
	packet_size[packetType] = packetSize;
}

Packet::Packet(int type)
: type(type)
{
	socket = NULL;
	sender = NULL;
}
Packet::~Packet(){};

Packet::Packet(const Packet & copy)
{
	data = copy.data;
	sender = NULL;
}
const Packet & Packet::operator= (const Packet & copy){
	data = copy.data;
	return *this;
}

/// Sends this packet's data to target Peer, using necessary packet-headers.
bool Packet::Send(Peer * peer){
	return Send(peer->primaryCommunicationSocket);
}	

bool Packet::Send(Socket * sock)
{
	if (!sock)
		return false;
	assert(size > 0);
	int bytesWritten = sock->Write((const char *) data.GetData(), data.Bytes());
	return bytesWritten > 0;
}







