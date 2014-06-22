/// Emil Hedemalm
/// 2013-12-13
/// Manager for all sprite-based animations and animation-sets.

#include "AnimationManager.h"
#include "AnimationSet.h"
#include "Animation.h"
#include "File/FileUtil.h"

AnimationManager * AnimationManager::animationManager = NULL;

AnimationManager::AnimationManager(){}

AnimationManager::~AnimationManager(){
	animationSets.ClearAndDelete();
}
void AnimationManager::Allocate(){
	assert(animationManager == NULL);
	animationManager = new AnimationManager();
}
void AnimationManager::Deallocate(){
	assert(animationManager);
	delete animationManager;
	animationManager = NULL;
}
AnimationManager * AnimationManager::Instance(){
	assert(animationManager);
	return animationManager;
}

AnimationSet * AnimationManager::GetAnimationSet(String name)
{
	for (int i = 0; i < animationSets.Size(); ++i)
	{
		if (animationSets[i]->name == name)
			return animationSets[i];
	}
	// Try to load it.
	AnimationSet * anim = this->LoadAnimationSet(name);
	return anim;
}

/// Attempts to oetketretjet
bool AnimationManager::LoadFromDirectory(String dir)
{
	List<String> dirs;
	bool result = GetDirectoriesInDirectory(dir, dirs);
	if (!result){
		std::cout<<"\nERROR: AnimationManager::LoadFromDirectory: Unable to fetch folders in target dir: "<<dir;
		return false;
	}
	// Load animation set from each dir!
	for (int i = 0; i < dirs.Size(); ++i){
		String fullPathToDir = dir + "/" + dirs[i];
		AnimationSet * newSet = LoadAnimationSet(fullPathToDir);
		if (newSet)
			animationSets.Add(newSet);
	}
	return true;
}

/// Retursn NULL upon failure and a new AnimationSet upon success!
AnimationSet * AnimationManager::LoadAnimationSet(String fromFolder)
{

	AnimationSet * animSet = new AnimationSet();
	bool result = animSet->Load(fromFolder);
	if (!result)
	{
		delete animSet;
		return NULL;
	}
	this->animationSets.Add(animSet);	
	return animSet;
}

