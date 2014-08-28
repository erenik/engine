/// Emil Hedemalm
/// 2014-08-26
/// A single arbitrary effect relevant to a battle. Each action may inflict several effects.

#ifndef BATTLE_EFFECT_H
#define BATTLE_EFFECT_H

#include "String/AEString.h"

class RBattleState;
class RuneBattler;

class BattleEffect
{
public:
	BattleEffect();

	/** Applies this effect to target battler. 
		This should eventually set the applied flag of this effect once all effects have 
		been successfully applied or if the spell fails for some reason.
		
		Long-time buffs/debuffs are copied onto the target.
	*/
	virtual void ApplyTo(RuneBattler * targetBattler);
	/// Adjusts the stat as the effect dictates. Which stat should be re-calculated using this is set in statType (see BattleStats.h for enum)
	int AdjustedStat(int baseStatValue);

	/// Called each frame when the battler it is attached to is being processed. Returns false when it should be removed from the battler.
	bool Process(RBattleState & battleState);


	enum 
	{
		INCREASE, // Constant or relative adjustments
		DECREASE,
		ADD_DAMAGE, // E.g. Enfire, adds elemental damage on a per-attack basis.
		DAMAGE, // Damage, probably magical or pseudo/semi-magical.
	};
	/// Name of this effect. Name will correspond  to type.
	String name;
	/// See enum above. Type may affect which functions/equations are used and how the effect is calculated overall.
	int type;
	/// For effects affecting one specific character attribute, this denotes which one is to be affected.
	int statType;
	/// If the effect is associated with any specific element.
	int element;

	/// Name of the equation to use.
	String equation;
	/// Most effect have some arguments which are then passed to the equation.
	String argument;

	/// For increase/decrease stats, holds the value applied when calling AdjustedStat
	int constantAdjustment;
//	float relativeAdjustment;

	/// ..
	enum durations
	{
		// Positive values are milliseconds.
		INSTANTANEOUS = 0,
		PERMANENT = -1,
	};
	int durationInMs;
	/// Starts at 0, increments when Process() is called.
	int timeAttached;

	/// When the effect is to be applied by a spell, set to true once it has been successfully and fully been applied.
	bool applied;
};


#endif


