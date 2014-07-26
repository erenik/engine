/// Emil Hedemalm
/// 2014-07-09
/// Defines one single render pass.

#include "Shader.h"
#include "String/AEString.h"

struct GraphicsState;

namespace RenderTarget 
{
	enum {
		// Outputs. 
		DEFERRED_GATHER, // Renders to several textures.
		FINAL_GATHER,	 // Render to final gather texture for the initial lighting pass.
		// Inputs
		ENTITIES,
		DEFAULT,
	};
};

class RenderPass 
{
public:
	RenderPass();

	// Renders this pass..!
	void Render(GraphicsState * graphics);

	// Names are always good.
	String name;
	// Only one shader is used, always, per pass.
	Shader * shader;
	String shaderName;

	// Input type. 
	int input;
	int output;

	// Output
	int renderTarget;

};