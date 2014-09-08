/// Emil Hedemalm
/// 2014-08-28
/// An animation, which controls rendered entities on the screen as a spell is being animated, or controls particles effects or w/e is needed.

#include "BattleAnimation.h"
#include "RuneBattleAction.h"
#include "RuneBattler.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMAnimate.h"

BattleAnimation::BattleAnimation()
{
	isOver = false;
	triggererAction = NULL;
}

/// Creates a simple Attack-animation to play for the caster/user of the skill.
BattleAnimation BattleAnimation::Attack()
{
	BattleAnimation attack;
	attack.spriteAnimationName = "Attack";
	attack.target = SELF;
	attack.type = SIMPLE_SPRITE_ANIMATION;
	return attack;
}


/// o.o
void BattleAnimation::Process(int timeInMs)
{
	/// Should not process no more?
	if (isOver)
		return;
	// Fetch targets?
	FetchTargetEntities();
	switch(type)
	{
		case SIMPLE_SPRITE_ANIMATION:
		{
			// Play it!
			for (int i = 0; i < targetEntities.Size(); ++i)
			{
				Entity * entity = targetEntities[i];
				Graphics.QueueMessage(new GMPlayAnimation(spriteAnimationName, entity));
			}
			break;
		}
	}
	isOver = true;
}


void BattleAnimation::FetchTargetEntities()
{
	switch(target)
	{
		case SELF:
		{
			/// Fetch action that triggered?
			assert(triggererAction && triggererAction->primarySubject);
			targetEntities.Add(triggererAction->primarySubject->entity);
			break;	
		}
		default:
			assert(false);
			break;
	}
}

