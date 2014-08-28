/// Emil Hedemalm
/// 2014-08-26
/// A single arbitrary effect relevant to a battle. Each action may inflict several effects.

#include "BattleEffect.h"
#include "RuneBattler.h"

BattleEffect::BattleEffect()
{
	applied = false;
}


/** Applies this effect to target battler. 
	This should eventually set the applied flag of this effect once all effects have 
	been successfully applied or if the spell fails for some reason.
	
	Long-time buffs/debuffs are copied onto the target.
*/
void BattleEffect::ApplyTo(RuneBattler * battler)
{
	switch(type)
	{
		case BattleEffect::DECREASE:
		case BattleEffect::INCREASE:
		{
			/// Add this effect to the target battler. Let them re-calculate their stats as needed. o.o
			battler->appliedEffects.Add(*this);
			break;
		}
		default:
			assert(false && "Implement");
	}
}


/// Adjusts the stat as the effect dictates. Which stat should be re-calculated using this is set in statType (see BattleStats.h for enum)
int BattleEffect::AdjustedStat(int baseStatValue)
{
	if (type == BattleEffect::INCREASE)
		return baseStatValue + constantAdjustment;
	else if (type == BattleEffect::DECREASE)
		return baseStatValue - constantAdjustment;
	assert(false);
	return -5;
}

/// Called each frame when the battler it is attached to is being processed. Returns false when it should be removed from the battler.
bool BattleEffect::Process(RBattleState & battleState)
{
	/// Starts at 0, increments when Process() is called.
	timeAttached += battleState.timeInMs;
	if (timeAttached > durationInMs)
		return false;

	return true;
}
