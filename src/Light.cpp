/// Emil Hedemalm
/// 2014-01-19
/// Simple structure for a light (any kind)

#include "Light.h"
#include <fstream>


Light::Light(){
	name = "DefaultLight"; 
	attenuation.x = 1.0f; 
	type = 1;
	lastUpdate = 0;
	spotExponent = 5;
	spotCutoff = 30;
	data = NULL;
	owner = NULL;
}

Light::Light(const Light & otherLight){
	type = otherLight.type;
	ambient = otherLight.ambient;
	diffuse = otherLight.diffuse;
	specular = otherLight.specular;
	position = otherLight.position;
	attenuation = otherLight.attenuation;
	currentlyActive = otherLight.currentlyActive;
	lastUpdate = otherLight.lastUpdate;
	spotDirection = otherLight.spotDirection;
	spotExponent = otherLight.spotExponent;
	spotCutoff = otherLight.spotCutoff;
	name = otherLight.name;
	data = otherLight.data;
	owner = NULL;
}


// Versions
#define LIGHT_VERSION_0 0// Initial version.
int lightVersion = LIGHT_VERSION_0;

/// Writes to file stream.
void Light::WriteTo(std::fstream & file){
	// Write version
	file.write((char*)&lightVersion, sizeof(int));
	// Write name
	name.WriteTo(file);
	// Write type
	file.write((char*)&type, sizeof(int));

	// Write spotlight stats
	file.write((char*)&spotCutoff, sizeof(float));
	file.write((char*)&spotExponent, sizeof(int));
	
	// Write all them vectors.
	ambient.WriteTo(file);
	diffuse.WriteTo(file);
	specular.WriteTo(file);
	position.WriteTo(file);
	attenuation.WriteTo(file);
	spotDirection.WriteTo(file);
	
}
/// Reads from file stream.
void Light::ReadFrom(std::fstream & file){
	// Read version
	int version;
	file.read((char*)&version, sizeof(int));
	assert(version == LIGHT_VERSION_0);
	// Read name
	name.ReadFrom(file);
	// Read type
	file.read((char*)&type, sizeof(int));
	// Read spotlight stats
	file.read((char*)&spotCutoff, sizeof(float));
	file.read((char*)&spotExponent, sizeof(int));

	// Read all them vectors.
	ambient.ReadFrom(file);
	diffuse.ReadFrom(file);
	specular.ReadFrom(file);
	position.ReadFrom(file);
	attenuation.ReadFrom(file);
	spotDirection.ReadFrom(file);
}


void Light::SetName(const String newName){ 
	assert(abs(newName.Length()) < 5000);
	name = newName; 
}