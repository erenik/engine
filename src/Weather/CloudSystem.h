/// Emil Hedemalm
/// 2015-02-22
/// Cloud system. In addition to handling particle simulation for clouds which are nearby 
/// (mainly above the playing field), it also contains functionality for handling
/// sprite-based clouds at distances far away, as well as shadow functions which affect
/// The shadow mapping pass when rendering the sun's lighting pass.

#ifndef CLOUD_SYSTEM_H
#define CLOUD_SYSTEM_H

#include "Graphics/Particles/ParticleSystem.h"

class WeatherSystem;

class CloudSystem : public ParticleSystem
{
public:
	CloudSystem(WeatherSystem * weatherSystem);
	virtual ~CloudSystem();
	/// Integrates all particles.
	virtual void ProcessParticles(float & timeInSeconds);
	/// Spawns new particles depending on given settings here and within the weather-system.
	virtual void SpawnNewParticles(int & timeInMs);

//	virtual void Render(GraphicsState & graphicsState);

	/// Base altitude for the clouds. This would be lower for a coastal region and higher for a mountainous region.
	float altitude;
	/// Amount for the global precipitation. Default 0. 1 is 1 rain-drop or snowflake per square-meter (assuming quadratic emitter area).
	float cloudAmount;
	/// Defines the area of the global precipitation's spawning location possilbities.
	Emitter globalEmitter;
protected:
	/// For setting specific uniforms after most other properties have been set up.
	virtual void SetUniforms();

private:
	/// If no cloud-system, spawns globally, based on the given boundaries (.
	void SpawnNewGlobal(int timeInMs);
	WeatherSystem * weather;
};

#endif




