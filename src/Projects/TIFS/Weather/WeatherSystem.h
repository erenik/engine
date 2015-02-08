/// Emil Hedemalm
/// 2015-02-08
/// Weather system, which combines weather-simulation, dynamic particle effects (rain, now, dust), static particle effects (puddles, ice, snow)
/// as well as sun-shadow systems.

#ifndef WEATHER_SYSTEM_H
#define WEATHER_SYSTEM_H

#include "MathLib.h"

class PrecipitationSystem;
class ParticleEmitter;

class Entity;

class WeatherSystem 
{
	friend class PrecipitationSystem;
public:
	WeatherSystem();
	virtual ~WeatherSystem();
	/// Allocates resources.
	void Initialize();
	/// Deallocates resources.
	void Shutdown();

	/// Starts the rain.
	void Rain(float amount);
	void Snow(float amount);
	/// Sets global wind velocity, affecting rain, snow, etc.
	void Wind(ConstVec3fr globalWind);

	/// Stops the wind, rain, etc.
	void Stop();

private:
	/// Rain-snow?
	PrecipitationSystem * precipitationSystem;
	ParticleEmitter * precipitationEmitter;
	Vector3f globalWind;
	/// The sun.
	Entity * sun;
};


#endif




