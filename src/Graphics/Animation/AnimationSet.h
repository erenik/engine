/// Emil Hedemalm
/// 2013-12-13
/// Animation set: Set of animations grouped together for a target model/creature/entity-thingy.

#ifndef ANIMATION_SET_H
#define ANIMATION_SET_H

#include "String/AEString.h"
#include "List/List.h"

struct Animation;

struct AnimationSet {
	AnimationSet();
	~AnimationSet();
	/// Getter!
	Animation * GetAnimation(String byName);

	List<Animation*> animations;
	String name;
	String source;
	/// Path to base-frame to use when no animation has been queued yet.
	String baseFrame;
};

#endif