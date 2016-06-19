/// Emil Hedemalm
/// 2015-02-08
/// Weather system, which combines weather-simulation, dynamic particle effects (rain, now, dust), static particle effects (puddles, ice, snow)
/// as well as sun-shadow systems.

#ifndef WEATHER_SYSTEM_H
#define WEATHER_SYSTEM_H

#include "MathLib.h"
#include "Time/Time.h"

class CloudSystem;
class PrecipitationSystem;
class ParticleEmitter;
class Message;
class Light;
class Entity;
class AABB;

class WeatherSystem 
{
	friend class PrecipitationSystem;
	friend class CloudSystem;
public:
	WeatherSystem();
	virtual ~WeatherSystem();
	/// Allocates resources. <- does what exactly?
	void Initialize();
	/// Sets active area for shadow mapping etc.
	void SetActiveArea(AABB & activeArea);
	/// Deallocates resources.
	void Shutdown();

	/// o.o;
	void Process(int timeInMs);
	void ProcessMessage(Message * message);

	/// Starts the rain.
	void Rain(float amount);
	void Snow(float amount);
	/// Get current sun time.
	float SunTime() const;
	/// Hour in 24-hour format, 0.0 to 24.0.
	void SetSunTime(float hour);
	/// Sets global wind velocity, affecting rain, snow, etc.
	void Wind(ConstVec3fr globalWind);

	/// Stops the wind, rain, etc.
	void Stop();

private:
	/// Rain-snow?
	CloudSystem * cloudSystem;
	PrecipitationSystem * precipitationSystem;
	ParticleEmitter * precipitationEmitter;
	Vector3f globalWind;
	/// The sun.
	Entity * sun;
	bool initialized;

	// o.o 
	float sunHour;
	float sunDistance;
	float sunHours;
	float sunUp;
	Light * sunLight;
	float inGameSecondsPerSecond;
	/// Or weather-time?
	Time inGameTime;

	Vector3f smoothedSunPosition;
	Vector4f sunColor;
	Vector4f smoothedSunColor;
	/// Ambient color, used to paint the sky-box by default too.
	Vector3f smoothedAmbience;
};


#endif




