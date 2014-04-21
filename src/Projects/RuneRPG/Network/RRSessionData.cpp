/// Emil Hedemalm
/// 2014-01-30
/// Session data related to space race.

#include "RRSessionData.h"
#include "Network/Session/GameSessionTypes.h"
#include "Network/Socket/Socket.h"

RRSessionData::RRSessionData()
: SessionData(SessionType::GAME, GameSessionType::SPACE_RACE)
{
	srSocket = NULL;
	udpWorking = 0;
	lastReceivedUdpPacket = 0;
	requestedProtocol = TransmissionProtocol::UDP;
	ready = false;

	/// Some numbers for the udp initiation test
	udpTestPacketsSent = 0;
	udpTestPacketsReceived = 0;
	udpTestReceivedPacketsReceived = 0;
	udpWorkingPacketsSent = 0;
	udpWorkingPacketsReceived = 0;
	
	udpWorkingForUs = udpWorkingForPeer = false;
	udpWorking = false;

	connectionState = NOT_CONNECTED;
}
