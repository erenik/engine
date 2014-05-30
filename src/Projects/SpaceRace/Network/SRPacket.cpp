/// Emil Hedemalm
/// 2014-01-28
/// Packet-handling for space-race packets

#include "SRPacket.h"
#include "Network/Socket/Socket.h"
#include "Network/NetworkSettings.h"
#include "SRPacketTypes.h"
#include "String/StringUtil.h"
#include "SRPacketTypes.h"
#include "Network/Peer.h"
#include "SRSessionData.h"
#include "Network/Session/SessionTypes.h"
#include "Timer/Timer.h"
#include <cstdio>

#define SR_PACKET_HEADER_START	"SR_PACKET 1.0"
#define SR_PACKET_HEADER_END "SR_PACKET_BODY"
#define SR_PACKET_END "SR_PACKET_END"

Peer * SRPacket::defaultSender = NULL;

SRPacket::SRPacket(int srPacketType)
: Packet(PacketType::SR_PACKET), srPacketType(srPacketType)
{
	parsed = false;
	timeCreated = 0;
	headerCreated = false;
}

/// Virtual destructor so that subclass destructors are run correctly.
SRPacket::~SRPacket(){

}

/// Sends this packet's data to target Peer, using necessary packet-headers.
bool SRPacket::Send(Peer * peer)
{
	if (!peer){
		std::cout<<"\nSRPacket::Send: Trying to send to NULL peer, aborting.";
		return false;
	}
	/// Fetch primary socket as according to the SRData that the peer should have.
	SRSessionData * srsd = (SRSessionData*)peer->GetSessionData(SessionType::GAME);
	assert(srsd && "Peer lacking SRSessionData despite trying to send an SRPacket!");
	if (!srsd)
		return false;
	/// If no socket (disconnected player), don't send.
	if (!srsd->srSocket)
	{
		std::cout<<"\nSRSessionData lacking valid SRSocket to send to!";
		return false;
	}
	int result = 0;
	result = srsd->srSocket->Write(data.c_str(), data.Length());
	return result > 0;
}

/// Sends this packet's data to target Peer, using UDP only.
bool SRPacket::SendUdp(Peer * peer, UdpSocket * usingSocket, int andPort){
	if (!peer){
		std::cout<<"\nSRPacket::SendUdp: Trying to send to NULL peer, aborting.";
		return false;
	}
	if (!usingSocket){
		std::cout<<"\nSRPacket::SendUdp: Trying to send to a NULL UdpSocket.";
		return false;
	}
	int result = 0;
	/// IP addresses: 33000 SIP, 33001 SR TCP, 33002 SR Host UDP, 33003 SR Client UDP
	result = usingSocket->WriteTo(peer->ipAddress, String::ToString(andPort), data.c_str(), data.Length());
	return result > 0;
}

/// Sends this packet's data to target QTcpSocket, using necessary packet-headers.
bool SRPacket::Send(Socket * sock)
{
	int result = sock->Write(data.c_str(), data.Length());
	return result > 0;
}

/// Attempts to parse packets from a given buffer of raw data. Returns amount of packets successfully parsed.
int SRPacket::ParsePackets(char * fromBuffer, int upToThisNumberOfBytes, List<SRPacket*> & andStorePacketsHere)
{
	/// Set null-sign at the end if not already there.
	fromBuffer[upToThisNumberOfBytes] = '\0';

	String s = fromBuffer;
	List<String> lines = s.GetLines();
	/// Parse lines
	enum parseStates {
		NULL_STATE, STATE_NULL = NULL_STATE,
		PRE_HEADER,
		HEADER,
		BODY,
	};
	int state = NULL_STATE;
	// Evaluate each line, using the given state to know what to do with it!
	SRPacket * packet = NULL;
	List<SRPacket*> packets;
	for (int i = 0; i < lines.Size(); ++i){
		String line = lines[i];
	//	std::cout<<"\nLine "<<i<<": "<<lines[i];
		/// If line-count exceeds a certain limit, halt processing and read more later.
		if (i > 500)
		{
			std::cout<<"\nToo many lines, ignoring rest of socket buffer.";
			if (packet)
				delete packet;
			packet = NULL;
			break;
		}
		switch(state){
			case NULL_STATE:
			case PRE_HEADER: {
				// If we see a valid header-start, change state!
				bool validHeader = false;
				if (line.Contains(SR_PACKET_HEADER_START)){
					validHeader = true;
				}
				// Optionally add an additional check here to make sure that it's a type of SIP packet that we are interested in,
				// for example INVITE/OK/REGISTER/etc.
				if (validHeader){
				//	std::cout<< "\nHeader: "<<line;
					// If mid-loop (several packets), add the packet here before creating a new with the same pointer.
					if (packet)
						packets.Add(packet);
					packet = NULL;
					// Line deemed valid, create a new packet and start inserting header-rows into it!
					packet = new SRPacket(SRPacketType::UNIDENTIFIED);
				//	packet->header.Add(line);
					// And change state!
					state = HEADER;
				}
				break;
			}
			case HEADER:
				// If we find an empty line, we're at the end of the header!
				if (line.Contains(SR_PACKET_HEADER_END)){
			//		std::cout << "\nEnd of header found, parsing it..";
					packet->ParseHeader();
					state = BODY;
				}
				// If not, continue to add lines to the header.
				else
					packet->header.Add(line);
				break;
			case BODY:
				if (line == SR_PACKET_END){
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
	packet = NULL;
	// Copy stuff into data field of them so that we may easily re-transmit them.
	for (int i = 0; i < packets.Size(); ++i){
		SRPacket * srp = packets[i];
		srp->headerCreated = true;
		srp->CreateData();
	}
	// Save packets read into the argument list-reference.
	andStorePacketsHere += packets;
	return packets.Size();
}

/// True upon success, false if socket failure. Packets are concatenated to the list.
bool SRPacket::ReadPackets(Socket * sock, List<SRPacket*> & packetsRead){
	char packetBufferData [NETWORK_BUFFER_SIZE];
	int bytesRead = sock->Read(packetBufferData, NETWORK_BUFFER_SIZE);
	if (bytesRead <= 0)
		return false;
	int lastByte = bytesRead;
	if (lastByte >= NETWORK_BUFFER_SIZE)
		lastByte = NETWORK_BUFFER_SIZE - 1;
	List<SRPacket*> packets;
	ParsePackets(packetBufferData, lastByte, packets);
	for (int i = 0; i < packets.Size(); ++i){
		SRPacket * packet = packets[i];
		packet->socket = sock;
	}
	packetsRead += packets;
	return true;
}

/// True upon success, false if socket failure. Packets are concatenated and headers are built.
bool SRPacket::SendPackets(Socket * toSocket, List<SRPacket*> & packetList){
	String data;
	for (int i = 0; i < packetList.Size(); ++i){
		SRPacket * p = packetList[i];
		data += p->data;
	}
	/// Write
	int result = toSocket->Write(data.c_str(), data.Length());
	return result >= 1;
}

/// Creates a header using givent type, generates a time-stamp, packet number, etc.
void SRPacket::CreateHeader(){
	header.Clear();
	header.Add(String::ToString(srPacketType)+" "+this->GetPacketName(srPacketType));
	if (sender == NULL)
		sender = defaultSender;
	header.Add("Sender: "+this->sender->name);
	/// Allow for custom times to be used, but if 0 use current time.
	if (timeCreated == 0)
		timeCreated = Timer::GetCurrentTimeMs();
	char c[256];
	sprintf(c, "%lld", timeCreated);
	String timeString = "Time: "+String(c);
	header.Add(timeString);
	headerCreated = true;
}

/// Parses type of packet, time sent etc.
void SRPacket::ParseHeader(){
	// Header rwo 2 for type.
	for (int i = 0; i < header.Size(); ++i){
		String line = header[i];
		switch(i){
			case 0:
			{
				String type = line.Tokenize(" ")[1];
				this->srPacketType = GetTypeByString(type);
				break;
			}
			// Sender
			case 1:
			{
				break;
			}
			// Time
			case 2:
			{
				String timePart = line.Tokenize(" ")[1];
				timeCreated = StringToLongLong(timePart.c_str());
			}
		}
	}
}

/// E.g. Name: Boo will return Boo if forKey is "Name:"
String SRPacket::GetHeaderValue(String forKey){
	for (int i = 0; i < header.Size(); ++i){
		String line = header[i];
		if (line.Contains(forKey)){
			String ret = line - forKey;
			ret.RemoveInitialWhitespaces();
			return ret;
		}
	}
	return "";
}

/// Woo
String SRPacket::GetPacketName(int byType){
	switch(byType){
		case SRPacketType::PACKET_NULL: return "NULL_PACKET";
		case SRPacketType::UNIDENTIFIED: return "Unidentified";
		case SRPacketType::REGISTER: return "Register";
		case SRPacketType::CHAT: return "Chat";
		case SRPacketType::OK:	return "OK";
		case SRPacketType::DECLINE: return "Decline";
		case SRPacketType::REQUEST_PLAYERS: return "RequestPlayers";
		case SRPacketType::PLAYERS: return "Players";
		case SRPacketType::RACE: return "Race";
		case SRPacketType::PLAYER_POSITION: return "PlayerPosition";
		case SRPacketType::PLAYER_MOVE: return "PlayerMove";
		case SRPacketType::PAUSE: return "Pause";
		case SRPacketType::READY: return "Ready";
		default:
			assert(false && "Unknown type, add?");
			return "Unknown or not implemented";
	}
}

int SRPacket::GetTypeByString(String type){
	for (int i = SRPacketType::REGISTER; i < SRPacketType::PACKETS_IN_USE; ++i)
	{
		if (GetPacketName(i) == type)
			return i;
	}
	return SRPacketType::PACKET_NULL;
}

/// Merges header and body fields into the main data used by all sockets when sending.
void SRPacket::CreateData(){
	assert(headerCreated);
	List<String> lines;
	lines += SR_PACKET_HEADER_START;
	lines += header;
	lines += SR_PACKET_HEADER_END;
	lines += body;
	lines += SR_PACKET_END;
	/// Empty line between so we get decent new lines between each packet (necessary).
	lines += "";
	data = MergeLines(lines);
}

// Will have name, so getter function for it.
String SRPacket::GetSender(){
	return this->GetHeaderValue("Sender:");
}

/// Name and text, yup.
SRChatPacket::SRChatPacket(String name, String text)
: SRPacket(SRPacketType::CHAT)
{
	body.Add(text);
	CreateHeader();
	CreateData();
}


/// Extracts the Text-field from the body.
String SRChatPacket::GetText()
{
	return MergeLines(body);
}


SRRegister::SRRegister(Peer * me)
: SRPacket(SRPacketType::REGISTER)
{
	sender = me;
	body.Add("Name: "+me->name);
	body.Add(me->ipAddress);
	CreateHeader();
	CreateData();
}


SRReplyPacket::SRReplyPacket(int type, SRPacket *packetToReply)
: SRPacket(type)
{
	body.Add("Request: "+GetPacketName(packetToReply->srPacketType));
}

/// Returns the topic/request/line that holds what this packet was replying to.
String SRReplyPacket::GetRequest(){
	String request = body[0];
	assert(request.Contains("Request: "));
	request.Remove("Request: ");
	return request;
}

SROKPacket::SROKPacket(SRPacket *packetToReply)
: SRReplyPacket(SRPacketType::OK, packetToReply)
{
		CreateHeader();
		CreateData();
}

SRDeclinePacket::SRDeclinePacket(SRPacket *packetToReply, String reason)
: SRReplyPacket(SRPacketType::DECLINE, packetToReply)
{
	body.Add("Reason: "+reason);
	CreateHeader();
	CreateData();
}

/// Returns the same reason that was set in creation.
String SRDeclinePacket::GetReason()
{
	String d = body[1];
	assert(d.Contains("Reason: "));
	d.Remove("Reason: ");
	return d;
}


/// From client. Args: Action (see SRPlayers enum above), number of affected local players, additional arguments.
SRRequestPlayersPacket::SRRequestPlayersPacket(int action, int amount, List<String> names)\
: SRPacket(SRPacketType::REQUEST_PLAYERS)
{
	body.Add(String::ToString(action));
	body.Add(String::ToString(amount));
	for (int i = 0; i < names.Size(); ++i){
		names[i].Remove(";", true);
	}
	body.Add(MergeLines(names, ";"));
	CreateHeader();
	CreateData();
}

/// Call once after receiving the packet in order to extract the below data. Flags the parsed variable to true upon completion.
void SRRequestPlayersPacket::Parse(int & action, int & amount, List<String> & names)
{
	action = body[0].ParseInt();
	amount = body[1].ParseInt();
	names = body[2].Tokenize(";");
}

/// Args: a number of all players, a list of all player names, a list of all indices which this peer should be in control of.
SRPlayersPacket::SRPlayersPacket(int number, List<String> playerNames, List<String> playerShips, List<int> clientPlayerIndices)
: SRPacket(SRPacketType::PLAYERS)
{
	body.Add(String::ToString(number));
	String playerNamesString = MergeLines(playerNames, ";");
	String playerShipsString = MergeLines(playerShips, ";");
	String playerIndices;
	assert(number == playerNames.Size());
	for (int i = 0; i < clientPlayerIndices.Size(); ++i){
		playerIndices += String::ToString(clientPlayerIndices[i]) + ";";
	}
	body.Add(playerNamesString);
	body.Add(playerShipsString);
	body.Add(playerIndices);
	CreateHeader();
	CreateData();
}

void SRPlayersPacket::Parse(int & number, List<String> & playerNames, List<String> & playerShips, List<int> & clientPlayerIndices){
	number = body[0].ParseInt();
	playerNames = body[1].Tokenize(";");
	playerShips = body[2].Tokenize(";");
	List<String> strI;
	if (body.Size() > 2){
		strI = body[3].Tokenize(";");
		for (int i = 0; i < strI.Size(); ++i)
			clientPlayerIndices.Add(strI[i].ParseInt());
	}
	parsed = true;
}


SRPlayerMovePacket::SRPlayerMovePacket(int playerIndex)
: SRPacket(SRPacketType::PLAYER_MOVE)
{
	body.Add(String::ToString(playerIndex));
	body.Add("DummyMessageString");
}

void SRPlayerMovePacket::SetMessage(String msg){
	body[1] = msg;
	/// Create data now that custom message has been set.
	CreateHeader();
	CreateData();
}

/// Extracts data from this packet
bool SRPlayerMovePacket::Parse(int & playerID, String & msg)
{
	/// Bad data!
	if (body.Size() < 2)
		return false;
	playerID = body[0].ParseInt();
	msg = body[1];
	return true;
}

Vector3f ParseVector3f(String s)
{
	List<String> sl = s.Tokenize(" ");
	assert(sl.Size() > 2);
	if (sl.Size() < 3)
		return Vector3f();
	return Vector3f(sl[0].ParseFloat(), sl[1].ParseFloat(), sl[2].ParseFloat());
}

SRPlayerPositionPacket::SRPlayerPositionPacket(int playerID, Vector3f position, Vector3f velocity, Vector3f rotation, String state, long long gameTime)
: SRPacket(SRPacketType::PLAYER_POSITION)
{
	timeCreated = gameTime;
	body.Add(String::ToString(playerID));
	body.Add(VectorString(position));
	body.Add(VectorString(velocity));
	body.Add(VectorString(rotation));
	body.Add(state);
	CreateHeader();
	CreateData();
}

void SRPlayerPositionPacket::Parse(int & playerID, Vector3f & position, Vector3f & velocity, Vector3f & rotation, String & state)
{
	int i = 0;
	playerID = body[i++].ParseInt();
	position = ParseVector3f(body[i++]);
	velocity = ParseVector3f(body[i++]);
	rotation = ParseVector3f(body[i++]);
	state = body[i++];
	parsed = true;
}

SRRacePacket::SRRacePacket(String msg, String additionalData)
: SRPacket(SRPacketType::RACE)
{
	body.Add(msg);
	body.Add(additionalData);
	CreateHeader();
	CreateData();
}

void SRRacePacket::Parse(String & msg, String & additionalData)
{
	msg = body[0];
	if (body.Size() > 1)
		additionalData = body[1];
	parsed = true;
}
