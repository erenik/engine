/// Emil Hedemalm
/// 2014-07-09
/// Defines one single render pass.
/// A Render-pass in this sense is just one group of graphics related actions, often painting some data onto a buffer or texture object.

#include "Shader.h"
#include "String/AEString.h"
#include "GraphicsState.h"
#include "Entity/Entities.h"
#include "Graphics/Camera/Camera.h"
#include "Mesh/Mesh.h"
#include "FrameBuffer.h"
#include "RenderBuffer.h"
#include "Viewport.h"
#include "File/LogFile.h"
#include "Model/Model.h"
#include "Model/ModelManager.h"
#include "RenderInstancingGroup.h"
#include "Graphics/FrameStatistics.h"

class FrameBuffer;
class Viewport;
class RenderInstancingGroup;

namespace RenderTarget 
{
	enum {
		UNKNOWN,
		// Outputs. 
		SHADOW_MAPS, // output of shadow maps when rendering from light's point of view.
		DEFERRED_GATHER, // Renders to several textures.
		DEFERRED_OUTPUT, // Renders to 1 Color + 1 Depth texture.
		MINIFICATION_BUFFERS, // A set of buffers for minification. Exact contents depend on the input used previously.
		POST_PROCESS_OUTPUT, // Renders to 1 color texture.
		FINAL_GATHER,	 // Render to final gather texture for the initial lighting pass. <- wat
		// Inputs
		ENTITIES, // All entities registered for rendering.
		ENTITY_GROUP, // Entities part of a specific render-group.
		REMAINING_ENTITIES, // Entities not part of any specific entity-group.
		SHADOW_CASTING_ENTITIES, // Yup.
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
	// Statistic.
	int renderTimeMs;

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
	String inputGroup; // Identifier for the group.
	int output;

	// Default false. If true, shader will be supplied with shadow maps and the matrices used to create them.
	bool shadows;

	// Output
	int renderTarget;

	/// Default true.
	bool depthTestEnabled;

	/// Axis, 1 for X, 2 for Y, 3 for Z, 0 for no sorting.
	int sortBy;
	bool sortByIncreasing; // For the sorting. If increasing or decreasing manner.

private:
	/// Based on available shader data. Updated each frame.
	bool instancingEnabled;
	/// Place 'em here.
	Entities entitiesToRender;
	List<RenderInstancingGroup*> entityGroupsToRender;

	// Parts of rendering.
	bool SetupOutput();
	bool SetupLightPOVCamera();

	/// o.o
	void RenderEntities();
	/// Used for e.g. shadow-mapping.
	void RenderEntitiesOnlyVertices(); 
	void RenderAlphaEntities();
	void RenderSkyBox();

	/// Current
	Viewport * viewport;
	/// Creates it as needed.
	bool BindShadowMapFrameBuffer();
	/// Set up/fetch render buffers as needed. (For output)
	bool BindDeferredGatherFrameBuffer();
	bool BindDeferredOutputFrameBuffer();
	bool BindPostProcessOutputFrameBuffer();
	/// If iterations is non-0.
	bool PerformIterativePingPongRenders();
	void SetupDeferredGatherAsInput(); // (For input);
	void SetupDeferredOutputAsInput();

	FrameBuffer * GetInputFrameBuffer();

	/// Renders a -1,1, -1,1 quad, covering the screen (or current framebuffer). Uses a nullified projection and view matrix to achieve it.
	void RenderQuad();
	/// In pixels.
	int shadowMapResolution;
	/// For ping-pong render-passes such as to MinificationBuffers, determines amount of buffers and times to render between them. Default 1.
	int iterations;
	bool clear; /// Default true. If false, does not clear render buffers it is rendering to.
};