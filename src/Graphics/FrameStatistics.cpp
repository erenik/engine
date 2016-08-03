/// Emil Hedemalm
// 2013-07-28

#include "FrameStatistics.h"
#include "Globals.h"
#include <iostream>
#include "GraphicsState.h"
#include "Render/RenderPass.h"
#include "Render/RenderPipeline.h"

FrameStatistics * FrameStatistics::frameStatistics = NULL;

FrameStatistics::FrameStatistics()
{
	ResetGraphics();
	ResetPhysics();

    /// Default FPS values
    maxFramesToConsider = 10;
    averageFrameTime = 0.015f;
	fps = 60;
	printQueued = false;
}

FrameStatistics::~FrameStatistics(){}

void FrameStatistics::Allocate(){
	assert(frameStatistics == NULL);
	frameStatistics = new FrameStatistics();
}

void FrameStatistics::Deallocate(){
	assert(frameStatistics);
	delete frameStatistics;
	frameStatistics = NULL;
}

FrameStatistics * FrameStatistics::Instance(){
	return frameStatistics;
}

void FrameStatistics::ResetGraphics(){
	/// And the stats!
	lastSceneTime = sceneTime;
	lastAlphaTime = alphaTime;
	lastEffectsTime = effectsTime;
	lastUITime = uiTime;
	sceneTime = alphaTime = effectsTime = uiTime = 0.0f;
	renderSortEntities = renderEntities = 0;
	renderPrePipeline = renderPostPipeline = 0;
	updateLighting = graphicsRepositionEntities = graphicsProcess = 0;
	swapBuffers = 0;

	particleProcessing = 0;
	particleSpawning = 0;
	particleBufferUpdate = 0;
	particleProcessingIntegrate = particleProcessingOldify = particleProcessingRedead = 0;
}

void FrameStatistics::ResetPhysics()
{
	physicsIntegration = 0;
	physicsIntegrationRecalcMatrices = 0;
	physicsRecalcAABBs = physicsRecalcOBBs = physicsRecalcProps = 0;
	physicsCollisions = 0;
	physicsCollisionDetection = physicsCollisionResolution = physicsCollisionCallback = 0;
	physicsCollisionDetectionAABBSweep = physicsCollisionDetectionChosenDetector = 0;

}

/// Pushes the frame time which is then used to calculate the average frame-time.
void FrameStatistics::PushFrameTime(float frameTime){
    frameTimes.Add(frameTime);
    while (frameTimes.Size() > maxFramesToConsider){
        frameTimes.RemoveIndex(0, ListOption::RETAIN_ORDER);
    }
}

/// Returns the average fps, calculated using the PushFrameTime function and a predefined amount of frames to consider when calculating average frame-time.
float FrameStatistics::FPS(){
    if (frameTimes.Size() >= maxFramesToConsider){
        averageFrameTime = 0;
        for (int i = 0; i < frameTimes.Size(); ++i)
            averageFrameTime += frameTimes[i];
        averageFrameTime /= maxFramesToConsider;
   //     std::cout<<" Average frame time: "<<averageFrameTime;
        fps = 1000 / averageFrameTime;
    //    std::cout<<"\nFPS: "<<fps;
        return fps;
    }
    return 60;
}

void FrameStatistics::QueuePrint()
{
	printQueued = true;
}




void FrameStatistics::Print(){
	std::cout
		<<"\nFrame statistics (total): "
		<<"\n- Physics total: "<< totalPhysics
		<<"\n	- processing: "<<physicsProcessing
		<<"\n		- integration: " <<physicsIntegration
		<<"\n		- recalc matrices: " <<physicsIntegrationRecalcMatrices
		<<"\n		- recalc AABBs: " <<physicsRecalcAABBs
		<<"\n		- recalc OBBs: " <<physicsRecalcOBBs
		<<"\n		- recalc props: " <<physicsRecalcProps
		<<"\n		- collisions: " <<physicsCollisions
		<<"\n			- detection: " <<physicsCollisionDetection
		<<"\n				- AABB Sweep: "<<physicsCollisionDetectionAABBSweep
		<<"\n				- Chosen detector: "<<physicsCollisionDetectionChosenDetector
		<<"\n			- resolution: " <<physicsCollisionResolution 
		<<"\n			- callbacks: " <<physicsCollisionCallback
		<<"\n	- messages: "<<physicsMessages
		<<"\n- Graphics total: "<<totalGraphics
		<<"\n	- messages: "<<graphicsMessages
		<<"\n	- updateLighting: "<<updateLighting
		<<"\n	- repositionEntities: "<<graphicsRepositionEntities
		<<"\n	- process: "<<graphicsProcess
		<<"\n		- particleProcessing: "<<particleProcessing
		<<"\n			- integrate: "<<particleProcessingIntegrate
		<<"\n			- oldify: "<<particleProcessingOldify
		<<"\n			- redead: "<<particleProcessingRedead
		<<"\n		- particleSpawning: "<<particleSpawning
		<<"\n		- particleBufferUpdate: "<<particleBufferUpdate
		<<"\n	- render: "<<renderTotal;
	// Print the separate passed.
	String renderPassed = "\n		- RenderPasses:";
	for (int i = 0; i < graphicsState->renderPipe->renderPasses.Size(); ++i)
	{
		RenderPass * rp = graphicsState->renderPipe->renderPasses[i];
		renderPassed += 
			"\n			- "+rp->name+": "+String(rp->renderTimeMs);
	}

	std::cout
		<<"\n		- prePipeline: "<<renderPrePipeline
		<<"\n		- sortEntities: "<<renderSortEntities
		<<renderPassed
		<<"\n		- postPipeline: "<<renderPostPipeline
		<<"\n		- swapBuffers: "<<swapBuffers
		<<"\n- Multimedia/Audio: "<<multimedia
		<<"\n- Average FPS: "<<fps;
	printQueued = false;
}
