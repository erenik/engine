// Emil Hedemalm
// 2013-03-17

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "GraphicsMessages.h"
#include "GraphicsState.h"

/// Sets copy of the given lighting setup.
GMSetLighting::GMSetLighting(Lighting & lighting)
	: GraphicsMessage(GM_SET_LIGHTING), lighting(lighting), lightingPtr(NULL)
{
	
}

/// Sets copy of the given lighting setup. Should be removed since pointers imply setting a newly allocated object.
GMSetLighting::GMSetLighting(Lighting * lightingPtr) 
: GraphicsMessage(GM_SET_LIGHTING), lightingPtr(lightingPtr)
{

};

GMSetLighting::~GMSetLighting(){
}

void GMSetLighting::Process()
{
	if (lightingPtr)
		GraphicsMan.lighting = *lightingPtr;
	else 
		GraphicsMan.lighting = lighting;
}
