/// Emil Hedemalm
/// 2014-03-01
/// Effects that can be applied to a battle action. 

#include "Effect.h"

void Effect::SetType(String string)
{
	if (string.Contains("damage"))
		type = EffectType::DAMAGE;
	else if (string.Contains("adjust stat"))
		type = EffectType::ADJUST_STAT;
	else if (string.Contains("adjust stat"))
		type = EffectType::ADJUST_POSITION;
	else if (string.Contains("adjust stat"))
		type = EffectType::ADJUST_ENVIRONMENT;
	else if (string.Contains("adjust stat"))
		type = EffectType::DRAIN_STAT;
	else if (string.Contains("adjust stat"))
		type = EffectType::ADD_STATUS_EFFECT;
	else if (string.Contains("adjust stat"))
		type = EffectType::REMOVE_STATUS_EFFECT;
	else if (string.Contains("adjust stat"))
		type = EffectType::SUMMON;
	else if (string.Contains("adjust stat"))
		type = EffectType::SHIELD;
	else if (string.Contains("adjust stat"))
		type = EffectType::DEATH;
	else if (string.Contains("adjust stat"))
		type = EffectType::SLOW;
	else 
		type = EffectType::NONE;
}
