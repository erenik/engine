/// Emil Hedemalm
/// 2014-03-01
/// Effects that can be applied to a battle action. 

#include "String/AEString.h"

namespace EffectType {
	enum types{
		NONE,
		DAMAGE,				/// For specific equations, various types of damage, such as physical or magical, etc.
		ADJUST_STAT,		/// This includes HP, MP, static and percentages, etc.
		ADJUST_POSITION,	/// Displacing battlers on the battlefield
		ADJUST_ENVIRONMENT,	/// Or the entire battlefield.
		DRAIN_STAT,			/// Take and receive.
		ADD_STATUS_EFFECT,	/// Boolean status effects.
		REMOVE_STATUS_EFFECT,
		SUMMON,				/// Summon! :D
		SHIELD,				/// To dodge, abosrb or otherwise change effects received.
		DEATH,				/// Instant heffects!
		SLOW,				/// Adjusting initiative gain.
};};

namespace EffectDuration {
	enum spellDurations {
		INSTANTANEOUS,
		PERMANENT,
	};
};

class Effect {
public:
	/// Types.
	void SetType(String fromString);
	int type;
	/// Name will probably coincide with the effect,... if it is to be used at all
	String name;
	/// General indication of what kind of duration it has.
	int duration;
	/// Extra variables should be subclassed.
};

