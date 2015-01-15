/// Emil Hedemalm
/// 2014-01-24
/// SessionData class for the SIP Session.

#include "SIPSessionData.h"
#include "Network/Peer.h"
#include "Network/Session/SessionTypes.h"
#include "SIPEvent.h"
#include "Timer/Timer.h"
#include "Network/NetworkManager.h"
#include "SIPSession.h"
#include "Network/Peer.h"
#include "Game/Game.h"

SIPSessionData::SIPSessionData(Peer * owner)
: SessionData(Type()), owner(owner)
{
	Reset();
}

SIPSessionData::~SIPSessionData()
{
	eventSubscriptions.ClearAndDelete();
}

// ID used for template function identification.
int SIPSessionData::Type()
{
	return SessionType::SIP;
}


/// Resets variables for the session
void SIPSessionData::Reset()
{
	registeredWithUs = 
	registeredWithPeer = false;
	peerSubscribedToNewPeerConnections = 0;
	meSubscribedToNewPeerConnections = 0;
	for (int i = 0; i < eventSubscriptions.Size(); ++i){
		SIPEventSubscription * ses = eventSubscriptions[i];
		std::cout<<"\nDeleting subscription: "<<ses->name;
	}
	eventSubscriptions.ClearAndDelete();
}


// Does it! See SenderData() and senderData.
void SIPSessionData::UpdateSenderData(Peer * owner){
	String lowerCaseName = owner->name; //.toLower();
	String fullAddress = owner->ipAddress;
	if (owner->port == -1)
		fullAddress.Add(":invalidPort");
	else 
		fullAddress.Add(":"+String::ToString(owner->port));
	senderData = owner->name+" <"+lowerCaseName+"@"+fullAddress+">";
	if (theirTag.Length()){
		senderData.Add(";tag="+theirTag);
	}
}

/// Woo. Subscribes for a while.
void SIPSessionData::SubscribeToEvent(String eventType, int duration)
{
	SIPEventSubscription * target = NULL; 
	for (int i = 0; i < eventSubscriptions.Size(); ++i)
	{
		SIPEventSubscription * e = eventSubscriptions[i];
		if (e->name == eventType)
		{
			std::cout<<"\nTarget already subscribed to event.";
			target = e;
			break;
		}
	}
	/// If not existing, create.
	if (!target){
		target = new SIPEventSubscription();
		target->name = eventType;
		eventSubscriptions.Add(target);
		/// Notify
		if (eventType == GAME_SEARCH_EVENT){
			List<Game*> games = NetworkMan.GetAvailableGames();
			List<String> gamesList;
			for (int i = 0; i < games.Size(); ++i){
				gamesList.Add(games[i]->ToString());
			}
			SIPNotifyPacket notify(GAME_SEARCH_EVENT);
			notify.body = gamesList;
			// Send it to the owner of the subscription.
			notify.Send(owner);
		}
	}
	/// Set new duration and start-time.
	target->duration = duration;
	target->startTime = Timer::GetCurrentTimeMs();
}

bool SIPSessionData::SubscribedToEvent(String eventName){
	for (int i = 0; i < eventSubscriptions.Size(); ++i){
		SIPEventSubscription * e = eventSubscriptions[i];
		if (e->name == eventName)
			return true;
	}
	return false;
}
