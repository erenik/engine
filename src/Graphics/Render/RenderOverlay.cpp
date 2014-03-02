
#include "Graphics/GraphicsManager.h"
#include "Texture.h"
#include "GraphicsState.h"

#define PRINT_ERROR	std::cout<<"\nGLError in Render "<<error;

void GraphicsManager::RenderOverlay(){
	// Clear errors upon entering.
	GLuint error = glGetError();
	if (overlayTexture == NULL)
		return;
	if (overlayTexture->glid == -1)
		overlayTexture->Bufferize();

	// Render overlay texture if applicable
	if (overlayTexture != NULL && overlayTexture->glid != -1){
		/// Use default shader for overlays
		glUseProgram(0);
		
		// Fill the polygons!
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		// Reset projection matrix for this
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
	//	glLoadMatrixd(graphicsState->projectionMatrixD.getPointer());
		

		glOrtho(0, width, 0, height, 1, 10);
		// Reset modelmatrix too
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		float z = -1.01f;		
		glTranslatef(0,0,z);
		Matrix4d modelView = graphicsState->viewMatrixD * graphicsState->modelMatrixD;
	//	glLoadMatrixd(modelView.getPointer());

		// Disable depth-testing in-case deferred rendering is enabled D:
		glDisable(GL_DEPTH_TEST);

		// Enable blending
		glEnable(GL_BLEND);	
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		// Enable texturing
		glEnable(GL_TEXTURE_2D);
		if (graphicsState->currentTexture != overlayTexture){
			glBindTexture(GL_TEXTURE_2D, overlayTexture->glid);
			graphicsState->currentTexture = overlayTexture;
		}
		// Buffer it again..
		error = glGetError();
		if (error != GL_NO_ERROR){
			PRINT_ERROR
		}

		// Set texturing parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Disable lighting
		glDisable(GL_LIGHTING);
		error = glGetError();
		if (error != GL_NO_ERROR){
			PRINT_ERROR
		}
		glDisable(GL_COLOR_MATERIAL);
	///	glDisable(GL_COLOR);

		

		// Specifies how the red, green, blue and alpha source blending factors are computed
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable( GL_LIGHTING );
		glEnable(GL_BLEND);
		glBegin(GL_QUADS);
			glColor4f(0.6f,0.4f,0.5f, 1);
			glTexCoord2f(0.0f, 0.0f);			glVertex3f(0,	0,	z);
			glTexCoord2f(1.0f, 0.0f);			glVertex3f(GLfloat(width),	0,	z);
			glTexCoord2f(1.0f, 1.0f);			glVertex3f(GLfloat(width),	GLfloat(height),		z);
			glTexCoord2f(0.0f, 1.0f);			glVertex3f(0,	GLfloat(height),		z);
		glEnd();
		error = glGetError();
		if (error != GL_NO_ERROR){
			PRINT_ERROR
		}
		glDisable(GL_TEXTURE_2D);
		// Load projection matrix again
		glLoadMatrixd(graphicsState->projectionMatrixD.getPointer());
	}
	
	error = glGetError();
	if (error != GL_NO_ERROR){
		PRINT_ERROR
	}
	// Enable disabled stuffs.
	glEnable(GL_DEPTH_TEST);
}