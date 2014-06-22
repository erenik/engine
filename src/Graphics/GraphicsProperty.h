// Emil Hedemalm
// 2013-06-15

#ifndef GRAPHICS_PROPERTY_H
#define GRAPHICS_PROPERTY_H

#include "Util/List/List.h"
#include "String/AEString.h"
#include "MathLib.h"
#include "String/Text.h"
#include "System/DataTypes.h"

struct GraphicEffect;
struct CompactGraphics;
class Light;
class Texture;
struct AnimationSet;
struct Animation;
class ParticleSystem;
class Camera;

/// Flags for toggling stuff
namespace RenderFlags {
	const int DISABLE_DEPTH_WRITE		=	0x00000001;
	const int DISABLE_BACKFACE_CULLING	=	0x00000002;
	const int REQUIRES_DEPTH_SORTING	=	0x00000004; // For alpha effects, these usually require depth-sorting!
};

/// Class for holding any relevant data beyond static single models/textures
struct GraphicsProperty {
	friend class GMSetEntity;
	friend class GMQueueAnimation;
	friend class Entity;
public:
	GraphicsProperty();
	~GraphicsProperty();
	/// Loads save data from target CompactGraphics equivalent that can be saved to file.
	bool LoadDataFrom(const CompactGraphics * cGraphics);
	/// Fetches relevant texture for current frame time. This assumes that the element has an active animation playing.
	Texture * GetTextureForCurrentFrame(int64 & frameTime);
	/// Meaning: text-based animation. If true then the GetTextureForCurrentFrame should work as intended!
	bool hasAnimation;
	/// For flags, see above: example DISABLE_DEPTH_WRITING (for this model only)
	int flags;
	/// Render at all?
	bool visible;
	/// Effects are graphics that are rendered after the regular render-passes,
	/// usually with other (unique) blending modes.
	List<GraphicEffect*> * effects;
	/// Particle systems attached to le entity
	List<ParticleSystem*> * particleSystems;
	/// Lights that are attached to the entity (relative position/direction/etc.)
	/// Dynamic lights are updated each frame whilst the static lights are added
	/// to the primary lighting-setup on attachment.
	List<Light*> * dynamicLights, * staticLights;

	// For rendering a text string next to an entity
	Text text;
	// Relative to the 1.0 unit length used by entities? so 1 = 1 meter. 0.01 = 1 cm?
	float textSizeRatio;
	Vector4f textColor;
	// Offset?
	Vector4f textPositionOffset; 
private:
	/// Sets current animation. Only called from the GMSetEntity message. If faulty, animation will be nullified.
	void SetAnimation(String name);
	/// Sets queued animation. Only called from the GMSetEntity message.
	void SetQueuedAnimation(String name);

	/// Set of animations, usually belong to one model/character.
	AnimationSet * animationSet;
	/// Animation, a series of textures that make up an animation, with times in between, flags for repeatability etc.
	Animation * currentAnimation;
	/// Wooo
	Animation * queuedAnimation;
	/// Time that the current animation started.
	long long animStartTime;
	
	// Filter to enable per-viewport disabled rendering.
	List<Camera*> cameraFilter;
};

#endif
