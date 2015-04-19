/// Emil Hedemalm
/// 2013-12-13
/// Animation set: Set of animations grouped together for a target model/creature/entity-thingy.

#include "AnimationSet.h"
#include "Animation.h"
#include "File/File.h"
#include "File/FileUtil.h"

#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

AnimationSet::AnimationSet()
{
};

AnimationSet::~AnimationSet(){
	animations.ClearAndDelete();
}


/// Expects a valid path to a folder including a base.png, subfolders for each animation and an AnimSet.txt file for specifying animation details
bool AnimationSet::Load(String fromFolder)
{
	sourceFolder = fromFolder;
	// Search for all directories within, create an animation for each.
	std::cout<<"\nSearching through directory: "<<fromFolder;
	List<String> foldersWithin;
	bool result = GetDirectoriesInDirectory(fromFolder, foldersWithin);
	if (!result){
		std::cout<<"\nNo folders found within:"<<fromFolder;
		return NULL;
	}
	/// Set name using the path?
	List<String> tokens = fromFolder.Tokenize("/");
	name = tokens[tokens.Size()-2] + "/" + tokens[tokens.Size()-1];
	// For each folder within, try to load an associated animation using its name!
	for (int i = 0; i < foldersWithin.Size(); ++i)
	{
		String fullPathToFolder = fromFolder + "/" + foldersWithin[i];
		Animation * anim = LoadAnimationFromFolder(fullPathToFolder);
		if (anim)
			animations.Add(anim);
	}
	baseFrame = fromFolder + "/Base.png";
	
	/// Look for a AnimSet.txt file.
	List<String> lines = File::GetLines(fromFolder + "/AnimSet.txt");
	Animation * animation = NULL;
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		if (line.StartsWith("//"))
			continue;
		line.SetComparisonMode(String::NOT_CASE_SENSITIVE);
		List<String> tokens = line.Tokenize(" \t");
		if (tokens.Size() < 2)
			continue;
		String key = tokens[0];
		String value = tokens[1];
		if (key == "AnimationSet")
			name = value;
		if (key == "animation")
		{
			animation = GetAnimation(value);
		}
		if (!animation)
			continue;
		else if (key == "loop" ||
			key == "repeat")
		{
			animation->repeatable = value.ParseBool();
		}
		else if (key == "duration" || key == "totalDurationMs")
		{
			animation->totalDurationMs = value.ParseInt();
		}
	}
	// Look for a "timing.txt" or similar file.
	// Set various details of the animations based on the info in the txt-file.
	std::cout<<"\nAnimation set loaded from folder: "<<fromFolder;
	return true;
}


Animation * AnimationSet::GetAnimation(String byName)
{
	for (int i = 0; i < animations.Size(); ++i){
		Animation * anim = animations[i];
	//	std::cout<<"\nAnimation "<<i<<" name: "<<anim->name;
		if (anim->name == byName){
			return anim;
		}
	}
	return NULL;
}

/// Returns NULL upon failure and a new Animation upon success.
Animation * AnimationSet::LoadAnimationFromFolder(String fromFolder)
{
	/// Fetch files in folder
	List<String> files;
	int result = GetFilesInDirectory(fromFolder, files);
	if (!files.Size()){
		std::cout<<"\nNo files in target directory: "<<fromFolder;
		return NULL;
	}
	// Create animation
	Animation * anim = new Animation();
	anim->repeatable = true; // Default repeatable?
	List<String> tokens = fromFolder.Tokenize("/");
	anim->name = tokens[tokens.Size()-1];
	List<String> unsortedSources;
	for (int i = 0; i < files.Size(); ++i)
	{
		String file = files[i];
		if (!file.Contains(".png"))
			continue;
		file = fromFolder + "/" + file;
		unsortedSources.Add(file);
	}
	// Sort by name?
	List<String> sortedSources;
	int leastIndex = -1;
	int leastValue = INT_MAX;
	while(unsortedSources.Size())
	{
		leastIndex = -1;
		leastValue = INT_MAX;
		for (int i = 0; i < unsortedSources.Size(); ++i)
		{
			String & source = unsortedSources[i];
		//	std::cout<<"\nSource found: "<<source;
			int value = source.ParseInt();
			if (value < leastValue)
			{
				leastValue = value;
				leastIndex = i;
			}
		}
		// Take the least index and add it to the sorted array
		if (leastIndex >= 0)
		{
			String source = unsortedSources[leastIndex];
			sortedSources.Add(source);
			unsortedSources.RemoveIndex(leastIndex);
		//	std::cout<<"\nSorted: "<<source;
		}
		else 
			break;
	}
	anim->frameSources = sortedSources;

	anim->frames = anim->frameSources.Size();
	// Calculate frame-times.
	anim->totalDurationMs = 1000;
	anim->frameDurations.Clear();
	for (int i = 0; i < anim->frames; ++i){
		anim->frameDurations.Add(anim->totalDurationMs / anim->frames);
	}
	return anim;
}