/// Emil Hedemalm
/// 2014-06-20
/// Further dividing the source files...

#ifndef RUNE_BATTLE_H
#define RUNE_BATTLE_H

#include "String/AEString.h"
#include "DataTypes.h"

class RuneBattler;

class RBattleState
{
public:
	/// Temporary log. May be modified by actions so that text is presented in either log-form of in-game.
	String log;
	int timeInMs;
	List<RuneBattler*> battlers;
};

// To read in pre-defined battles from .txt or otherwise!
struct RuneBattle
{
	/// Loads the expressions/functions/equations for battle from target file.
	static void LoadBattleFunctions(String fromSource);

	/// Parses data concerning this battle.
	bool Load(String fromSource);


	/// List of player battlers to participate
	List<String> playerNames;
	/// List of enemy battlers to participate
	List<String> enemyNames;


	/// Where does the battle partake? This will probably be more advanced later on... D:
	String backgroundImage;

	// Name of the battle
	String name;
	// Source file it was loaded from.
	String source;
	// If set, adds all current players to the battle (true by default).
	bool addCurrentPlayers;
};

#endif
