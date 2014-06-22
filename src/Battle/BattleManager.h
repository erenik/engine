// Emil Hedemalm
// 2013-07-09

// An RPG-based class/struct

#ifndef BATTLE_MANAGER_H
#define BATTLE_MANAGER_H

#include <List/List.h>
#include <Queue/Queue.h>
//#include "BattleState.h"

#define BattleMan (*BattleManager::Instance())

enum battleTimeType {
	NO_TIME, TURN_BASED, REAL_TIME, BATTLE_TIME_TYPES,
};

struct BattleState;
class Battler;
class BattleAction;
/*
struct QueuedBattleAction {
    BattleAction * ba;
    List<Battler*> subjects;
    List<Battler*> targets;
};
*/
class BattleManager {
	BattleManager();
	~BattleManager();
	static BattleManager * battleManager;
public:
	static BattleManager * Instance();
	static void Allocate();
	static void Deallocate();
	static bool IsAllocated();

	/// Clears active battlers and prepares loading in new ones, using previously set default values.
	void NewBattle();
	/// Adds a battler to the battle! NULL = random-generated test-battler.
	void AddBattler(Battler * battler = NULL, bool playerControlled = false);
	/// Fixes stuff before processing may begin, like placing them on the "battlefield", et al.
	void Setup();

    /// Returns all battlers
    Battler * GetBattlerByName(String byName) const;
    List<Battler*> GetBattlers() const { return battlers; };
    List<Battler*> GetPlayers() const;
    /// Returns all idle player-battlers! Checked via the Battler::IsIdle()-function!
	List<Battler*> GetIdlePlayerBattlers() const;
	/// See Battler.h for filters (relevant targets) 
	Battler * RandomTarget(int filter, Battler * subject);
	/// Changes state to ended.
	void EndBattle();

    /// Queue waeh.
	void QueueAction(BattleAction * ba);

	/// TURN_BASED or REAL_TIME
	void SetBattleTimeType(int type);
	/// Milliseconds or turns, depending on type
	void Process(int timeDiff);
	/// Queues it up!
	void QueueBattle(String battle);
	/// Returns the queued battle-source/name, if it exists.
	String QueuedBattle();
	/// Starts up the queued battle, moving it to be the active one and empties the queued string.
	void StartBattle();
private:
	/// Woo
	String queuedBattle;
	String activeBattle;
    /// Active action
  //  List<Battler*> activeSubjects, activeTargets;
    List<BattleAction *> activeActions;
    /// Queue
    Queue<BattleAction*> actionQueue;
	List<Battler*> battlers;
	BattleState * battleState;
};

#endif
