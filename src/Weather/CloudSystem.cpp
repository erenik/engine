/// Emil Hedemalm
/// 2015-02-22
/// Cloud system. In addition to handling particle simulation for clouds which are nearby 
/// (mainly above the playing field), it also contains functionality for handling
/// sprite-based clouds at distances far away, as well as shadow functions which affect
/// The shadow mapping pass when rendering the sun's lighting pass.

#include "CloudSystem.h"
#include "WeatherSystem.h"
#include "Window/Window.h"
#include "Viewport.h"
#include "Graphics/Camera/Camera.h"

#include "Timer/Timer.h"
#include "Graphics/FrameStatistics.h"

#include "Shader.h"
#include "File/LogFile.h"
#include "ShaderManager.h"
#include "Graphics/OpenGL.h"
#include "GraphicsState.h"

CloudSystem::CloudSystem(WeatherSystem * weatherSystem)
: ParticleSystem("CloudSystem", false), weather(weatherSystem)
{
	globalEmitter.type = EmitterType::PLANE_XZ;
	globalEmitter.SetScale(1500.f); // Create a 500 wide and long plane to spawn along.

	altitude = 1000.f;
	cloudAmount = 0;
	scale = Vector2f(1.f,1.f) * 50.f;
	this->color = Vector4f(0.9f,0.92f,0.95f,1.f);
	blendFuncDest = GL_ONE_MINUS_SRC_ALPHA;

	scaleVariance = 0.2f;
	maxParticles = 500000;
	cloudSpeed = Vector3f(40,0,0);

	shaderName = "CloudParticles";
	modelName = "sphere";
}

CloudSystem::~CloudSystem()
{
}


/// Integrates all particles.
void CloudSystem::ProcessParticles(float & timeInSeconds)
{
	Timer timer;
	timer.Start();
#ifdef USE_SSE
	__m128 sseTime = _mm_load1_ps(&timeInSeconds);
#endif
	/// Move/Process all alive particles
	Vector3f wind = weather->globalWind + cloudSpeed;
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
	FrameStats.particleProcessingOldify = (float)timer.GetMs();


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
#endif
	}
	timer.Stop();
	FrameStats.particleProcessingRedead += timer.GetMs();
}

/// Spawns new particles depending on given settings here and within the weather-system.
void CloudSystem::SpawnNewParticles(int & timeInMs)
{
	if (emissionPaused)
		return;
	float timeInSeconds = (timeInMs % 200) * 0.001f;
	SpawnNewGlobal(timeInMs);
}

/// If no cloud-system, spawns globally, based on the given boundaries (.
void CloudSystem::SpawnNewGlobal(int timeInMs)
{
	/// Require 20+ fps.
	timeInMs = timeInMs % 50;
	// Try and emit each particles that the emitter wants to emit.
	/// check size of the emitter.
	float area = globalEmitter.SurfaceArea();
	int cloudiclesPerSecond = (int) (area * cloudAmount);
	int cloudiclesToEmit = (int) (cloudiclesPerSecond * timeInMs * 0.001f);

	Vector3f positionOffsetDueToWind = -weather->globalWind * 0;
	Vector3f allPositionOffsets = positionOffsetDueToWind + Vector3f(0,altitude,0);
	Vector3f position;
	Vector3f velocity;
	Vector2f pScale;
	float lifeTime;
	Vector4f pColor;


	for (int j = 0; j < cloudiclesToEmit; ++j)
	{
		// Grab free index.
		int freeIndex = aliveParticles;
		// Skip if reaching max.
		if (freeIndex >= this->maxParticles)
		{
			break;
		}
#ifdef SSE_PARTICLES
		// Position based on the global emitter (default an XZ plane.
		globalEmitter.Position(position);
		// Add random from 0 to 1.0 to get some variation in height?
		position.y += rand()*oneDivRandMaxFloat * scale.x;
		// Add all offsets, such as altitude, camera position and offset due to wind.
		position += allPositionOffsets;
		/// Big rain (5 mm), 9 m/s, drizzle (0.5mm), 2 m/s.

		lifeTime = particleLifeTime;
		Vector2f cloudScale = scale * (1 + cloudRand.Randf(scaleVariance));
		float floats[4] = {lifeTime , 0, scale.x, scale.y};


		/// Copy over data.
		positionsSSE[freeIndex].data = position.data;
		colorsSSE[freeIndex].data = color.data;
		velocitiesSSE[freeIndex].data = velocity.data;
		ldsSSE[freeIndex].data = _mm_loadu_ps(floats);
		// Increment amount of living particles.
		++aliveParticles;
	
#else // Not SSE_PARTICLES
#endif
	}
}

/// For setting specific uniforms after most other properties have been set up.
void CloudSystem::SetUniforms()
{

}



