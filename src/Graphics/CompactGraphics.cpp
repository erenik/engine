// Emil Hedemalm
// 2013-07-20

#include "CompactGraphics.h"
#include "GraphicsProperty.h"
#include <fstream>

CompactGraphics::CompactGraphics(){
	flags = NULL;
	cGraphicEffects = NULL;
	cParticleSystems = NULL;
	cLights = NULL;
}
CompactGraphics::~CompactGraphics(){
	/*
	DELETE_LIST_IF_VALID(cGraphicEffects);
	DELETE_LIST_IF_VALID(cParticleSystems);
	DELETE_LIST_IF_VALID(cLights);
	*/
}
CompactGraphics::CompactGraphics(GraphicsProperty * graphicsProperty){
	flags = graphicsProperty->flags;
	cGraphicEffects = NULL;
	cParticleSystems = NULL;
	cLights = NULL;
}

// Versions
#define CG_VERSION_1  0x00000001		// Current version only includes the flags :)
#define CG_CURRENT_VERSION	CG_VERSION_1

/// Extras, which all have own reader/writers.
#define HAS_GRAPHIC_EFFECTS		0x00000001
#define HAS_PARTICLE_SYSTEMS	0x00000002
#define HAS_LIGHTS				0x00000004

/// Reads data from file stream
bool CompactGraphics::ReadFrom(std::fstream& file){
	int version = 0;
	file.read((char*) &version, sizeof(int));

	/// First version, only ze flags ö-ö
	file.read((char*) &flags, sizeof(int));
	return true;
}

/// Write data to file stream
bool CompactGraphics::WriteTo(std::fstream& file){
	int version = CG_CURRENT_VERSION;
	file.write((char*) &version, sizeof(int));

	/// First version, only ze flags ö-ö
	file.write((char*) &flags, sizeof(int));

	/*
	int extras = 0;
	if (cGraphicEffects){
		extras 
	}
	*/
	return true;
}