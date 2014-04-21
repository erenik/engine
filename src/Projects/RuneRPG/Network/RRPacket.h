/// Emil Hedemalm 
/// 2014-01-28
/// Packet-handling for space-race packets

#ifndef SR_PACKET_H
#define SR_PACKET_H

#include "Network/Packet/Packet.h"
#include "MathLib.h"
#include "Network/Socket/UdpSocket.h"

/// For now, use it similar to SIP, i.e. text-based?
class RRPacket : public Packet {
public:
	/// Type as defined in RRPacketTypes.h
	RRPacket(int type);

	/// Virtual destructor so that subclass destructors are run correctly.
	virtual ~RRPacket();
	
	/// Sends this packet's data to target Peer, using necessary packet-headers.
	virtual bool Send(Peer * peer);
	/// Sends this packet's data to target Peer, using UDP only.
	virtual bool SendUdp(Peer * toPeer, UdpSocket * usingSocket, int andPort);
	/// Sends this packet's data to target QTcpSocket, using necessary packet-headers.
	virtual bool Send(Socket * sock);

	/** Attempts to parse packets from a given buffer of raw data. 
		Returns amount of packets successfully parsed.
		The amount of bytes to parsed MUST be less than the size of the buffer.
	*/
	static int ParsePackets(char * fromBuffer, int upToThisNumberOfBytes, List<RRPacket*> & andStorePacketsHere);

	/// True upon success, false if socket failure. Packets are concatenated to the list.
	static bool ReadPackets(Socket * fromSocket, List<RRPacket*> & packetList);
	/// True upon success, false if socket failure. Packets are concatenated and headers are built.
	static bool SendPackets(Socket * toSocket, List<RRPacket*> & packetList);
	
	/// Creates a header using givent type, generates a time-stamp, packet number, etc.
	void CreateHeader();
	/// Parses type of packet, time sent etc.
	void ParseHeader();
	/// E.g. Name: Boo will return Boo if forKey is "Name:"
	String GetHeaderValue(String forKey);
	/// Woo
	static String GetPacketName(int byType);
	int GetTypeByString(String type);

	// Getter function that parses header for the name-field.
	String GetSender();

	/// Yes.
	static Peer * defaultSender;
	
	/// Time the packet was created on the sender part.
	long long timeCreated;

	/// Lines for header and body.
	List<String> header;
	List<String> body;
	/// See RRPacketTypes.
	int srPacketType;
	/// If the packet has been parsed. This will be used for certain packets that may require further parsing after retrieval in order to extract relevant data.
	bool parsed;
protected:
	/// Merges header and body fields into the main data used by all sockets when sending.
	void CreateData();
private:
	bool headerCreated;
};

class RRChatPacket : public RRPacket {
public:
	/// Name and text, yup.
	RRChatPacket(String name, String text);

	/// Extracts the Text-field from the body.
	String GetText();
};

/// Registration class, so synchronize with the data previously provided in similar SIP requests.
class SRRegister : public RRPacket {
public:
	SRRegister(Peer * me);
};

/// Mid-class to subclass for all reply packets to respond somehow.
class SRReplyPacket : public RRPacket {
public:
	SRReplyPacket(int type, RRPacket * packetToReply);
	/// Returns the topic/request/line that holds what this packet was replying to.
	String GetRequest();
};

/// Explicit acknowledgements for requests
class SROKPacket : public SRReplyPacket {
public:
	SROKPacket(RRPacket * packetToReply);
};
/// Explicit refusals
class SRDeclinePacket : public SRReplyPacket {
public:
	SRDeclinePacket(RRPacket * packetToReply, String reason);
	/// Returns the same reason that was set in creation.
	String GetReason();
};

class SRRequestPlayersPacket : public RRPacket {
public:
	/** From client. Args: Action (see RRPlayers enum above), number of affected local players, additional arguments.
		Note that any semi-colons in the names will be removed.
	*/
	SRRequestPlayersPacket(int action, int amount, List<String> names);
	/// Call once after receiving the packet in order to extract the below data. Flags the parsed variable to true upon completion.
	void Parse(int & action, int & amount, List<String> & names);

	enum actions {
		N,
		ADD,
		SET,
		REMOVE	
	};

};

class RRPlayersPacket : public RRPacket {
public:
	/// From host. Args: a number of all players, a list of all player names, a list of all indices which this peer should be in control of.
	RRPlayersPacket(int number, List<String> playerNames, List<int> clientPlayerIndices);
	/// Call once after receiving the packet in order to extract the below data. Flags the parsed variable to true upon completion.
	void Parse(int & number, List<String> & playerNames, List<int> & clientPlayerIndices);
};

class RRPlayerMovePacket : public RRPacket {
public:
	RRPlayerMovePacket(int playerIndex);
	void SetMessage(String msg);
	/// Extracts data from this packet
	bool Parse(int & playerID, String & msg);
};

/// Update packet for a single player.
class RRPlayerPositionPacket : public RRPacket {
public:
	/// Player index, position, rotation and state for synchronizing visual effects correctly.
	RRPlayerPositionPacket(int playerID, Vector3f position, Vector3f velocity,  Vector3f rotation, String state, long long gameTime);
	/// Call once after receiving the packet in order to extract the below data. Flags the parsed variable to true upon completion.
	void Parse(int & playerID, Vector3f & position, Vector3f & velocity, Vector3f & rotation, String & state);
	/*
	int playerID;
	Vector3f position;
	*/
};

/// Will contain messages Start, Countdown and Finished. Start starts the race, Countdown starts countdown before the start. Finished notices the race is finished.
class  RRGeneralPacket : public RRPacket {
public:
	RRGeneralPacket(String msg, String additionalData = "");
	/// Call once after receiving the packet in order to extract the below data. Flags the parsed variable to true upon completion.
	void Parse(String & msg, String & additionalData);
};


#endif
