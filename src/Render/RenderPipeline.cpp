/// Emil Hedemalm
/// 2014-07-09
/** Render configuration, containing selection of shaders to use, 
	which passes to render and at what resolution (with respective to the active window)
*/

#include "RenderPipeline.h"
#include "RenderPass.h"
#include "File/File.h"

RenderPipeline::RenderPipeline()
{
	type = RenderPipeline::FIXED;
}

namespace ParseState 
{
	enum {
		NONE,
		RENDER_PASS,
	};
};

bool RenderPipeline::Load(String fromFile)
{
	// Delete old allocated data concerning render-passes.
	renderPasses.ClearAndDelete();

	List<String> lines = File::GetLines(fromFile);
	if (lines.Size() == 0)
	{
		return false;
	}

	RenderPass * rp = NULL;
	
	int parseState = 0;
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		if (line.StartsWith("//"))
			continue;
		else if (line.Contains("Fixed"))
		{
			type = RenderPipeline::FIXED;
		}
		else if (line.Contains("Multi-pass"))
			type = RenderPipeline::MULTI_PASS;
		// A new render-pass!
		else if (line.Contains("RenderPass"))
		{
			rp = new RenderPass();
			rp->name = line.Tokenize(" \t")[1];
			renderPasses.Add(rp);
			parseState = ParseState::RENDER_PASS;
		}
		/// Specific stuffs.
		if (parseState == ParseState::RENDER_PASS)
		{
			List<String> tokens = line.Tokenize(" \t");
			if (tokens.Size() == 0)
				continue;
			String key = tokens[0];
			if (key == "DisableDepthTest")
			{
				rp->depthTestEnabled = false;
			}
			if (tokens.Size() < 2)
				continue;
			String arg = tokens[1];
			if (line.Contains("Input"))
			{
				if (arg == "Entities")
					rp->input = RenderTarget::ENTITIES;
				else if (arg == "DeferredGather")
					rp->input = RenderTarget::DEFERRED_GATHER;
			}
			else if (line.Contains("Output"))
			{
				if (arg == "Default")
					rp->output = RenderTarget::DEFAULT;
				else if (arg == "DeferredGather")
					rp->output = RenderTarget::DEFERRED_GATHER;
			}
			else if (line.Contains("Shader"))
			{
				rp->shader = NULL;
				rp->shaderName = arg;
			}
		}
		// Misc. other stuff.
		List<String> tokens = line.Tokenize(" \t");
		if (tokens.Size() < 2)
			continue;
		String key = tokens[0], 
			value = tokens[1];
		if (key == "Name")
			name = value;
	}
	source = fromFile;
	return true;
}
void RenderPipeline::Save(String toFile)
{

}


/// Call before calling individual entities/model's render-calls.
void RenderPipeline::BeginFrame()
{
	// Set up all needed buffers if the size of the viewport has changed.

	// Bind frames
}

void RenderPipeline::Render(GraphicsState * graphics)
{
	for (int i = 0; i < renderPasses.Size(); ++i)
	{
		RenderPass * rp = renderPasses[i];
		bool ok = rp->Render(graphics);
		if (!ok)
			std::cout<<"\nError in render-pass: "<<rp->name;
	}
}

/// Call after calling all individual entities/model's render-calls.
void RenderPipeline::EndFrame()
{

}

