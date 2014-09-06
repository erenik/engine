/// Emil Hedemalm
/// 2014-08-27
/// Battle-specific stats. See RuenBattler.h for their implementation equivalents.

#include "BattleStats.h"

/// Checks if the stat string is present within the target string.
int GetStatByString(String str)
{
	str.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (str.Contains("Max") && str.Contains("HP"))
		return RStat::MAX_HP;
	if (str.Contains("All"))
		return RStat::ALL_CORE_AND_SURVIVABILITY_STATS;

	for (int i = 0; i < RStat::NUM_STATS; ++i)
	{
		String statName = GetStatString(i);
		String shortStatName = GetStatShortString(i);
		if (str.Contains(statName))
			return i;
		if (str.Contains(shortStatName))
			return i;
	}

//	std::cout<<"\nUndefined stat string: "<<str;
//	assert(false && "Undefined stat string!");
	return -1;
}

String GetStatString(int byStat)
{	
	switch(byStat)
	{
		/// Gear-based stats
		
		/// Char stats
		case RStat::ATTACK_POWER: return "Attack power";
		case RStat::DEFENSE_POWER: return "Defense";
		case RStat::AGILITY: return "Agility";
		case RStat::SPEED: return "Speed";
		case RStat::MAX_HP: return "Max HP";
		case RStat::MAGIC_SKILL: return "Magic skill";
		case RStat::MAGIC_POWER: return "Magic power";

		// From gear.
		case RStat::WEAPON_MODIFIER: return "Weapon Modifier";
		case RStat::ARMOR_RATING: return "Armour Rating";
		case RStat::MAGIC_ARMOR_RATING: return "Magic Armour Rating";


		case RStat::BATTLER_TEMPERATURE: return "Battler Temperature";

		// Environment
		case RStat::WORLD_TEMP: return "World temperature";
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
		case RStat::ATTACK_POWER: return "AttackPower";
		case RStat::DEFENSE_POWER: return "DefensePower";
		case RStat::AGILITY: return "Agility";
		case RStat::SPEED: return "Speed";
		case RStat::MAX_HP: return "MaxHP";
		case RStat::CURRENT_HP: return "CurrentHP";
		case RStat::MAX_MP: return "MaxMP";
		case RStat::CURRENT_MP: return "CurrentMP";
		case RStat::MAGIC_POWER: return "MagicPower";
		case RStat::MAGIC_SKILL: return "MagicSkill";
		
		case RStat::BATTLE_GEAR_WEIGHT_LIMIT: return "BattleGearWeightLimit";
		case RStat::BATTLE_GEAR_WEIGHT: return "BattleGearWeight"; // From gear.

		// From skill.
		case RStat::SPELL_POWER: return "SpellPower";
		case RStat::SPELL_CAST_TIME: return "SpellCastTime";

		// From gear.
		case RStat::WEAPON_MODIFIER: return "WeaponModifier";
		case RStat::ARMOR_RATING: return "ArmorRating";
		case RStat::MAGIC_ARMOR_RATING: return "MagicArmorRating";

		case RStat::SHIELD_BLOCK_RATING: return "ShieldBlockRating";
		case RStat::SHIELD_DEFENSE_MODIFIER: return "ShieldDefenseModifier";

		case RStat::PARRY_RATING:	return "ParryRating";

		case RStat::BATTLER_TEMPERATURE: return "Targettemp";

		// Environment stats
		case RStat::WORLD_TEMP: return "WorldTemp";
		default:
			assert(false);
	}
	return String();
}
