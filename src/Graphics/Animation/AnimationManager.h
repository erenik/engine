/// Emil Hedemalm
/// 2013-12-13
/// Manager for all sprite-based animations and animation-sets.

#ifndef ANIMATION_MANAGER_H
#define ANIMATION_MANAGER_H

#include "String/AEString.h"
#include "List/List.h"

struct AnimationSet;
struct Animation;

/// Singleton o-o
#define AnimationMan (*AnimationManager::Instance())

class AnimationManager {
private:
	static AnimationManager * animationManager;
	AnimationManager();
	~AnimationManager();
public:
	static AnimationManager * Instance();
	static void Allocate();
	static void Deallocate();
	/// Attempts to oetketretjet
	bool LoadFromDirectory(String dir);
	/// Wooo.
	AnimationSet * GetAnimationSet(String byName);
private:
	/// Retursn NULL upon failure and a new AnimationSet upon success!
	AnimationSet * LoadAnimationSet(String fromFolder);
	/// Returns NULL upon failure and a new Animation upon success.
	Animation * LoadAnimationFromFolder(String fromFolder);
	String directory;
	/// Woo.
	List<AnimationSet*> animationSets;
};

#endif