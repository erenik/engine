/// Emil Hedemalm
/// 2013-12-13
/// Animation set: Set of animations grouped together for a target model/creature/entity-thingy.

#include "AnimationSet.h"
#include "Animation.h"

AnimationSet::AnimationSet(){
	name = source = "";
	baseFrame = "";
};

AnimationSet::~AnimationSet(){
	animations.ClearAndDelete();
}

Animation * AnimationSet::GetAnimation(String byName){
	for (int i = 0; i < animations.Size(); ++i){
		Animation * anim = animations[i];
	//	std::cout<<"\nAnimation "<<i<<" name: "<<anim->name;
		if (anim->name == byName){
			return anim;
		}
	}
	return NULL;
}