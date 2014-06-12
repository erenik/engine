// Emil Hedemalm
// 2013-07-09

#ifndef RUNE_BATTLER_MANAGER_H
#define RUNE_BATTLER_MANAGER_H

#include "RuneBattler.h"

#define RuneBattlers (*RuneBattlerManager::Instance())

class RuneBattlerManager {
	RuneBattlerManager();
	~RuneBattlerManager();
	static RuneBattlerManager * runeBattlerManager;
public:
	static void Allocate();
	static void Deallocate();
	static RuneBattlerManager * Instance();
	/// Looks for a "Battlers.list" which should then specify the battler-files to load.
	bool LoadFromDirectory(String dir);
	bool LoadBattles(String fromDirectory);
	Battle * LoadBattle(String source);
	Battle GetBattleBySource(String source);
	/// The default directory and file ending will be added automatically as needed. 
	RuneBattler GetBattlerBySource(String bySource);
	/// Deprecate or rename this function. God...
	RuneBattler GetBattlerType(String byName);
	/// Stuff!
	bool IsIdle();
private:
	List<RuneBattler*> battlerTypes;
	List<Battle*> battles;


};

#endif
