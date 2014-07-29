/// Emil Hedemalm
/// 2014-01-29
/// Structure for handling available games (mainly for networking)

#ifndef GAME_H
#define GAME_H

#include "String/AEString.h"
#include "Time/Time.h"

class Entity;

class Game {
public:

	Game(String name);
	Game(String name, String type, String host, String port, int currentPlayers, int maxPlayers);
	// Resets variables (mainly called on creation)
	void Nullify();

	/// Call on a per-frame basis.
	virtual void Process() = 0;
	/// Fetches all entities concerning this game.
	virtual List<Entity*> GetEntities() = 0;


	bool LoadFrom(String);
	String ToString();

	/// Game-time in milliseconds. Should be updated by the game's global game state (specifically needed for networked-games).
	int64 gameTime;


	/// If paused..
	bool paused;
	/// User-defined name of the game.
	String name;
	/// Type of game, this will be set by game
	String type; 
	/// Host name or IP address to connect to to join this game
	String host;
	/// Port to use while connecting to join this game
	int port;
	/// Port for game rapid communication.
	int udpPort;
	/// Current and maximum players
	int currentPlayers, maxPlayers;
};

#endif