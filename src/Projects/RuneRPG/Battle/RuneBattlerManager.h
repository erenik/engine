// Emil Hedemalm
// 2013-07-09

#ifndef RUNE_BATTLER_MANAGER_H
#define RUNE_BATTLER_MANAGER_H

#include "RuneBattler.h"
#include "RuneBattle.h"

#define RuneBattlers (*RuneBattlerManager::Instance())

class RuneBattlerManager 
{
	RuneBattlerManager();
	~RuneBattlerManager();
	static RuneBattlerManager * runeBattlerManager;
public:
	static void Allocate();
	static void Deallocate();
	static RuneBattlerManager * Instance();

	/// Reloads all battlers by their respective source file.
	void ReloadBattles();
	/// Reloads all battlers by their respective source file.
	void ReloadBattlers();

	List<RuneBattler*> GetBattlers(){return battlers;};

	/// Looks for a "Battlers.list" which should then specify the battler-files to load.
	bool LoadFromDirectory(String dir);
	bool LoadBattles(String fromDirectory);
	/// Loads new battler type from file.
	const RuneBattler * LoadBattler(String fromSource);
	RuneBattle * LoadBattle(String source);

	RuneBattle GetBattleBySource(String source);
	/// Gets battler by source. You must then create a copy of the const reference yourself before using it. 
	const RuneBattler * GetBattlerBySource(String bySource);
	/// Deprecate or rename this function. God...
	RuneBattler GetBattlerType(String byName);
	/// Stuff!
	bool IsIdle();

	static String rootBattlerDir;
	
	/// Loads battlers from target CSV file.
	bool LoadBattlersFromCSV(String file);
private:


	List<RuneBattler*> battlers;
	List<RuneBattle*> runeBattles;


};

#endif
