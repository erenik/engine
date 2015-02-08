/// Emil Hedemalm
/// 2014-06-26
/// Emits what may look like hot sparks, decaying over time into transparency.

#include "Sparks.h"
#include "Graphics/OpenGL.h"
#include "GraphicsState.h"
#include "Entity/Entity.h"
#include "Graphics/Camera/Camera.h"
#include "TextureManager.h"
#include "../GraphicsManager.h"
#include "../FrameStatistics.h"


/// Creates a global particle system
Sparks::Sparks(bool emitWithEmittersOnly)
: ParticleSystem("Sparks", emitWithEmittersOnly)
{
	InitSparks();
}
	
/// Creates a particle system which will be attached to a specific entity.
Sparks::Sparks(Entity * reference, bool emitWithEmittersOnly)
: ParticleSystem("Sparks", emitWithEmittersOnly)
{
	InitSparks();
	/// Set link to entity.
	relativeTo = reference;
}

Sparks::~Sparks()
{
    std::cout<<"\nSparks Destructor.....";
}

void Sparks::InitSparks()
{
	Initialize();
	SetAlphaDecay(DecayType::CUBIC);
}


/// Update buffers to use when rendering.
void Sparks::UpdateBuffers()
{
	ParticleSystem::UpdateBuffers();
	return;
	for (int i = 0; i < aliveParticles; ++i)
	{
		Vector3f & pos = positions[i];
		int index = i * 4;
		particlePositionSizeData[index] = pos[0];
		particlePositionSizeData[index+1] = pos[1];
		particlePositionSizeData[index+2] = pos[2];

		Vector4f & color = colors[i];
		particleColorData[index] = color[0] * 255;
		particleColorData[index+1] = color[1] * 255;
		particleColorData[index+2] = color[2] * 255;
		particleColorData[index+3] = ((1.f - lifeDurations[i] / lifeTimes[i])) * 255;
		i = i;

	}

}


void Sparks::PrintData(){
}


void Sparks::AttachTo(Entity * entity, Matrix4f relativePosition)
{
	assert(false);
}
