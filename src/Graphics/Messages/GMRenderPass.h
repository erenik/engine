/// Emil Hedemalm
/// 2015-02-05
/// Messages for managing/handling a render-pipeline's render-passes in real-time.

#ifndef GM_RENDER_PASS_H
#define GM_RENDER_PASS_H

#include "GraphicsMessage.h"

class RenderPass;

class GMAddRenderPass : public GraphicsMessage 
{
public:
	GMAddRenderPass(RenderPass * rs);
	virtual void Process();
private:
	RenderPass * rs;
};

#endif
