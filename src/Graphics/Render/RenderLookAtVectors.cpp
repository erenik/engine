// Emil Hedemalm
// 2013-08-03

#include "Graphics/GraphicsManager.h"
#include "GraphicsState.h"
#include "RenderRay.h"

// For rendering LookAt's, velocities? et al.
void GraphicsManager::RenderEntityVectors(){

	// Optionally render the global-light as a small... tile?
	// TODO: if so.
	return;

	ShadeMan.SetActiveShader(NULL);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glLoadMatrixf(graphicsState->projectionMatrixF.getPointer());
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLoadMatrixf(graphicsState->viewMatrixF.getPointer());

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	//	glBindTexture(GL_TEXTURE_2D, 0);
	//	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_LIGHTING);

	glLoadIdentity();
	glLoadMatrixf(graphicsState->viewMatrixF.getPointer());


	glBegin(GL_LINES);

	/// Render all lights provided in the shiberiwa!
	for (int i = 0; i < registeredEntities.Size(); ++i){
		// Reset view/proj matrices

		// Now just render a single line first.
		Entity * e = registeredEntities[i];
		Vector3f position = e->position;
		Vector3f lookAt = e->rotationMatrix * Vector4d(0,0,1,0);
		Vector3f upVec = e->rotationMatrix * Vector4d(0,1,0,0);

		float arrowSize = 50.5f;
		upVec *= arrowSize * 0.25f;

		Vector3f destination = position + lookAt * arrowSize;
		Vector3f nearlyDest = position + lookAt * arrowSize * 0.75f;

		/// Look at!
		glColor4f(0.0f,0.0f,1.0f,1.0f);
		glVertex3f(position.x, position.y, position.z);
		glColor4f(1.0f,1.0f,1.0f,1.0f);
		glVertex3f(destination.x, destination.y, destination.z);

		// + 2 guidelines to so we know that it's an arrow too..
		glVertex3f(nearlyDest.x + upVec.x, nearlyDest.y + upVec.y, nearlyDest.z + upVec.z);
		glVertex3f(destination.x, destination.y, destination.z);

		glVertex3f(nearlyDest.x - upVec.x, nearlyDest.y - upVec.y, nearlyDest.z - upVec.z);
		glVertex3f(destination.x, destination.y, destination.z);
	}
	glEnd();
}
