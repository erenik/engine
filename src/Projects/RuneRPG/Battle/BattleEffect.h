/// Emil Hedemalm
/// 2014-08-26
/// A single arbitrary effect relevant to a battle. Each action may inflict several effects.

#ifndef BATTLE_EFFECT_H
#define BATTLE_EFFECT_H

#include "String/AEString.h"

class BattleEffect
{
public:
	enum {
		INCREASE, // Constant or relative adjustments
		DECREASE,
		ADD_DAMAGE, // E.g. Enfire, adds elemental damage on a per-attack basis.
		DAMAGE, // Damage, probably magical or pseudo/semi-magical.
	};
	/// Name of this effect. Name will correspond  to type.
	String name;
	/// See enum above. Type may affect which functions/equations are used and how the effect is calculated overall.
	int type;
	/// For effects affecting one specific character attribute, this denotes which one is to be affected.
	int statType;
	/// If the effect is associated with any specific element.
	int element;

	/// Name of the equation to use.
	String equation;
	/// Most effect have some arguments which are then passed to the equation.
	String argument;

	/// Because MS is cool.
	enum durations
	{
		// Positive values are milliseconds.
		INSTANTANEOUS = 0,
		PERMANENT = -1,
	};
	int durationInMs;
};


#endif


