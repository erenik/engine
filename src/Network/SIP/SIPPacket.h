/// Emil Hedemalm
/// 2013-11-28
/// Packet encapsulation

#ifndef SIP_PACKET_H
#define SIP_PACKET_H

#include "Network/Packet/Packet.h"

class TcpSocket;
class Peer;

/// Enumeraton to simplify handling of incoming packets.
enum SIPPacketTypes{
	SIP_NULL = 0,
	SIP_REGISTER,
	SIP_INVITE,
	SIP_ACK,
	SIP_CANCEL,
	SIP_BYE,
	SIP_OPTIONS,
	SIP_PRACK,
	SIP_PUBLISH,
	SIP_INFO,
	SIP_REFER,
	SIP_MESSAGE,
	SIP_UPDATE,

	// As per rfc 3265, https://ietf.org/doc/rfc3265/?include_text=1
	SIP_SUBSCRIBE,
	SIP_NOTIFY,
	
	/// Replies below!
	SIP_PROVISIONAL, // 100-199
	SIP_OK, // 200
	SIP_REDIRECTION, // 300-399
	SIP_BAD_REQUEST, // 400
	SIP_BAD_EVENT,	// 489
	SIP_SERVER_INTERNAL_ERROR,  // 500-599, as with OK the internal error is only just for 500, 500-599 is actually "Server Failure" in general
	SIP_GLOBAL_FAILURE, // 600-699
	SIP_PACKET_TYPES,
};

static const char * SIPVersion = "2.0";

/// Packet class that conforms to the SIP Protocol that handles header-information and such appropriately in the send-function as well as constructor.
class SIPPacket : public Packet {
	friend class SIPSession;
public:
	/// A type must always be specified, see enum above.
	SIPPacket(int type = SIP_NULL);
	/// Virtual destructor so that subclass destructors are run correctly.
	virtual ~SIPPacket();
	/// Parses the given raw data, splicing it up into body/header parts as needed. Returns false if it is not a valid SIP packet.
	bool Parse(String rawData);
	/// Returns for example "SIPRegister" for for SIP_REGISTER type. 
	String PacketTypeName();
	/// Returns name of target packet type.
	static String PacketTypeName(int type);

	/// Sends this packet to target client. If header has not been set by a subclass it will be generated using the given client data and known sender data.
	virtual bool Send(Peer * peer);
	/// 
	virtual bool Send(Socket * socket);

	/// Builds a packet using set stats and returns it as a string.
	String BuildPacket();
	
	/// Replaces all occurrences of peer with replacement.
	static void ReplacePeer(Peer * peer, Peer * replacement);
	/// Sets an empty body,setting the bodySet flag to true.
	void SetNullBody();
	/// Builds a default header if it doesn't exist.
	void EnsureHeader();
	/// Fetches type-integer from the header-data.
	void ParseHeader();
	// Ref: http://tools.ietf.org/html/rfc3261#section-24
	/// Builds the header using the previously set parameters. Call this again if you change any of them!
	void BuildHeader();
	/// Calculates body length from the string-list.
	int BodyLength();
	/// Sets all recipient data based on client/peer-data.
	void SetRecipient(Peer * client);
	Peer * Recipient() { return recipient; };
	/// Sets relevant sender data.
	void SetSender(Peer * client);
	Peer * Sender() { return sender; };
	/// Sets default sender, also calculating it's name automatically using given extra parameters. Must be called at least ONCE before any packet may be used!
	static void SetDefaultSender(Peer * defaultSender);

	/** Extracts target header-data, for example specifying "CSeq:" might return the following: "314159".
		The header-data will, if available, be sent into the second parameter string reference.
		Upon failure (lack of such data), it will return false AND set the target string to an empty String.
	*/
	bool GetHeaderData(String tagName, String & data);
	/// For debugging purposes
	void PrintHeaderData();
	/// Prints the entire packet to console.
	void Print();
	/// Just concatenates the body into a single string and returns it.
	String GetBodyAsString();
	/// Attempts to set the body of the packet, reformatting end-lines etc. in the process.
	bool SetBody(String body);
	/// Separation of contents.
	List<String> header, body;
	
	String name, ip;
	/// Expiration time as described in the header-field "Expires:"
	int expirationTime;
	/// Length as described in the header-field "Content-Length:"
	int contentLength;

	/// Both formatted using "Name <name@url>"
	String senderData, recipientData;
	/// In the form of "name@domain.end" used in the header's first line.
	String recipientAddress, senderAddress;
	/// E.g. "Bob"
	String recipientName, senderName;
	/// Both pure numbers.
	String senderTag, recipientTag;
	/// Content-type, only used if body is non-zero length. E.g. "application/sdp"
	String contentType;
	/// "Call-sequence" or something, used to keep track of replies and topics, pretty much? Stored as a separate variable for eased access.
	String cSeq;
	/// Unique identifier for any particular invitation/registration.
	String callID;
	/// Used for Subscribe message.
	String event;
	/// If false, this means that the sender has not yet been registered with us.
	bool RegisteredSender() {return registeredSender; };

	/// See SIPPacketTypes
	int sipType;

	/// Returns next available cSeq ID (starting at 1 and incrementing steadily!).
	static String NextCSeq();
protected:
	/// Used to generate cSeq's
	static int nextCSeq;

	/// Sets recipient to the provided argument IF the current recipient is NULL.
	void EnsureRecipient(Peer * recipient);
	/// If false, this means that the sender has not yet been registered with us.
	bool registeredSender;
	/// Pointers for where to fetch appropriate header-data.
	Peer * recipient;
	Peer * sender;
	/// Default sender, usually 'me'
	static Peer * defaultSender;
	/// Has to be set manually at least ONCE.
	static bool defaultSenderSet;
	/// If true, the SIPPacket will not modify them further before sending them (it will build a default header otherwise).
	bool headerSet, bodySet, senderSet, recipientSet;

	// List of all packets, used to adjust references when peers are deleted.
	static List<SIPPacket*> packets;
};

/// Registration-packet!
class SIPRegisterPacket : public SIPPacket {
public:
	/** A SIPRegister should be sent once every time-interval to refresh connectivity? 
		-registrationTime in minutes
	*/
	SIPRegisterPacket(int registrationTime);
};



// ====================================================================================================================================
/// General class for all replies
class SIPReplyPacket : public SIPPacket {
public:
	/// Type is for example SIP_OK or SIP_TRYING TODO: Look up an error-code like SIP_FAILED?
	SIPReplyPacket(SIPPacket * packetToReply, int type);
};

class SIPOKPacket : public SIPReplyPacket {
public:
	/// Default contructor that builds a header using relevant data from the packet to which it is replying.
	SIPOKPacket(SIPPacket * packetToReply);
};

class SIPBadRequestPacket : public SIPReplyPacket{
public:
	/// Default constructor that sets given error message body.
	SIPBadRequestPacket(SIPPacket* packetToReply, String errorMessage);
};

/// Custom information packet, only transported via SIP for conformalities' sake.
class SIPInfoPacket : public SIPPacket {
public:
	/// Empty constructor, setting recipient if wanted. Recipient can be set later too as needed using SetRecipient(Client*)
	SIPInfoPacket();
	/// Body and recipient. Recipient can be set later too as needed using SetRecipient(Client*)
	SIPInfoPacket(String body);
	/// Returns the body as a single string.
	String GetBodyAsString();
};

/// Packet class that handles the SIP INVITE message for initiating a media session of some sort.
class SIPInvitePacket : public SIPPacket {
public:
	/// Default constructor that uses a custom body, setting the content-type to the default application/customP2PMediaProtocol
	SIPInvitePacket(String body);
};

/// Packet class that handles SIP BYE message for terminating media sessions.
class SIPByePacket : public SIPPacket {
public:
	/// Default constructor. Note that a proper BYE message should probably contain more info and this has not been regarded so far!
	SIPByePacket();
};

// ====================================================================================================================================
// SUBSCRIBE & NOTIFY as defined in rfc3265: https://ietf.org/doc/rfc3265/?include_text=1
class SIPSubscribePacket : public SIPPacket {
public:
	// Constructor, String for URL/identifier of the event, duration in minutes and call-sequence identifier
	SIPSubscribePacket(String eventToSubscribeTo, int subscriptionDuration, int cSeq);
};
class SIPNotifyPacket : public SIPPacket {
public:
	/// Constructor with event name
	SIPNotifyPacket(String event);
};
/// Packet sent when a subscription contains an invalid event-name.
class SIPBadEventPacket : public SIPReplyPacket {
public:
	SIPBadEventPacket(SIPPacket * packetToReply);
};


#endif // SIP_PACKET_H
