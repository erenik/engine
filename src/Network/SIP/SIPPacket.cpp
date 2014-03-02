/// Emil Hedemalm
/// 2013-11-28
/// Packet encapsulation

#include "SIPPacket.h"
#include "Network/Packet/PacketTypes.h"
#include "Network/Session/SessionTypes.h"
#include "SIPSessionData.h"
#include "String/StringUtil.h"
#include "Network/Peer.h"
#include "Network/Socket/Socket.h"
#include <cassert>
#include "Timer/Timer.h"


//================================================================
// SIP stuff
//================================================================
// Internet Message format Ref: http://tools.ietf.org/html/rfc2822
// SIP Protocol Ref: http://tools.ietf.org/html/rfc3261
Peer *  SIPPacket::defaultSender = NULL;
// List of all packets, used to adjust references when peers are deleted.
List<SIPPacket*> SIPPacket::packets;
/// Used to generate cSeq's
int SIPPacket::nextCSeq = 1;

/// Returns next available cSeq ID (starting at 1 and incrementing steadily!).
String SIPPacket::NextCSeq()
{
	return String::ToString(nextCSeq++);
}

/// TODO: Make sure CRLF standard is conformed to throughout the protocol-handling.
SIPPacket::SIPPacket(int i_type)
: Packet(PacketType::SIP)
{
//	std::cout<<"SIPPacket constructor";
	sipType = i_type;
	bodySet = headerSet = false;
	senderSet = recipientSet = false;
	// Pointers for where to fetch appropriate header-data.
	recipient = sender = NULL;
	expirationTime = -1;
	contentLength = 0;
	registeredSender = false;
	// List of all packets, used to adjust references when peers are deleted.
	packets.Add(this);
}

SIPPacket::~SIPPacket(){
	// List of all packets, used to adjust references when peers are deleted.
	packets.Remove(this);
}

/// Replaces all occurrences of peer with replacement.
void SIPPacket::ReplacePeer(Peer * peer, Peer * replacement)
{
	for (int i = 0; i < packets.Size(); ++i)
	{
		SIPPacket * packet = packets[i];
		if (packet->sender == peer)
			packet->sender = replacement;
		if (packet->recipient == peer)
			packet->recipient = replacement;
	}
}

/// For debugging purposes
void SIPPacket::PrintHeaderData()
{
	std::cout<<"SIPPacket::PrintHeaderData:";
	std::cout<<"- Type: "<<PacketTypeName();
	std::cout<<"- To: "<<recipientData;
	std::cout<<"- From: "<<senderData;
	std::cout<<"- CSeq: "<<cSeq;
	std::cout<<"- Content-Length: "<<contentLength;
}

/// Prints the entire packet to console.
void SIPPacket::Print()
{
	for (int i = 0; i < header.Size(); ++i)
		std::cout<<header[i].c_str();
	std::cout<<"";
	for (int i = 0; i < body.Size(); ++i)
		std::cout<<body[i].c_str();

}
	

/// Just concatenates the body into a single string and returns it.
String SIPPacket::GetBodyAsString(){
	return MergeLines(body, "\n\r");
}


/// Parses the given raw data, splicing it up into body/header parts as needed. Returns false if it is not a valid SIP packet.
bool SIPPacket::Parse(String rawData){
	assert(false);
	// Is this even used anymore?
	return true;
}


// Object-function: Returns for example "SIPRegister" for for SIP_REGISTER type. 
String SIPPacket::PacketTypeName(){
	return PacketTypeName(sipType);
}
// Static-class fucntion: Returns name of target packet type.
String SIPPacket::PacketTypeName(int type){
	switch(type){
		case SIP_REGISTER: return "REGISTER";
		case SIP_INFO:	return "INFO";
		case SIP_INVITE:	return "INVITE";
		case SIP_ACK: 	return "ACK";
		case SIP_BYE:	return "BYE";
		case SIP_NULL: return "NULL - Forgot to implement this or set proper type in SIPPacket constructor?";
		// Replies below!
		case SIP_PROVISIONAL: return "100 Provisional"; // 100-199
		case SIP_OK:	return "200 OK";
		case SIP_REDIRECTION: return "300 Redirection"; // 300-399
		case SIP_BAD_REQUEST: return "400 Bad Request";
		case SIP_BAD_EVENT:	return "489 Bad event";
		case SIP_SERVER_INTERNAL_ERROR: return "500 Server Internal Error";	// 500-599, as with OK the internal error is only just for 500, 500-599 is actually "Server Failure" in general
		case SIP_GLOBAL_FAILURE: return "600 Global Failure"; // 600-699
		// Subscribe/Notify
		case SIP_SUBSCRIBE: return "SUBSCRIBE";
		case SIP_NOTIFY: return "NOTIFY";
	}
	return "ERROR: SIPPacket::PacketTypeName: Undefined or unimplemented SIP packet.";
}

/// Builds a default header if it doesn't exist.
void SIPPacket::EnsureHeader(){
	if (header.Size() != 0)
		return;
	// Assign default-sender if not done already.
	if (sender == NULL)
		SetSender(defaultSender);
	// Build it.
	BuildHeader();
}

/* RFC 3261 part 7.3.1. Header field format, some details from it.

Whitespaces:
---------------------
      Subject:            lunch
      Subject      :      lunch
      Subject            :lunch
      Subject: lunch

   Thus, the above are all valid and equivalent, but the last is the
   preferred form.

Case sensitivty:
---------------------
   	When comparing header fields, field names are always case-
   insensitive.  Unless otherwise stated in the definition of a
   particular header field, field values, parameter names, and parameter
   values are case-insensitive.  Tokens are always case-insensitive.
   Unless specified otherwise, values expressed as quoted strings are
   case-sensitive.  For example,

      Contact: <sip:alice@atlanta.com>;expires=3600

   is equivalent to

      CONTACT: <sip:alice@atlanta.com>;ExPiReS=3600

*/

// Fetches type-integer from the header-data. See above for RFC details.
void SIPPacket::ParseHeader(){
//	std::cout << "SIPPacket::ParseHeader called.";
	String line;
	List<String> tokens;
	sipType = SIP_NULL;
	// Evaluate basic type of the packet, first need to get the first valid line...
	String firstLine;
	int firstLineIndex;
	for (int i = 0; i < header.Size(); ++i){
		firstLine = header[i];
		if (firstLine.Length()){
			firstLineIndex = i;
			break;
		}
	}
	// Determine type from first line
	for (int i = 0; i < SIP_PACKET_TYPES; ++i){
		String typeName = PacketTypeName(i);
		if (firstLine.Contains(typeName)){
			sipType = i;
			break;
		}
	}
	// Try and see if it's a numerical response-code we don't know?
	if (sipType == SIP_NULL){
		// Split using white-space, get first element, try and parse numberrr!
		String firstPart = firstLine.Tokenize(" \t\n\r")[0];
		int number = firstPart.ParseInt();
		if (number >= 700)
			sipType = SIP_NULL;
		else if (number >= 600)
			sipType = SIP_GLOBAL_FAILURE;
		else if (number >= 500)
			sipType = SIP_SERVER_INTERNAL_ERROR;
		else if (number >= 400)
			sipType = SIP_BAD_REQUEST;
		else if (number >= 300)
			sipType = SIP_REDIRECTION;
		else if (number >= 200)
			sipType = SIP_OK; // Should not even get here, but eh..
		else if (number >= 100)
			sipType = SIP_PROVISIONAL;
	}
	// If still bad type, don't parse anymore? :|
	if (sipType == SIP_NULL){
//		std::cout <<"Type of packet could not be determined in SIPPacket::ParseHeader, firstline: "<<firstLine;
		return;
	}
	// Evaluate the rest of the lines
	for (int i = firstLineIndex+1; i < header.Size(); ++i){
		line = header[i];
		// Split using all white-space characters and the colon for header-fields!
		// First find where attribute name and value start/end (the first colon)
		String attributeName = String();
		String attributeValue = String();
		// Remove only up to the first colon?
		int firstColonIndex = -1;
		for(int i = 0; i < line.Length(); ++i){
			if (line.CharAt(i) == ':'){
				firstColonIndex = i;
				break;
			}
		}

		attributeName = line.Part(0, firstColonIndex);
		attributeValue = line.Part(firstColonIndex+1);
		attributeName.RemoveInitialWhitespaces();
		attributeValue.RemoveInitialWhitespaces();

// Macro for the comparison, getting tired of this...
// First macro checks if they are lexicographically the same (i.e. identical), except not case sensitive
#define IsAttributeName(a) (attributeName == a)
// And the second macros does the same but compares if it is either the same to a OR b, inclusive.
#define IsAttributeName2(a,b) (IsAttributeName(a) || IsAttributeName(b))
		// Set comparison mode to not case sensitive.
		attributeName.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		// Apparently all fields can have compact forms too, so spam all the comparisons..
		if (IsAttributeName2("Call-ID","i")){
			callID = attributeValue;
		}
		else if (IsAttributeName2("Content-Length","l")){
			String s = attributeValue;
			contentLength = s.ParseInt();
		}
		else if (IsAttributeName2("Content-Type","c")){
			contentType = attributeValue;
		}
		else if (IsAttributeName("CSeq")){
			cSeq = attributeValue;
		}
		else if (IsAttributeName("Expires")){
			String s = attributeValue;
			expirationTime = s.ParseInt();
		}
		else if (IsAttributeName2("From","f")){
			senderData = attributeValue;
		}
		else if (IsAttributeName("Require")){
			// Although an optional header field, the Require MUST NOT be
   			// ignored if it is present.
   			assert(false && "Not implemented, sorry. ):");
		}
		else if (IsAttributeName2("To","t")){
			recipientData = attributeValue;
		}
		else if (IsAttributeName("Warning")){
			String warning = attributeValue;
//			std::cout<<"Warning attribute found on packet: "<<attributeValue;
		}
		else if (IsAttributeName("Event")){
			event = attributeValue;
		}
		else {
//			std::cout << "Could not find anything relevant in line: "<<line;
		}
	}
	if (type == SIP_REGISTER && !cSeq.Length()){
		std::cout << "WARNING: Lacking CSeq in Register message!";
	}
	// Set default packet type?
	if (type == SIP_NULL)
		std::cout <<"WARNING: Packet of type SIP_NULL. Either the packet lacks a proper header or this function (SIPPacket::ParseHeader) requires expansion.";

}

/** Extracts target header-data, for example specifying "CSeq:" might return the following: "314159".
	The header-data will, if available, be sent into the second parameter string reference.
	Upon failure (lack of such data), it will return false AND set the target string to an empty String.
*/
bool SIPPacket::GetHeaderData(String tagName, String & data){
	for (int i = 0; i < header.Size(); ++i){
		String line = header[i];
		if (!line.Contains(tagName))
			continue;
		// Split using all white-space characters!
		List<String> tokens = line.Tokenize(" \n\r\t");
		if (tokens.Size() < 2)
			continue;
		data = tokens[1];
		return true;
	}
	data = String();
	return false;
}

/// Sets recipient to the provided argument IF the current recipient is NULL.
void SIPPacket::EnsureRecipient(Peer * i_recipient)
{
	if (recipient == NULL)
		recipient = i_recipient;
	assert(recipient);
}

// Sends this packet to target client. If header has not been set by a subclass it will be generated using the given client data and known sender data.
bool SIPPacket::Send(Peer * receiver)
{
	assert(receiver);
	// With a few exceptions, demand a valid receiver before proceeding.
	switch(sipType)
	{
		case SIP_REGISTER:
		case SIP_BAD_REQUEST:
			break;
		default:
			if (!receiver->isValid)
			{
				std::cout<<"SIPPacket::Send:\nPacket: ";
				Print();
				receiver->Print();

				std::cout<<"ERROR: Unable to send to receiver. Receiver not valid or not synchronized correctly. See receiver and packet data above.";
				assert(false);
				return false;
//				assert(receiver->isValid);
			}
	}
	// Set recipient if not already done so.
	EnsureRecipient(receiver);

	// Check if header is set.
	EnsureHeader();
	/// Construct message
	String fullPacket = BuildPacket();
	List<String> lines = GetLines(fullPacket);
	std::cout << "\nWriting packet...\n" <<fullPacket<<"\nWith lines: "<<lines.Size();
	// Send it.
	int bytesWritten = receiver->primaryCommunicationSocket->Write(fullPacket.c_str(), fullPacket.Length());
	assert(bytesWritten);
//	int bytesWritten = sock->Write(fullPacket.c_str(), fullPacket.Length());
	return true;
}	

/// Sends this packet's data to target QTcpSocket, using necessary packet-headers.
bool SIPPacket::Send(Socket * sock){
	assert(sock);
	// Check if header is set.
	EnsureHeader();
	/// Construct message
	String fullPacket = BuildPacket();
	int bytesWritten = sock->Write(fullPacket.c_str(), fullPacket.Length());
	return true;
}

/// Builds a packet using set stats and returns it as a string.
String SIPPacket::BuildPacket(){
	// Check other stuff.
	String fullPacket;
	// Initial CRLF
	fullPacket.Add("\r\n");
	for (int i = 0; i < header.Size(); ++i){
		fullPacket.Add(header[i]);
		fullPacket.Add("\r\n");
	}
	// Add separating CRLF
	fullPacket.Add("\r\n");
	for (int i = 0; i < body.Size(); ++i){
		fullPacket.Add(body[i]);
		fullPacket.Add("\r\n");
	}
	fullPacket.Add("\r\n\r\n");
	return fullPacket;
}

/// Sets an empty body,setting the bodySet flag to true.
void SIPPacket::SetNullBody(){
	body.Clear();
	bodySet = true;
}

/* Note: SIP URI 
SIP URIs are defined in Section
   19.1.  It has a similar form to an email address, typically
   containing a username and a host name.  In this case, it is
   sip:bob@biloxi.com, where biloxi.com is the domain of Bob's SIP
   service provider.  Alice has a SIP URI of sip:alice@atlanta.com.
   Alice might have typed in Bob's URI or perhaps clicked on a hyperlink
   or an entry in an address book.  SIP also provides a secure URI,
   called a SIPS URI.  An example would be sips:bob@biloxi.com.  A call
   made to a SIPS URI guarantees that secure, encrypted transport
   (namely TLS) is used to carry all SIP messages from the caller to the
   domain of the callee.  From there, the request is sent securely to
   the callee, but with security mechanisms that depend on the policy of
   the domain of the callee.
   */

void SIPPacket::SetRecipient(Peer * recipient){
	if (!recipient)
		return;
	std::cout<<"\nSIPPacket::SetRecipient for packet: "<<PacketTypeName();
	assert(recipient);
	// With a few exceptions, make sure that the recipient is valid.
	switch(type)
	{
		case SIP_REGISTER:
		case SIP_BAD_REQUEST:
			break;
		default:
			// Validity should have been set when the peer is created.
			// assert(recipient->isValid);
			;
	}
	this->recipient = recipient;
	// Port assertion probably not viable 
	// assert(recipient->port != -1);
	// Add base IP-address
	recipientName = recipient->name;
	std::cout<<"recipient: "<<recipient->name<<" ip address:" << recipient->ipAddress;
	if (recipient->ipAddress.Length())
		recipientAddress = recipient->name+"@"+recipient->ipAddress;
	else 
	{
		recipientAddress = recipient->name+"@nullIPAddressPleaseSetItBeforeSendingPackets";
	//	assert(false && "Bad recipient address. Correct this immediately before proceeding.");
	}
	// Add port
	if (recipient->port == -1)
		; // Skip port if set to -1. // recipientAddress.Add(":invalidPort");
	else 
		recipientAddress.Add(":"+String::ToString(recipient->port));
//	recipientAddress = recipientAddress.toLower();
	recipientData = recipientName+" <"+recipientAddress+">";
	recipientSet = true;
}
// Sets relevant sender data.
void SIPPacket::SetSender(Peer * sender){
	this->sender = sender;
	assert(sender);
	assert(sender->port != -1);
	// Add base IP-address
	senderName = sender->name;
	if (sender->ipAddress.Length())
		senderAddress = sender->name+"@"+sender->ipAddress;
	else
		senderAddress = sender->name+"@invalidSocket";
	// Add port
	if (sender->port == -1)
		senderAddress.Add(":invalidPort");
	else 
		senderAddress.Add(":"+String::ToString(sender->port));
	
//	senderAddress = senderAddress.toLower();
	senderData = senderName+" <"+senderAddress+">";
	senderSet = true;
}

// Ref: http://tools.ietf.org/html/rfc3261#section-24
// Builds the header using the previously set parameters. Call this again if you change any of them!
void SIPPacket::BuildHeader(){
	// Make sure body is set before calculating header-parameters!
	if (!bodySet){
		std::cout<< "ERROR: Body not set before calling SIPPacket::BuildHeader! Fix this, yo.";
		assert(bodySet);	
	}
	// Set default sender (us) if no sender already specified.
	if (!sender)
		sender = defaultSender;
	if (!senderSet){
		SetSender(sender);
	}
	if (!recipientSet){
		SetRecipient(recipient);
	}
	header.Clear();
	String typeName = PacketTypeName();
	switch(type){
		case SIP_REGISTER:
		case SIP_INFO:
		case SIP_INVITE:
		case SIP_BYE:
		case SIP_ACK:
		case SIP_CANCEL:
		case SIP_OPTIONS:
		case SIP_SUBSCRIBE:
		case SIP_NOTIFY:
			header.Add(typeName+" sip:"+recipientAddress+" SIP/"+SIPVersion);
			break;
		case SIP_OK:
		case SIP_BAD_REQUEST:
			header.Add("SIP/"+SIPVersion+" "+typeName);
			break;
		default:
			std::cout<<"Unrecognized packet type: "<<type<<" typeName: "<<typeName;
			assert(false && "Unrecognized packet-type when setting first row in SIP message header. Add a new case for it?");
			break;
	}
	String toString = "To: "+recipientData;
	/// Only do this if the recipient is known.
	SIPSessionData * recipientSessionData = NULL;
	if (recipient){
		recipientSessionData = (SIPSessionData*)recipient->GetSessionData(SessionType::SIP);
		if (recipientSessionData->theirTag.Length()){
			toString.Add(";tag="+recipientSessionData->theirTag);
		}
	}
	header.Add(toString);
	String senderString = "From: "+senderData;
	if (recipientSessionData){
		if (recipientSessionData->ourTag.Length()){
			senderString.Add(";tag="+recipientSessionData->ourTag);
		}
	}
	header.Add(senderString);
	// TODO: Add Call-ID ?
	// Add CSeq
	if (cSeq.Length())
		header.Add("CSeq: "+cSeq);
	// Event-type, for Subscribe messages
	if (event.Length()){
		header.Add("Event: "+event);
	}
	// TODO: Add Contact
	if (expirationTime >= 0)
		header.Add("Expires: "+String::ToString(expirationTime));
	int bodyLength = BodyLength();
	if (bodyLength > 0){
		if (!contentType.Length())
			contentType = "application/customP2PMediaProtocol";
		header.Add("Content-Type: "+contentType);
	}
	header.Add("Content-Length: "+String::ToString(bodyLength));
	headerSet = true;
}
/// Calculates body length from the string-list.
int SIPPacket::BodyLength(){
	int l = 0;
	for (int i = 0; i < body.Size(); ++i){
		l += body[i].Length();
		// TODO: Add new-lines correctly.
		l += 2;
	}
	return l;
}

/// Sets default sender, also calculating it's name automatically using given extra parameters. Must be called at least ONCE before any packet may be used!
void SIPPacket::SetDefaultSender(Peer * sender){
	defaultSender = sender;
}

// =============================================================================================================================================================
/// SIP REGISTER
/// Name, IP and registration-time. A SIPRegister should be sent once every time-interval to refresh connectivity?
SIPRegisterPacket::SIPRegisterPacket(int registrationTime)
: SIPPacket(SIP_REGISTER)
{
	assert(defaultSender && "Trying to create packet before setting default sender! Use SIPPacket::SetDefaultSender first!");
	std::cout << "Default sender: "<<defaultSender->name<< " "<<defaultSender->ipAddress;
	// Use name and ip to overload the ..defaultSender?
	// Set cSeq
	cSeq = "1826 REGISTER";
	// Register sets both sender and receiver to be the same, for some reason.
	sender = recipient = defaultSender;
	// Set a requested tag for us.
	srand((unsigned int)Timer::GetCurrentTimeMs());
	int tag = rand() % 10000;
	senderTag = String::ToString(tag);
	std::cout << "Sender tag: "<<senderTag;
	// Set desired registration-expiration time
	assert(registrationTime > 0);
	this->expirationTime = registrationTime;
	// Set body as set!
	SetNullBody();
//	BuildHeader();
}

// =============================================================================================================================================================
/// Default contructor that builds a header using relevant data from the packet to which it is replying.
SIPReplyPacket::SIPReplyPacket(SIPPacket * packetToReply, int type)
: SIPPacket(type){
	SetSender(defaultSender);
	assert(defaultSender);
	cSeq = packetToReply->cSeq;
}
SIPOKPacket::SIPOKPacket(SIPPacket * packetToReply)
: SIPReplyPacket(packetToReply, SIP_OK)
{
	/// Add optional extra data.
	SetNullBody();
	// Build the header later otherwise?
	// Might for example want to set an Expires-header field for Subscribe messages which will 
	// be ignored if we build the header now (and don't build it explicitly later).
//	BuildHeader();
}

/// Default constructor that sets given error message body.
SIPBadRequestPacket::SIPBadRequestPacket(SIPPacket* packetToReply, String errorMessage)
: SIPReplyPacket(packetToReply, SIP_BAD_REQUEST){
	// Set body!
	body.Clear();
	body.Add(errorMessage);
	bodySet = true;
	// Rebuild-header.
//	BuildHeader();
}

// =============================================================================================================================================================
/* 	SIP INFO Packet
	Following RFC should be studied if extended/more up-to-date usage of the INFO messages is to be used
	RFC 6086: http://tools.ietf.org/html/rfc6086
*/
// Empty constructor
SIPInfoPacket::SIPInfoPacket()
: SIPPacket(SIP_INFO)
{
	cSeq = "3326 "+PacketTypeName();	
}
// Custom information packet, only transported via SIP for conformalities' sake.
SIPInfoPacket::SIPInfoPacket(String i_body)
: SIPPacket(SIP_INFO){
	SetBody(i_body);
	cSeq = "3326 "+PacketTypeName();	
}
/// Returns the body as a single string.
String SIPInfoPacket::GetBodyAsString(){
	String string = MergeLines(body, "\r\n");
	return string;
}

/// Attempts to set the body of the packet, reformatting end-lines etc. in the process.
bool SIPPacket::SetBody(String i_body){
	// Clear old body
	body.Clear();
	// Split the body using CR, LF and CRLF, concatenating them all using only CRLF.
	List<String> lines = GetLines(i_body);
	// TODO: Ensure that the body is rid of any lone CRs or LFs?
	body = lines;
	bodySet = true;
	return true;
}


/// Default constructor that uses a custom body, setting the content-type to the default application/customP2PMediaProtocol
SIPInvitePacket::SIPInvitePacket(String i_body)
: SIPPacket(SIP_INVITE)
{

	cSeq = NextCSeq()+" "+PacketTypeName();
	body.Clear();
	// Split the body using CR, LF and CRLF, concatenating them all using only CRLF.
	List<String> lines = GetLines(i_body);
	body = lines;
	bodySet = true;
}

/// Default constructor. Note that a proper BYE message should probably contain more info and this has not been regarded so far!
SIPByePacket::SIPByePacket()
: SIPPacket(SIP_BYE)
{
	// Just give it a default cSeq number and an empty body, implying that we should terminate all media sessions.
	cSeq = "1200 "+PacketTypeName();
	body.Clear();
	bodySet = true;
}

// =============================================================================================================================================================
// SUBSCRIBE & NOTIFY as defined in rfc3265: https://ietf.org/doc/rfc3265/?include_text=1

// Constructor, Duration in minutes?
SIPSubscribePacket::SIPSubscribePacket(String eventToSubscribeTo, int subscriptionDuration, int i_cSeq)
: SIPPacket(SIP_SUBSCRIBE)
{
	expirationTime = subscriptionDuration;
	event = eventToSubscribeTo;
	cSeq = String::ToString(i_cSeq)+" SUBSCRIBE";
	assert(event.Length() && "Trying to send SIPSubscribePacket with no event url?");
	std::cout<<"SIPSubscribePacket constructor";
	// Let header be created when we send it..
	bodySet = true;
};

SIPNotifyPacket::SIPNotifyPacket(String event)
: SIPPacket(SIP_NOTIFY)
{
	cSeq = "133 NOTIFY";
	this->event = event;
	std::cout<<"SIPNotifyPacket constructor";
	bodySet = true;
};

SIPBadEventPacket::SIPBadEventPacket(SIPPacket * packetToReply)
: SIPReplyPacket(packetToReply, SIP_BAD_EVENT){
	body.Clear();
	body.Add("Invalid event requested. List of event types below.. once implemented.");
	bodySet = true;
//	BuildHeader();
}















