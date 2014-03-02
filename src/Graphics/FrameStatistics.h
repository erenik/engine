/// Emil Hedemalm
// 2013-07-28

#ifndef RENDER_STATISTICS_H
#define RENDER_STATISTICS_H

#include "List/List.h"

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
	void Reset();
	void Print();

	/// And the stats!
	float sceneTime, alphaTime, effectsTime, uiTime;


	/// Pushes the frame time which is then used to calculate the average frame-time.
	void PushFrameTime(float frameTime);

	/// Returns the average fps, calculated using the PushFrameTime function and a predefined amount of frames to consider when calculating average frame-time.
	float FPS();

private:
    int maxFramesToConsider;
    float averageFrameTime;
	List<float> frameTimes;
	float fps;

	float lastSceneTime, lastAlphaTime, lastEffectsTime, lastUITime;


};

#endif

