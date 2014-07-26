// Emil Hedemalm
// 2013-03-17

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "GraphicsMessages.h"
#include "GraphicsState.h"

GMSetLighting::GMSetLighting(Lighting * lighting) 
: GraphicsMessage(GM_SET_LIGHTING), lighting(lighting)
{
};

GMSetLighting::~GMSetLighting(){
}

void GMSetLighting::Process()
{
	GraphicsState * graphicsState = Graphics.graphicsState;
	graphicsState->lighting = lighting;	
}
