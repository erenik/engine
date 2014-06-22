/// Emil Hedemalm
/// 2013-12-13
/// Animation set: Set of animations grouped together for a target model/creature/entity-thingy.

#ifndef ANIMATION_SET_H
#define ANIMATION_SET_H

#include "String/AEString.h"
#include "List/List.h"

struct Animation;

struct AnimationSet 
{
	AnimationSet();
	~AnimationSet();

	/// Expects a valid path to a folder including a base.png, subfolders for each animation and an AnimSet.txt file for specifying animation details
	bool Load(String fromFolder);

	/// Getter!
	Animation * GetAnimation(String byName);

	List<Animation*> animations;
	String name;
	String sourceFolder;
	/// Path to base-frame to use when no animation has been queued yet.
	String baseFrame;

private:
	/// Returns NULL upon failure and a new Animation upon success.
	Animation * LoadAnimationFromFolder(String folderPath);
};

#endif