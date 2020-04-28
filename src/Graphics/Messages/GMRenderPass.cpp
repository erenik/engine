/// Emil Hedemalm
/// 2015-02-05
/// Messages for managing/handling a render-pipeline's render-passes in real-time.

#include "GMRenderPass.h"
#include "GraphicsMessages.h"

#include "Graphics/GraphicsManager.h"

#include "File/LogFile.h"
#include "Render/RenderPipeline.h"

GMAddRenderPass::GMAddRenderPass(RenderPass * rs)
: GraphicsMessage(GM_ADD_RENDER_PASS), rs(rs)
{
}

void GMAddRenderPass::Process(GraphicsState* graphicsState)
{
	RenderPipeline * pipe = NULL;
	// Add the render-pass to the current pipeline.
	pipe = graphicsState->renderPipe;
	if (!pipe)
	{
		LogGraphics("Unablet o add render pass to pipeline - no valid pipeline selected", ERROR);
		return;
	}
	pipe->renderPasses.Add(rs);
}
