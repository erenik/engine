/// Emil Hedemalm
/// 2014-01-24
/// SessionData class for the SIP Session.

#ifndef SIP_SESSION_DATA_H
#define SIP_SESSION_DATA_H

#include "SIPPacket.h"
#include "SIPSessionData.h"
#include "Network/Session/SessionData.h"

class SIPEventSubscription;

/// Subclass this to include any variables needed.
class SIPSessionData : public SessionData {
public:
	/// Constructor
	SIPSessionData(Peer * peer);
	~SIPSessionData();

	/// Resets variables for the session
	void Reset();

	/// Updates sender data (what is actually sent in the package). This function should be called every time name, IP address or tag has been changed!
	void UpdateSenderData(Peer * owner);
	/// Returns the "Name <name@ip>(;tag=#)" SIP from/to field to associate a peer.
	String SenderData() { return senderData; };

	/// Woo. Subscribes for a while.
	void SubscribeToEvent(String eventType, int duration);
	bool SubscribedToEvent(String eventName);
	/// Should probably extend this to include durations of the subscriptions too..
	List<SIPEventSubscription*> eventSubscriptions;


	/// If the peer has successfully registered with us. (peer connecting to us).
	bool registeredWithUs;
	/// If we have successfully registered ourselves with the peer. (connecting to peer). 
	bool registeredWithPeer;


	
	/// Current requested search-string from this peer. Media conforming to this should be sent to the peer. Should be 0-length when no search is active.
    String gameSearchString;
	/// List of all games that this peer has that conforms to OUR last requested search-string.
    List<String> soughtGames;

	/// Current requested search-string from this peer. Media conforming to this should be sent to the peer. Should be 0-length when no search is active.
    String mediaSearchString;
    /// List of all media that this peer has that conforms to OUR last requested search-string.
    List<String> soughtMedia;


	// Used in the SIP-protocol, should try and keep one unique per peer.
	/// Peers tag (as according to us)
	String theirTag;
	/// Our tag (as according to peer)
	String ourTag;
	
	/// Contains a copy of this peer's latest media request packet (SIP INVITE)
	SIPPacket sipInvitePacket;
	/** Specifies if this peer has subscribed to the event from us and if so the duration of the subscription.
		For the whole SUBSCRIBE/NOTIFY mechanism to auto-connect new peers to the network. 
	*/
	int peerSubscribedToNewPeerConnections;
	/** Specifies if we are currently subscribed to the event from this peer and if so the duration of the subscription.
		For the whole SUBSCRIBE/NOTIFY mechanism to auto-connect new peers to the network. 
	*/
	int meSubscribedToNewPeerConnections;
	/// True only when we are currently streaming media from this peer to us. Used to query who to send related (playback) network messages to.
	bool streamingFrom;

	/// Uses the "name <name@ip>(;tag=#)" SIP from/to field to associate a peer.
	String senderData;	

	/// Owner of this session data.
	Peer * owner;
};

#endif