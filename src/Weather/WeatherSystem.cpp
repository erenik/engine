/// Emil Hedemalm
/// 2015-02-08
/// Weather system, which combines weather-simulation, dynamic particle effects (rain, now, dust), static particle effects (puddles, ice, snow)
/// as well as sun-shadow systems.

#include "WeatherSystem.h"
#include "PrecipitationSystem.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMParticles.h"
#include "Graphics/Messages/GMLight.h"
#include "Message/Message.h"
#include "Light.h"

WeatherSystem::WeatherSystem()
{
	precipitationSystem = NULL;
	precipitationEmitter = NULL;
	sun = NULL;
	inGameTime = Time(TimeType::MILLISECONDS_NO_CALENDER);
	inGameSecondsPerSecond = 1.f;
	sunHours = 14.f;
	sunDistance = 50.f;
}
WeatherSystem::~WeatherSystem()
{
	Stop();
}
/// Allocates resources.
void WeatherSystem::Initialize()
{
	precipitationSystem = new PrecipitationSystem(this);
	GraphicsMan.QueueMessage(new GMRegisterParticleSystem(precipitationSystem, true));
	sunLight = new Light("SunLight");
	sunLight->position = Vector3f(0, 50.f, 0);
	// Add a light?
	sunLight->shadowMapFarplane = 55.f;
	sunLight->shadowMapZoom = 50.f;
	sunLight->type = LightType::DIRECTIONAL; // Orthogonal.
	sunLight->castsShadow = true;
	sunLight->diffuse = sunLight->specular = Vector4f(1,1,1,1);
	GraphicsMan.QueueMessage(new GMAddLight(sunLight));
	// Set ambient light.
	GraphicsMan.QueueMessage(new GMSetAmbience(Vector3f(0.5f,0.5f,0.5f)));
}

void WeatherSystem::Shutdown()
{
	GraphicsMan.QueueMessage(new GMUnregisterParticleSystem(precipitationSystem, true));
}

void WeatherSystem::Process(int timeInMs)
{
	inGameTime.AddMs(timeInMs * this->inGameSecondsPerSecond);
	// Get hour?
	int hour = inGameTime.Hour();
	int minute = inGameTime.Minute();
	float hours = hour + minute / 60.f;
	SetSunTime(hours);
}

void WeatherSystem::ProcessMessage(Message * message)
{
	String msg = message->msg;
	switch(message->type)
	{
		case MessageType::STRING:
		{
			if (msg.StartsWith("Wind("))
			{
				String vecStr = msg.Tokenize("()")[1];
				globalWind.ReadFrom(vecStr);
			}
			else if (msg.StartsWith("PrecipitationSpawnArea"))
			{
				String str = msg.Tokenize("()")[1];
				precipitationSystem->globalEmitter.SetScale(str.ParseFloat());
			}
			else if (msg.StartsWith("PrecipitationAltitude"))
			{
				String str = msg.Tokenize("()")[1];
				precipitationSystem->altitude = str.ParseFloat();
			}
			else if (msg.Contains("SunDistance"))
			{
				sunDistance = msg.Tokenize("() ")[1].ParseFloat();
			}
			else if (msg.Contains("SunUp"))
			{
				sunUp = msg.Tokenize("() ")[1].ParseFloat();
			}
			else if (msg.Contains("SunHours"))
			{
				sunHours = msg.Tokenize("() ")[1].ParseFloat();
			}
			else if (msg.Contains("SetDayCycle"))
			{
				String str = msg.Tokenize("()")[1];
				// Parse number?
				float totalTimeSeconds = str.ParseFloat();
				if (str.Contains("minute"))
					totalTimeSeconds *= 60;
				if (str.Contains("hour"))
					totalTimeSeconds *= 3600;
				if (totalTimeSeconds < 1)
					totalTimeSeconds = 1.f;
				inGameSecondsPerSecond = (24 * 3600) / totalTimeSeconds;
			}
			else if (msg.Contains("SetSunTime"))
			{
				List<String> tokens = msg.Tokenize("(:)");
				if (tokens.Size() < 3)
					return;
				String hourStr = tokens[1];
				int hour = hourStr.ParseInt();
				int minute = tokens[2].ParseInt();
				// Set sun position?
				inGameTime.SetHour(hour);
				inGameTime.SetMinute(minute);
			}
			else if (msg.Contains("Rain"))
			{
				float amount = msg.Tokenize("(,)")[1].ParseFloat();
				Rain(amount);
			}
			else if (msg.Contains("Snow"))
			{
				float amount = msg.Tokenize("(,)")[1].ParseFloat();
				Snow(amount);
			}
		}
	}
}

/// Starts the rain.
void WeatherSystem::Rain(float amount)
{
	precipitationSystem->SetAlphaDecay(DecayType::NONE);	
	std::cout<<"\nWeather: Rain set to "<<amount;
	precipitationSystem->rainAmount = amount;
	precipitationSystem->scale = Vector2f(0.02f,0.12f);
	precipitationSystem->color = Vector4f(0.9f,0.92f,0.94f,1);
	precipitationSystem->emissionVelocity = 9.f;
}
void WeatherSystem::Snow(float amount)
{
	precipitationSystem->SetAlphaDecay(DecayType::NONE);	
	std::cout<<"\nWeather: Snow set to "<<amount;
	precipitationSystem->rainAmount = amount;
	precipitationSystem->scale = Vector2f(0.04f,0.04f);
	precipitationSystem->color = Vector4f(1,1,1,1);
	precipitationSystem->emissionVelocity = 2.f;
}

/// Hour in 24-hour format.
void WeatherSystem::SetSunTime(float hour)
{
	float hoursIntoLight = hour - sunUp;
	Angle angle(hoursIntoLight / sunHours * PI);
	float cosine = angle.Cosine();
	float sine = angle.Sine();
	// Get an angle?
	Vector3f position(cosine, sine, 0);
	// Derive color?
	sunLight->castsShadow = position.y > 0;
	Vector4f color;
	Vector4f yellow = Vector4f(1,1,0.9f,1.f);
	Vector4f red = Vector4f(3.f,0.5f,0.2f,1.f);
	if (sine > 0.f)
	{
		// Go towards red?
		color = sine * yellow + (1 - sine) * red;
		// Once approaching 0, fade it?
		if (sine < 0.05f)
		{
			color *= sine / 0.05;
		}
	}
	else {
		color = Vector4f(0,0,0,0);
	}

	/// Place sun 50 away?
	position *= sunDistance;
	// Set position.
	GraphicsMan.QueueMessage(new GMSetLight(sunLight, LightTarget::POSITION, position));
	GraphicsMan.QueueMessage(new GMSetLight(sunLight, LightTarget::COLOR, color));
}

/// Sets global wind velocity, affecting rain, snow, etc.
void WeatherSystem::Wind(ConstVec3fr globalWind)
{
	this->globalWind = globalWind;
}

/// Stops the wind, rain, etc.
void WeatherSystem::Stop()
{
	precipitationSystem->PauseEmission();
}


