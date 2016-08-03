/// Emil Hedemalm
/// 2014-07-09
/** Render configuration, containing selection of shaders to use, 
	which passes to render and at what resolution (with respective to the active AppWindow)
*/

#include "RenderPipeline.h"
#include "RenderPass.h"
#include "File/File.h"
#include "Timer/Timer.h"

RenderPipeline::RenderPipeline()
{
	type = RenderPipeline::FIXED;
}

RenderPipeline::~RenderPipeline()
{
	std::cout<<"\npppp";
	/// Delete render-passes on deletion of this pipeline.
	renderPasses.ClearAndDelete();
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
		if (line.StartsWith("EndParse"))
			break;
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
			String arg = tokens[1], arg2;
			if (tokens.Size() > 2)
				arg2 = tokens[2];
			if (line.StartsWith("Camera"))
			{
				if (arg == "Light")
					rp->camera = RenderPass::LIGHT;
				else if (arg == "Default")
					rp->camera = RenderPass::DEFAULT_CAMERA;
			}
			else if (line.StartsWith("Shadows"))
			{
				rp->shadows = arg.ParseBool();
			}
			else if (line.StartsWith("SkyPass"))
				rp->skyPass = arg.ParseBool();
			else if (line.StartsWith("ShadowMapping"))
			{
				rp->shadowMapping = true;
			}
			else if (line.StartsWith("SortBy"))
			{
				List<String> args = line.Tokenize(" \t");
				String axisStr = arg,
					increasingStr = arg2;
				rp->sortBy = axisStr == "Z"? 3 : (axisStr == "Y"? 2 : 1);
				rp->sortByIncreasing = increasingStr == "Increasing";
			}
			else if (line.StartsWith("ShadowMapResolution"))
			{
				rp->shadowMapResolution = arg.ParseInt();
			}
			else if (line.StartsWith("Lights"))
			{
				rp->lights = RenderPass::PRIMARY_LIGHT;
			}
			else if (line.Contains("Input"))
			{
				if (arg == "Entities")
					rp->input = RenderTarget::ENTITIES;
				else if (arg == "SolidEntities")
					rp->input = RenderTarget::SOLID_ENTITIES;
				else if (arg == "ShadowCastingEntities")
					rp->input = RenderTarget::SHADOW_CASTING_ENTITIES;
				else if (arg == "EntityGroup")
				{
					rp->input = RenderTarget::ENTITY_GROUP;
					rp->inputGroup = arg2;
				}
				else if (arg == "RemainingEntities")
				{
					// All other entities, not defined in other (EntityGroup) groups
					rp->input = RenderTarget::REMAINING_ENTITIES;
				}
				else if (arg == "AlphaEntities")
				{
					rp->input = RenderTarget::ALPHA_ENTITIES;
					rp->depthTestEnabled = false;
				}
				else if (arg == "DeferredGather")
					rp->input = RenderTarget::DEFERRED_GATHER;
				else if (arg == "SkyBox")
					rp->input = RenderTarget::SKY_BOX;
			}
			else if (line.Contains("Output"))
			{
				if (arg == "Default")
					rp->output = RenderTarget::DEFAULT;
				else if (arg == "DeferredGather")
					rp->output = RenderTarget::DEFERRED_GATHER;
				else if (arg == "ShadowMaps")
					rp->output = RenderTarget::SHADOW_MAPS;
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

void RenderPipeline::Render(GraphicsState & graphics)
{
	Timer timer;
	for (int i = 0; i < renderPasses.Size(); ++i)
	{
		timer.Start();
		RenderPass * rp = renderPasses[i];
		bool ok = rp->Render(graphics);
		timer.Stop();
		rp->renderTimeMs = timer.GetMs();
		if (!ok)
			std::cout<<"\nError in render-pass: "<<rp->name;
	}
}

/// Call after calling all individual entities/model's render-calls.
void RenderPipeline::EndFrame()
{

}

