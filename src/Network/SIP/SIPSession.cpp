/// Emil Hedemalm
/// 2014-01-24
/// Session handle for SIP. This includes handling of a Tcp Server, SIP peers etc.

#include "SIPSession.h"
#include "Network/Session/SessionTypes.h"
#include "SIPSessionData.h"
#include "SIPPacket.h"
#include "Network/Server/TcpServer.h"
#include "Network/Peer.h"
#include "String/StringUtil.h"
#include "Network/Socket/TcpSocket.h"
#include "SIPEvent.h"
#include "Game/Game.h"
#include "Message/MessageManager.h"
#include "Network/NetworkManager.h"

SIPSession::SIPSession()
: Session("Primary communication session", "SIP Session", SessionType::SIP)
{
	registerPacket = NULL;
}

/// Creates the SIPRegister packet using the data in the given Peer-structure for ourselves.
void SIPSession::Initialize(){
	assert(registerPacket == NULL);
	/// Register for an hour by default (seconds)
	registerPacket = new SIPRegisterPacket(3600);
	/// Create sip session data for ourselves
	SIPSessionData * sipSessionData = new SIPSessionData(me);
	me->sessionData.Add((SessionData*)sipSessionData);
	sipSessionData->ourTag = "1";
}

/// Connects to address/port.
bool SIPSession::ConnectTo(String ipAddress, int port)
{
	TcpSocket * sock = new TcpSocket();
	bool success = sock->ConnectTo(ipAddress, port);
	if (success){
		sockets.Add(sock);
		return true;
	}
	lastErrorString = sock->GetLastErrorString();
	return false;
}

/// Hosts!
bool SIPSession::Host(int port /*= 33000 */)
{
	/// Use default TcpServer configuration
	bool success = Session::Host(port);
	return success;
}

/// Stops the session, disconnecting any connections and sockets the session might have.
void SIPSession::Stop(){
	/// Send reigster 0 duration packets first?
	/// Use default stop.
	Session::Stop();
}

/// Called when the host disconnects.
void SIPSession::OnHostDisconnected(Peer * host)
{

}

/// Function to process new incoming connections but also disbard old connections that are no longer active.
void SIPSession::EvaluateConnections()
{
    // If hosting, check for new connections
	if (this->tcpServer){
		assert(this->tcpServer);
		Socket * newSocket = this->tcpServer->NextPendingConnection();
		if (newSocket)
		{
		    sockets.Add(newSocket);
            std::cout<<"\nNew socket accepted in SIP session.";
        }
    }

	/// Get list of all connected clients.
	List<Peer*> connectedPeers = ConnectedPeers();

	/// Deletes those sockets that either have errors or have been flagged for deletion (other errors)
	DeleteFlaggedSockets();

	/// Get list of all connected clients after deleting flagged sockets.
	List<Peer*> connectedPeersPostDeletion = ConnectedPeers();

	/// Check for differences in the lists. All that have disappeared now will be added to the disconnected list.
	for (int i = 0; i < connectedPeers.Size(); ++i){
		if (connectedPeersPostDeletion.Exists(connectedPeers[i]))
			continue;
		disconnectedPeers.Add(connectedPeers[i]);
	}

	/// For each unknown socket, send a new SIPRegister message.
	for (int i = 0; i < sockets.Size(); ++i){
		Socket * s = sockets[i];
		if (s->peer == NULL && s->messagesSent <= 0){
			/// Use stored registration message so that we can send it straight away?
			registerPacket->Send(s);
		}
	}
}

/// Returns a list of all peers that have connected since the last call to GetNewPeers
List<Peer*> SIPSession::GetNewPeers(){
	List<Peer*> listToReturn = newPeers;
	newPeers.Clear();
	return listToReturn;
}

/// Returns a list of all peers that have disconnected since the last call to EvaluateConnections();
List<Peer*> SIPSession::GetDisconnectedPeers(){
	List<Peer*> listToReturn = disconnectedPeers;
	disconnectedPeers.Clear();
	return listToReturn;
}

/// Sends target packet to all peers in this session using default targetsm, via host if possible.
void SIPSession::Send(Packet * packet)
{
	/// Send to all peers if we are host, else just send to host.
	Session::Send(packet);
}

/// Reads packets, creating them and returning them for processing. Note that some packets will already be handled to some extent within the session (for exampling many SIP messages).
List<Packet*> SIPSession::ReadPackets()
{
	List<SIPPacket*> packets;
	List<Packet*> p;
	// For each socket
	for (int i = 0; i < sockets.Size(); ++i){
		// Reach packets
		Socket * socket = sockets[i];
		sipPacketParser.ReadPackets(socket, packets);
		for (int i = 0; i < packets.Size(); ++i){
			// Handle SIP-based packets straight away
			SIPPacket * packet = packets[i];
			packet->ParseHeader();
			this->HandlePacket(packet, socket->peer);
			p.Add(packet);
		}
	}

	return p;
}

/// Returns a list of available games from all peers.
List<Game*> SIPSession::GetAvailableGames(){
	List<Game*> games;
	for (int i = 0; i < peers.Size(); ++i){
		Peer * p = peers[i];
		SIPSessionData * peerSIPSessionData = this->GetSessionData(p);
		for (int j = 0; j < peerSIPSessionData->soughtGames.Size(); ++j){
			Game * game = new Game();
			game->LoadFrom(peerSIPSessionData->soughtGames[j]);
			games.Add(game);
		}
	}
	return games;
}

/// Finds peers belonging to target packet and sets the appropriate pointers within (sender & recipient)
void SIPSession::FindPeersForPacket(SIPPacket * packet, Peer * sender)
{
 //   std::cout<<"\nNetworkManager::FindPeersForPacket "<<packet->PacketTypeName();
    // Fetch string with name and stuff ?
    if (!packet->sender)
    {
        // Unless we have much reason to do so, just set the sender to be the one that actually sent us the packet?
        packet->sender = sender;
        /*
        String sender = packet->senderData;
        std::cout << "Sender: "<<sender;
        packet->sender = GetPeerByData(sender);
        assert(packet->sender);
        */
    }
    // For recipient, we have to look through our list of recipients and see who it's meant for (although for our solution they are always sent to us?)
    if (!packet->recipient)
    {
        // Check the special case for the register packet where sender and recipient are nearly the same.
   //     std::cout<<"recipientData: "<<packet->recipientData<<" senderData:"<<packet->senderData;
        if (packet->senderData.Contains(packet->recipientData) ||
            packet->recipientData.Contains(packet->senderData))
        {
            packet->recipient = packet->sender;
        //    std::cout<<"Contains o-o";
            return;
        }
        String recipient = packet->recipientData;
		assert(packet->recipientData.Length());
     //   std::cout<< "recipient: "<<recipient;
        String address = packet->recipientData;

       // packet->recipient = GetPeerByAddress(packet->);
        packet->recipient = GetPeerByData(recipient);
       // assert(packet->recipient);
    }
}

/// Uses the "name <name@ip>" from/to field to associate a peer.
Peer * SIPSession::GetPeerByData(String data)
{
 //   std::cout<<"\nNetworkManager::GetPeerByData data: "<<data;
    // Extract name, ip and port.
    List<String> tokens = data.Tokenize(" <>@:");
 //   std::cout<<"Tokens: "<<tokens;
    String name, ipAddress, portString;
    if (tokens.Size() < 4)
        return NULL;
    assert(tokens.Size() >= 4);
    name = tokens[0];
    ipAddress = tokens[2];
    portString = tokens[3];
 //   std::cout<<"PortString: "<<portString;
    int port = portString.ParseInt();
    for (int i = 0; i < peers.Size(); ++i){
        Peer * peer = peers[i];
  //      std::cout<<"Peer "<<i<<" name: "<<peer->name<<" ip: "<<peer->ipAddress<<" port:"<<peer->port;
        if (peer->name == name &&
            peer->ipAddress == ipAddress &&
            peer->port == port)
            return peer;
    }
	SIPSessionData * mySIPSessionData = (SIPSessionData*)me->GetSessionData(SessionType::SIP);
    String mySenderData = mySIPSessionData->SenderData();
 //   std::cout<<"data: "<<data<<" me->senderData:"<<mySenderData;
 //   std::cout<<"Me: "<<me->name<<" ip:"<<me->ipAddress<<" port:"<<me->port;
 //   std::cout<<"Extracted: "<<name<<" ip:"<<ipAddress<<" port:"<<port;
    if (me->name == name &&
        me->ipAddress == ipAddress &&
        me->port == port){
//        std::cout<<" is me";
        return me;
    }
    std::cout<<"\nWARNING: Unable to find sender for target packet.";
    return NULL;
}

/// Send a NOTIFY message to all previously connected peers to inform of the new peer's arrival to the network
void SIPSession::InformPeersOfNewConnectedPeer(Peer * newPeer)
{
    for (int i = 0; i < peers.Size(); ++i)
    {
        Peer * peer = peers[i];
		SIPSessionData * peerSIPSessionData = (SIPSessionData*)peer->GetSessionData(SessionType::SIP);
        if (peer == newPeer)
            continue;
        if (peerSIPSessionData->peerSubscribedToNewPeerConnections)
        {
            SIPNotifyPacket notifyPacket(PEER_CONNECTED_EVENT);
            String newPeerString = newPeer->ipAddress+" "+String::ToString(newPeer->port);
            notifyPacket.body.Add(newPeerString);
            notifyPacket.Send(peer);
        }
    }
}

/** Contrary to the above method, this one informs a single newly connected peer of the entire list of connected peers.
    This should be sent once at the start of a subscription!
*/
void SIPSession::InformPeerOfConnectedPeers(Peer * newPeer)
{
    std::cout<<"\nNetworkManager::InformPeerOfConnectedPeers";
    SIPNotifyPacket notifyPacket(PEER_CONNECTED_EVENT);
    List<String> newPeersList;
    for (int i = 0; i < peers.Size(); ++i)
    {
        Peer * peer = peers[i];
        if (peer == newPeer)
            continue;
        Socket * socket = peer->primaryCommunicationSocket;
        assert(socket);
        String peerString = peer->ipAddress+" "+String::ToString(peer->port);
        newPeersList.Add(peerString);
    }
    notifyPacket.body += newPeersList;
    notifyPacket.Send(newPeer);
}

/// Sets list of media to be made publicly available.
void SIPSession::SetAvailableMedia(List<String> list)
{
	/*
    availableMedia = list;
    // Inform all connected peers of the update
    for (int i = 0; i < peers.Size(); ++i){
        Peer * peer = peers[i];
        InformPeerOfAvailableSoughtMedia(peer);
    }
	*/
}
/// Informs a peer of the entire list of media available to it, conforming to it's requested search-string.
void SIPSession::InformPeerOfAvailableSoughtMedia(Peer * peer)
{
	/*
    SIPNotifyPacket notifyPacket(MEDIA_SEARCH_EVENT);
    String string = peer->mediaSearchString;
    // Not seeking if the media search-string is 0-length, so abort if so.
    if (string.Length() == 0)
        return;
    List<String> relevantMedia;
    for (int i = 0; i < availableMedia.Size(); ++i){
        String mediaEntry = availableMedia[i];
        bool add = false;
        if (string == "*")
            add = true;
        if (mediaEntry.Contains(string, Qt::CaseInsensitive))
            add = true;
        if (add)
            relevantMedia.Add(mediaEntry);
    }
    notifyPacket.body = relevantMedia;
    notifyPacket.bodySet = true;
    notifyPacket.Send(peer);
	*/
}

// Extracts the "tag"-Identifier for the peer, hopefully provided in the packet.
bool SIPSession::ExtractPeerTag(Peer * peer, SIPPacket * packet)
{
    std::cout<<"NetworkManager::ExtractPeerTag for peer: "<<peer->name<<" and packet: "<<packet;
    String from = packet->senderData;
	List<String> tagTokens = from.Tokenize("=");
	SIPSessionData * peerSIPSessionData = GetSessionData(peer);
    if (tagTokens.Size() > 1)
    {
        String tag = tagTokens[1];
        std::cout << "Setting peer tag to requested value: "<<tag;
        peerSIPSessionData->theirTag = tag;
        peer->isValid = true;
		return true;
    }
	return false;
}

/// Builds and sends a SIP Register message to target peer.
void SIPSession::RegisterWithPeer(Peer * peer)
{
    // Register with the peer for 3600 seconds by default.
    SIPRegisterPacket pack(3600);
    pack.Send(peer);
}

/// Number of peers reigsterd successfully.
int SIPSession::NumRegisteredPeers(){
	int validPeers = 0;
	for (int i = 0; i < peers.Size(); ++i){
		if (peers[i]->isValid)
			validPeers++;
	}
	return validPeers;
}


/// Builds and sends a SIP Subscribe message to target peer in order to automatically discover new connected peers to the network.
void SIPSession::SubscribeToNewPeerConnections(Peer * peer)
{
	SIPSessionData * peerSIPSessionData = GetSessionData(peer);
    if (peerSIPSessionData->meSubscribedToNewPeerConnections){
        return;
    }
    // Once we successfully register ourselves with a client, subscribe to events like new peers connecting!
	SIPSubscribePacket subscribe(PEER_CONNECTED_EVENT, 300, SIPPacket::NextCSeq());
    subscribe.Send(peer);
    std::cout<<"Sent Subscription for event: "<<PEER_CONNECTED_EVENT;
}


// Ensures that all peer-parameters have been set apporpriately, using the given packet's data as needed.
bool SIPSession::EnsurePeerData(Peer * peer, SIPPacket * packet){
	assert(peer && "Peer not valid, Create it before calling EnsurePeerData");
	// This function should only have to run exactly ONCE. Return if peer already valid (marked so at the end of the function.)
    if (peer->isValid)
        return true;
    assert(peer && packet);
    String nameChange = "EnsurePeerData: NameChange: "+peer->name+" -> ";
    // Extract name of peer from the packet.
	peer->name = packet->senderData.Tokenize(" ")[0];
	SIPSessionData * peerSIPSessionData = GetSessionData(peer);
    nameChange.Add(peer->name);
//    Q_EMIT ConnectionEvent(nameChange);

    std::cout<<"\nNetworkManager::EnsurePeerData for peer: "<<peer->name;
    String from = packet->senderData;

    // Try and extract peer-tag.
    bool success = ExtractPeerTag(peer,packet);
	if (!success){
		std::cout<<"\nUnable to fetch peer tag!";
		return false;
	}

    String senderData = packet->senderData;
    // Parse sender data for name and stuff
    List<String> tokens = senderData.Tokenize(" <>\n\r\t@;:");
    std::cout << "Tokens: "<<tokens;
    if (tokens.Size() < 4)
        return false;
    assert(tokens.Size() >= 4);
    peer->ipAddress = tokens[2];
    // Parse port
    String portString = GetSection(senderData, ':', '>');
    std::cout<<"portString: "<<portString;
    if (portString.Length())
    {
		peer->port = portString.ParseInt();
    }
    // Parse tag
	tokens = senderData.Tokenize("=");
    if (tokens.Size() >= 2){
        String tagString = tokens[1];
        peerSIPSessionData->theirTag = tagString;
        std::cout<<"senderData: "<<senderData;
        std::cout<<"tagString: "<<tagString;
    }

    std::cout<<"Tokens: "<<tokens.Size();
    std::cout<<"Peer port set to: "<<peer->port;
    std::cout<<"Peer tag set to: "<<peerSIPSessionData->theirTag;

    // Update sender-data no matter what?
    peerSIPSessionData->UpdateSenderData(peer);
    /// Set validity after extracting all necessary data above. Peer is now ready to be used to send packets to.
    peer->isValid = true;
    return true;
}

/// Handles packet received from target peer. Returns the validity of the peer after processing the packet. If false, the peer has been invalidated.
bool SIPSession::HandlePacket(SIPPacket* packet, Peer* peer)
{
    // Make sure the packet's sender and recipient are both parsed, if possible.
    FindPeersForPacket(packet, peer);
    bool peerInvalidated = false;
	SIPSessionData * peerSIPSessionData = NULL;
	// If peer is null, fetch it from the packet if possible
	if (!peer)
		peer = packet->sender;
	// If we found a peer, get session data, if not, it might be a new peer thats registering.
	if (peer)
		peerSIPSessionData = GetSessionData(peer);

    std::cout<<"Packet received: \"";
    packet->Print();
    std::cout<<"\"";

    switch(packet->sipType)
    {
        case SIP_BAD_REQUEST:
        {
			std::cout << "ERROR: Bad Request returned from server T___T";
            // TODO: Give a valid GUI response or some other kind of itnernal handling?
//            QMessageBox::information(NULL, "Network Error", "Bad Request from server: "+packet->data);
			break;
		}
        case SIP_REGISTER:
        {
			/// If already registered, just increment registration duration, yo?
            if (peerSIPSessionData && peerSIPSessionData->registeredWithUs){
                std::cout<<"Peer already registered and valid ^^";
                break;
            }
			/// Ensure the packet has required fields
            if (!packet->cSeq.Length())
            {
                std::cout <<"ERROR: SIP_REGISTER packet lacking cSeq, will not reply nor perform registering.";
                packet->PrintHeaderData();
                SIPBadRequestPacket pack(packet, "Missing CSeq header field");
                if (peer)
					pack.Send(peer);
                break;
            }
			/// Check that the name isn't already registered.
			String name = packet->senderData.Tokenize(" ")[0];
			/// Flag so we know if we should add it to the list later of if it already exists.
			bool newPeer = false;
			if (!peer)
				peer = NetworkMan.GetPeerByName(name);
			/// The name is registered
			if (peer){
				/// Check if already connected,... somehow.
				if (peer->primaryCommunicationSocket){
					// If so, refuse this connection.
					SIPBadRequestPacket pack(packet, "Peer with same name already exists. Use a new name and try again.");
					pack.Send(packet->socket);
					break;
				}
				/// If not, use it
				// ...
			}
			// If no peer exists for the given data, create it.
			else {
				/// Create a new peer!
				peer = new Peer();
				newPeer = true;
			}
			/// Ensure peer has SIPSessionData
			peerSIPSessionData = this->GetSessionData(peer);
			if (!peerSIPSessionData){
				/// Create SIP Session data for the peer if not done already.
				peerSIPSessionData = new SIPSessionData(peer);
				peer->sessionData.Add(peerSIPSessionData);
			}
			peerSIPSessionData = GetSessionData(peer);
			/// Reset SIP data since its ish a new connection.
			peerSIPSessionData->Reset();
			assert(peerSIPSessionData);

			/// Bind the socket that sent the packet to the new peer.
			peer->primaryCommunicationSocket = packet->socket;
			packet->socket->peer = peer;

			// Extract Peer-data from packet into the new peer structure.
            bool result = EnsurePeerData(peer, packet);
            if (!result)
            {
				delete peer;
				delete peerSIPSessionData;
                std::cout<<"ERROR: Unable to set peer data, ignoring registration.";
                packet->PrintHeaderData();
                SIPBadRequestPacket pack(packet, "Bad peer data. Required format: <name@ip:port>;tag=requestedTag");
                pack.Send(packet->socket);
                break;
            }
			/// Mark peer as registered!
            peerSIPSessionData->registeredWithUs = true;

            // Inform them of the register by sending an OK-packet!
            SIPOKPacket okPack(packet);
            okPack.Send(peer);

			/// Save new peer into list of peers
			if (newPeer)
				peers.Add(peer);
			newPeers.Add(peer);

            // If we're not registered with them yet, go ahead and do that now too, since we want mutual registration here, fully P2P..?
            if (!peerSIPSessionData->registeredWithPeer)
            {
                // Register with this peer!
                RegisterWithPeer(peer);
            }
            break;
        }
		case SIP_OK:
        {
            std::cout<<"\nEvaluating SIP_OK message";
			std::cout << "\nSIP_OK received! Check the cSeq (subject it's replying to)";
            String cSeq = packet->cSeq;
			std::cout<< "\ncSeq: " << cSeq;
			/// Confirmed registration
			if (cSeq.Contains("REGISTER"))
            {
                std::cout<<"\nSIP_OK received for REGISTER! Sending a subscribe-message!";
				/// If not peer, create it
				if (!peer)
					peer = NetworkMan.GetPeerByName(name);
				if (!peer){
					peer = new Peer();
					peer->primaryCommunicationSocket = packet->socket;
					peers.Add(peer);
					newPeers.Add(peer);
				}
				// Ensure peer-contact data before replying.
                EnsurePeerData(peer, packet);
                Peer * prePeer = peer;
                // Merge duplicate peers, if any. Merge should occur here or in the SIP_OK when we have registerd with a peer, as the mutual registrations and connection initiations are executed first.
            //    peer = MergeDuplicatePeers(peer);
                if (peer != prePeer)
                {
                    std::cout<<"Merge done!";
                    peerInvalidated = true;
                }
                // Bool that WE are registered with the peer.
                peerSIPSessionData->registeredWithPeer = true;

                // Subscribe to new peer-connections
                SubscribeToNewPeerConnections(peer);
				// Subscribe to available games, search for 60 seconds
				SIPSubscribePacket sub(GAME_SEARCH_EVENT, 60, SIPPacket::NextCSeq());
				sub.Send(peer);
			}
            // Subscription successful.
            else if (cSeq.Contains("SUBSCRIBE"))
            {
                // Alright, so identify WHICH subscription was successful...
                packet->Print();
                List<String> cSeqList = cSeq.Tokenize(" ");
                String cSeqIntStr = cSeqList[0];
				int cSeqInt = cSeqIntStr.ParseInt();


/*
				switch(cSeqInt)
                {
                    case PEER_CONNECTED_EVENT_CSEQ:
                        peerSIPSessionData->meSubscribedToNewPeerConnections = true;
                        break;
                    case MEDIA_SEARCH_EVENT_CSEQ:
                        break;
                    default:
                        assert(false);
                        break;
                }
  */
				// Yey, subscription was successful.. now what?
                std::cout<<"\nSubscription was successful. Yay.";
                // Now a timer should be started to correctly re-subscribe before the subscription expires.
               // assert(false);
            }
            // Inform relevant entities to for additional processing.
         //   if (peer->isValid && peerSIPSessionData->registeredWithUs)
          //      ;// Q_EMIT SIPOKPacketReceived(packet);
            break;
		}
        case SIP_SUBSCRIBE:
        {
            // Avoid handling notify messages until the peer has been verified.
			if (!peer->isValid){
				assert(false && "Invalid peer performing subscribe requests");
				break;
			}
            std::cout<<"\nEvaluating SIP_SUBSCRIBE message";
            // Determine if we are to accept the subscription-request.
            bool subscribeRequestOK = true;
            SIPSubscribePacket * pack = (SIPSubscribePacket*) packet;
            // Peer-discovery!
        	String eventType;
			bool success = packet->GetHeaderData("Script", eventType);
		//	peerSIPSessionData->sub
            int requestedExpirationTime = pack->expirationTime;

			/// Add the event type to its sbscruption if its deemed ok.
			if (eventType == PEER_CONNECTED_EVENT ||
				eventType == MEDIA_SEARCH_EVENT ||
				eventType == GAME_SEARCH_EVENT)
			{
				std::cout<<"Subscription for game search event received.";
				peerSIPSessionData->SubscribeToEvent(eventType, requestedExpirationTime);
				subscribeRequestOK = true;
			}
            // If not deemed ok,  then don't send the OK-packet back below..!
            if (!subscribeRequestOK){
                // Send error packet! :3
                SIPBadEventPacket badEventPack(packet);
                badEventPack.Send(peer);
                break;
            }
            // Send an OK-message with expiration date.
            SIPOKPacket ok(packet);
            /*  Expiration needed as per the rfc: https://ietf.org/doc/rfc3265/?include_text=1

                "200-class responses to SUBSCRIBE requests also MUST contain an
                "Expires" header.  The period of time in the response MAY be shorter
                but MUST NOT be longer than specified in the request.  The period of
                time in the response is the one which defines the duration of the
                subscription." */
            // Set expiration time, max 300 .. units.
            ok.expirationTime = requestedExpirationTime < 300? requestedExpirationTime : 300;
            ok.Send(peer);
            break;
        }
        case SIP_NOTIFY:
        {
            // Avoid handling notify messages until the peer has been verified.
            if (!peer->isValid)
                break;
            bool validNotification = false;
            SIPNotifyPacket * notifyPacket = (SIPNotifyPacket*) packet;
            std::cout<<"SIP_NOTIFY received with event: "<<notifyPacket->event;
            std::cout<<" "<<notifyPacket->GetBodyAsString();
            // Check if we're subscribing to zis?
            // ...
            if (notifyPacket->event == PEER_CONNECTED_EVENT)
            {
                // Arara! New peer connected o-o
                List<String> newPeers = notifyPacket->body;
                // Find new peers from within the list if possible and connect to them.
                DiscoverPeers(newPeers);
                // Send a 200 OK message that we understood this.
                SIPOKPacket ok(packet);
                ok.Send(peer);
            }
            else if (notifyPacket->event == MEDIA_SEARCH_EVENT)
            {
                std::cout<<"Got search data.";
                List<String> media = notifyPacket->body;
                peerSIPSessionData->soughtMedia = media;
          //      Q_EMIT SoughtMediaUpdatedInPeers(peers);
                SIPOKPacket ok(packet);
                ok.Send(peer);
            }
			else if (notifyPacket->event == GAME_SEARCH_EVENT)
			{
				std::cout<<"Got game search data.";
                List<String> games = notifyPacket->body;
				peerSIPSessionData->soughtGames = games;
				MesMan.QueueMessages("OnAvailableGamesUpdated");
				SIPOKPacket ok(packet);
                ok.Send(peer);
			}
            /* "The latter behavior is invalid, and
               MUST receive a "481 Subscription does not exist" response (unless
               some other 400- or 500-class error code is more applicable)
            */
            if (!validNotification){
                // Send some error message
            }
            break;
        }
		case SIP_NULL:
        {
			std::cout <<"SIP_NULL packet received D:";
			break;
		}
        // Application-specific packets.
        case SIP_INFO:
        {
            if (peer->isValid && peerSIPSessionData->registeredWithUs)
				std::cout<<"\nValid SIP Info packet received"; // Q_EMIT SIPInfoPacketReceived(packet);
            else {
                // TODO: Reply with some kind of error-package?
                std::cout<<"Peer not registered yet! Handle appropriately.";
                SIPBadRequestPacket * pack = new SIPBadRequestPacket(packet, "Not registered! Please register before sending any other requests.");
                pack->Send(peer);
                break;
            }
            // Send the packet to the MainWindow for default-processing?
          //  std::cout<<"SIPInfo packet received, emitting signal for it to be handled elsewhere (as it's probably content-specific/-related)";
            break;
        }
        case SIP_INVITE:
        {
            // Inform relevant connections to deal with the invite accordingly.
            if (peer->isValid && peerSIPSessionData->registeredWithUs)
                std::cout<<"\nValid SIP Invite packet received";;//Q_EMIT SIPInvitePacketReceived(packet);
            break;
        }
        case SIP_BYE:
        {
            // Informs relevant connections to deal with the SIP message accordingly.
            std::cout<<"\nSIP Bye packet received"; // Q_EMIT SIPByePacketReceived(packet);
            break;
        }
        default:
        {
            std::cout <<"NetworkManager::HandlePacket: WARNING: Unimplemented packet type received!";
            break;
        }
    }
    return !peerInvalidated;
}



/// Returns sessionData associated with the peer.
SIPSessionData * SIPSession::GetSessionData(Peer * forPeer){
	assert(forPeer);
	return (SIPSessionData*)forPeer->GetSessionData(SessionType::SIP);
}

/// Returns a list of currently connected peers (by checking if the primaryCommunicationSocket is valid).
List<Peer*> SIPSession::ConnectedPeers()
{
	List<Peer*> connectedPeers;
	for (int i = 0; i < peers.Size(); ++i)
	{
		if (peers[i]->primaryCommunicationSocket)
			connectedPeers.Add(peers[i]);
	}
	return connectedPeers;
}

/// Attempts to discover peers using the string format "ip port", e.g. "127.0.0.1 33000"
void SIPSession::DiscoverPeers(List<String> peersList){
    std::cout<<"NetworkManager::DiscoverPeers";
/*
	int peersConnectedTo = 0;
    for (int i = 0; i < peersList.Size(); ++i){
        String str = peersList[i];
        List<String> tokens = str.Tokenize(" ");
        if (tokens.Size() < 2){
            std::cout<< "NetworkManager::DiscoverPeers: Not enough tokens in \""<<str<<"\", please use the format \"ip port\"";
            continue;
        }
        String ip = tokens.at(0);
        String portStr = tokens.at(1);
        int port;
        bool ok;
        port = portStr.toInt(&ok);
        if (!ok){
            std::cout << "NetworkManager::DiscoverPeers: Bad port: "<<str;
            continue;
        }
        bool connected = ConnectTo(ip, port);
        // If we managed to connect, continue, but if not, try using the default ports 33000 through 33010.
        if (!connected)
        {
            /*
            for (port = 33000; port < 33010; ++port)
            {
                connected = ConnectTo(ip,port);
                if (connected)
                    break;
            }
            */
/*
        }
        // Increment debug var for each successful connection.
        if (connected)
            ++peersConnectedTo;
    }
    std::cout<<"NetworkManager::DiscoverPeers called, managed to connect to "<<peersConnectedTo<<" out of "<<peersList<String>.Size()<<" peers.";
*/
}

/// Sets event state. If it didn't exist before it will be created.
void SIPSession::SetEventState(String eventName, String eventState){
	for (int i = 0; i < events.Size(); ++i){
		SIPEvent * e = events[i];
		if (e->name == eventName){
			/// Inform current subscribers.
			e->state = eventState;
			OnEventUpdated(e);
			return;
		}
	}
	SIPEvent * e = new SIPEvent();
	e->name = eventName;
	e->state = eventState;
	OnEventUpdated(e);
}

/// Called every time a socket is deleted. Any references to the socket should then be removed.
void SIPSession::OnSocketDeleted(Socket * sock){
	Session::OnSocketDeleted(sock);
}

/// Informs peers currently subscribing of the new event state.
void SIPSession::OnEventUpdated(SIPEvent * e){
	for (int i = 0; i < peers.Size(); ++i){
		Peer * p = peers[i];
		SIPSessionData * ssd = GetSessionData(p);
		if (ssd->SubscribedToEvent(e->name)){
			// Send a notify
		}
	}
}
