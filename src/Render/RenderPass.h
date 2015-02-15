/// Emil Hedemalm
/// 2014-07-09
/// Defines one single render pass.
/// A Render-pass in this sense is just one group of graphics related actions, often painting some data onto a buffer or texture object.

#include "Shader.h"
#include "String/AEString.h"

#include "Entity/Entities.h"

class FrameBuffer;
class GraphicsState;
class Viewport;

namespace RenderTarget 
{
	enum {
		// Outputs. 
		SHADOW_MAPS, // output of shadow maps when rendering from light's point of view.
		DEFERRED_GATHER, // Renders to several textures.
		FINAL_GATHER,	 // Render to final gather texture for the initial lighting pass.
		// Inputs
		ENTITIES, // All entities registered for rendering.
		SOLID_ENTITIES, // All non-Alpha entities (most regular ones).
		ALPHA_ENTITIES, // All entities which some either texture or material transparency.
		PARTICLE_SYSTEMS, // All particle systems
		SKY_BOX, // Used for SkyBox rendering.
		DEFAULT,
	};
};


/// A Render-pass in this sense is just one group of graphics related actions, often painting some data onto a buffer or texture object.
class RenderPass 
{
	friend class RenderPipeline;
public:
	RenderPass();
	virtual ~RenderPass();

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
	enum 
	{
		DEFAULT, DEFAULT_CAMERA = DEFAULT,
		LIGHT,
	};
	/// If true, is a shadow-mapping pass. Default false. 
	bool shadowMapping;
	/// For skyBox rendering. Default false.
	bool skyPass;
	/** Determines which lights to use when doing the shadow-mapping pass. Just 1? All?
		Just 1 for now.
	*/
	enum {
		PRIMARY_LIGHT,
		ALL_LIGHTS,
	};
	int lights;
	/// Can be light or default camera.
	int camera;
	// Input type. 
	int input;
	int output;

	// Default false. If true, shader will be supplied with shadow maps and the matrices used to create them.
	bool shadows;

	// Output
	int renderTarget;

	/// Default true.
	bool depthTestEnabled;

private:
	/// Place 'em here.
	Entities entitiesToRender;

	// Parts of rendering.
	bool SetupOutput();
	bool SetupLightPOVCamera();

	/// o.o
	void RenderEntities();
	void RenderAlphaEntities();
	void RenderSkyBox();

	/// Current
	Viewport * viewport;
	/// Creates it as needed.
	bool BindShadowMapFrameBuffer();
	/// In pixels.
	int shadowMapResolution;
};