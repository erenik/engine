/// Emil Hedemalm
/// 2014-07-09
/// Defines one single render pass.
/// A Render-pass in this sense is just one group of graphics related actions, often painting some data onto a buffer or texture object.

#include "Shader.h"
#include "String/AEString.h"

class GraphicsState;

namespace RenderTarget 
{
	enum {
		// Outputs. 
		DEFERRED_GATHER, // Renders to several textures.
		FINAL_GATHER,	 // Render to final gather texture for the initial lighting pass.
		// Inputs
		ENTITIES, // All entities registered for rendering.
		PARTICLE_SYSTEMS, // All particle systems
		DEFAULT,
	};
};


/// A Render-pass in this sense is just one group of graphics related actions, often painting some data onto a buffer or texture object.
class RenderPass 
{
public:
	RenderPass();

	// Renders this pass. Returns false if some error occured, usually mid-way and aborting the rest of the procedure.
	virtual bool Render(GraphicsState & graphics);

	// Names are always good.
	String name;
	// Only one shader is used, always, per pass.
	Shader * shader;
	String shaderName;

	enum {
		RENDER_ENTITIES, // Renders all entities.
		RENDER_APP_STATE, // Calls the active application state's Render-function.
	};
	/// Basic type this pass is. See enum above.
	int type;
	// Input type. 
	int input;
	int output;

	// Output
	int renderTarget;

	/// Default true.
	bool depthTestEnabled;

};