// Emil Hedemalm
// 2013-07-09

// An RPG-based class/struct
#ifndef BATTLER_H
#define BATTLER_H

#include "String/AEString.h"
#include "BattleState.h"

struct BattleActionCategory;
class BattleAction;

namespace BattlerSide {
    enum {
        NULL_SIDE,
        PLAYER,
        SIDE2,
        /// More sides for confusing battles.. lol?
        SIDE3,
        SIDE4,
        MAX_BATTLER_SIDES
    };
};

// Should be subclassed to get the behaviour your game wants!
class Battler{
public:
	Battler();
	virtual ~Battler();
	virtual void Process(BattleState &battleState) = 0;
	virtual bool IsIdle() = 0;

	/// Make actions available for this battler. Creates and links the necessary categories for managing them as needed.
	void AddActions(List<BattleAction*> actions);
	BattleActionCategory * GetCategory(String byName);

	String name;
	/// Which side of the battle it's on. (default player = 0, enemies = 1) See above!;
	int side;
	/// Is it an AI or player-controlled unit?
    bool isAI;
    /// What state is it in? These can be defined by your subclass or wherever.
    int state;

    /// What kind of actions the battler possess.
	List<BattleActionCategory*> actionCategories;
	List<BattleAction*> actions;
	/// Before they've been bound.
	List<String> actionNames;

private:

};

#endif
