/// Emil Hedemalm
/// 2014-08-27
/// Battle-specific stats. See RuenBattler.h for their implementation equivalents.

#include "BattleStats.h"

/// Checks if the stat string is present within the target string.
int GetStatByString(String str)
{
	str.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (str.Contains("Max") && str.Contains("HP"))
		return Stat::MAX_HP;

	for (int i = 0; i < Stat::NUM_STATS; ++i)
	{
		String statName = GetStatString(i);
		if (statName.Length() == 0)
			continue;
		if (str.Contains(statName))
			return i;
	}

	std::cout<<"\nUndefined stat string: "<<str;
	assert(false && "Undefined stat string!");
	return -1;
}

String GetStatString(int byStat)
{	
	switch(byStat)
	{
		/// Gear-based stats
		
		/// Char stats
		case Stat::ATTACK_POWER: return "Attack power";
		case Stat::DEFENSE_POWER: return "Defense";
		case Stat::AGILITY: return "Agility";
		case Stat::SPEED: return "Speed";
		case Stat::MAX_HP: return "Max HP";
		// Environment
		case Stat::WORLD_TEMP: return "Worldtemp";
	}
	return String();
}

/*
		INVALID = -1,
		/// Player stats
		ATTACK_POWER, // Same thing there..
		DEFENSE_POWER, // Same thing as "Vitality" in some games.
		AGILITY,
		SPEED,
		MAX_HP,
		CURRENT_HP,
		MAX_MP,
		CURRENT_MP,

		MAGIC_POWER,
		MAGIC_SKILL,

		// From gear or skill
		WEAPON_DAMAGE,
		SPELL_POWER,
		MAGIC_ARMOR,
*/

/// The short-hand string used to refer to a stat within e.g. functions.
String GetStatShortString(int byStat)
{
	switch(byStat)
	{
		case Stat::ATTACK_POWER: return "AttackPower";
		case Stat::DEFENSE_POWER: return "DefensePower";
		case Stat::AGILITY: return "Agility";
		case Stat::SPEED: return "Speed";
		case Stat::MAX_HP: return "MaxHP";
		case Stat::CURRENT_HP: return "CurrentHP";
		case Stat::MAX_MP: return "MaxMP";
		case Stat::CURRENT_MP: return "CurrentMP";
		case Stat::MAGIC_POWER: return "MagicPower";
		case Stat::MAGIC_SKILL: return "MagicSkill";

		case Stat::BATTLE_GEAR_WEIGHT_LIMIT: return "BattleGearWeightLimit";
		case Stat::BATTLE_GEAR_WEIGHT: return "BattleGearWeight"; // From gear.

		// From skill.
		case Stat::SPELL_POWER: return "SpellPower";
		case Stat::SPELL_CAST_TIME: return "SpellCastTime";

			// From gear.
		case Stat::WEAPON_DAMAGE: return "WeaponDamage";
		case Stat::ARMOR_RATING: return "ArmorRating";
		case Stat::MAGIC_ARMOR: return "MagicArmor";

		case Stat::SHIELD_BLOCK_RATING: "ShieldBlockRating";
		case Stat::SHIELD_DEFENSE_MODIFIER: "ShieldDefenseModifier";

		// Environment stats
		case Stat::WORLD_TEMP: return "WorldTemp";
		default:
			assert(false);
	}
	return String();
}
