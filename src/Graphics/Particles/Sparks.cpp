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
	Initialize();
}
	
/// Creates a particle system which will be attached to a specific entity.
Sparks::Sparks(Entity * reference, bool emitWithEmittersOnly)
: ParticleSystem("Sparks", emitWithEmittersOnly)
{
   Initialize();
   /// Set link to entity.
   relativeTo = reference;
}

Sparks::~Sparks()
{
    std::cout<<"\nSparks Destructor.....";
}
	

void Sparks::Render(GraphicsState & graphicsState)
{
	if (!FetchTextures())
		return;
	if (useInstancedRendering && GL_VERSION_3_3_OR_HIGHER)
		RenderInstanced(graphicsState);
	else
		RenderOld(graphicsState);

}

void Sparks::PrintData(){
}


void Sparks::AttachTo(Entity * entity, Matrix4f relativePosition)
{
	assert(false);
}
