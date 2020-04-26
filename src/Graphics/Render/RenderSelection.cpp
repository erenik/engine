// Emil Hedemalm
// 2013-07-19

#include "Graphics/GraphicsManager.h"
#include "Graphics/Camera/Camera.h"
#include "GraphicsState.h"

/// Actively manipulated entities
//extern Entities editorSelection;

// Renders the selected(active) entities in the editor/debugg-mode
void GraphicsManager::RenderSelection()
{
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
	ShadeMan.SetActiveShader("Wireframe", graphicsState);
	Shader * shader = ActiveShader();

	/// Set rainbow factor for XYZ ^w^
	glUniform1f(glGetUniformLocation(shader->shaderProgram, "rainbowXYZFactor"), 0.0f);
	
	Camera * camera = GraphicsThreadGraphicsState->camera;

	// Update view and projection matrix in specified shader
	if (shader && shader->uniformProjectionMatrix != -1)
		glUniformMatrix4fv(shader->uniformProjectionMatrix, 1, false, GraphicsThreadGraphicsState->projectionMatrixF.getPointer());
	// Update view and projection matrix in specified shader
	if (shader && shader->uniformViewMatrix != -1)
		glUniformMatrix4fv(shader->uniformViewMatrix, 1, false, GraphicsThreadGraphicsState->viewMatrixF.getPointer());
	// Update camera in the world
	if (shader && shader->uniformEyePosition != -1)
		glUniform4f(shader->uniformEyePosition, camera->Position()[0], camera->Position()[1], camera->Position()[2], 1.0);


	PrintGLError("Unable to set wireframe shader");
	// Set color of the wireframes of all selected objects
	glUniform4f(shader->uniformPrimaryColorVec4, 1.0f, 1.0f, 0.0f, 0.7f);
	PrintGLError("Unable to set primary color");
	// Set to wireframe
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glLineWidth(3.f);
	EntitySharedPtr* activeEntity = selectionToRender->GetArray();
	int amount = selectionToRender->Size();
	GraphicsThreadGraphicsState->settings &= ~ENABLE_SPECIFIC_ENTITY_OPTIONS;
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