/// Emil Hedemalm
/// 2013-07-09
/// A battler in a the RuneRPG battles!

#ifndef RUNE_BATTLER_H
#define RUNE_BATTLER_H

#include "String/AEString.h"
#include "RuneBattle.h"
#include "BattleEffect.h"
#include "MathLib/Variable.h"
#include "RuneRPG/Item/RuneItem.h"

class Entity;
class RuneBattleAction;
class RuneBattleActionCategory;

class RuneBattler
{
	friend class RuneBattlerManager;
public:
	RuneBattler();
	/// from 0 to 4, 0 being player, 1-3 being enemies 1-3
	RuneBattler(int defaultTypes);
	virtual ~RuneBattler();

	/// Attempts to equip the provided gear.
	void Equip(List<RuneItem> gear);

	/// Adds the Attack, Item and Flee commands.
	void AddDefaultActions();

	/// Increments action points and other stuff according to current effects.
	/// For AI, it will also queue up actions accordingly to the state of the battle at large.
	void Process(RBattleState & battleState);

	/// Queues a new battle action to be executed when possible.
	void QueueAction(RuneBattleAction * rba);

	/// Creates the Entity to animate and visualize this battler while in action. Will add the entity to the active map straight away (using MapMan.CreateEntity())
	void CreateEntity();

	/// If dead, for example, it is not.
	virtual bool IsARelevantTarget();

	virtual void OnActionFinished();

    /// Returns true if no actions are queued, false if not.
    virtual bool IsIdle();
	
	/// Sets MP/HP to max, etc.
	virtual void ResetStats();

	/// Returns a list of variables containing each stat using their specified short-name, taken into consideration all effects that have been applied.
	List<Variable> GetCurrentStats();

	/// Take damage o-o
	bool Damage(int dmg);
	/// Physical damage! 
	bool PhysicalDamage(int dmg);
	/// Perform magic-damage-reduction before applying it.
	bool MagicDamage(int dmg);

	/// Updates which actions it has available using the RuneBattleAction library...?
	bool UpdateActions();
	/// Divides the actions into categories depending on the given scheme.
	void UpdateActionCategories(int usingSortingScheme);

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

	/// Returns the base-stat variable containing HP, so that it may be adjusted.
	Variable * GetHP();

	/// Returns current HP, including buffs/debuffs.
	int HP();
	int MaxHP();
	int MP();
	int MaxMP();
	///// Returns current Attack power, including buffs/debuffs.
	//int AttackPower();
	///// Returns current physical and magical defense power, including buffs/debuffs.
	//int DefensePower();
	///// Returns current offensive magical power, including buffs/debuffs applied to it.
	//int MagicPower();
	///// Returns current agility, including buffs/debuffs.
	//int Agility();
	///// Returns current speed, including buffs/debuffs.
	//int Speed();
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

	/// o.o
    String name;
	String source;

	/// Equipped gear. Assume a primary weapon.
	// Pointer to our weapon, present in allGear.
	RuneItem * weapon, * headPiece, * torsoPiece, * handsPiece, * feetPiece, * offHand;
	List<RuneItem> allGear;

	enum {
		IDLE,
		PREPARING_FOR_ACTION,
		CASTING,
		EXECUTING_ACTION,
		DEAD,
	};
	/// Current state. See enum above.
	int state;

	/// When initializing/loading, the battle actions known to the battler will be stored here. Assumes unique action names.
	List<String> actionNames;
	List<RuneBattleAction*> actions;
	/** For sorting the actions into categories. Exactly how the sorting is done will depend on current player settings.
		Sorting schemes:
		0 - No sorting. No categories.
	*/
	List<RuneBattleActionCategory*> actionCategories;

	/// List of currently queued actions.
	List<RuneBattleAction*> queuedActions;
	List<RuneBattleAction*> activeBattleActions;

	/// Current effects applied to this battler, mostly buffs/debuffs.
	List<BattleEffect> appliedEffects;


	/// True for all non-players.
	bool isAI;


	/// All stats related to a battler! o.o See BattleStats.h for names, short names and the enum defining the list.
	/// Base-stats define the permanent stats as gained by leveling up, etc, while the currentStats defines the stats as they currently look like.
	List<Variable> baseStats;
	/// Base-stats define the permanent stats as gained by leveling up, etc, while the currentStats defines the stats as they currently look like.
	List<Variable> currentStats;

	/// Updates all current stats. Should be called every time effects have been applied or any other change is done to the base-stats.
	void UpdateCurrentStats();


	// Some basic stats, maybe move them elsewhere? (Stat class/struct?)
//	int hp, mp;
//	int agility, magicPower, magicSkill, speed;
	/// Player stat, default 5. Max theoretical 405. Defines player's physical strength
//	int attackPower;
	/// Player stat, default 5. Max theoretical 405. Defines player's physical and magical defense (reduces incoming damage)
//	int defensePower;
	

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
	/** Points for spending on battle actions. 
		Max action points = 100 + Max(0, (Speed - 50) * 0.5)
	*/
	int maxActionPoints;

	/// o.o
	bool canBlock, canParry, canDodge;
	
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
	/// For mana gain over time.
	int64 timeSinceLastManaGain;

	/// Statuses and stuff
	void Nullify();
};

#endif // RUNE_BATTLER_H
