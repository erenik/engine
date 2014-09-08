/// Emil Hedemalm
/// 2014-08-26
/// A single arbitrary effect relevant to a battle. Each action may inflict several effects.

#ifndef BATTLE_EFFECT_H
#define BATTLE_EFFECT_H

#include "String/AEString.h"
#include "RuneRPG/RuneRPG.h"

class RBattleState;
class RuneBattler;



class BattleEffect
{
public:
	BattleEffect();

	/** Parses effects in the form of a string, where each effect ends with a parenthesis containing arguments of the effect's potency.
		E.g: Increase MaxMP Constant(50)
	*/
	static List<BattleEffect> ParseEffects(String fromString);
	/// Used for creating the constant stat adjustments applied by gear
	static BattleEffect ConstantStatIncrease(int statType, int increase);

	/** Applies this effect to target battler. 
		This should eventually set the applied flag of this effect once all effects have 
		been successfully applied or if the spell fails for some reason.
		
		Long-time buffs/debuffs are copied onto the target.
	*/
	virtual void ApplyTo(RuneBattler * targetBattler, RuneBattler * subject, RBattleState & bs);
	/// Adjusts the stat as the effect dictates. Which stat should be re-calculated using this is set in statType (see BattleStats.h for enum)
	int AdjustedStat(int baseStatValue);

	/// Called each frame when the battler it is attached to is being processed. Returns false when it should be removed from the battler.
	bool Process(RBattleState & battleState);


	enum types
	{
		BAD_TYPE = -1,
		INCREASE, // Constant or relative adjustments
		DECREASE,
		REMOVE_STAT_BUFFS_AND_DEBUFFS, // Removes all INCREASE and DECREASE type effects.
		ADD_DAMAGE, // E.g. Enfire, adds elemental damage on a per-attack basis.
		DAMAGE, // Damage, probably magical or pseudo/semi-magical.
		DEATH, // Argument contains chance or potency.
		RESTORE, // ..
		SPAWN_ELEMENTAL, // o.o;
		PAUSES_ACTIONBAR, // D:
		ATTEMPT_FLEE,
	};
	/// Name of this effect. Name will correspond  to type.
	String name;
	/// See enum above. Type may affect which functions/equations are used and how the effect is calculated overall.
	int type;
	/// For effects affecting one specific character attribute, this denotes which one is to be affected.
	int statType;
	/// If the effect is associated with any specific element.
	int element;

	/// Power of this effect, no matter what kind it be. This is generally parsed from the argument to this effect... lol waste 
//	int spellPower;

	/// Name of the equation to use.
	String equation;
	/// Most effect have some arguments which are then passed to the equation.
	String argument;

	/// For increase/decrease stats, holds the value applied when calling AdjustedStat
	int constantAdjustment;
//	float relativeAdjustment;

	/// If true, this is a buff or debuff that is attached to the target upon application.
	bool attachToTarget;
	/// If this is a "Spikes" type spell, is isa buff/debuff which applies some kind of state or damage to attackers on this battler.
	bool applyOnAttacker;

	/// If true, this effect will repeat itself X times before being considered fully applied.
	bool repeat;
	/// The number of iterations which this effect will repeat itself.
	int iterations;

	enum durationTypes
	{
		// Positive values are milliseconds.
		INSTANTANEOUS = 0,
		TIME_IN_MS,
		ATTACKS,
		PERMANENT,
	};
	/// o.o
	int durationType;
	int durationValue;
	/// Starts at 0, increments when Process() is called.
	int timeAttached;

	/// When the effect is to be applied by a spell, set to true once it has been successfully and fully been applied.
	bool applied;
};


#endif


