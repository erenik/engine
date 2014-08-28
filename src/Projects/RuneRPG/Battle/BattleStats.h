/// Emil Hedemalm
/// 2014-08-27
/// Battle-specific stats. See RuenBattler.h for their implementation equivalents.

#ifndef BATTLE_STATS_H
#define BATTLE_STATS_H

#include "String/AEString.h"

namespace Stat
{
	enum 
	{
		INVALID = -1,
		/// Player stats
		ATTACK_POWER, // Same thing there..
		DEFENSE_POWER, // Same thing as "Vitality" in some games.
		AGILITY,
		SPEED,
		MAX_HP,

		/// World-/Environment stats
		WORLD_TEMP,

		NUM_STATS,

	};	
};

extern String statNames[Stat::NUM_STATS];

/// Checks if the stat string is present within the target string.
int GetStatByString(String str);
String GetStatString(int byStat);

#endif