// Emil Hedemalm
// 2013-07-20

#include "../GraphicsManager.h"
#include "GraphicsState.h"
#include "Graphics/Camera/Camera.h"

/// Sorts and renders the alpha-entities that were queued up earlier.
void GraphicsManager::RenderAlphaEntities()
{
	GraphicsThreadGraphicsState.settings |= RENDER_SORTED_ENTITIES;
//	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//	ShadeMan.SetActiveShader("Flat");
	GraphicsThreadGraphicsState.entitiesRequiringSorting.SortByDistance(GraphicsThreadGraphicsState.camera->Position());
	// Render from back to front!
	for (int i = GraphicsThreadGraphicsState.entitiesRequiringSorting.Size() - 1; i >= 0; --i){
		GraphicsThreadGraphicsState.entitiesRequiringSorting[i]->Render(graphicsState);
	}
	/// Clear it here every frame, or elsewhere (at start of render frame perhaps?)
	GraphicsThreadGraphicsState.entitiesRequiringSorting.Clear();
	GraphicsThreadGraphicsState.settings &= ~RENDER_SORTED_ENTITIES;
}