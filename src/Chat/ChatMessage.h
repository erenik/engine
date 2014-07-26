// Emil Hedemalm
// 2013-08-09

#ifndef CHAT_MESSAGE_H
#define CHAT_MESSAGE_H

#include "String/AEString.h"

class Peer;

/// Not related to the Message class used for the game engine's general messaging system.
class ChatMessage
{
public:
    enum chatMessageTypes {
        NULL_TYPE,
        PUBLIC = 1, /// Sent to all
        PRIVATE = 2, /// Sent to me only
        SPECIAL = 3, /// If sending to a group of people?
        GLOBAL_ANNOUNCEMENT = 4,

    };
    ChatMessage(Peer * peerWhoSentIt, String text, int type = PUBLIC);

    /// See enum above.
    int type;
    /// Store player name and IP in-case they
    String playerName;
    String fromIP;
    /// Time sent or received. Set automatically in the constructor by default.
	time_t timeCreated;
    time_t timeSent;
	time_t timeReceived;
    String timeString;
    /// Note that this player pointer can be invalid if the player is removed after the message has been sent! TODO: Fix in chat manager when a player disappears.
    Peer * from;
    String text;
};

#endif
