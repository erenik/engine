/// Emil Hedemalm
/// 2013-12-13
/// Animation...!

#include "Animation.h"
#include "TextureManager.h"

/// Fetches texture for current frame!
Texture * Animation::GetTexture(int animationTime){
	int index = 0;
	while (animationTime > 0){
		animationTime -= frameDurations[index];
		++index;
		if (index >= frames)
			index = 0;
	}
	return TexMan.GetTextureBySource(frameSources[index]);
}
