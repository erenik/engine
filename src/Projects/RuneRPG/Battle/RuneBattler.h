// Emil Hedemalm
// 2013-07-09

#ifndef RUNE_BATTLER_H
#define RUNE_BATTLER_H

#include "Battle/Battler.h"

/// States for our battler.
enum states {
    BLEH,
    /// Default, most prominent state .. ish.
    WAITING_FOR_INITIATIVE,
    IDLE,
    ACTION_QUEUED,
	INCAPACITATED,
};

// To read in pre-defined battles from .txt or otherwise!
struct Battle {
	List<String> playerNames;
	List<String> enemyNames;
	String name;
	String source;
};

class RuneBattler : public Battler {
public:
	RuneBattler();
	/// from 0 to 4, 0 being player, 1-3 being enemies 1-3
	RuneBattler(int defaultTypes);
	virtual ~RuneBattler();
	virtual void Process(BattleState &battleState);
	virtual void OnActionFinished();
    /// Checks the initiative-parameter!
    virtual bool IsIdle();
	/// Sets MP/HP to max, etc.
	virtual void ResetStats();

	/// Take damage o-o
	bool Damage(int dmg);
	/// Physical damage! 
	bool PhysicalDamage(int dmg);
	/// Perform magic-damage-reduction before applying it.
	bool MagicDamage(int dmg);

	/// Bleh o-o
	bool LoadFromFile(String source);
	String Source() const { return source; };

	/// Name is inherited from Battler
    /// String name;

	// Some basic stats, maybe move them elsewhere? (Stat class/struct?)
	int hp, mp;
	float attack, agility, defense, magicPower, magicSkill, speed;
	int weaponDamage, spellDamage;
	int armor, magicArmor;
	int maxHP, maxMP;

	/// BAd name, yo.
	int delay;

    /** Initiative. When 0 or below the battler is idle and ready for action. Any value above that is waiting duration in milliseconds
    */
    int initiative;

	// Statuses
	bool unconscious;

private:
	String source;
	/// Statuses and stuff
	void Nullify();
};

#endif // RUNE_BATTLER_H
