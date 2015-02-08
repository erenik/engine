/// Emil Hedemalm
/// 2014-08-09
/// Particle effects

#include "Graphics/Particles/ParticleSystem.h"

/// Particle system class for handling at least some of the particle effects generated by the player tools.
class ToolParticleSystem : public ParticleSystem 
{
public:
	ToolParticleSystem();
	virtual ~ToolParticleSystem();
	/** Processes the particles within the system, moving them wherever they are going or performing whatever transformation they should ungergo.
		If emitters are attached to this system they may also automatically create or emit new particles.
	*/
    virtual void Process(float timeInSeconds);
	/// Renders the particles
	virtual void Render(GraphicsState * graphicsState);

private:
	/// Easiest check for the particles, set a max distance for them
	float * maxDistanceSquared, * distanceTraveledSquared, * scale;
	// So that we use dense data only.
	int livingParticles;
};

/// Particle emitter for the
class ToolParticleEmitter : public ParticleEmitter 
{
public:
	ToolParticleEmitter();
	/// Default new particle.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity);
	/// Extended particle emission.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity, float & scale, float & lifeTime, Vector4f & color);
	
	/// sets positions and updates direction as well as max distance.
	void SetPositionAndTarget(ConstVec3fr position, ConstVec3fr target);

	// Attributes to set to the new particles to spawn.
	float particleScale;
	float particleVelocity;
	float lifetime;
	Vector4f color;
	Vector3f direction;
	Vector3f position;
	Vector3f targetPosition;
private:
};

