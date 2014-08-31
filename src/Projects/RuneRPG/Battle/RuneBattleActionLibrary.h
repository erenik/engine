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
	
	/// Creates Attack, Flee and.. Use Item?
	void CreateDefaultActions();

	/// ///////////////////////////////
	/// Taken from RuneSpellManager
	/// ///////////////////////////////
	/// Returns all spells, skills and mundane actions together.
	List<RuneBattleAction*> AllActions();
	/// Returns list of all spells.
	List<RuneBattleAction*> GetSpells();
	/// Returns list of all non-spell, non-mundane skills.
	List<RuneBattleAction*> GetSkills();


	/// Creates a new spell, adding it to the list and returning it.
//	RuneSpell * New();

	/// Load spells from a CSV file (Comma-separated values).
	bool LoadSpellsFromCSV(String file);
	/// Load skills from a CSV file (Comma-separated values).
	bool LoadSkillsFromCSV(String file);
	/// Loads mundane abilities, like Attack but also enemy actions.
	bool LoadMundaneAbilitiesFromCSV(String file);

private:
	/// Loads battle actions from target CSV file (actions, spell, mundane skills, w/e). Returns the list of actions loaded this way. 
	List<RuneBattleAction*> LoadBattleActionsFromCSV(String file);

	const RuneBattleAction * LoadBattleAction(String bySource);
	
	/// List of all actions. no.
//	List<RuneBattleAction*> runeBattleActions;

	/// Skills with all actions of various types.
	List<RuneBattleAction*> spells, skills, mundaneActions;
};
