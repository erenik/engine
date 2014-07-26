/// Emil Hedemalm
/// 2014-07-10
/// Manager for handling rendering-pipelines in their entirety.

#define RenderPipeMan (*RenderPipelineManager::Instance())

#include "RenderPipeline.h"

class RenderPipelineManager 
{
	RenderPipelineManager();
	~RenderPipelineManager();
	static RenderPipelineManager * renderPipelineManager;
public:
	static void Allocate();
	static void Deallocate();
	static RenderPipelineManager * Instance();
	
	// Selects next pipeline in the list, returning it. 0 may be returned!
	RenderPipeline * Next();
	RenderPipeline * Previous();

	// Loads from render/PipelineConfig.txt, creating new pipelines and setting initial active one.
	void LoadFromPipelineConfig();

	RenderPipeline * activePipeline;
	List<RenderPipeline*> renderPipelines;
};

