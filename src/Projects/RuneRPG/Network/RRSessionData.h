/// Emil Hedemalm
/// 2014-01-30
/// Session data related to space race.

#include "Network/Session/SessionData.h"
#include "List/List.h"
#include "Network/Socket/UdpSocket.h"
#include "Network/Protocol.h"

class Socket;
class RRPlayer;

enum connectionStates {
	NOT_CONNECTED,
	CONNECTED,
	DISCONNECTED,
};

class RRSessionData : public SessionData {
public:
	RRSessionData();
	/// Socket to be used for primary Space-race communication.
	Socket * srSocket;

	/// Address used to send udp packets to this peer. Negotiated via TCP before sending.
	sockaddr udpAddress;

	/// NOT_CONNECTED, CONNECTED, DISCONNECTED.
	int connectionState;

	/// Some numbers for the udp initiation test
	int udpTestPacketsSent;
	int udpTestPacketsReceived;
	int udpTestReceivedPacketsReceived;
	int udpWorkingPacketsSent;
	int udpWorkingPacketsReceived;
	/// To know if we should use udp or fallback to TCP.
	bool udpWorkingForUs;
	bool udpWorkingForPeer;
	/// Only set to true after both testing that udp works for us and our peer.
	bool udpWorking;	

	/// For throwing away old packets.
	long long lastReceivedUdpPacket;
	/// Client's requested protocol. 0 = TCP, 1 = UDP.
	int requestedProtocol;

	/// If ready to play.
	bool ready;

	/// Players this peer is in control of.
	List<RRPlayer*> players;
};
