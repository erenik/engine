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
}
	
/// Creates a particle system which will be attached to a specific entity.
Sparks::Sparks(EntitySharedPtr reference, bool emitWithEmittersOnly)
: ParticleSystem("Sparks", emitWithEmittersOnly)
{
	/// Set link to entity.
	relativeTo = reference;
}

Sparks::~Sparks()
{
    std::cout<<"\nSparks Destructor.....";
}

void Sparks::Initialize()
{
	ParticleSystem::Initialize();
	SetAlphaDecay(DecayType::CUBIC);
}


/// Update buffers to use when rendering.
void Sparks::UpdateBuffers()
{
	ParticleSystem::UpdateBuffers();
}


void Sparks::PrintData(){
}


