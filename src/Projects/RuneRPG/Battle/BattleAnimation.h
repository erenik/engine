/// Emil Hedemalm
/// 2014-08-28
/// An animation, which controls rendered entities on the screen as a spell is being animated, or controls particles effects or w/e is needed.

#ifndef BATTLE_ANIMATION
#define BATTLE_ANIMATION

#include "String/AEString.h"

class RuneBattleAction;
class Entity;

/// This also includes animation of the character! o.o .. or?
class BattleAnimation 
{
public:
	BattleAnimation();
	
	/// Creates a simple Attack-animation to play for the caster/user of the skill.
	static BattleAnimation Attack();

	/// o.o
	void Process(int timeInMs);


	/// Type of animation
	enum {
		SIMPLE_SPRITE_ANIMATION,
	};
	int type;

	/// Target o.o
	enum 
	{
		SELF, // Caster or user of the skill triggering this animation.
		PRIMARY_TARGET, // Get target from the action that initiated this animation.
	};
	int target;

	/// If the animation includes a sprite animation of some entity.
	String spriteAnimationName;

	/// When it is over o-o
	bool isOver;

	/// Action that triggered this animation.
	RuneBattleAction * triggererAction;

private:

	void FetchTargetEntities();
	List<Entity*> targetEntities;
};

#endif

