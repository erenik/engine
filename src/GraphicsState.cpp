// Emil Hedemalm
// 2013-07-20

#include "GraphicsState.h"

GraphicsState::GraphicsState()
{
	activeWindow = NULL;
	activeViewport = NULL;
	lighting = NULL;
	activeShader = NULL;
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

