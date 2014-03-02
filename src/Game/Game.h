/// Emil Hedemalm
/// 2014-01-29
/// Structure for handling available games (mainly for networking)

#ifndef GAME_H
#define GAME_H

#include "String/AEString.h"

class Game {
public:

	Game();
	Game(String name, String type, String host, String port, int currentPlayers, int maxPlayers)
		: name(name), type(type), host(host), port(port), currentPlayers(currentPlayers), maxPlayers(maxPlayers)
	{
	}

	bool LoadFrom(String);
	String ToString();

	/// Game-time in milliseconds. Should be updated by the game's global game state (specifically needed for networked-games).
	long long gameTime;

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