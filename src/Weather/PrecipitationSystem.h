/// Emil Hedemalm
/// 2015-02-08
/// A custom particle-system which encompasses a multitude of precipitation types, as well as interaction with weather, wind and sun, via the WeatherSystem.

/// http://en.wikipedia.org/wiki/Rain
/*
	Raindrops impact at their terminal velocity, which is greater for larger drops due to their larger mass to drag ratio. At sea level and without wind, 0.5 mm (0.020 in) drizzle impacts at 2 m/s (6.6 ft/s) or 7.2 km/h (4.5 mph), while large 5 mm (0.20 in) drops impact at around 9 m/s (30 ft/s) or 32 km/h (20 mph).[32]
	Rain falling on loosely packed material such as newly fallen ash can produce dimples that can be fossilized.[33] The air density dependence of the maximum raindrop diameter together with fossil raindrop imprints has been used to constrain the density of the air 2.7 billion years ago.[34]
	The sound of raindrops hitting water is caused by bubbles of air oscillating underwater.[35][36]
	The METAR code for rain is RA, while the coding for rain showers is SHRA.[37]
*/


#ifndef PRECIPITATION_SYSTEM_H
#define PRECIPITATION_SYSTEM_H

#include "Graphics/Particles/ParticleSystem.h"

class WeatherSystem;

class PrecipitationSystem : public ParticleSystem
{
public:
	PrecipitationSystem(WeatherSystem * weatherSystem);
	virtual ~PrecipitationSystem();
	/// Integrates all particles.
	virtual void ProcessParticles(float & timeInSeconds);
	/// Spawns new particles depending on given settings here and within the weather-system.
	virtual void SpawnNewParticles(int & timeInMs);

	/// Altitude for the global precipitation.
	float altitude;
	/// Amount for the global precipitation. Default 0. 1 is 1 rain-drop or snowflake per square-meter (assuming quadratic emitter area).
	float rainAmount;
	/// Defines the area of the global precipitation's spawning location possilbities.
	Emitter globalEmitter;
private:
	/// If no cloud-system, spawns globally, based on the given boundaries (.
	void SpawnNewGlobal(int timeInMs);
	WeatherSystem * weather;
};

#endif
