/// Emil Hedemalm
/// 2014-07-09
/** Render configuration, containing selection of shaders to use, 
	which passes to render and at what resolution (with respective to the active AppWindow)
*/

#ifndef RENDER_PIPELINE_H
#define RENDER_PIPELINE_H

#include "Graphics/Shader.h"
#include "List/List.h"
#include "String/AEString.h"

class RenderPass;
class GraphicsState;

class RenderPipeline
{
public:

	RenderPipeline();
	virtual ~RenderPipeline();


	/// Renders the entire pipeline, using data available within the GraphicsState.
	void Render(GraphicsState & graphics);	

	// Loads from target source file.
	bool Load(String fromFile);
	void Save(String toFile);

	// Found inside the source.
	String name;
	// Is set upon loading.
	String source;


	
	enum {
		FIXED,
		MULTI_PASS,
	};
	/** 0 - Fixed. Renders to output straight away. Re-uses all old code available in the relevant Render-functions/-files.
		1 - Multiple render-passes (e.g. Deferred).
	*/
	int type;

	// Render-passes to process.
	List<RenderPass*> renderPasses;

private:
	/// Call before calling individual entities/model's render-calls.
	void BeginFrame();
	/// Call after calling all individual entities/model's render-calls.
	void EndFrame();

};

#endif

