// Emil Hedemalm
// 2013-07-09

// An RPG-based class/struct
#include "Battler.h"
#include "BattleManager.h"
#include "BattleState.h"
#include "BattleAction.h"

// Singleton stuff
BattleManager * BattleManager::battleManager = NULL;

BattleManager::BattleManager()
{
	battleState = NULL;
}
BattleManager::~BattleManager(){

}
BattleManager * BattleManager::Instance(){
	assert(battleManager);
	return battleManager;
}
void BattleManager::Allocate(){
	assert(battleManager == NULL);
	battleManager = new BattleManager();
}
void BattleManager::Deallocate(){
	assert(battleManager);
	delete battleManager;
	battleManager = NULL;
}
bool BattleManager::IsAllocated(){
	return battleManager? true : false;
}

/// Clears active battlers and prepares loading in new ones, using previously set default values.
void BattleManager::NewBattle()
{
    battlers.ClearAndDelete();
	if (battleState)
		delete battleState;
    /// Clear the action queue of any old stuff.
    while(!actionQueue.isOff()){
        BattleAction * ba = actionQueue.Pop();
        delete ba;
    }
    activeActions.ClearAndDelete();
	/// Refresh the battle-state.
	battleState = new BattleState();
	// By default, add two sides, the players and enemies.
	battleState->sides.Add(new Side("Players"));
	battleState->sides.Add(new Side("Enemies"));
}

/// Adds a battler to the battle! NULL = random-generated test-battler.
void BattleManager::AddBattler(Battler * battler /* = NULL*/, bool playerControlled /*= false*/)
{
	assert(battleState->state != BATTLE_ENDED);
	if (battler == NULL){
		std::cout<<"\nERROR: No valid battler supplied!";
		return;
	}
#ifdef ASSERT_ON_NO_ACTIONS
	assert(battler->actions.Size() && "Battler has no actions at all. WTH?!");
    assert(battler->actionCategories.Size() && "Battler has no action categories.. fix this, yo?");
#endif

	// Add to the global array
	battlers.Add(battler);
	/// Add to all arrays needed in the battle-state
	battleState->battlers.Add(battler);
	if (playerControlled){
		battler->side = 0;
		battler->isAI = false;
		battleState->players.Add(battler);
		battleState->sides[0]->battlers.Add(battler);
	}
	else {
		battler->side = 1;
		battler->isAI = true;
		battleState->npcs.Add(battler);
		battleState->sides[1]->battlers.Add(battler);
	}

}
/// Fixes stuff before processing may begin, like placing them on the "battlefield", et al.
void BattleManager::Setup(){

}

void BattleManager::QueueAction(BattleAction * ba)
{
    actionQueue.Push(ba);
}

Battler * BattleManager::GetBattlerByName(String byName) const
{
    for (int i = 0; i < battlers.Size(); ++i){
        Battler * batt = battlers[i];
        if (batt->name == byName)
			return batt;
    }
    return NULL;
}

List<Battler*> BattleManager::GetPlayers() const
{
    List<Battler*> players;
    for (int i = 0; i < battlers.Size(); ++i){
        Battler * batt = battlers[i];
        if (!batt->isAI)
            players.Add(batt);
    }
    return players;
}

/// Returns all idle player-battlers! Checked via the Battler::IsIdle()-function!
List<Battler*> BattleManager::GetIdlePlayerBattlers() const 
{
    List<Battler*> idles;
    for (int i = 0; i < battlers.Size(); ++i){
        Battler * batt = battlers[i];
        if (!batt->isAI && batt->IsIdle())
            idles.Add(batt);
    }
    return idles;
}

/// See 
Battler * BattleManager::RandomTarget(int filter, Battler * subject)
{
	List<Battler*> relevantBattlers;
	for (int i = 0; i < battlers.Size(); ++i){
		Battler * b = battlers[i];
		///
		if (!b->IsARelevantTarget())
			continue;
		switch(filter)
		{
			case TargetFilter::ENEMY:
				if (subject->side == b->side)
					continue;
				relevantBattlers.Add(b);
				break;
		}
	}
	if (!relevantBattlers.Size())
		return NULL;
	return relevantBattlers[rand()%relevantBattlers.Size()];
}

/// TURN_BASED or REAL_TIME
void BattleManager::SetBattleTimeType(int type){

}
/// Milliseconds or turns, depending on type
void BattleManager::Process(int timeInMs)
{
	if (battleState->state == BATTLE_ENDED)
		return;

    /// First process active battle actions, if any.
	for (int i = 0; i < activeActions.Size(); ++i)
	{
		BattleAction * activeAction = activeActions[i];
		bool done = false;
		// If canceled, don't process it no more.
		if (activeAction->state == BattleAction::CANCELED)
		{
			done = true;
		}
		else
			done = activeAction->Process(*battleState);
        if (done)
		{
			// Remove it from the list.
			activeActions.Remove(activeAction);
			--i;
			// Call onEnd
            activeAction->OnEnd(*battleState);
			// and deallocate it as needed
			if (activeAction->deleteOnEnd)
				delete activeAction;
            activeAction = NULL;
        }
		/// Return if the action pauses battle (beside itself) and isn't over yet.
        else if (activeAction->PausesBattleProcessing())
            return;
    }

    /// Clear the action
    if(!actionQueue.isOff())
	{
        BattleAction * ba = actionQueue.Pop();
        activeActions.Add(ba);
        ba->OnBegin(*battleState);
    }

    /// Process battlers in order to queue up more actions!
	battleState->timeDiff = timeInMs;
	for (int i = 0; i < battlers.Size(); ++i)
	{
		Battler * b = battlers[i];
		b->Process(*battleState);
		if (battleState->state == BATTLE_ENDED){
			std::cout<<"\nBattle is over.";
			return;
		}
	}

}

/// Queues it up!
void BattleManager::QueueBattle(String battle){
	assert(queuedBattle == String());
	queuedBattle = battle;
}
	
/// Returns the queued battle-source/name, if it exists.
String BattleManager::QueuedBattle(){
	return queuedBattle;
}

/// Starts up the queued battle, moving it to be the active one and empties the queued string.
void BattleManager::StartBattle(){
	assert(queuedBattle.Length());
	activeBattle = queuedBattle;
	queuedBattle = String();
}


/// Changes state to ended.
void BattleManager::EndBattle(){
	battleState->state = BATTLE_ENDED;
	activeBattle = String();
}