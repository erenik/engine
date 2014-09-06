// Emil Hedemalm
// 2013-07-19

#include "Graphics/GraphicsManager.h"
#include "Graphics/Camera/Camera.h"
#include "GraphicsState.h"

/// Actively manipulated entities
//extern Entities editorSelection;

// Renders the selected(active) entities in the editor/debugg-mode
void GraphicsManager::RenderSelection(){
	/// woshi o-o;
	if (!selectionToRender)
		return;

	// Clear errors upon entering.
	GLuint error = glGetError();

	// Make sure the device is set as appropriate output
	// TODO...

	// Disable depth-test so that all selections are rendered to screen no matter how we've done stuff earlier.
	glEnable(GL_DEPTH_TEST);

//	return;
	// Set default shading program
	PrintGLError("Bleh");
	ShadeMan.SetActiveShader("Wireframe");


	/// Set rainbow factor for XYZ ^w^
	glUniform1f(glGetUniformLocation(graphicsState->activeShader->shaderProgram, "rainbowXYZFactor"), 0.0f);
	
	Camera * camera = graphicsState->camera;

	// Update view and projection matrix in specified shader
	if (graphicsState->activeShader && graphicsState->activeShader->uniformProjectionMatrix != -1)
		glUniformMatrix4fv(graphicsState->activeShader->uniformProjectionMatrix, 1, false, graphicsState->projectionMatrixF.getPointer());
	// Update view and projection matrix in specified shader
	if (graphicsState->activeShader && graphicsState->activeShader->uniformViewMatrix != -1)
		glUniformMatrix4fv(graphicsState->activeShader->uniformViewMatrix, 1, false, graphicsState->viewMatrixF.getPointer());
	// Update camera in the world
	if (graphicsState->activeShader && graphicsState->activeShader->uniformEyePosition != -1)
		glUniform4f(graphicsState->activeShader->uniformEyePosition, camera->Position().x, camera->Position().y, camera->Position().z, 1.0);


	PrintGLError("Unable to set wireframe shader");
	// Set color of the wireframes of all selected objects
	glUniform4f(graphicsState->activeShader->uniformPrimaryColorVec4, 1.0f, 1.0f, 0.0f, 0.7f);
	PrintGLError("Unable to set primary color");
	// Set to wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(3.f);
	Entity ** activeEntity = selectionToRender->GetArray();
	int amount = selectionToRender->Size();
	graphicsState->settings &= ~ENABLE_SPECIFIC_ENTITY_OPTIONS;
	// Render all entities in the selection, yo
	for (int i = 0; i < amount; ++i){
		activeEntity[i]->Render(graphicsState);
	}
	// Render the vfcOctree again.
	//if (vfcOctree)
	//	vfcOctree->Render();
	// Reset to fill mode
//	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
}