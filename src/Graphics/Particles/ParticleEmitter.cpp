// Emil Hedemalm
/// 2014-06-27
/// Particle emitter.

#include "ParticleEmitter.h"
#include "ParticleSystem.h"
#include "Random/Random.h"

#include "Entity/Entity.h"

ParticleEmitter::ParticleEmitter()
: type(EmitterType::NONE)
{
	Initialize();	
}

ParticleEmitter::~ParticleEmitter()
{
}

ParticleEmitter::ParticleEmitter(Contour contour)
	: contour(contour), type(EmitterType::CONTOUR)
{
	Initialize();
}  

ParticleEmitter::ParticleEmitter(Vector3f point, Vector3f direction)
	: position(point), direction(direction), type(EmitterType::POINT_DIRECTIONAL)
{
	Initialize();
	this->direction.Normalize();
}

ParticleEmitter::ParticleEmitter(int type)	
	: type(type)
{
	Initialize();
	switch(type)
	{
		case EmitterType::POINT_DIRECTIONAL:
		{
			direction = Vector3f(0,1,0);
			break;
		}
	}
	position = Vector3f();
	emissionVelocity = 1.f;
}

/// Point-based circular emitter
ParticleEmitter::ParticleEmitter(Vector3f point)
	: position(point), type(EmitterType::POINT_CIRCLE)
{
	Initialize();
}

void ParticleEmitter::Initialize()
{
	entityToTrack = NULL;
	enabled = true;
	elapsedDurationMs = 0;
	deleteAfterMs = 0;
	particlesPerSecond = 1000;
	ps = NULL;

	inheritColor = true;
	inheritEmissionVelocity = true;
	inheritParticleLifeTime = true;
	inheritScale = true;
	inheritEmissionsPerSecond = true;

	secondsEmitted = 0;
	emissions = 0;
}


/// Attaches this emitter to target system.
void ParticleEmitter::AttachTo(ParticleSystem * targetPS)
{
	this->ps = targetPS;
	// Extract default attributes, unless they have been state explicitly earlier!
	if (inheritColor)
		this->color = ps->color;
	if (inheritParticleLifeTime)
		this->particleLifeTime = ps->particleLifeTime;
	if (inheritEmissionVelocity)
		this->emissionVelocity = ps->emissionVelocity;
	if (inheritScale)
		this->scale = ps->particleSize;
	if (inheritEmissionsPerSecond)
		this->particlesPerSecond = ps->emissionsPerSecond;

	if (!particleSystems.Exists(targetPS))
		particleSystems.Add(targetPS);
}

/// Called each frame to update position and other according to automations (if any)
void ParticleEmitter::Update()
{
	if (entityToTrack)
		position = entityToTrack->position;
}

/// Query how many particles this emitter wants to emit, considering the time that has elapsed.
int ParticleEmitter::ParticlesToEmit(float timeInSeconds)
{
	secondsEmitted += timeInSeconds;
	int64 totalParticlesShouldHaveEmittedByNow = secondsEmitted * particlesPerSecond;
	int particlesToEmit = totalParticlesShouldHaveEmittedByNow - emissions;
	if (particlesToEmit < 1)
		particlesToEmit = 1;
	emissions += particlesToEmit;
	return particlesToEmit;
}

/// Default new particle.
bool ParticleEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity)
{
	switch(type)
	{
		case EmitterType::CONTOUR:
		{
			if (!contour.points.Size())
				return false;
			// Get a random index in the contour.
			int randomIndex = rand() % contour.points.Size();
			position = contour.points[randomIndex];
			// Get next point.
			Vector3f nextPoint = contour.points[(randomIndex+1) % contour.points.Size()];
			Vector3f pToNext = nextPoint - position;
			/// Set the point between this and the next point, actually!
			float r = (rand() % 100) * 0.01f;
			position += pToNext * r;
			// Use an arbitrary up-vector?
			Vector3f upVec(0,0,-1);
			Vector3f crossProduct = upVec.CrossProduct(pToNext);
			crossProduct.Normalize();
			velocity = crossProduct;
			break;
		}
		case EmitterType::POINT_DIRECTIONAL:
		{
			position = this->position;
			velocity = this->direction;
			// Randomize it a bit.
			Vector3f upVec(0,0,-1);
			Vector3f crossProduct = velocity.CrossProduct(upVec);
			crossProduct.Normalize();
			velocity += crossProduct * (rand()% 100)*0.01f; 
			break;
		}
		case EmitterType::POINT_CIRCLE:
		{
			position = this->position;
			// Random angle.
			float angle = random.Randf() * 2 * PI;
			float x = cos(angle);
			float y = sin(angle);
			velocity = Vector3f(x,y,0);
			break;
		}
		default:
			std::cout<<"\nINVALID PARTICLE EMITTER TYPE";
			assert(false);
	}
	velocity *= emissionVelocity;
	return true;
}
	

/// Stuff.
bool ParticleEmitter::GetNewParticle(Vector3f & position, Vector3f & velocity, float & newParticleScale, float & lifeTime, Vector4f & newParticleColor)
{
	GetNewParticle(position, velocity);
	lifeTime = particleLifeTime;
	newParticleColor = this->color;
	newParticleScale = scale;
	return true;
}

void ParticleEmitter::SetParticleLifeTime(float timeInSeconds)
{
	this->particleLifeTime = timeInSeconds;
	inheritParticleLifeTime = false;
}

void ParticleEmitter::SetEmissionVelocity(float vel)
{
	emissionVelocity = vel;
	inheritEmissionVelocity = false;
}

void ParticleEmitter::SetColor(Vector4f newColor)
{
	this->color = newColor;
	inheritColor = false;
}

void ParticleEmitter::SetScale(float newScale)
{
	this->scale = newScale;
	inheritScale = false;
}