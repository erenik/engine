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

	/// Weapon damage, including buffs/debuffs. Will mostly vary within the 1.0 to 20.0 interval.
	float WeaponDamage();
	/// Average armor rating of all equipment pieces, including buffs/debuffs. Will mostly vary within the 1.0 to 20.0 interval.
	float ArmorRating();
	/// Average magic armor rating of all equipment pieces, including buffs/debuffs. Will mostly vary within the 1.0 to 20.0 interval.
	float MagicArmorRating();
	/// Shield block rating, including buffs/debuffs. Will mostly vary within the 0.0 to 20.0 interval.
	float ShieldBlockRating();
	/// Shield defense modifier, including buffs/debuffs. Will mostly vary within the 0.0 to 20.0 interval.
	float ShieldDefenseModifier();

	/// Returns current HP, including buffs/debuffs.
	int HP();
	/// Returns current Attack power, including buffs/debuffs.
	int AttackPower();
	/// Returns current physical and magical defense power, including buffs/debuffs.
	int DefensePower();
	/// Returns current offensive magical power, including buffs/debuffs applied to it.
	int MagicPower();
	/// Returns current agility, including buffs/debuffs.
	int Agility();
	/// Returns current speed, including buffs/debuffs.
	int Speed();
	/// Returns current amount of action points, floored.
	int ActionPoints();

	/// Returns true if a dodge occurred. The resulting damage is stored in the second parameter.
	bool Dodge(int preDodgeDamage, int & dodgeModifiedDamage);
	/// Returns true if a parry occurred. The resulting damage is stored in the second parameter.
	bool Parry(int preParryDamage, int & parryModifiedDamage);
	/// Returns true if a block occurred. The resulting damage is stored in the second parameter.
	bool ShieldBlock(int preBlockDamage, int & blockModifiedDamage);
	/// Returns true if a critical occurred. The resulting damage is stored in the second parameter.
	bool Critical(int preCriticalDamage, int & postCriticalDamage);



	/// When getting knocked out/incapacitated, cancels queued actions, etc.
	void OnKO();

	/// Bleh o-o
	bool Load(String fromFile);
	String Source() const { return source; };

	/// Name is inherited from Battler
    /// String name;

	// Some basic stats, maybe move them elsewhere? (Stat class/struct?)
	int hp, mp;

	int agility, magicPower, magicSkill, speed;

	/// Player stat, default 5. Max theoretical 405. Defines player's physical strength
	int attackPower;
	/// Player stat, default 5. Max theoretical 405. Defines player's physical and magical defense (reduces incoming damage)
	int defensePower;
	

	/// These require further investigation later on!
	float battleGearWeight;
	float battleWeightLimit;

	/** The amount with which the action bar is filled up for this character per millisecond. See base formulae below.

		Action bar fill-up speed
			= (100 + Speed^1.2) action points per 12 seconds.
	*/
	float actionPointsFillUpSpeed;
	/// Called if actionPointsFillUpSpeed is set to 0. If speed changes, set it to 0 and it will be automatically re-calculated upon next iteration.
	void RecalculateActionPointsFillUpSpeed();
	/** Points for spending on battle actions. 
		Max action points = 100 + Max(0, (Speed - 50) * 0.5)
	*/
	float actionPoints;
	
	int weaponDamage, spellDamage;
	int armor, magicArmor;
	int maxHP, maxMP;

	/// BAd name, yo.
	int delay;

    /** Initiative. When 0 or below the battler is idle and ready for action. Any value above that is waiting duration in milliseconds
    */
//    int initiative;

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
