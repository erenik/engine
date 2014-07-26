// Emil Hedemalm
// 2013-07-18

#include "GMParticleMessages.h"
#include "GraphicsMessages.h"

GMGenerateParticles::GMGenerateParticles(String particleTypeName, void * extraData)
: GraphicsMessage(GM_GENERATE_PARTICLES), name(particleTypeName), data(extraData){
}

void GMGenerateParticles::Process(){
	name.SetComparisonMode(String::NOT_CASE_SENSITIVE);
	if (name == "CollisionSparks"){
	//	assert(false && "Do stuff!");
	}
	else {
		assert(false && "Undefined particle type name in GMGenerateParticles!");
	}
}

