// Emil Hedemalm
/// 2014-06-27
/// Particle emitter.

#include "ParticleEmitter.h"
#include "ParticleSystem.h"
#include "Random/Random.h"

#include "Entity/Entity.h"


float oneDivRandMaxFloat = 0;


Emitter::Emitter()
{
	up = Vector3f(0,1,0);
	left = Vector3f(-1,0,0);
	forward = Vector3f(0,0,-1);
}

// Scales all 3 base vectors.
void Emitter::Scale(float scale)
{
	up *= scale;
	left *= scale;
	forward *= scale;
}

/// Randomizes acordingly.
void Emitter::Position(Vector3f & vec)
{
	switch(type)
	{
		case EmitterType::LINE_Y:
			vec = (rand() * oneDivRandMaxFloat - 0.5f) * up;
			break;
		case EmitterType::PLANE_XY:
			vec = (rand() * oneDivRandMaxFloat - 0.5f) * left +
				(rand() * oneDivRandMaxFloat - 0.5f) * up;
			break;
		case EmitterType::PLANE_XZ:
			vec = (rand() * oneDivRandMaxFloat - 0.5f) * left +
				(rand() * oneDivRandMaxFloat - 0.5f) * forward;
			break;
		case EmitterType::CIRCLE_XY:
		{
			// Random angle.
			float angle = (rand() * oneDivRandMaxFloat * 2 * PI);
			vec = Vector3f(cos(angle), sin(angle), 0) * (rand() * oneDivRandMaxFloat);
			break;
		}
		default:
			assert(false);
	}
}

void Emitter::Velocity(Vector3f & vec)
{
	 return Position(vec);
}


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
	if (oneDivRandMaxFloat == 0)
		oneDivRandMaxFloat = 1.f / RAND_MAX;

	newType = false;

	entityToTrack = NULL;
	enabled = true;
	elapsedDurationMs = 0;
	deleteAfterMs = 0;
	particlesPerSecond = 1000;
	ps = NULL;


	instantaneous = false;
	constantEmission = 0;

	inheritColor = true;
	inheritEmissionVelocity = true;
	inheritParticleLifeTime = true;
	inheritScale = true;
	inheritEmissionsPerSecond = true;

	secondsEmitted = 0;
	emissions = 0;

	upVec = Vector3f(0,1,0);
	leftVec = Vector3f(-1,0,0);
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
		position = entityToTrack->position + positionOffset;
}

/// Query how many particles this emitter wants to emit, considering the time that has elapsed.
int ParticleEmitter::ParticlesToEmit(float timeInSeconds)
{
	if (instantaneous)
	{
		return constantEmission;
	}
	secondsEmitted += timeInSeconds;
	int64 totalParticlesShouldHaveEmittedByNow = secondsEmitted * particlesPerSecond;
	int particlesToEmit = totalParticlesShouldHaveEmittedByNow - emissions;
	if (particlesToEmit < 1)
		particlesToEmit = 1;
	emissions += particlesToEmit;
	return particlesToEmit;
}

/// Default new particle.
bool ParticleEmitter::GetNewParticle(Vector3f & particlePosition, Vector3f & particleVelocity)
{
	if (newType)
	{
		positionEmitter.Position(particlePosition);
		velocityEmitter.Velocity(particleVelocity);
		// Add own position to particle.
		particlePosition += position;
		particleVelocity *= emissionVelocity;
		return true;
	}

	switch(type)
	{
		case EmitterType::CONTOUR:
		{
			if (!contour.points.Size())
				return false;
			// Get a random index in the contour.
			int randomIndex = rand() % contour.points.Size();
			particlePosition = contour.points[randomIndex];
			// Get next point.
			Vector3f nextPoint = contour.points[(randomIndex+1) % contour.points.Size()];
			Vector3f pToNext = nextPoint - position;
			/// Set the point between this and the next point, actually!
			float r = (rand() % 100) * 0.01f;
			particlePosition += pToNext * r;
			// Use an arbitrary up-vector?
			Vector3f upVec(0,0,-1);
			Vector3f crossProduct = upVec.CrossProduct(pToNext);
			crossProduct.Normalize();
			particleVelocity = crossProduct;
			break;
		}
		/*
		case EmitterType::PLANE:
		{
			float up = rand() * oneDivRandMaxFloat - 0.5f;
			float left = rand() * oneDivRandMaxFloat - 0.5f;
			// o.o
			particlePosition = up * upVec + left * leftVec + this->position;
			particleVelocity = this->direction;
			break;
		}
		*/
		case EmitterType::POINT_DIRECTIONAL:
		{
			particlePosition = this->position;
			particleVelocity = this->direction;
			// Randomize it a bit.
			Vector3f upVec(0,0,-1);
			Vector3f crossProduct = particleVelocity.CrossProduct(upVec);
			crossProduct.Normalize();
			particleVelocity += crossProduct * (rand()% 100)*0.01f; 
			break;
		}
		case EmitterType::POINT_CIRCLE:
		{
			particlePosition = this->position;
			// Random angle.
			float angle = random.Randf() * 2 * PI;
			float x = cos(angle);
			float y = sin(angle);
			particleVelocity = Vector3f(x,y,0);
			break;
		}
		default:
			std::cout<<"\nINVALID PARTICLE EMITTER TYPE";
			assert(false);
	}
	particleVelocity *= emissionVelocity;
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

void ParticleEmitter::SetParticlesPerSecond(int num)
{
	particlesPerSecond = num;
	inheritEmissionsPerSecond = false;
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