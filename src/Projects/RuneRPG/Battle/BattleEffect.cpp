/// Emil Hedemalm
/// 2014-08-26
/// A single arbitrary effect relevant to a battle. Each action may inflict several effects.

#include "BattleEffect.h"
#include "RuneBattler.h"
#include "BattleStats.h"

#include "Elements.h"
#include "MathLib/Function.h"

BattleEffect::BattleEffect()
{
	applied = false;
	durationType = BattleEffect::INSTANTANEOUS;
	attachToTarget = false;
	applyOnAttacker = false;
	repeat = false;
	constantAdjustment = 0;
}

/** Parses effects in the form of a string, where each effect ends with a parenthesis containing arguments of the effect's potency.
	E.g: Increase MaxMP Constant(50)
*/
List<BattleEffect> BattleEffect::ParseEffects(String fromString)
{
	List<BattleEffect> effects;
	List<String> tokens = fromString.Tokenize(" ");
	List<String> effectTokens = fromString.Tokenize("()");
	// Add them one at a time.
	for (int i = 0; i < effectTokens.Size(); i += 2)
	{
		String effectStr = effectTokens[i];
		String argument;
		if (effectTokens.Size() > i + 1)
		{
			argument = effectTokens[i+1];
		}
		effectStr.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		BattleEffect effect;
		effect.name = effectStr;
		effect.argument = argument;
		if (effectStr.Contains("Increase"))
		{
			effect.type = BattleEffect::INCREASE;
			effect.attachToTarget = true;
			// Parse amount from the argument while at it.
		}
		else if (effectStr.Contains("Decrease"))
		{
			effect.type = BattleEffect::DECREASE;
			effect.attachToTarget = true;
		}
		else if (effectStr.Contains("Add Damage"))
		{
			effect.type = BattleEffect::ADD_DAMAGE;
			/// Parse element here too then.
			effect.element = GetElementByString(effectStr);
			/// Check which equation type to use.
			if (effectStr.Contains(" P "))
				effect.equation = "Magic Skill damage";
			else if (effectStr.Contains(" M "))
				effect.equation = "Magic Spell damage";
			effect.attachToTarget = true;
		}
		else if (effectStr.Contains("Damage"))
		{
			effect.type = BattleEffect::DAMAGE;
			effect.element = GetElementByString(effectStr);
			/// Check which equation type to use.
			if (effectStr.Contains(" P "))
				effect.equation = "Magic Skill damage";
			else if (effectStr.Contains(" M "))
				effect.equation = "Magic Spell damage";
		}
		/// Special specifier to make the previous effect multiply itself X times?
		else if (effectStr.Contains("*"))
		{
			/// Mark the previous effect as repeating.
			BattleEffect & previousEffect = effects.Last();
			// Parse number of repeats.
			previousEffect.repeat = true;
			previousEffect.iterations = effectStr.ParseInt();
			assert(previousEffect.iterations > 0);
			continue;
		}
		/// Repetition of a previous effect. o.o
		else if (effectStr.Contains("&"))
		{
			// ..
			effect.type = effects.Last().type;
		}
		else if (effectStr.Contains("Instakill Chance"))
		{
			effect.type = BattleEffect::DEATH;
		}
		else if (effectStr.Contains("Restore"))
		{
			effect.type = BattleEffect::RESTORE;
		}
		else if (effectStr.Contains("Combo"))
			continue;
		else if (effectStr.Contains("Spawn Elemental"))
		{
			effect.type = BattleEffect::SPAWN_ELEMENTAL;
			effect.element = GetElementByString(effectStr);
		}
		else if (effectStr.Contains("Pause actionbar"))
		{
			effect.type = BattleEffect::PAUSES_ACTIONBAR;
		}
		else if (effectStr.Contains("Use Basic Attack"))
		{
			effect.type = BattleEffect::DAMAGE;
			effect.equation = "Basic Attack";
		}
		else if (effectStr.Contains("Attempt Flee"))
		{
			effect.type = BattleEffect::ATTEMPT_FLEE;
			effect.argument = argument;
		}
		else if (effectStr == "Opens spell tab" ||
			effectStr == "Opens item tab")
			continue;
		else if (effectStr == "N/A")
			continue;
		else if (effectStr.Length() < 3)
			continue;
		else 
		{
			std::cout<<"\nUnidentified effect string: "<<effectStr;
			/// Always add some shit. Add something ludicurous? lol.
			effect.type = BattleEffect::BAD_TYPE;
			assert(false && "Bad type");
		}
		if (effectStr.Contains("Return"))
		{
			effect.applyOnAttacker = true;
			effect.attachToTarget = true;
		}
		switch(effect.type)
		{
			case BattleEffect::DECREASE:
			case BattleEffect::INCREASE:
				{
					// Check next token.
					effect.statType = GetStatByString(effectStr);
					if (effect.statType == RStat::INVALID)
						continue;
					effect.argument = argument;
					/// This is actually a resetter, so change its type.
					if (argument.Contains("Default"))
					{
						effect.type = BattleEffect::REMOVE_STAT_BUFFS_AND_DEBUFFS;
						effect.attachToTarget = false;
					}
					effect.constantAdjustment = argument.ParseInt();
				}
				break;
		}
		// Just add the effect without parsing anything else.
		effects.Add(effect);
	}
	return effects;
}

/// Used for creating the constant stat adjustments applied by gear
BattleEffect BattleEffect::ConstantStatIncrease(int statType, int increaseAmount)
{
	BattleEffect increase;
	increase.type = BattleEffect::INCREASE;
	increase.statType = statType;
	increase.durationType = BattleEffect::PERMANENT;
	increase.constantAdjustment = increaseAmount; 
	return increase;
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
		target->UpdateCurrentStats();
		return;
	}
	switch(type)
	{
		case BattleEffect::REMOVE_STAT_BUFFS_AND_DEBUFFS:
		{
			int removedEffects = 0;
			for (int i = 0; i < target->appliedEffects.Size(); ++i)
			{
				BattleEffect * effect = &target->appliedEffects[i];
				switch(effect->type)
				{
					case BattleEffect::INCREASE:
					case BattleEffect::DECREASE:
						target->appliedEffects.RemoveIndex(i);
						--i;
						++removedEffects;
				}
			}
			bs.log = String(removedEffects) + " effects removed from "+target->name;
			break;	
		}
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
				if (var.name == GetStatShortString(RStat::SPELL_POWER))
				{
					var.iValue = RParseInt(argument);
					if (var.iValue == 0 && element != Element::PHYSICAL)
						std::cout<<"\nNote: Spell power 0. Disregard this message for regular attacks for the time being.";
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
					damage = 0;
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
//	assert(constantAdjustment != 0);
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
