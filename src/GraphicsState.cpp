// Emil Hedemalm
// 2013-07-20

#include "GraphicsState.h"
#include "Lighting.h"
#include "Graphics/Camera/Camera.h"

GraphicsState::GraphicsState()
{
	activeWindow = NULL;
	activeViewport = NULL;
	lighting = NULL;
//	activeShader = NULL;
	currentMesh = NULL;
	currentTexture = NULL;
	currentSpecularMap = NULL;
	currentNormalMap = NULL;
	currentFont = NULL;
	gridSpacing = 10.0f;
	gridSize = 20;
	camera = NULL;
	settings = 0;
	optimizationLevel = 0;
	viewportX0 = viewportY0 = 0;
	promptScreenshot = recording = false;
	screenshotsTaken = 0;
	framesRecorded = 0;

	flags = 0;
	fogBegin = 500.0f;
	fogEnd = 2500.0f;
};

GraphicsState::~GraphicsState()
{
	if (lighting)
		delete lighting;
}


/// Calls glScissor, and updates locally tracked scissor. Appends current viewport x0/y0 co-ordinates automatically to the GL call.
void GraphicsState::SetGLScissor(const Rect & newScissor)
{
	CheckGLError("GraphicsState::SetGLScissor 1");
	bool scissorDisabled = (settings & SCISSOR_DISABLED) > 0;
    if (scissorDisabled)
	{
		glDisable(GL_SCISSOR_TEST);
		return;
	}

	this->scissor = newScissor;
	Vector2i size = scissor.Size();
	glScissor((GLint)(scissor.min[0] + viewportX0), (GLint)(scissor.min[1] + viewportY0), size[0] < 0 ? 0 : size[0], size[1] < 0 ? 0 : size[1]);

	CheckGLError("GraphicsState::SetGLScissor 2");
}


void GraphicsState::SetCamera(Camera * camera)
{	
	this->camera = camera;
	projectionMatrixF = projectionMatrixD = camera->ProjectionMatrix4d();
	viewMatrixF = viewMatrixD = camera->ViewMatrix4d();
}			