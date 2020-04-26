/// Emil Hedemalm
/// 2014-06-27
/// Particle emitter.

#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

#include "MathLib/Emitter.h"
#include "PhysicsLib/Shapes/Contour.h"
#include "Entity/Entity.h"

class Mesh;
class ParticleSystem;

/// Pre-calculated
extern float oneDivRandMaxFloat;

class ParticleEmitter 
{
	friend class ParticleSystem;
public:
	// Default constructor with no assigned type or anything. MUST overload GetNewParticle if so!
	ParticleEmitter();
	/// Virtual destructor for subclasses.
	virtual ~ParticleEmitter();
	ParticleEmitter(const Contour & contour);
    ParticleEmitter(Mesh * mesh);
	/// See EmitterTypes above.
    ParticleEmitter(int type);
	/// Point-based directional emitter
	ParticleEmitter(const Vector3f & point, const Vector3f & direction);
	/// Point-based circular emitter
	ParticleEmitter(const Vector3f & point);
	/// Triangle-list-based emitter
	ParticleEmitter(List<Triangle> tris);
	/// Initializes and allocates stuff.
	virtual void Initialize();

	/// Attaches this emitter to target system.
	virtual void AttachTo(ParticleSystem * ps);

	/// Called each frame to update position and other according to automations (if any)
	void Update();
	/// Query how many particles this emitter wants to emit, considering the time that has elapsed.
	int ParticlesToEmit(float timeInSeconds);
	/// Default new particle.
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity);
	/// Extended particle emission. lds is lifetime (current), duration (max before death), scale (xy, stored in zw).
#ifdef USE_SSE
	virtual bool GetNewParticle(SSEVec & position, SSEVec & velocity, SSEVec & color, SSEVec & lds);
#endif
	virtual bool GetNewParticle(Vector3f & position, Vector3f & velocity, float & scale, float & lifeTime, Vector4f & color);
	void SetParticleLifeTime(float timeInSeconds);
	void SetEmissionVelocity(float vel);
	void SetParticlesPerSecond(int num);
	void SetColor(const Vector4f & color);
	void SetScale(float scale);


	/// Constant emission every specified interval.
	int constantEmission;
	bool instantaneous;
	/// Default 1000?
	float particlesPerSecond;
	/// If specified (non 0), the emitter will be deleted after the specified time has elapsed (while emitting).
	int deleteAfterMs;
	/// For temporary disabling. True by default
	bool enabled;

	
	/// For re-work.
	bool newType;
	Emitter positionEmitter;
	Emitter velocityEmitter;


	/** Default true. If the correspond Set- function is called before, these will be set to false.
		Inheritance allows a particle system to transfer attributes so that not every emitter has 
		to be re-set with the same variables every time they are to be attached.
	*/
	bool inheritColor,
		inheritEmissionVelocity,
		inheritParticleLifeTime,
		inheritScale,
		inheritEmissionsPerSecond;

	/// For point-based emitters.
	Vector3f position, direction;
	/// For line-, plane- and cube-based emitters.
	Vector3f upVec, leftVec;
	/// Position will be relative to the tracked entity, if any.
	EntitySharedPtr entityToTrack;
	Vector3f positionOffset;
	/// That it is currently attached to.
	List<ParticleSystem*> particleSystems;

	// o.o
	float emissionVelocity;
	// Particle scale
	float scale;
	/// Particle life time in.. seconds?
	float particleLifeTime;
	/// Default (1,1,1,1) or the same as the particle system it is used in.
	Vector4f color;

protected:

	// Time we started emitting particles. Used to calculate how many particles we should emit next!
	float secondsEmitted;
	// Total emissions performed.
	int64 emissions;

	/** Belonging particle system. An emitter almost always belongs to just 1 system. 
		Re-write and virtualize the system in sub-classes if you want to be obnoxious about this.
	*/
	ParticleSystem * ps;

	/// Calculate in ParticlesToEmit, decremented each time GetNewParticle is called.
	int toSpawn;


	/// Current elapsed duration for the emitter.
	int elapsedDurationMs;

    int shapeType;
    Mesh * m;
	Contour contour;
	// See enum above.
	int type;
	/// o.o
	List<Triangle> tris;
	
	// o.o
	Random random;
};

#endif
