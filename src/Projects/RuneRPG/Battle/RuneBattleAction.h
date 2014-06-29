/// Emil Hedemalm
/// 2013-10-10

#ifndef RUNEBATTLEACTION_H
#define RUNEBATTLEACTION_H

#include "System/DataTypes.h"
#include "Battle/BattleAction.h"

class RuneBattler;

class RuneBattleAction : public BattleAction
{
public:
    RuneBattleAction();
    RuneBattleAction(const BattleAction & ref);

	/// Will depend on the filter.
	bool HasValidTargets();
    /// Sets relevant vars and pointers to 0/NULL upon creation.
	void Nullify();
	virtual ~RuneBattleAction();

    virtual void OnBegin(BattleState & battleState);
    /// Should return true once the action (including animation, sound etc.) has been finished.
    virtual bool Process(BattleState & battleState);
    virtual void OnEnd(BattleState & battleState);

	/// Battler targets for this specific instance of the action.
    List<RuneBattler*> subjects, targets;

	/// Based on the one in BattleAction?
    virtual bool Load(String fromFile);

protected:

	void EvaluateLine(String line);
	void PhysicalDamage(String line);
	void MagicDamage(String line);
	void Damage(String line);

private:
	/// Send to battle narrator.
	void Narrate(String line);

	/// From 1 to 100. Used for all magical effects.
	int spellPower;

	/// Set during processing.
	String narr;
	RuneBattler * primarySubject;
	RuneBattler * primaryTarget;
	bool died;
	// Start time of the action?
    int64 startTime;
};

#endif // RUNEBATTLEACTION_H
