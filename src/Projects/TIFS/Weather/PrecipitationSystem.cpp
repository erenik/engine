/// Emil Hedemalm
/// 2015-02-08
/// A custom particle-system which encompasses a multitude of precipitation types, as well as interaction with weather, wind and sun, via the WeatherSystem.

#include "PrecipitationSystem.h"
#include "WeatherSystem.h"

PrecipitationSystem::PrecipitationSystem(WeatherSystem * weatherSystem)
: ParticleSystem("PrecipitationSystem", false), weather(weatherSystem)
{
	global.type = EmitterType::PLANE_XZ;
	global.SetScale(50.f); // Create a 500 wide and long plane to spawn along.

	altitude = 10.f;
	rainAmount = 0;
	scale = Vector2f(0.8f,1.f);
	this->color = Vector4f(0.9f,0.92f,0.95f,1.f);

	maxParticles = 500000;
}

PrecipitationSystem::~PrecipitationSystem()
{
}


/// Integrates all particles.
void PrecipitationSystem::ProcessParticles(float & timeInSeconds)
{
	/// Move/Process all alive particles
	for (int i = 0; i < aliveParticles; ++i)
	{
		positions[i] += (velocities[i] + weather->globalWind)* timeInSeconds;
		// No velocity decay.
//		velocities[i] *= velocityDecay;
		lifeDurations[i] += timeInSeconds;
		// If duration has elapsed life-time..
		if (lifeDurations[i] > lifeTimes[i])
		{
			int lastIndex = aliveParticles - 1;
			// Kill it, by moving in the last used data to replace it.
			positions[i] = positions[lastIndex];
			velocities[i] = velocities[lastIndex];
			lifeDurations[i] = lifeDurations[lastIndex];
			colors[i] = colors[lastIndex];
			lifeTimes[i] = lifeTimes[lastIndex];
			scales[i] = scales[lastIndex];

			// Decrement i so we don't skip processing of the one we moved back.
			--i;
			// Decrement alive particles.
			--aliveParticles;
		}
	}
}

/// Spawns new particles depending on given settings here and within the weather-system.
void PrecipitationSystem::SpawnNewParticles(int & timeInMs)
{
	if (emissionPaused)
		return;
	float timeInSeconds = (timeInMs % 200) * 0.001f;
	SpawnNewGlobal(timeInMs);
}


/// If no cloud-system, spawns globally, based on the given boundaries (.
void PrecipitationSystem::SpawnNewGlobal(int timeInMs)
{
	/// Require 20+ fps.
	timeInMs = timeInMs % 50;
	// Try and emit each particles that the emitter wants to emit.
	/// check size of the emitter.
	float area = global.SurfaceArea();
	int dropsPerSecond = area * rainAmount;
	int dropsToEmit = dropsPerSecond * timeInMs * 0.001f;

	Vector3f rainSpeed = Vector3f(0,-emissionVelocity,0);
	int seconds = altitude / AbsoluteValue(rainSpeed.y) + 3;

	for (int j = 0; j < dropsToEmit; ++j)
	{
		// Grab free index.
		int freeIndex = aliveParticles;
		// Skip if reaching max.
		if (freeIndex >= this->maxParticles)
		{
//				std::cout<<"\nEmitter unable to spawn particle. Max particles reached.";
			break;
		}
		Vector3f & position = positions[freeIndex];
		Vector3f & velocity = velocities[freeIndex];
		Vector2f & pScale = scales[freeIndex];
		float & lifeTime = lifeTimes[freeIndex];
		Vector4f & pColor = colors[freeIndex];

		// Position based on the global emitter (default an XZ plane.
		global.Position(position);
		// Add random from 0 to 1.0 to get some variation in height?
		position.y += rand()*oneDivRandMaxFloat;
		// Move up position?
		position.y += altitude;

		lifeTime = seconds;
		/// Big rain (5 mm), 9 m/s, drizzle (0.5mm), 2 m/s.
		velocity = rainSpeed;
		// Reset duration to 0 to signify that it is newly spawned.
		lifeDurations[freeIndex] = 0;
		// Increment amount of living particles.
		++aliveParticles;
		// Big scale and color for the time being.
		pScale = scale;
		pColor = color;
	}
}



