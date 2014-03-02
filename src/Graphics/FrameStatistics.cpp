/// Emil Hedemalm
// 2013-07-28

#include "FrameStatistics.h"
#include "Globals.h"
#include <iostream>

FrameStatistics * FrameStatistics::frameStatistics = NULL;

FrameStatistics::FrameStatistics(){
	Reset();

    /// Default FPS values
    maxFramesToConsider = 10;
    averageFrameTime = 0.015f;
	fps = 60;
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

void FrameStatistics::Reset(){
	/// And the stats!
	lastSceneTime = sceneTime;
	lastAlphaTime = alphaTime;
	lastEffectsTime = effectsTime;
	lastUITime = uiTime;
	sceneTime = alphaTime = effectsTime = uiTime = 0.0f;
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




void FrameStatistics::Print(){
	std::cout
		<<"\nNew frame statistics (total): "
		<<"\n- Scene: "<<lastSceneTime
		<<"\n- Alphas: "<<lastAlphaTime
		<<"\n- Effects: "<<lastEffectsTime
		<<"\n- UI: "<<lastUITime
		<<"\n- Average FPS: "<<fps;
}
