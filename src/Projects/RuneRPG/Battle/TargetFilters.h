/// Emil Hedemalm
/// 2014-09-06
/// Dedicated file with targetting filters for actions.

#ifndef TARGET_FILTER_H
#define TARGET_FILTER_H

#include "String/AEString.h"

/// For target filtering
namespace TargetFilter 
{ 
	enum {
		NULL_TARGET,
		/// The caster
		SELF, 
		/// For selecting a battler target.
		ALLY,
		ENEMY,
		RANDOM,
		ALLIES,
		ENEMIES,
		ALL,
		/// For selecting a target based on the map
		POINT,
		TARGET_FILTERS,
	};
};

int ParseTargetFilter(String fromString);

#endif
