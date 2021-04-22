/// Emil Hedemalm
/// 2014-01-29
/// Structure for handling available games (mainly for networking)

#ifndef GAME_H
#define GAME_H

#include "String/AEString.h"
#include "Time/Time.h"
#include "MathLib.h"

class Integrator;
class Entity;
class Message;

namespace GameType {
	enum gameTypes 
	{
		Game2D,
	};
};

class Game {
public:
	Game(String name);
	Game(String name, int type, String host, String port, int currentPlayers, int maxPlayers);
	virtual ~Game();
	// Resets variables (mainly called on creation)
	void Nullify();

	/// Performs one-time initialization tasks. This should include initial allocation and initialization.
	virtual void Initialize() = 0;
	/// o--0
	virtual void ProcessMessage(Message * message) = 0;

	/// Resets the entire game. Similar to a hardware reset on old console games.
	virtual void Reset() = 0;

	/// Call on a per-frame basis.
	virtual void Process() = 0;
	/// Fetches all entities concerning this game.
	virtual List< Entity* > GetEntities() = 0;
	/// Fetches all static entities.
	virtual List< Entity* > StaticEntities() = 0;

	/// Sets if the game should use mouse input in a default manner for the player to play the game or not.
	virtual void UseMouseInput(bool value);
	/// Sets the pause boolean
	virtual void SetPause(bool newPausedState);

	/// Sets frame size of the game
	void SetFrameSize(Vector2i size);

	bool LoadFrom(String);
	String ToString();

	/// Game-time in milliseconds. Should be updated by the game's global game state (specifically needed for networked-games).
	int64 gameTime;

	/// User-defined name of the game.
	String name;
	/// Type of game, this will be set by game. See GameType namespace/gameTypes enum.
	int type; 
	/// Host name or IP address to connect to to join this game
	String host;
	/// Port to use while connecting to join this game
	int port;
	/// Port for game rapid communication.
	int udpPort;
	/// Current and maximum players
	int currentPlayers, maxPlayers;

	/// o.o
	bool useMouseInput;
	/// If paused..
	bool paused;

	/// Current state of the game.
	int gameState;
	enum 
	{
		SETTING_UP_PLAYFIELD,
		GAME_BEGUN,
	};
	/// Size of the playing field. Used for 2D single-viewport games
	Vector2f gameSize;
};

#endif