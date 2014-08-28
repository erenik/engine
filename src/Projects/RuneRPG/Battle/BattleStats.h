/// Emil Hedemalm
/// 2014-08-27
/// Battle-specific stats. See RuenBattler.h for their implementation equivalents.

#ifndef BATTLE_STATS_H
#define BATTLE_STATS_H

#include "String/AEString.h"


//	hp = maxHP = maxMP = mp = 10;
//	weaponDamage = spellDamage = 1;
//	magicArmor = magicPower = magicSkill = spellDamage = 0;
//	initiative = 1000;
	///// Default stats start always at 5.
	//attackPower = 
	//	defensePower =
	//	speed =
	//	magicSkill =
	//	magicPower = 5;



namespace Stat
{
	enum 
	{
		INVALID = -1,
		/// Player stats
		ATTACK_POWER, FIRST_CORE_STAT = ATTACK_POWER,
		DEFENSE_POWER, // Same thing as "Vitality" in some games.
		AGILITY,
		SPEED, 
		MAGIC_POWER,
		MAGIC_SKILL, LAST_CORE_STAT = MAGIC_SKILL,

		MAX_HP, FIRST_SURVIVAL_STAT = MAX_HP,
		CURRENT_HP,
		MAX_MP,
		CURRENT_MP, LAST_SURVIVAL_STAT = CURRENT_MP,

		BATTLE_GEAR_WEIGHT_LIMIT, // ...
		BATTLE_GEAR_WEIGHT, // From gear.

		// From current/active skill
		SPELL_POWER,
		SPELL_CAST_TIME,

		// From gear 
		WEAPON_DAMAGE,

		ARMOR_RATING, // Physical def form armor
		MAGIC_ARMOR, // Magical def from armor

		SHIELD_BLOCK_RATING,
		SHIELD_DEFENSE_MODIFIER,

		/// World-/Environment stats
		WORLD_TEMP,

		/// Total stats.
		NUM_STATS = WORLD_TEMP + 1,

		/// Total battler stats...
		NUM_BATTLER_STATS = MAGIC_ARMOR+1,

	};	
};

/// Checks if the stat string is present within the target string.
int GetStatByString(String str);
String GetStatString(int byStat);
/// The short-hand string used to refer to a stat within e.g. functions.
String GetStatShortString(int byStat);

#endif