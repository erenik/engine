/// Emil Hedemalm
/// 2015-02-08
/// A custom particle-system which encompasses a multitude of precipitation types, as well as interaction with weather, wind and sun, via the WeatherSystem.

#include "PrecipitationSystem.h"
#include "WeatherSystem.h"
#include "Window/Window.h"
#include "Viewport.h"
#include "Graphics/Camera/Camera.h"

#include "Timer/Timer.h"
#include "Graphics/FrameStatistics.h"

PrecipitationSystem::PrecipitationSystem(WeatherSystem * weatherSystem)
: ParticleSystem("PrecipitationSystem", false), weather(weatherSystem)
{
	globalEmitter.type = EmitterType::PLANE_XZ;
	globalEmitter.SetScale(150.f); // Create a 500 wide and long plane to spawn along.

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
	Timer timer;
	timer.Start();
#ifdef USE_SSE
	__m128 sseTime = _mm_load1_ps(&timeInSeconds);
#endif
	/// Move/Process all alive particles
	const Vector3f wind = weather->globalWind;
	for (int i = 0; i < aliveParticles; ++i)
	{
#ifdef SSE_PARTICLES
		positionsSSE[i].data = _mm_add_ps(positionsSSE[i].data, _mm_mul_ps(sseTime, _mm_add_ps(velocitiesSSE[i].data, wind.data)));
#else // Not SSE_PARTICLES
		// Using SSE commands straight away reduced computation time to like 1 ms from 150ms when many particles were around (towards 500k somewhere)
#ifdef USE_SSE
		positions[i].data = _mm_add_ps(positions[i].data, _mm_mul_ps(sseTime, _mm_add_ps(velocities[i].data, weather->globalWind.data)));
#else
		positions[i] += (velocities[i] + weather->globalWind)* timeInSeconds;
#endif // USE_SSE
#endif // SSE_PARTICLES
	}
	timer.Stop();
	FrameStats.particleProcessingIntegrate += timer.GetMs();

	timer.Start();
	for (int i = 0; i < aliveParticles; ++i)
	{
#ifdef SSE_PARTICLES
		ldsSSE[i].y += timeInSeconds;	
#else // Not SSE_PARTICLES
		// No velocity decay.
		lifeDurations[i] += timeInSeconds;
#endif // SSE_PARTICLES
	}
	timer.Stop();
	FrameStats.particleProcessingOldify = timer.GetMs();


	timer.Start();
	for (int i = 0; i < aliveParticles; ++i)
	{
#ifdef SSE_PARTICLES
		if (ldsSSE[i].y > ldsSSE[i].x)
		{
			int lastIndex = aliveParticles - 1;
			positionsSSE[i] = positionsSSE[lastIndex];
			velocitiesSSE[i] = velocitiesSSE[lastIndex];
			colorsSSE[i] = colorsSSE[lastIndex];
			ldsSSE[i] = ldsSSE[lastIndex];
			// Decrement i so we don't skip processing of the one we moved back.
			--i;
			// Decrement alive particles.
			--aliveParticles;
		}			
#else // Not SSE_PARTICLES
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
#endif
	}
	timer.Stop();
	FrameStats.particleProcessingRedead += timer.GetMs();
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
	float area = globalEmitter.SurfaceArea();
	int dropsPerSecond = area * rainAmount;
	int dropsToEmit = dropsPerSecond * timeInMs * 0.001f;

	Vector3f rainSpeed = Vector3f(0,-emissionVelocity,0);
	int seconds = altitude / AbsoluteValue(rainSpeed.y) + 3;

	/// Assume 1 primary global camera.
	Vector3f cameraPosition;
	Window * window = MainWindow();
	if (window)
	{
		Camera * camera = window->MainViewport()->camera;
		if (camera)
		{
			cameraPosition = camera->Position();
		}
	}


	Vector3f cameraOffset = cameraPosition; // + camera->Velocity();
	Vector3f positionOffsetDueToWind = -weather->globalWind * seconds;
	Vector3f allPositionOffsets = cameraOffset + positionOffsetDueToWind + Vector3f(0,altitude,0);
	Vector3f position;
	Vector3f velocity;
	Vector2f pScale;
	float lifeTime;
	Vector4f pColor;

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
#ifdef SSE_PARTICLES
		// Position based on the global emitter (default an XZ plane.
		globalEmitter.Position(position);
		// Add random from 0 to 1.0 to get some variation in height?
		position.y += rand()*oneDivRandMaxFloat;
		// Add all offsets, such as altitude, camera position and offset due to wind.
		position += allPositionOffsets;
		lifeTime = seconds;
		/// Big rain (5 mm), 9 m/s, drizzle (0.5mm), 2 m/s.
		velocity = rainSpeed;

		/// Copy over data.
		positionsSSE[freeIndex].data = position.data;
		colorsSSE[freeIndex].data = color.data;
		velocitiesSSE[freeIndex].data = rainSpeed.data;
		float floats[4] = {lifeTime , 0, scale.x, scale.y};
		ldsSSE[freeIndex].data = _mm_loadu_ps(floats);
		/*
		ldsSSE[freeIndex].x = lifeTime;
		ldsSSE[freeIndex].y = 0;
		ldsSSE[freeIndex].z = scale.x;
		ldsSSE[freeIndex].w = scale.y;
		*/
		// Increment amount of living particles.
		++aliveParticles;
	
#else // Not SSE_PARTICLES
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
		position += cameraPosition;

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
#endif
	}
}



