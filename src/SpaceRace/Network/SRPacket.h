/// Emil Hedemalm 
/// 2014-01-28
/// Packet-handling for space-race packets

#ifndef SR_PACKET_H
#define SR_PACKET_H

#include "Network/Packet/Packet.h"
#include "MathLib.h"
#include "Network/Socket/UdpSocket.h"

/// For now, use it similar to SIP, i.e. text-based?
class SRPacket : public Packet {
public:
	/// Type as defined in SRPacketTypes.h
	SRPacket(int type);

	/// Virtual destructor so that subclass destructors are run correctly.
	virtual ~SRPacket();
	
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
	static int ParsePackets(char * fromBuffer, int upToThisNumberOfBytes, List<SRPacket*> & andStorePacketsHere);

	/// True upon success, false if socket failure. Packets are concatenated to the list.
	static bool ReadPackets(Socket * fromSocket, List<SRPacket*> & packetList);
	/// True upon success, false if socket failure. Packets are concatenated and headers are built.
	static bool SendPackets(Socket * toSocket, List<SRPacket*> & packetList);
	
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
	/// See SRPacketTypes.
	int srPacketType;
	/// If the packet has been parsed. This will be used for certain packets that may require further parsing after retrieval in order to extract relevant data.
	bool parsed;
protected:
	/// Merges header and body fields into the main data used by all sockets when sending.
	void CreateData();
private:
	bool headerCreated;
};

class SRChatPacket : public SRPacket {
public:
	/// Name and text, yup.
	SRChatPacket(String name, String text);

	/// Extracts the Text-field from the body.
	String GetText();
};

/// Registration class, so synchronize with the data previously provided in similar SIP requests.
class SRRegister : public SRPacket {
public:
	SRRegister(Peer * me);
};

/// Mid-class to subclass for all reply packets to respond somehow.
class SRReplyPacket : public SRPacket {
public:
	SRReplyPacket(int type, SRPacket * packetToReply);
	/// Returns the topic/request/line that holds what this packet was replying to.
	String GetRequest();
};

/// Explicit acknowledgements for requests
class SROKPacket : public SRReplyPacket {
public:
	SROKPacket(SRPacket * packetToReply);
};
/// Explicit refusals
class SRDeclinePacket : public SRReplyPacket {
public:
	SRDeclinePacket(SRPacket * packetToReply, String reason);
	/// Returns the same reason that was set in creation.
	String GetReason();
};

class SRRequestPlayersPacket : public SRPacket {
public:
	/** From client. Args: Action (see SRPlayers enum above), number of affected local players, additional arguments.
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

class SRPlayersPacket : public SRPacket {
public:
	/// From host. Args: a number of all players, a list of all player names, a list of all indices which this peer should be in control of.
	SRPlayersPacket(int number, List<String> playerNames, List<String> playerShips, List<int> clientPlayerIndices);
	/// Call once after receiving the packet in order to extract the below data. Flags the parsed variable to true upon completion.
	void Parse(int & number, List<String> & playerNames, List<String> & playerShips, List<int> & clientPlayerIndices);
};

class SRPlayerMovePacket : public SRPacket {
public:
	SRPlayerMovePacket(int playerIndex);
	void SetMessage(String msg);
	/// Extracts data from this packet
	void Parse(int & playerID, String & msg);
};

/// Update packet for a single player.
class SRPlayerPositionPacket : public SRPacket {
public:
	/// Player index, position, rotation and state for synchronizing visual effects correctly.
	SRPlayerPositionPacket(int playerID, Vector3f position, Vector3f rotation, String state, long long gameTime);
	/// Call once after receiving the packet in order to extract the below data. Flags the parsed variable to true upon completion.
	void Parse(int & playerID, Vector3f & position, Vector3f & rotation, String & state);
	/*
	int playerID;
	Vector3f position;
	*/
};

/// Will contain messages Start, Countdown and Finished. Start starts the race, Countdown starts countdown before the start. Finished notices the race is finished.
class  SRRacePacket : public SRPacket {
public:
	SRRacePacket(String msg, String additionalData = "");
	/// Call once after receiving the packet in order to extract the below data. Flags the parsed variable to true upon completion.
	void Parse(String & msg, String & additionalData);
};


#endif
