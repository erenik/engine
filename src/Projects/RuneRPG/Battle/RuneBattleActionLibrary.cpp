/// Emil Hedemalm
/// 2014-06-20
/// Library for managing all actions available for battlers in the RuneRPG game

#include "RuneBattleActionLibrary.h"

RuneBattleActionLibrary * RuneBattleActionLibrary::runeBattleActionLibrary = NULL;

RuneBattleActionLibrary::RuneBattleActionLibrary()
{
}
RuneBattleActionLibrary::~RuneBattleActionLibrary()
{
	runeBattleActions.ClearAndDelete();
}
void RuneBattleActionLibrary::Allocate()
{
	assert(runeBattleActionLibrary == NULL);
	runeBattleActionLibrary = new RuneBattleActionLibrary();
}
void RuneBattleActionLibrary::Deallocate()
{
	assert(runeBattleActionLibrary);
	delete runeBattleActionLibrary;
	runeBattleActionLibrary = NULL;
}
RuneBattleActionLibrary * RuneBattleActionLibrary::Instance()
{
	assert(runeBattleActionLibrary);
	return runeBattleActionLibrary;
}
/// Fetches battle action by source file.
const RuneBattleAction * RuneBattleActionLibrary::GetBattleAction(String bySource)
{
#define ACTION_DIR "data/BattleActions/"
	if (!bySource.Contains(ACTION_DIR))
		bySource = ACTION_DIR + bySource;
	if (!bySource.Contains(".ba"))
		bySource = bySource + ".ba";
	for (int i = 0; i < runeBattleActions.Size(); ++i)
	{
		RuneBattleAction * rab = runeBattleActions[i];
		if (rab->source == bySource)
			return rab;
	}
	return LoadBattleAction(bySource);
}

/// Reloads all battle actions.
void RuneBattleActionLibrary::Reload()
{
	for (int i = 0; i < runeBattleActions.Size(); ++i)
	{
		RuneBattleAction * rab = runeBattleActions[i];
		rab->Load(rab->source);
	}
}



const RuneBattleAction * RuneBattleActionLibrary::LoadBattleAction(String bySource)
{
	RuneBattleAction * newRab = new RuneBattleAction();
	bool result = newRab->Load(bySource);
	if (!result)
	{
		delete newRab;
		return NULL;
	}
	runeBattleActions.Add(newRab);
	return newRab;
}