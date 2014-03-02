// Emil Hedemalm
// 2013-07-09

// An RPG-based class/struct

#ifndef BATTLE_STATE_H
#define BATTLE_STATE_H

#include "String/AEString.h"

enum battleStates{
	BATTLE_NOT_STARTED,
	BATTLE_STARTED,
	BATTLE_ENDED,
};

class Battler;

struct Side {
	Side(){ sideName = "Unnamed side"; };
	Side(String name){sideName = name;};
	List<Battler*> battlers;
	String sideName;
};

/// Comprehensive structure for the current battle's state.
struct BattleState{
	BattleState();
	~BattleState();
	/// Player-portion of the battlers
	List<Battler*> players;
	/// AI/NPC-portion of the battlers
	List<Battler*> npcs;
	/// Whole group of battlers
	List<Battler*> battlers;
	/// The battlers divided into sides.
	List<Side*> sides;
	/// If it's begun, middle or ended. Simple flag!
	int state;
	/// Time passed since last processing, set by the BattleManager each "round", in milliseconds!
	int timeDiff;
};

#endif
