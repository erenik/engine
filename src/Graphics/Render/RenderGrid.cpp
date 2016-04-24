#include "Graphics/GraphicsManager.h"
#include "GraphicsState.h"

#define PRINT_ERROR	std::cout<<"\nGLError in Render "<<error;

void GraphicsManager::RenderGrid(){
	// Clear errors upon entering.
	GLuint error = glGetError();
	// Draw awesome grid for debugging, yo.
	if (true){
		glEnable(GL_DEPTH_TEST);
		ShadeMan.SetActiveShader(0);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(graphicsState->projectionMatrixD.getPointer());
		glMatrixMode(GL_MODELVIEW);
		graphicsState->modelMatrix.LoadIdentity(); // Don't inherit model from entities....
		Matrix4d modelView = graphicsState->viewMatrixD * graphicsState->modelMatrixD;
		glLoadMatrixd(modelView.getPointer());
		// Enable blending
		glEnable(GL_BLEND);	
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		float z = -4;
		glDisable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		graphicsState->currentTexture = NULL;
		// Disable lighting
		glDisable(GL_LIGHTING);
		error = glGetError();
		if (error != GL_NO_ERROR){
			PRINT_ERROR
		}
		glDisable(GL_COLOR_MATERIAL);
		// Specifies how the red, green, blue and alpha source blending factors are computed
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		error = glGetError();
		if (error != GL_NO_ERROR){
			PRINT_ERROR
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	//	glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x0101);
		glLineWidth(1.0f);
		// Do grid, yo.
		float spacing = graphicsState->gridSpacing; // 10.0f;
		int gridSize = graphicsState->gridSize; // 20;
		float start = -gridSize / 2 * spacing;
		for (int i = 0; i < gridSize; ++i){
			float x = i * spacing + start, x2 = (i+1) * spacing + start;
			for (int j = 0; j < gridSize; ++j){
				float z = j * spacing + start, z2 = (j+1) * spacing + start;
				glBegin(GL_QUADS);
					glColor4f(0.6f,0.4f,0.5f, 1);
					glVertex3f(x,	0,	z);
					glVertex3f(x2,	0,	z);
					glVertex3f(x2,	0,	z2);
					glVertex3f(x,	0,	z2);
				glEnd();
			}
		}
	
		
		glBegin(GL_TRIANGLES);
		glColor3f(1,0,0);
		glVertex3f(10, 0, 0);
		glColor3f(0,1,0);	
		glVertex3f(0, 10, 0);
		glColor3f(0,0,1);	
		glVertex3f(0, 0, 10);
		glEnd();

		error = glGetError();
		if (error != GL_NO_ERROR){
			PRINT_ERROR
		}
	}
//	glDisable(GL_LINE_STIPPLE);
}