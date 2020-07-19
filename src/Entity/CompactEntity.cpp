// Emil Hedemalm
// 2013-03-17

#include "CompactEntity.h"
#include "Entity.h"
#include "../Graphics/CompactGraphics.h"
#include "../Physics/CompactPhysics.h"
#include <cassert>
#include "File/FileUtil.h"
#include <cstring>

CompactEntity::CompactEntity(){
	cGraphics = NULL;
	cPhysics = NULL;
	extras = 0;
	strcpy(diffuseMap, "");
	strcpy(specularMap, "");
	strcpy(normalMap, "");
}
CompactEntity::~CompactEntity(){
	if (cGraphics)
		delete cGraphics;
	if (cPhysics)
		delete cPhysics;
}

/// Getter functions for total dependencies
List<String> CompactEntity::GetModelDependencies()
{
	List<String> dependencies;
	/*
	dependencies.Add(model);
	if (cPhysics){
		if (cPhysics->physicsMesh){
			dependencies.Add(cPhysics->physicsMesh);
		}
	}
	*/
	return dependencies;
}

List<String> CompactEntity::GetTextureDependencies(){
	List<String> dependencies;
//	dependencies.Add(diffuseMap);
	if (cGraphics){
		/// TODO: Iterate multiple textures...
	}
	return dependencies;
}

/// Version control for CEs.
#define CE_VERSION_1			0x00000001

bool CompactEntity::ReadFrom(std::fstream& file){
	int ceVersion;
	int sizeOfInt = sizeof(int);
	std::cout<<"\nSize of int: "<<sizeOfInt;
	file.read((char*) &ceVersion, sizeof(int));
	int nameLength, modelNameLength, textureNameLength, textureNameLength2, texNameLength3;
	file.read((char*) &nameLength, sizeof(int));
	file.read((char*) &modelNameLength, sizeof(int));
	file.read((char*) &textureNameLength, sizeof(int));
	file.read((char*) &textureNameLength2, sizeof(int));
	file.read((char*) &texNameLength3, sizeof(int));
	if (nameLength <= 0 || modelNameLength <= 0 || textureNameLength <= 0){
	    /// Convert endian-ness...
        SWAPFOUR(ceVersion);
        SWAPFOUR(nameLength);
        SWAPFOUR(modelNameLength);
        SWAPFOUR(textureNameLength);
        SWAPFOUR(textureNameLength2);
        SWAPFOUR(texNameLength3);
        std::cout<<"\nERROR: Null-length names in CompactEntity::ReadFrom(file)";
        assert(nameLength > 0 && modelNameLength > 0 && textureNameLength > 0);
	}
	file.read(name, nameLength);
	file.read(model, modelNameLength);
	file.read(diffuseMap, textureNameLength);
	file.read(specularMap, textureNameLength2);
	file.read(normalMap, texNameLength3);
	file.read((char*) &position, sizeof(Vector3f));
	file.read((char*) &scale, sizeof(Vector3f));
	file.read((char*) &rotation, sizeof(Vector3f));
	/// Read integer with flags for extra properties
	file.read((char*) &extras, sizeof(int));
	if (extras & HAS_PHYSICS){
	    if (cPhysics){
	        std::cout<<"\nWARNING: cPhysics non-NULL!";
            delete cPhysics;
            cPhysics = NULL;
            assert(cPhysics == NULL);

	    }

		cPhysics = new CompactPhysics();
		cPhysics->ReadFrom(file);
	}
	if (extras & HAS_GRAPHICS){
		assert(cGraphics == NULL);
		cGraphics = new CompactGraphics();
		cGraphics->ReadFrom(file);
	}
	return true;
}
bool CompactEntity::WriteTo(std::fstream& file){
	// Write block version
	int ceVersion = CE_VERSION_1;
	file.write((char*) &ceVersion, sizeof(int));
	int nameLength = int (strlen(name)+1),
		modelNameLength = int (strlen(model)+1),
		textureNameLength = int (strlen(diffuseMap)+1),
		textureNameLength2 = int (strlen(specularMap)+1),
		textureNameLength3 = int (strlen(normalMap)+1);
	file.write((char*) &nameLength, sizeof(int));
	file.write((char*) &modelNameLength, sizeof(int));
	file.write((char*) &textureNameLength, sizeof(int));
	file.write((char*) &textureNameLength2, sizeof(int));
	file.write((char*) &textureNameLength3, sizeof(int));
	assert(nameLength > 0 && modelNameLength > 0 && textureNameLength > 0);
	file.write(name, nameLength);
	file.write(model, modelNameLength);
	file.write(diffuseMap, textureNameLength);
	file.write(specularMap, textureNameLength2);
	file.write(normalMap, textureNameLength3);
	file.write((char*) &position, sizeof(Vector3f));
	file.write((char*) &scale, sizeof(Vector3f));
	file.write((char*) &rotation, sizeof(Vector3f));
	if (cPhysics)
		extras |= HAS_PHYSICS;
	if (cGraphics)
		extras |= HAS_GRAPHICS;
	file.write((char*) &extras, sizeof(int));
	if (extras & HAS_PHYSICS){
		assert(cPhysics);
		cPhysics->WriteTo(file);
	}
	if (extras & HAS_GRAPHICS){
		assert(cGraphics);
		cGraphics->WriteTo(file);
	}
	return true;
}
/// Calculates size the entity will take when saving, including any eventual sub-properties (if we place them here?)
int CompactEntity::Size(){
	int size = sizeof(int) * 3;
	size += int (strlen(name) + strlen(model) + strlen(diffuseMap));
	size += sizeof(Vector3f) * 3;
	return size;
}
