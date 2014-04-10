/// Emil Hedemalm
/// 2014-04-10
/// Item-type identifiers for the RuneRPG project.

#ifndef RUNE_ITEM_TYPES_H
#define RUNE_ITEM_TYPES_H

namespace RuneItemType 
{
enum {
	CONSUMABLE, // Used in or outside of battle at any time pretty much.
	GEAR,		// Anything used for combat or general gameplay
	KEY_ITEM,	// Quest-specific items that usually store extra info.
};
};

#endif
