/// Emil Hedemalm
/// 2014-02-28
/// Spell class that encompasses elements, effects and effect durations. 

#ifndef RUNE_SPELL_H
#define RUNE_SPELL_H

#include "RuneRPG/Battle/RuneBattleAction.h"

namespace Element {
	enum elements{
		FIRE, 
		WATER, 
		EARTH,
		AIR,
		LIFE,
		DEATH,
		CHAOS,
		BALANCE
	};
};

class RuneSpell : public RuneBattleAction {
public:
	RuneSpell();
	virtual ~RuneSpell();

	void SetElements(String toParse);

	/// Which elements are associated with this spell.
	List<int> elements;
	

};

#define RuneSpellMan (*RuneSpellManager::Instance())

/// We're gonna have many, so better have a manager to handle them all.
class RuneSpellManager {
	RuneSpellManager();
	~RuneSpellManager();
	static RuneSpellManager * runeSpellManager;
public:
	static RuneSpellManager * Instance();
	static void Allocate();
	static void Deallocate();

	/// Returns list of all spells, as battle-action objects. The object will be created as usual via the BattleActionLibrary.
	List<BattleAction*> GetSpellsAsBattleActions();

	/// Creates a new spell, adding it to the list and returning it.
	RuneSpell * New();
	/// Load from a CSV file (Comma-separated values).
	bool LoadFromCSV(String file);
	/// Sup.
	List<RuneSpell*> spells;
};


#endif

