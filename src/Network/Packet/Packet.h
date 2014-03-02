/// Emil Hedemalm
/// 2013-11-28
/// Packet encapsulation

#ifndef PACKET_H
#define PACKET_H

class Socket;
class Peer;

#include "String/AEString.h"
#include "PacketTypes.h"

/*
struct Packet{					// 24 byte
	int packetSize;				// Size of packet
	char target;				// Target player
	char sender;				// Sender player
	long long timeCreated;		// Time since packet creation
	in_addr senderIP;			// Because IP is nice?
	Packet(char type, char target = -1);
};
*/

/// Class to encapsulate all data that is sent, so it is possible to properly parse several at a time.
class Packet {
	friend class Session;
public:
	/// Public packet size registration function.
	static void SetPacketSize(int packetType, int packetSize);

	/// Empty constructor that sets data to "EmptyPacket"
	Packet(int type);
	/// Virtual destructor so that subclass destructors are run correctly.
	virtual ~Packet();
	/// Copy constructor that sets data.
	Packet(const Packet & copy);
	/// Operator overloading so that it works correctly.
	const Packet & operator= (const Packet & copy);

	/// Sends this packet's data to target Peer, using necessary packet-headers.
	virtual bool Send(Peer * peer);
	/// Sends this packet's data to target QTcpSocket, using necessary packet-headers.
	virtual bool Send(Socket * sock);

	/// Type of packet, 0 = default, 1 = SIP 
	int type;
	/// Total raw data that is to be sent.
	String data;
	/// Total size of received data.
	int size;
	/// Sender
	Peer * sender;
	/// Id of the packet as enumerated by the peer in question.
	int id;
	/// Recipients
	int target;
	/// Socket from which this packet was received.
	Socket * socket;

private:
	/// List of packet size types.
	static int packet_size[PacketType::PACKET_TYPES]; 
};

#endif

