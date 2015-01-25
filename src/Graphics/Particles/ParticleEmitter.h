/// Emil Hedemalm
/// 2014-06-27
/// Particle emitter.

#ifndef PARTICLE_EMITTER_H
#define PARTICLE_EMITTER_H

#include "PhysicsLib/Shapes/Contour.h"
#include "MathLib.h"
#include "Random/Random.h"

class Mesh;
class Entity;

/// Pre-calculated
extern float oneDivRandMaxFloat;

namespace EmitterType {
	enum emitterTypes
	{
		DEFAULT,
		CONTOUR,
		POINT_DIRECTIONAL,
		POINT_CIRCLE,


		/// New types using the Emitter sub-system.
		LINE_Y,
		PLANE_XY,
		PLANE_XZ, // Plane in XZ (left-right, forward-backward) axes.
		CIRCLE_XY,
		VECTOR, // Constant vector.

		NONE, // Default constructor.
	}; 
};

/// Based on types above, produces a given result.
class Emitter 
{
public:
	Emitter();
	// Sets default up/left/forward vectors.
	void DefaultVectors();
	// Scales all 3 base vectors.
	void Scale(float scale);
	// Sets scale of all 3 base vectors.
	void SetScale(float scale);
	/// Randomzies acordingly.
	void Position(Vector3f & vec);
	void Velocity(Vector3f & vec);
	int type;
	/// For later, Linear (pure random), weighted (constant and random part).
	int distribution;
	/// For configuring it.
	Vector3f up, left, forward;
	// Constant vector.
	Vector3f vec;
};


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
	/// Extended particle emission.
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
	Entity * entityToTrack;
	Vector3f positionOffset;
	/// That it is currently attached to.
	List<ParticleSystem*> particleSystems;
private:

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

	// o.o
	float emissionVelocity;
	// Particle scale
	float scale;

	/// Current elapsed duration for the emitter.
	int elapsedDurationMs;
	/// Particle life time in.. seconds?
	float particleLifeTime;

    int shapeType;
    Mesh * m;
	Contour contour;
	// See enum above.
	int type;
	
	/// Default (1,1,1,1) or the same as the particle system it is used in.
	Vector4f color;

	// o.o
	Random random;
};

#endif
