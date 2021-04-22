// Emil Hedemalm
// 2013-06-15

#ifndef GRAPHICS_PROPERTY_H
#define GRAPHICS_PROPERTY_H

#include "Util/List/List.h"
#include "String/AEString.h"
#include "MathLib.h"
#include "String/Text.h"
#include "System/DataTypes.h"
#include "Particles/ParticleSystem.h"

struct GraphicEffect;
struct CompactGraphics;
class Light;
class Texture;
struct AnimationSet;
struct Animation;
class ParticleSystem;
class Camera;
class Estimator;
class RenderInstancingGroup;

class Entity;
//

/// Flags for toggling stuff
namespace RenderFlag {
	const int DISABLE_DEPTH_WRITE		=	0x00000001;
	const int DISABLE_BACKFACE_CULLING	=	0x00000002;
//	const int REQUIRES_DEPTH_SORTING	=	0x00000004; // For alpha effects, these usually require depth-sorting!
	const int ALPHA_ENTITY				=	0x00000008; // These specifically require depth-sorting to render correctly. Should be same effect as REQUIRES_DEPTH_SORTING..
};

#define ADD_GRAPHICS_PROPERTY_IF_NEEDED(entity) {if (!entity->graphics) entity->graphics = new GraphicsProperty(entity);}

/// Class for holding any relevant data beyond static single models/textures
struct GraphicsProperty 
{
	friend class GMSetEntity;
	friend class GMSetEntitys;
	friend class GMSlideEntityf;
	friend class GMClearEstimators;
	friend class GMPlayAnimation;
	friend class GMQueueAnimation;
	friend class Entity;
	friend class GraphicsState;
public:
	
	GraphicsProperty(Entity* owner);
	~GraphicsProperty();
	/// Called when registered to Graphics Manager for rendering. Extracts initial position, etc.
	void OnRegister();
	/// Processes estimators related to this entity. Possibly particle effects too?
	virtual void Process(int timeInMs, GraphicsState& graphicsState);

	/// Loads save data from target CompactGraphics equivalent that can be saved to file.
	bool LoadDataFrom(const CompactGraphics * cGraphics);
	/// Fetches relevant texture for current frame time. This assumes that the element has an active animation playing.
	Texture * GetTextureForCurrentFrame(int64 & frameTime);

	/** Contrary to Entity-position, which stores the simulated position from the physics system, this position will hold the averaged or smoothed value which is to be used when rendering the entity. */
	Vector3f smoothedPosition;
	/// Graphical transform, similar to position, it is used to abstract rendering from physics in order to deal with temporal alisasing issues (stuttering effects).
	Matrix4f transform;
	/// Linked to the 2 above. Use for dynamic entities in fast-paced games.
	bool temporalAliasingEnabled;
	/// Entity group this entity belongs to (when it comes to rendering). This may help organize group-specific render-passes.
	String group;
	/// Meaning: text-based animation. If true then the GetTextureForCurrentFrame should work as intended!
	bool hasAnimation;
	/// For flags, see above: example DISABLE_DEPTH_WRITING (for this model only)
	int flags;
	/// Render at all?
	bool visible;
	/// Effects are graphics that are rendered after the regular render-passes,
	/// usually with other (unique) blending modes.
	List<GraphicEffect*> effects;
	/// Particle systems attached to le entity
	List<ParticleSystem*> particleSystems;
	/// Lights that are attached to the entity (relative position/direction/etc.)
	/// Dynamic lights are updated each frame whilst the static lights are added
	/// to the primary lighting-setup on attachment.
	List<Light*> dynamicLights, staticLights;

	// For rendering a text string next to an entity
	Text text;
	// Relative to the 1.0 unit length used by entities? so 1 = 1 meter. 0.01 = 1 cm?
	float textSizeRatio;

	/// Equivalent to "primaryColorVec4f" which is used in the shader's multiplicatively. Default should be (1,1,1,1)
	Vector4f color;

	/// The scale to be rendered for this entity.
	Vector3f scale;

	Color textColor;
	// Offset?
	Vector4f textPositionOffset; 
	/// For rendering the entity at a slightly different position than is stated by the Physics/NavMesh managers.
	Vector3f renderOffset;

	/// Currently calculated manually when sorting before rendering.
	float zDepth;

	/// E.g. GL_ONE for additive blending, or GL_ONE_MINUS_SRC_ALPHA for regular alpha-blending. Default GL_SRC_ALPHA for source and GL_ONE_MINUS_SRC_ALPHA for dest.
	int blendModeSource, blendModeDest; 
	/// If true, requires depth test while rendering, false skips. Default true.
	bool depthTest;
	/// Default true.
	bool depthWrite;

	/** If true, skeleton will be animated, and hopefully skin will be rendered accordingly if proper shaders are used.
		Default false. Set with GMPlaySkeletalAnimation()
	*/
	bool skeletalAnimationEnabled;
	/// If true, will try to render the skinned animation using shaders. If false, all vertices will be re-calculated in the CPU.
	bool shaderBasedSkeletonAnimation;

	/// Default 1.0
	float emissiveMapFactor;

	/// Pauses general automations but also things such as bone/skinning-updates o.o
	bool allAnimationsPaused;

	/// Default true.
	bool castsShadow;
	
	/// If true, the graphics manager and rendering pipeline should try and gather all similar entities (same model and texture) and render them in a single batch.
	bool renderInstanced;
	// See enum in Render/RenderInstancingGroup.h Default 0.
	bool instancedOptions;

	/// Set of animations, usually belong to one model/character.
	AnimationSet * animationSet;
	/// Animation, a series of textures that make up an animation, with times in between, flags for repeatability etc.
	Animation * currentAnimation;
	/// Wooo
	Animation * queuedAnimation;
	/// Time that the current animation started.
	long long animStartTime;
private:
	/// Estimators which are currently tweaking various graphic-specific values over time.
	List<Estimator*> estimators;

	/// Sets current animation. Only called from the GMSetEntity message. If faulty, animation will be nullified.
	void SetAnimation(String name);
	/// Sets queued animation. Only called from the GMSetEntity message.
	void SetQueuedAnimation(String name);

	/// Sets current animation. Only called from the GMSetEntity message. If faulty, animation will be nullified.
	void SetSkeletalAnimation(String name);
	/// Sets queued animation. Only called from the GMSetEntity message.
	void SetQueuedSkeletalAnimation(String name);
	
	// Filter to enable per-viewport disabled rendering.
	List<Camera*> cameraFilter;

	// Must be non-NULL.
	Entity* owner;
};

#endif
