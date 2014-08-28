/// Emil Hedemalm
/// 2014-08-26
/// A single arbitrary effect relevant to a battle. Each action may inflict several effects.

#include "BattleEffect.h"
#include "RuneBattler.h"
#include "BattleStats.h"

#include "MathLib/Function.h"

BattleEffect::BattleEffect()
{
	applied = false;
	durationType = BattleEffect::INSTANTANEOUS;
	attachToTarget = false;
	applyOnAttacker = false;
	repeat = false;
}


/** Applies this effect to target battler. 
	This should eventually set the applied flag of this effect once all effects have 
	been successfully applied or if the spell fails for some reason.
	
	Long-time buffs/debuffs are copied onto the target.
*/
void BattleEffect::ApplyTo(RuneBattler * target, RuneBattler * subject, RBattleState & bs)
{
	if (attachToTarget)
	{
		/// Add this effect to the target battler. Let them re-calculate their stats as needed. o.o
		target->appliedEffects.Add(*this);
		return;
	}
	switch(type)
	{
		case BattleEffect::DAMAGE:
		{
			// Check equation.
			Function func = Function::GetFunctionByName(this->equation);
			/// Grab all variables from the battlers!
			List<Variable> targetStats = target->GetCurrentStats(), 
				casterStats = subject->GetCurrentStats();

			/// Add variables which are relevant to this spell to the variable list.
			for (int i = 0; i < casterStats.Size(); ++i)
			{
				Variable & var = casterStats[i];
				if (var.name == GetStatShortString(Stat::SPELL_POWER))
				{
					var.iValue = RParseInt(argument);
					assert(var.iValue > 0);
				}
			}

			PrependVariables(targetStats, "Target");
			PrependVariables(casterStats, "Subject");
			
			int damage;
			// Calculate damage!
			ExpressionResult result = func.Evaluate(targetStats + casterStats);
			switch(result.type)
			{
				case DataType::FLOAT:
				{
					damage = result.fResult;
					break;
				}
				case DataType::INTEGER:
				{
					damage = result.iResult;	
					break;
				}
				case DataType::NO_TYPE:
				{
					std::cout<<"\nFunction evaluation error: "<<result.text;
					break;
				}
				default:
					assert(false);
			}
			/// Apply the damage to the target and narrate it.
			target->Damage(damage);		
			bs.log = target->name+" takes "+String(damage)+" points of damage.";
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
	if (durationType == TIME_IN_MS && timeAttached > durationValue)
		return false;
	else if (durationType == INSTANTANEOUS)
		return false;

	return true;
}
