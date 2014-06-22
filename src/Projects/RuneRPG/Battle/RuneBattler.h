// Emil Hedemalm
// 2013-07-09

#ifndef RUNE_BATTLER_H
#define RUNE_BATTLER_H

#include "Battle/Battler.h"

class Entity;

class RuneBattler : public Battler 
{
	friend class RuneBattlerManager;
public:
	RuneBattler();
	/// from 0 to 4, 0 being player, 1-3 being enemies 1-3
	RuneBattler(int defaultTypes);
	virtual ~RuneBattler();

	/// If dead, for example, it is not.
	virtual bool IsARelevantTarget();

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

	/// Updates which actions it has available using the RuneBattleAction library...?
	bool UpdateActions();

	/// When getting knocked out/incapacitated, cancels queued actions, etc.
	void OnKO();

	/// Bleh o-o
	bool Load(String fromFile);
	String Source() const { return source; };

	/// Name is inherited from Battler
    /// String name;

	// Some basic stats, maybe move them elsewhere? (Stat class/struct?)
	int hp, mp;

	int attack, agility, defense, magicPower, magicSkill, speed;
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

	// Used in the game engine.
	Entity * entity;

	bool isEnemy;

	/// Animation set to use.
	String animationSet;

private:
	String source;
	/// Statuses and stuff
	void Nullify();
};

#endif // RUNE_BATTLER_H
