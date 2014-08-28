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
	/// Fetches battle action by name or source file.
	const RuneBattleAction * GetBattleAction(String byNameOrSource);
	/// Fetches battle action by source file.
	const RuneBattleAction * GetBattleActionBySource(String source);
	/// Reloads all battle actions.
	void Reload();

	/// ///////////////////////////////
	/// Taken from RuneSpellManager
	/// ///////////////////////////////

	/// Returns list of all spells.
	List<RuneBattleAction*> GetSpells();

	/// Returns list of all non-spell, non-mundane skills.
	List<RuneBattleAction*> GetSkills();


	/// Creates a new spell, adding it to the list and returning it.
//	RuneSpell * New();

	/// Load spells from a CSV file (Comma-separated values).
	bool LoadSpellsFromCSV(String file);
	/// Sup.

private:
	const RuneBattleAction * LoadBattleAction(String bySource);
	
	/// List of all actions.
	List<RuneBattleAction*> runeBattleActions;
	/// Sublists with magic-focused Spells, Physical based magical skills and Mundane physical skills.
	List<RuneBattleAction*> spells, skills, mundaneActions;
};
