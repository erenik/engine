/// Emil Hedemalm
/// 2015-01-06
/// For rendering overlay over everything.

#include "Graphics/GraphicsManager.h"
#include "Texture.h"
#include "GraphicsState.h"

#include "File/LogFile.h"

#define PRINT_ERROR	std::cout<<"\nGLError in Render "<<error;

// Utility function used by e.g. RenderOverlay.
void GraphicsManager::RenderFullScreen(Texture * texture, float alpha)
{
	int width = GraphicsThreadGraphicsState->windowWidth;
	int height = GraphicsThreadGraphicsState->windowHeight;

	// Buffer if needed.
	if (texture->glid == -1)
		texture->Bufferize();

	/// Use default shader for overlays
	ShadeMan.SetActiveShader(nullptr, graphicsState);
	
	// Fill the polygons!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Reset projection matrix for this
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
//	glLoadMatrixd(GraphicsThreadGraphicsState->projectionMatrixD.getPointer());

	glOrtho(0, width, 0, height, 1, 10);
	// Reset modelmatrix too
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	float z = -1.01f;		
	glTranslatef(0,0,z);
	Matrix4d modelView = GraphicsThreadGraphicsState->viewMatrixD * GraphicsThreadGraphicsState->modelMatrixD;
//	glLoadMatrixd(modelView.getPointer());

	// Disable depth-testing in-case deferred rendering is enabled D:
	glDisable(GL_DEPTH_TEST);

	// Enable blending
	glEnable(GL_BLEND);	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Enable texturing
	glEnable(GL_TEXTURE_2D);
	if (GraphicsThreadGraphicsState->currentTexture != texture){
		glBindTexture(GL_TEXTURE_2D, texture->glid);
		GraphicsThreadGraphicsState->currentTexture = texture;
	}
	// Buffer it again..
	int error = glGetError();
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
		glColor4f(1.0f, 1.0f, 1.0f, alpha);
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
	glLoadMatrixd(GraphicsThreadGraphicsState->projectionMatrixD.getPointer());

	// Enable disabled stuffs.
	glEnable(GL_DEPTH_TEST);	
}

void GraphicsManager::RenderOverlay()
{
	// Clear errors upon entering.
	GLuint error = glGetError();
	String str = "\nRendering ";
	// If we have an overlay-texture, render it.
	if (overlayTexture)
	{
//		str += " overlay: " + overlayTexture->name;
		if (overlayTexture->glid == -1)
			overlayTexture->Bufferize();
		if (overlayTexture->glid != -1)
			RenderFullScreen(overlayTexture, 1.0f);
		else 
			LogGraphics("Trying to render unbuffered overlay texture.", WARNING);
	}
	// And if we have a queued overlay texture, render it on top!
	if (queuedOverlayTexture)
	{
		// Bufferize if needed first.
		if (queuedOverlayTexture->glid == -1)
			queuedOverlayTexture->Bufferize();

		// If overlay-fade start has not been set, do it now.
		if (overlayFadeInStart == 0)
			overlayFadeInStart = Timer::GetCurrentTimeMs();

		// Render the overlay-texture using target alpha.
		int64 currentTime = Timer::GetCurrentTimeMs();
		int timeFadedSoFar = currentTime - overlayFadeInStart;
		float alpha = float(timeFadedSoFar) / overlayFadeInTime;
		str += " queued overlay: " + queuedOverlayTexture->name + "with alpha: " + String::ToString(alpha);
		RenderFullScreen(queuedOverlayTexture, alpha);

		// Check if they should be swapped due to fade time reached.
		if (currentTime > overlayFadeInStart + overlayFadeInTime)
		{
			overlayTexture = queuedOverlayTexture;
			queuedOverlayTexture = NULL;
			overlayFadeInStart = 0;
		}
	}
	if (str.Length() > 20)
	{
		std::cout<<str;
	}
}