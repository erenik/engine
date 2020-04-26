/// Emil Hedemalm
// 2013-07-28

#ifndef RENDER_STATISTICS_H
#define RENDER_STATISTICS_H

#include "List/List.h"

class GraphicsState;

#define FrameStats (*FrameStatistics::Instance())

// Data handling single for render-statistics.
struct FrameStatistics {
private:
	FrameStatistics();
	~FrameStatistics();
	static FrameStatistics * frameStatistics;
public:
	static void Allocate();
	static void Deallocate();
	static FrameStatistics * Instance();

	/// Anulls the frame stats (only for current-frame statistics, not FPS et al)
	void ResetGraphics();
	void ResetPhysics();
	void Print(GraphicsState& graphicsState);

	/// And the stats!
	float sceneTime, alphaTime, effectsTime, uiTime;

	float graphicsMessages, renderTotal, totalGraphics;
	float updateLighting, graphicsRepositionEntities, graphicsProcess;
	float particleProcessing, particleSpawning, particleBufferUpdate,
		particleProcessingIntegrate, particleProcessingOldify, particleProcessingRedead;
	float renderSortEntities, renderEntities, renderPrePipeline, renderPostPipeline, swapBuffers;
	float multimedia;
	float physicsProcessing, physicsMessages, totalPhysics, 
		physicsIntegration, physicsIntegrationRecalcMatrices, physicsRecalcAABBs, physicsRecalcOBBs,
		physicsRecalcProps, physicsCollisions;
	float physicsCollisionDetection, physicsCollisionResolution, physicsCollisionCallback;
	/// Detection, broad-phase AABB, and narrow-phase based on chosen detector.
	float physicsCollisionDetectionAABBSweep, physicsCollisionDetectionChosenDetector;


	/// Pushes the frame time which is then used to calculate the average frame-time.
	void PushFrameTime(float frameTime);

	/// Returns the average fps, calculated using the PushFrameTime function and a predefined amount of frames to consider when calculating average frame-time.
	float FPS();

	void QueuePrint();
	bool printQueued;
private:
    int maxFramesToConsider;
    float averageFrameTime;
	List<float> frameTimes;
	float fps;


	float lastSceneTime, lastAlphaTime, lastEffectsTime, lastUITime;


};

#endif

