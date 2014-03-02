/// Emil Hedemalm
/// 2013-12-17
/// SIP-specific Packet Parser

#include "Network/Peer.h"
#include "SIPPacket.h"
#include "SIPPacketParser.h"
#include <cassert>
#include "Timer/Timer.h"
#include "Network/Socket/TcpSocket.h"
#include "Network/NetworkSettings.h"
#include <cstring>

SIPPacketParser::SIPPacketParser()
{
}

/** Class that takes care of packet parsing via TcpSockets. 
	Returns true upon success, false if the socket has disconnected. 
	Stores read packets in the List argument reference.
*/
bool SIPPacketParser::ReadPackets(Socket * sock, List<SIPPacket*> & packetsRead){
	List<SIPPacket*> packets;
	if (!sock)
		return false;
	static char buf[NETWORK_BUFFER_SIZE];
	memset(buf, 0, NETWORK_BUFFER_SIZE);
	int bytesRead =	sock->Read(buf, NETWORK_BUFFER_SIZE);
	if (bytesRead == 0)
		return true;
	else if (bytesRead == -1){
		return false;
	}
	std::cout << "Bytes read: "<<bytesRead;
	buf[bytesRead] = '\0';
	String buffer(buf);

	std::cout << "\n";
	std::cout << "\n";
	std::cout<<"\n=========================================";
	std::cout << "\nPackets received: \n"<<buffer;

//	buffer.PrintData();
		
	// New parser, using the fact that each line in SIP have to be ended with \r\n
	List<String> lines = buffer.GetLines();
	if (lines.Size() > 15){
		/// Debug please
		std::cout<<"Lall";
	}
	enum parseStates {
		NULL_STATE, STATE_NULL = NULL_STATE,
		PRE_HEADER,
		HEADER,
		BODY,
	};
	int state = NULL_STATE;
	// Evaluate each line, using the given state to know what to do with it!
	SIPPacket * packet = NULL;
	for (int i = 0; i < lines.Size(); ++i){
		String line = lines[i];
//		line.PrintData();
		switch(state){
			case NULL_STATE:
			case PRE_HEADER: {
				// If we see a valid header-start, change state!
				bool validHeader = false;
				if (line.Contains("SIP/")){
					validHeader = true;
				}
				// Optionally add an additional check here to make sure that it's a type of SIP packet that we are interested in, 
				// for example INVITE/OK/REGISTER/etc.
				if (validHeader){
			//		std::cout<< "\nHeader: "<<line;
					// If mid-loop (several packets), add the packet here before creating a new with the same pointer.
					if (packet)
						packets.Add(packet);
					packet = NULL;
					// Line deemed valid, create a new packet and start inserting header-rows into it!
					packet = new SIPPacket();
					packet->socket = sock;
					packet->header.Add(line);
					// And change state!
					state = HEADER;
				}
				break;
			}
			case HEADER:
				// If we find an empty line, we're at the end of the header!
				if (line.Length() == 0){
					packet->ParseHeader();
				//	std::cout << "\nEnd of header found, parsing it..";
					packet->PrintHeaderData();
					state = BODY;
				}
				// If not, continue to add lines to the header.
				else 
					packet->header.Add(line);
				break;
			case BODY:
				if (line.Length() == 0){
					state = STATE_NULL;
				}
				else {
					packet->body.Add(line);
				}
				break;
		}
	}
	// If packet is valid, save it to the list here as well!
	if (packet)
		packets.Add(packet);
	std::cout<<"\nPackets parsed/received: "<<packets.Size();
	packet = NULL;
	// Save packets read into the argument list-reference.
	packetsRead += packets;
	return true;	
};
