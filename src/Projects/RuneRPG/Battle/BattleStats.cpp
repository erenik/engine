/// Emil Hedemalm
/// 2014-08-27
/// Battle-specific stats. See RuenBattler.h for their implementation equivalents.

#include "BattleStats.h"

String statNames[Stat::NUM_STATS];

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
