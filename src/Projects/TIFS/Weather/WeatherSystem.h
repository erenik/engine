/// Emil Hedemalm
/// 2015-02-08
/// Weather system, which combines weather-simulation, dynamic particle effects (rain, now, dust), static particle effects (puddles, ice, snow)
/// as well as sun-shadow systems.

#ifndef WEATHER_SYSTEM_H
#define WEATHER_SYSTEM_H

#include "MathLib.h"

class PrecipitationSystem;
class ParticleEmitter;
class Message;
class Light;
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

	void ProcessMessage(Message * message);

	/// Starts the rain.
	void Rain(float amount);
	void Snow(float amount);
	/// Hour in 24-hour format.
	void SetSunTime(int hour);
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
	// o.o 
	Light * sunLight;
};


#endif




