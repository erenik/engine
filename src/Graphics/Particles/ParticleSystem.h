// Emil Hedemalm
// 2013-07-14

#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

struct GraphicsState;
class Entity;
class Texture;

#include "ParticleEmitter.h"
#include <String/AEString.h>


class ParticleSystem {
	friend class GraphicsManager;
public:
    ParticleSystem(String type);
    virtual ~ParticleSystem();
    virtual void Process(float timeInSeconds);
    virtual void Render(GraphicsState * graphicsState);
    virtual void PrintData();
    virtual void AttachTo(Entity * entity, Matrix4f relativePosition);
    virtual void SetPosition(Matrix4f relativePosition);
    /// Emission control.
    virtual void PauseEmission();
    virtual void ResumeEmission();
    virtual void SetColor(Vector4f color);

	/// Sets the emitter to be a contour. Default before calling this is a point or a plane.
	virtual void SetEmitter(Contour contour);
	virtual void SetEmitter(List<ParticleEmitter*> newEmitters);

	/// Name & type
	String type;
	String name;

	/// Time control
	bool emissionPaused;

	/// Settings
	bool pointsOnly;
	Texture * diffuse;
	int maxParticles;
	int emissionsPerSecond;
	// To easily toggle between, say, 1.0 and 0.0, amount of emitted particles 
	float emissionRatio; 
	// Relative emission velocity from 0.0 to 1.0
	float emissionVelocity;

	/// Positioning
	/// If this is NULL, the relative model matrix will be in world coordinates. Simple as that!
	Entity * relativeTo;
	Matrix4f relativeModelMatrix;
	/// Before respawning/dying
	float maxRange, maxRangeSq;
	float maxLifeTime;
	float particleSize;

	/// Primary colors
	Vector4f color;

protected:

    /// Based on the optimization level, will probably be pow(0.5, optimizationLevel);
    float optimizationLevel;
    /// Based on maxParticles and the current optimization level.
    int particlesToProcess;

    /// Raw data
    float * lifeDuration, * lifeTime;
    Vector3f * positions;
    Vector3f * velocities;
    Vector4f * colors;

    // For getting new spawn positions
    List<ParticleEmitter*> emitters;

private:
	bool registeredForRendering;
};

#endif

