/// Emil Hedemalm
/// 2013-12-13
/// Animation, thought for 2D so pretty much made to handle texture-changes only at pre-defined intervals.

#ifndef ANIMATION_H
#define ANIMATION_H

#include "String/AEString.h"
#include "List/List.h"

class Texture;

/// Animation, a series of textures that make up an animation, with times in between, flags for repeatability etc.
struct Animation {
public:
	/// Fetches texture for current frame!
	Texture * GetTexture(int animationTime);

	/// Source-paths for the target textures.
	List<String> frameSources;
	/// Texture-objects for the designated frames.
	List<Texture*> frameTextures;
	/// Duration of each frame specified in milliseconds.
	List<int> frameDurations;
	/// If it should repeat.
	bool repeatable;
	/// Pretty much the name of the folder the files reside in.
	String name;
	/// Easy access
	int frames;
	/// In Milliseconds, total of the all frame-durations.
	int totalDuration;
};

#endif
