/// Emil Hedemalm
/// 2014-06-20
/// Library for managing all actions available for battlers in the RuneRPG game

#include "RuneBattleAction.h"

#define RBALib (*RuneBattleActionLibrary::Instance())

class RuneBattleActionLibrary 
{
	RuneBattleActionLibrary();
	~RuneBattleActionLibrary();
	static RuneBattleActionLibrary * runeBattleActionLibrary;
public:
	static void Allocate();
	static void Deallocate();
	static RuneBattleActionLibrary * Instance();
	/// Fetches battle action by source file.
	const RuneBattleAction * GetBattleAction(String bySource);
	/// Reloads all battle actions.
	void Reload();
private:
	const RuneBattleAction * LoadBattleAction(String bySource);
	/// List of the default templates used for all battle actions that all battlers may use.
	List<RuneBattleAction*> runeBattleActions;
};
