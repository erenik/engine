// Emil Hedemalm
// 2013-07-20

#include "../GraphicsManager.h"
#include "GraphicsState.h"
#include "Graphics/Camera/Camera.h"

/// Sorts and renders the alpha-entities that were queued up earlier.
void GraphicsManager::RenderAlphaEntities(){
	graphicsState.settings |= RENDER_SORTED_ENTITIES;
//	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
//	SetShaderProgram("Flat");
	graphicsState.entitiesRequiringSorting.SortByDistance(graphicsState.camera->Position());
	// Render from back to front!
	for (int i = graphicsState.entitiesRequiringSorting.Size() - 1; i >= 0; --i){
		graphicsState.entitiesRequiringSorting[i]->Render();
	}
	/// Clear it here every frame, or elsewhere (at start of render frame perhaps?)
	graphicsState.entitiesRequiringSorting.Clear();
	graphicsState.settings &= ~RENDER_SORTED_ENTITIES;
}