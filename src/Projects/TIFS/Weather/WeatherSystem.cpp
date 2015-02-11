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
	sunLight = new Light();
	// Add a light?
	sunLight->type = LightType::DIRECTIONAL; // Orthogonal.
	sunLight->castsShadow = true;
	GraphicsMan.QueueMessage(new GMAddLight(sunLight));
}

void WeatherSystem::Shutdown()
{
	GraphicsMan.QueueMessage(new GMUnregisterParticleSystem(precipitationSystem, true));
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
			else if (msg.Contains("SetSunTime"))
			{
				List<String> tokens = msg.Tokenize("()");
				if (tokens.Size() < 2)
					return;
				String hourStr = tokens[1];
				int hour = hourStr.ParseInt();
				// Set sun position?
				SetSunTime(hour);
			}
			else if (msg.Contains("Rain"))
			{
				float amount = msg.Tokenize(" ")[1].ParseFloat();
				Rain(amount);
			}
			else if (msg.Contains("Snow"))
			{
				float amount = msg.Tokenize(" ")[1].ParseFloat();
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
void WeatherSystem::SetSunTime(int hour)
{
	float x,y,z;
	x = 1;
	y = 1;
	z = 1;
	sunLight->position = Vector3f(x,y,z);
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


