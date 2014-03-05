/// Emil Hedemalm
/// 2013-10-10

#ifndef RUNEBATTLEACTION_H
#define RUNEBATTLEACTION_H

#include "Battle/BattleAction.h"

class RuneBattler;

class RuneBattleAction : public BattleAction
{
public:
    RuneBattleAction();
    RuneBattleAction(const BattleAction & ref);
    virtual ~RuneBattleAction();

    virtual void OnBegin(BattleState & battleState);
    /// Should return true once the action (including animation, sound etc.) has been finished.
    virtual bool Process(BattleState & battleState);
    virtual void OnEnd(BattleState & battleState);

    List<RuneBattler*> subjects, targets;

protected:
private:
    int startTime;
};

#endif // RUNEBATTLEACTION_H
