/// Emil Hedemalm
/// 2013-12-17
/// SIP-specific Packet Parser

#ifndef SIP_PACKET_PARSER_H
#define SIP_PACKET_PARSER_H

class TcpSocket;
class Peer;
class SIPPacket;

/// Class that takes care of packet parsing via TcpSockets, conforming to the SIP Protocol
class SIPPacketParser {
public:
	SIPPacketParser();
	/** Class that takes care of packet parsing via TcpSockets. 
		Returns true upon success, false if the socket has disconnected. 
		Stores read packets in the List argument reference.
	*/
	bool ReadPackets(Socket * sock, List<SIPPacket*> & packetsRead);
};

#endif
