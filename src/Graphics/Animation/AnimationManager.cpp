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

AnimationSet * AnimationManager::GetAnimationSet(String name){
	for (int i = 0; i < animationSets.Size(); ++i){
		if (animationSets[i]->name == name)
			return animationSets[i];
	}
	return NULL;
}

/// Attempts to oetketretjet
bool AnimationManager::LoadFromDirectory(String dir){
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
AnimationSet * AnimationManager::LoadAnimationSet(String fromFolder){
	// Search for all directories within, create an animation for each.
	std::cout<<"\nSearching through directory: "<<fromFolder;
	List<String> foldersWithin;
	bool result = GetDirectoriesInDirectory(fromFolder, foldersWithin);
	if (!result){
		std::cout<<"\nNo folders found within:"<<fromFolder;
		return NULL;
	}
	/// Create set
	AnimationSet * animSet = new AnimationSet();
	/// Set name
	List<String> tokens = fromFolder.Tokenize("/");
	animSet->name = tokens[tokens.Size()-2] + "/" + tokens[tokens.Size()-1];
	for (int i = 0; i < foldersWithin.Size(); ++i){
		String fullPathToFolder = fromFolder + "/" + foldersWithin[i];
		Animation * anim = LoadAnimationFromFolder(fullPathToFolder);
		if (anim)
			animSet->animations.Add(anim);
	}
	animSet->baseFrame = fromFolder + "/base.png";
	
	// Look for a "timing.txt" or similar file.
	// Set various details of the animations based on the info in the txt-file.
	
	return animSet;
}

/// Returns NULL upon failure and a new Animation upon success.
Animation * AnimationManager::LoadAnimationFromFolder(String fromFolder){
	/// Fetch files in folder
	List<String> files;
	int result = GetFilesInDirectory(fromFolder, files);
	if (!files.Size()){
		std::cout<<"\nNo files in target directory: "<<fromFolder;
		return NULL;
	}
	// Create animation
	Animation * anim = new Animation();
	List<String> tokens = fromFolder.Tokenize("/");
	anim->name = tokens[tokens.Size()-1];
	for (int i = 0; i < files.Size(); ++i){
		String file = files[i];
		file = fromFolder + "/" + file;
		anim->frameSources.Add(file);
	}
	anim->frames = anim->frameSources.Size();
	// Calculate frame-times.
	anim->totalDuration = 1000;
	anim->frameDurations.Clear();
	for (int i = 0; i < anim->frames; ++i){
		anim->frameDurations.Add(anim->totalDuration / anim->frames);
	}
	return anim;
}