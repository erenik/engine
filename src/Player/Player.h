// Emil Hedemalm
// 2013-07-10
// A meta-structure for keeping track of a player and all it's external relations.


#ifndef PLAYER_H
#define PLAYER_H

#include "String/AEString.h"
#include "MathLib.h"

class Entity;
class Viewport;
class Peer;

/// Subclass in order to include more data specific to your game. Note that this can be an AI too.
class Player {
	friend class InputManager;
	friend class PlayerManager;
public:
	Player(String name = "DefaultPlayerName");
	/// Virtual destructor for subclassing..!
	virtual ~Player();
	/// Name of ze playah, of course!
	String name, type;
	/// Primary entity Games may feature any kinds of entity configurations, and should thus be added only in sub-classes of the Player.
//	Entity* entity;
	/// True for non-networkers
	bool isLocal;
	/// For AI-players.
	bool isAI;
	/// Usually used, a color is.
	Vector4f color;
	/// View port dedicated to this player!
	Viewport * viewport;

	/// Will be an enum as displayed in Input/InputDevices.h
	int InputDevice() const { return inputDevice; };

	/// Requested ID will be 1 when it has been requested, and 0 when the request is fulfilled!
	int requestedID;
	
    /// Index of their multiplayer client. 0 for host, valid integers for the rest. ...what << PRobably obsolete?
	int clientIndex;
	/// Network client who controls this player. A single peer may control multiple players (consider split-screen)
	Peer * owner;

    /// Can be set manually, but perhaps a function should be added in either the InputManager or PlayerManager for switching this?
	int inputDevice;

	/// Getter
	int ID() { return id; };

private:
	/// Used to set ID to new players.
	static int idEnumerator;
	/// ID used for multiplayer, but could be used for more.
	int id;
	
};



#endif
