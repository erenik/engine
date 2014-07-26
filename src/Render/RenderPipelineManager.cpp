/// Emil Hedemalm
/// 2014-07-10
/// Manager for handling rendering-pipelines in their entirety.

#include "RenderPipelineManager.h"
#include "File/File.h"

RenderPipelineManager::RenderPipelineManager()
{
	activePipeline = NULL;
}
RenderPipelineManager::~RenderPipelineManager()
{
	renderPipelines.ClearAndDelete();
}

RenderPipelineManager * RenderPipelineManager::renderPipelineManager = NULL;

void RenderPipelineManager::Allocate()
{
	assert(renderPipelineManager == 0);
	renderPipelineManager = new RenderPipelineManager();
}
void RenderPipelineManager::Deallocate()
{
	assert(renderPipelineManager);
	delete renderPipelineManager;
	renderPipelineManager = NULL;
}
RenderPipelineManager * RenderPipelineManager::Instance()
{
	assert(renderPipelineManager);
	return renderPipelineManager;
}
	
// Selects next pipeline in the list, returning it. 0 may be returned!
RenderPipeline * RenderPipelineManager::Next()
{
	int indexOf = renderPipelines.GetIndexOf(activePipeline);
	indexOf++;
	std::cout<<"\nCycling render-pipeline to next one: ";
	// Cycle up..
	if (indexOf < renderPipelines.Size())
	{
		activePipeline = renderPipelines[indexOf];
		std::cout<<activePipeline->name;
	}
	// And resetting..
	else 
	{
		activePipeline = 0;
		std::cout<<0;
	}
	return activePipeline;
}

// Selects next pipeline in the list, returning it. 0 may be returned!
RenderPipeline * RenderPipelineManager::Previous()
{
	int indexOf = renderPipelines.GetIndexOf(activePipeline);
	if (indexOf == -1)
		indexOf = renderPipelines.Size() - 1;
	else
		indexOf--;
	std::cout<<"\nCycling render-pipeline to previous one: ";
	// Cycle down..
	if (indexOf >= 0)
	{
		activePipeline = renderPipelines[indexOf];
		std::cout<<activePipeline->name;
	}
	// And resetting..
	else 
	{
		activePipeline = 0;
		std::cout<<0;
	}
	return activePipeline;
}


// Loads from render/PipelineConfig.txt 
void RenderPipelineManager::LoadFromPipelineConfig()
{
	// Delete old pipelines.
	renderPipelines.ClearAndDelete();

	List<String> lines = File::GetLines("render/PipelineConfig.txt");
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		List<String> tokens = line.Tokenize(" \t");
		if (!tokens.Size())
			continue;
		if (tokens.Size() < 2)
			continue;
		String key = tokens[0];
		String value = tokens[1];
		if (key == "LoadPipeline")
		{
			RenderPipeline * pipe = new RenderPipeline();
			pipe->Load("render/" + value + ".pipe");
			renderPipelines.Add(pipe);
		}
		else if (key == "UsePipeline")
		{
			for (int i = 0; i < renderPipelines.Size(); ++i)
			{
				RenderPipeline * pipe = renderPipelines[i];
				if (pipe->name == value)
				{
					activePipeline = pipe;
					break;
				}
			}
		}
	}
	if (!activePipeline)
		activePipeline = renderPipelines[0];
}


