// Emil Hedemalm
// 2013-03-17

#include "GraphicsMessage.h"
#include "../GraphicsManager.h"
#include "GraphicsMessages.h"
#include "GraphicsState.h"

GMSetLighting::GMSetLighting(Lighting i_lighting) : GraphicsMessage(GM_SET_LIGHTING) {
	i_lighting.VerifyData();
	lighting = i_lighting;
	lighting.VerifyData();
};

GMSetLighting::~GMSetLighting(){
}

void GMSetLighting::Process(){
	lighting.VerifyData();
	if (Graphics.lighting)
		delete Graphics.lighting;
	Graphics.lighting = new Lighting(lighting);
	
	/// Add static lights
	for (int i = 0; i < Graphics.staticLights.Size(); ++i){
		Graphics.lighting->Add(*Graphics.staticLights[i]);
	}

	/// Assign a new lighting to the graphics state
	if (Graphics.graphicsState->lighting == NULL)
		Graphics.graphicsState->lighting = new Lighting();
	*Graphics.graphicsState->lighting = Lighting(*Graphics.lighting);

//	Graphics.graphicsState->lighting = new Lighting(lighting);
}
